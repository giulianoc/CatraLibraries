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

#ifndef EventsFactory_h
    
#define EventsFactory_h

#include <deque>
#include <map>
#include <unordered_map>
#include <mutex>
#include <iostream>
#include <memory>
#include "Event2.h"

using namespace std;

class EventsFactory
{
private:
    using UsedEventsMap = map<pair<long,long>, shared_ptr<Event2>>;

    struct EventTypeInfo
    {
        long                _eventTypeIdentifier;
        long                _blockEventsNumber;
        long                _maxEventsNumberToBeAllocated;

        long                _currentEventsNumber;
        deque<shared_ptr<Event2>>    _freeEvents;
        UsedEventsMap       _usedEvents;
    };

    using EventsTypeHashMap = unordered_map<long, shared_ptr<EventTypeInfo>>;

    recursive_mutex     _mtEvents;
    EventsTypeHashMap   _ethmEvents;

    const long          _defaultBlockEventsNumber = 50;
    const long          _defaultMaxEventsNumberToBeAllocated = 10000;

public:
    EventsFactory (void);

    ~EventsFactory (void);

    EventsFactory (const EventsFactory& eventsFactory);

    EventsFactory (EventsFactory&& eventsFactory);
    
    friend ostream& operator << (ostream& os, const EventsFactory& eventsFactory);

    void addEventType (long eventTypeIdentifier, long blockEventsNumber, 
        long maxEventsNumberToBeAllocated);

    template<typename T>
    shared_ptr<T> getFreeEvent (long eventTypeIdentifier)
    {
        lock_guard<recursive_mutex> locker(_mtEvents);

        EventsTypeHashMap::const_iterator itEventType = _ethmEvents.find(eventTypeIdentifier);
        if (itEventType == _ethmEvents.end())
        {
            addEventType(eventTypeIdentifier, _defaultBlockEventsNumber, _defaultMaxEventsNumberToBeAllocated);

            itEventType = _ethmEvents.find(eventTypeIdentifier);
            if (itEventType == _ethmEvents.end())
            {
                throw runtime_error(string("Error adding the Event Type identifier") 
                        + ", eventTypeIdentifier: " + to_string(eventTypeIdentifier));
            }
        }

        const shared_ptr<EventTypeInfo>& eventTypeInfo = itEventType->second;
        deque<shared_ptr<Event2>>& freeEvents = eventTypeInfo->_freeEvents;

        if (freeEvents.size() == 0)
        {
            if (eventTypeInfo->_currentEventsNumber + eventTypeInfo->_blockEventsNumber >= eventTypeInfo->_maxEventsNumberToBeAllocated)
            {
                throw runtime_error(string("No more events to be allocated, reached the max number for the event type") 
                        + ", _currentEventsNumber: " + to_string(eventTypeInfo->_currentEventsNumber)
                        + ", _blockEventsNumber: " + to_string(eventTypeInfo->_blockEventsNumber)
                        + ", _maxEventsNumberToBeAllocated: " + to_string(eventTypeInfo->_maxEventsNumberToBeAllocated)
                        );
            }

            for (int eventIndex = 0; eventIndex < eventTypeInfo->_blockEventsNumber; eventIndex++)
            {
                shared_ptr<Event2>   event   = make_shared<T>();

                event->setEventKey(make_pair(eventTypeInfo->_eventTypeIdentifier, eventTypeInfo->_currentEventsNumber));

                freeEvents.push_back(event);

                eventTypeInfo->_currentEventsNumber += 1;
            }                    
        }

        {
            UsedEventsMap   &usedEvents  = eventTypeInfo->_usedEvents;

            shared_ptr<T> event	= dynamic_pointer_cast<T>(freeEvents.front());

            event->setStartProcessingTime(chrono::system_clock::now());

            freeEvents.pop_front();
            usedEvents.insert(make_pair(make_pair(event->getEventKey().first,event->getEventKey().second), event));

            
            return event;
        }
    }

    template<typename T>
    void releaseEvent (shared_ptr<T>& event)
    {
        lock_guard<recursive_mutex> locker(_mtEvents);

        EventsTypeHashMap::const_iterator itEventType = _ethmEvents.find(event->getEventKey().first);
        if (itEventType == _ethmEvents.end())
        {
            throw runtime_error(string("Event Type was not found") 
                    + ", eventTypeIdentifier: " + to_string(event->getEventKey().first));
        }

        const shared_ptr<EventTypeInfo>& eventTypeInfo = itEventType->second;

        deque<shared_ptr<Event2>> &freeEvents = eventTypeInfo->_freeEvents;
        UsedEventsMap   &usedEvents  = eventTypeInfo->_usedEvents;

        UsedEventsMap::const_iterator itEvent = usedEvents.find(make_pair(event->getEventKey().first, event->getEventKey().second));
        if (itEvent == usedEvents.end())
        {
            throw runtime_error(string("Event was not found") 
                    + ", event->getEventKey().first: " + to_string(event->getEventKey().first)
                    + ", event->getEventKey().second: " + to_string(event->getEventKey().second)
                    );
        }

        freeEvents.push_back(dynamic_pointer_cast<Event2>(event));
        usedEvents.erase(itEvent);
    }
} ;

#endif
