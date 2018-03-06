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

#include <iostream>
#include "EventsFactory.h"


EventsFactory:: EventsFactory ()
{
}

EventsFactory:: ~EventsFactory (void)
{
    _ethmEvents.clear();
}

EventsFactory:: EventsFactory (const EventsFactory& eventsFactory)
{

    _ethmEvents = eventsFactory._ethmEvents;
}

EventsFactory::EventsFactory(EventsFactory&& eventsFactory)
{
    _ethmEvents = eventsFactory._ethmEvents;
    
    eventsFactory._ethmEvents.clear();
}

void EventsFactory::addEventType (
    long eventTypeIdentifier, long blockEventsNumber, long maxEventsNumberToBeAllocated)
{
    lock_guard<recursive_mutex> locker(_mtEvents);

    EventsTypeHashMap::const_iterator itEventType = _ethmEvents.find(eventTypeIdentifier);
    if (itEventType != _ethmEvents.end())
    {
        throw runtime_error(string("Event Type is already present")
                + ", eventTypeIdentifier: " + to_string(eventTypeIdentifier));
    }

    shared_ptr<EventTypeInfo>   eventTypeInfo = make_shared<EventTypeInfo>();
    eventTypeInfo->_eventTypeIdentifier     = eventTypeIdentifier;
    eventTypeInfo->_blockEventsNumber       = blockEventsNumber;
    eventTypeInfo->_maxEventsNumberToBeAllocated    = maxEventsNumberToBeAllocated;

    eventTypeInfo->_currentEventsNumber     = 0;

    _ethmEvents.insert(make_pair(eventTypeInfo->_eventTypeIdentifier, eventTypeInfo));
}

ostream& operator << (ostream& os, const EventsFactory& eventsFactory)
{

    for (const pair<const long, shared_ptr<EventsFactory::EventTypeInfo>>& itEventTypeInfo: eventsFactory._ethmEvents)
    {
        const shared_ptr<EventsFactory::EventTypeInfo>& eventTypeInfo = itEventTypeInfo.second;

        cout
                << "Event type identifier: " << eventTypeInfo->_eventTypeIdentifier
                << ", block events number: " << eventTypeInfo->_blockEventsNumber
                << ", max events number to be allocated: " << eventTypeInfo->_maxEventsNumberToBeAllocated
                << ", current events number: " << eventTypeInfo->_currentEventsNumber
                << endl;

        cout << "Event free (" << eventTypeInfo->_freeEvents.size() << "): " << endl;
        for (const shared_ptr<Event2>& event: eventTypeInfo->_freeEvents)
        {
            cout << *event << endl;
        }

        cout << "Event used (" << eventTypeInfo->_usedEvents.size() << "): " << endl;
        for (const pair<pair<long,long>, shared_ptr<Event2>>& itEvent: eventTypeInfo->_usedEvents)
        {
            cout << *(itEvent.second) << endl;
        }
    }

    return os;
}
