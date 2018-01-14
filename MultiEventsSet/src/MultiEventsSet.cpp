/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#ifdef WIN32
    #include <winsock2.h>
#endif
#include "MultiEventsSet.h"
#include <chrono>

MultiEventsSet:: MultiEventsSet (void)
{

}

MultiEventsSet:: ~MultiEventsSet (void)
{

}

void MultiEventsSet:: addEvent (shared_ptr<Event> event)
{
    string destination =      event->getDestination();
    chrono::system_clock::time_point expirationTimePoint = event->getExpirationTimePoint();

    unique_lock<mutex> locker(_evSetMutex);

    MultiEventsSetHashMap::iterator itDestinations	= _esmMultiEventsSetHashMap.find(destination);
    if (itDestinations == _esmMultiEventsSetHashMap.end ())
    {
        addDestination(destination);

        itDestinations	= _esmMultiEventsSetHashMap.find(destination);
        if (itDestinations == _esmMultiEventsSetHashMap.end ())
        {
            throw invalid_argument(string("Error adding the destination")
                + ", destination: " + destination
                    );
        }
    }

    shared_ptr<DestinationEvents> destinationEvents	= itDestinations->second;
    chrono::system_clock::time_point expirationTimePointOfHeadEvent;
    shared_ptr<Event> headEvent;
    
    if ((destinationEvents->_eventsMultiMap).begin() != (destinationEvents->_eventsMultiMap).end())
    {
        headEvent = ((destinationEvents->_eventsMultiMap).begin())->second;
        expirationTimePointOfHeadEvent  = headEvent->getExpirationTimePoint();
    }

    (destinationEvents->_eventsMultiMap).insert (make_pair(expirationTimePoint, event));

    if (headEvent == nullptr)
    {
        // Every destination has his own condition variable.
        // When an event is added it is sufficient that just one
        // thread waiting the event for that 'destination' is waken up.
        // This is the reason it is used signal and not broadcast.
        (destinationEvents->_addedEvent).notify_one();
    }
    else
    {
        if (expirationTimePoint < expirationTimePointOfHeadEvent)
        {
            // Every destination has his own condition variable.
            // When an event is added it is sufficient that just one
            // thread waiting the event for that 'destination' is waken up.
            // This is the reason it is used signal and not broadcast.
            (destinationEvents -> _addedEvent).notify_one();
        }
    }
}

void MultiEventsSet::deleteEvent (shared_ptr<Event>& event)
{
    lock_guard<mutex> locker(_evSetMutex);

    MultiEventsSetHashMap::iterator itDestinations	= 
            _esmMultiEventsSetHashMap.find(event->getDestination());
    if (itDestinations == _esmMultiEventsSetHashMap.end ())
    {
        throw invalid_argument(string("Destination was not found")
                + ", event->getDestination(): " + event->getDestination()
                );
    }

    shared_ptr<DestinationEvents> destinationEvents	= itDestinations->second;

    pair<EventsMultiMap::iterator, EventsMultiMap::iterator>    findingRange    = 
            (destinationEvents->_eventsMultiMap).equal_range (
            event->getExpirationTimePoint());
    bool found = false;
    EventsMultiMap::iterator itEvent;
    for (itEvent = findingRange.first; itEvent != findingRange.second; itEvent++)
    {
        if (itEvent->second->getEventKey() == event->getEventKey())
        {
            found       = true;

            break;
        }
    }
    
    if (!found)
    {
        time_t  tExpirationTimePoint =  chrono::system_clock::to_time_t(event->getExpirationTimePoint());
        
        throw invalid_argument(string("Event was not found")
                + ", event->getExpirationTimePoint(): " + ctime(&tExpirationTimePoint)
                );
    }

    (destinationEvents->_eventsMultiMap).erase(itEvent);

}

shared_ptr<Event>& MultiEventsSet:: getFirstEvent (
    string destination, bool blocking,
    chrono::milliseconds milliSecondsToBlock,
    bool &eventExpired)

{
    unique_lock<mutex> locker(_evSetMutex);

    MultiEventsSetHashMap:: iterator itDestinations		=
            _esmMultiEventsSetHashMap.find (destination);
    if (itDestinations == _esmMultiEventsSetHashMap.end ())
    {
        addDestination(destination);

        itDestinations		= _esmMultiEventsSetHashMap.find (destination);
        if (itDestinations == _esmMultiEventsSetHashMap.end ())
        {
            throw invalid_argument(string("Error adding the destination")
                + ", destination: " + destination
                    );
        }
    }

    shared_ptr<DestinationEvents> destinationEvents      = itDestinations -> second;

    if ((destinationEvents->_eventsMultiMap).empty())
    {
        if (blocking)
        {                        
            if ((destinationEvents->_addedEvent).wait_for(locker, milliSecondsToBlock,
                    [&](){ return !((destinationEvents->_eventsMultiMap).empty()); }
                    ) == false)
            {
                // time expired
                throw invalid_argument(string("No events found (time expired)")
                        + ", destination: " + destination
                        + ", milliSecondsToBlock: " + to_string(milliSecondsToBlock.count())
                        );
            }
            else
            {
                // an event must be added

                if ((destinationEvents->_eventsMultiMap).empty())
                {
                    throw invalid_argument(string("getFirstEvent failed, event has to be present")
                            + ", destination: " + destination
                            + ", milliSecondsToBlock: " + to_string(milliSecondsToBlock.count())
                            );
                }
            }
        }
        else
        {
            throw invalid_argument(string("No events found")
                    + ", destination: " + destination
                    );
        }
    }

    EventsMultiMap:: iterator itEvents  = (destinationEvents->_eventsMultiMap).begin ();
    shared_ptr<Event> &event		= itEvents -> second;

    chrono::system_clock::time_point expirationTimePoint    = event->getExpirationTimePoint();
    if (chrono::system_clock::now() < expirationTimePoint)
        eventExpired    = false;
    else
        eventExpired    = true;

    return event;
}

shared_ptr<Event> MultiEventsSet:: getAndRemoveFirstEvent (
    string destination, bool blocking,
    chrono::milliseconds milliSecondsToBlock)

{
    chrono::system_clock::time_point    initialTimePoint    = chrono::system_clock::now();

    unique_lock<mutex> locker(_evSetMutex);

    MultiEventsSetHashMap:: iterator itDestinations		=
            _esmMultiEventsSetHashMap.find (destination);
    if (itDestinations == _esmMultiEventsSetHashMap.end ())
    {
        addDestination(destination);

        itDestinations		= _esmMultiEventsSetHashMap.find (destination);
        if (itDestinations == _esmMultiEventsSetHashMap.end ())
        {
            throw invalid_argument(string("Error adding the destination")
                + ", destination: " + destination
                    );
        }
    }

    shared_ptr<DestinationEvents> destinationEvents  = itDestinations -> second;

    if ((destinationEvents->_eventsMultiMap).empty())
    {
        if (blocking)
        {
            if ((destinationEvents->_addedEvent).wait_for(locker, milliSecondsToBlock,
                    [&](){ return !(destinationEvents->_eventsMultiMap).empty(); }) == false)
            {
                // time expired
                throw invalid_argument(string("No events found (time expired)")
                        + ", destination: " + destination
                        + ", milliSecondsToBlock: " + to_string(milliSecondsToBlock.count())
                        );
            }
            else
            {
                // an event must be added

                if ((destinationEvents->_eventsMultiMap).empty())
                {
                    throw invalid_argument(string("getAndRemoveFirstEvent failed, event has to be present")
                            + ", destination: " + destination
                            + ", milliSecondsToBlock: " + to_string(milliSecondsToBlock.count())
                            );
                }
            }
        }
        else
        {
            throw invalid_argument(string("No events found")
                    + ", destination: " + destination
                    );
        }
    }

    // sure there is an event inside the set but, after the call of timedWait
    // of the condition, the event could not be present anymore

    {
        bool foundExpiredEvent  = false;
        chrono::milliseconds    remainingTimeInMilliSecs;
        EventsMultiMap:: iterator itEvents;
        shared_ptr<Event> event;

        while (!foundExpiredEvent)
        {
            itEvents  = (destinationEvents->_eventsMultiMap).begin();
            if (itEvents == (destinationEvents->_eventsMultiMap).end())
            {
                throw invalid_argument(string("No events found")
                        + ", destination: " + destination
                        );
            }

            event		= itEvents -> second;
            chrono::system_clock::time_point expirationTimePoint    = event->getExpirationTimePoint();
            chrono::system_clock::time_point now = chrono::system_clock::now();

            if (now < expirationTimePoint)
            {
                if (blocking)
                {
                    remainingTimeInMilliSecs    = milliSecondsToBlock
                            - chrono::duration_cast<chrono::milliseconds>(now - initialTimePoint);

                    if (remainingTimeInMilliSecs <= chrono::milliseconds(0))
                    {
                        // blocking time expired

                        throw invalid_argument(string("Event not expired yet"));
                    }

                    if (expirationTimePoint - now <= remainingTimeInMilliSecs)
                    {
                        remainingTimeInMilliSecs    = chrono::duration_cast<chrono::milliseconds>(expirationTimePoint - now);   
                    }

                    if ((destinationEvents -> _addedEvent).wait_for(locker,
                        remainingTimeInMilliSecs,
                        [&](){ return !(destinationEvents -> _eventsMultiMap).empty(); }) == false)
                    {
                        // time expired
                    }
                    else
                    {
                        // we have to check if we still have an event available
                        // continue to do the check

                        continue;
                    }
                }
                else
                {
                    throw invalid_argument(string("Event not expired yet")
                            + ", destination: " + destination
                            );
                }
            }
            else
                foundExpiredEvent		= true;
        }

        (destinationEvents->_eventsMultiMap).erase(itEvents);
        
        return event;
    }
}

void MultiEventsSet:: addDestination (string destination)
{
    MultiEventsSetHashMap:: iterator	itDestinations;
    shared_ptr<DestinationEvents> destinationEvents = make_shared<DestinationEvents>();

    destinationEvents->_destination   = destination;

    lock_guard<mutex> locker(_evSetMutex);

    itDestinations  = _esmMultiEventsSetHashMap.find (destinationEvents->_destination);
    if (itDestinations != _esmMultiEventsSetHashMap.end ())
    {
        throw invalid_argument(string("Destination is already present")
                + ", destination: " + destination
                );
    }

    _esmMultiEventsSetHashMap.insert (make_pair(
        destinationEvents->_destination, destinationEvents));
}


