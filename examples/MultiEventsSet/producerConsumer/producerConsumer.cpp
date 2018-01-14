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
#include <thread>
#include <memory>
#include "MultiEventsSet.h"

using namespace std;

#define SOURCE      "Producer"
#define DESTINATION "Consumer"

#define GETDATAEVENT_TYPE   0
#define EVENT_TYPE          1
#define ENDEVENT_TYPE       2

class GetDataEvent: public Event {
private:
    int     dataId;
    
public:
    int getDataId() const {
        return dataId;
    }

    void setDataId(int dataId) {
        this->dataId = dataId;
    }

    friend ostream& operator << (ostream& os, GetDataEvent &gde)
    {
        cout
            << "Event. "
            << "identifier: " << gde.getIdentifier()
            << ", type identifier: " << gde.getTypeIdentifier()
            << ", source: " << gde.getSource()
            << ", destination: " << gde.getDestination()
            // << ", expiration time point: " << event._expirationTimePoint
            // << ", start processing time: " << event._startProcessingTime
            << ", dataId: " << gde.dataId 
            << endl;
        
        return os;
    }
};

class EndEvent: public Event {
private:

    friend ostream& operator << (ostream& os, EndEvent &gde)
    {
        cout
            << "Event. "
            << "identifier: " << gde.getIdentifier()
            << ", type identifier: " << gde.getTypeIdentifier()
            << ", source: " << gde.getSource()
            << ", destination: " << gde.getDestination()
            // << ", expiration time point: " << event._expirationTimePoint
            // << ", start processing time: " << event._startProcessingTime
            << endl;
        
        return os;
    }
};

class Consumer 
{
public:
    void operator ()(MultiEventsSet& multiEventsSet) 
    {
        bool blocking = true;
        chrono::milliseconds milliSecondsToBlock(100);
        
        cout << "consumer thread started" << endl;

        bool endEvent = false;
        while(!endEvent)
        {
            shared_ptr<Event> event = multiEventsSet.getAndRemoveFirstEvent(DESTINATION, blocking, milliSecondsToBlock);
            switch(event->getTypeIdentifier())
            {
                case GETDATAEVENT_TYPE:
                {
                    cout << "getAndRemoveFirstEvent: GETDATAEVENT_TYPE" << endl;
                    
                    shared_ptr<GetDataEvent>    getDataEvent = dynamic_pointer_cast<GetDataEvent>(event);
                    multiEventsSet.getEventsFactory().releaseEvent<GetDataEvent>(getDataEvent);
                }
                break;
                case ENDEVENT_TYPE:
                {
                    cout << "getAndRemoveFirstEvent: ENDEVENT_TYPE" << endl;
                    endEvent = true;
                    
                    shared_ptr<EndEvent>    endEvent = dynamic_pointer_cast<EndEvent>(event);
                    multiEventsSet.getEventsFactory().releaseEvent<EndEvent>(endEvent);
                }
                break;
                case EVENT_TYPE:
                {
                    cout << "getAndRemoveFirstEvent: EVENT_TYPE" << endl;
                    
                    multiEventsSet.getEventsFactory().releaseEvent<Event>(event);

                }
                break;
                default:
                    throw invalid_argument("Event type identifier not managed");
            }
            
        }

        cout << "consumer thread terminated" << endl;
    }
};

class Producer 
{
public:
    void operator ()(MultiEventsSet& multiEventsSet) 
    {
        cout << "producer thread started" << endl;

        for (long counter = 0; counter < 1; counter++)
        {
            switch(counter % 2)
            {
                case GETDATAEVENT_TYPE:
                {
                   shared_ptr<GetDataEvent>    getDataEvent =
                            multiEventsSet.getEventsFactory().getFreeEvent<GetDataEvent>(GETDATAEVENT_TYPE);
                    
                    getDataEvent->setDestination(DESTINATION);
                    getDataEvent->setExpirationTimePoint(chrono::system_clock::now());
                    getDataEvent->setSource(SOURCE);
                    getDataEvent->setDataId(counter);
                    
                    shared_ptr<Event>    event = dynamic_pointer_cast<Event>(getDataEvent);
                    multiEventsSet.addEvent(event);

                    cout << "addEvent: GETDATAEVENT_TYPE" << endl;
                }
                break;
                case EVENT_TYPE:
                {
                    shared_ptr<Event>    event =
                            multiEventsSet.getEventsFactory().getFreeEvent<Event>(EVENT_TYPE);
                    
                    event->setDestination(DESTINATION);
                    event->setExpirationTimePoint(chrono::system_clock::now());
                    event->setSource(SOURCE);
                    
                    multiEventsSet.addEvent(event);
                    
                    cout << "addEvent: EVENT_TYPE" << endl;
                }
                break;
            }
        }
        
        {
            shared_ptr<EndEvent>    endEvent =
                    multiEventsSet.getEventsFactory().getFreeEvent<EndEvent>(ENDEVENT_TYPE);

            endEvent->setDestination(DESTINATION);
            endEvent->setExpirationTimePoint(chrono::system_clock::now());
            endEvent->setSource(SOURCE);

            shared_ptr<Event>    event = dynamic_pointer_cast<Event>(endEvent);
            multiEventsSet.addEvent(event);

            cout << "addEvent: ENDEVENT_TYPE" << endl;
        }

        cout << "producer thread terminated" << endl;
    }
};

// Fare il producer e assicurarci che lo stesso evento esce e rientra!!!
// setEventKey/setStartProcessingTime deve essere chiamato solo da EventsSet

int main ()
{      
    MultiEventsSet multiEventsSet;
    Consumer consumer;
    Producer producer;

    thread consumerThread(consumer, ref(multiEventsSet));
    thread producerThread(producer, ref(multiEventsSet));
    producerThread.join();
    consumerThread.join();

    /*
    for (long counter = 0; counter < 1000000000; counter++)
    {
        cout << "counter: " << counter << endl;
        

        long eventTypeIdentifier_1            = 1;
        {
            long blockEventsNumber_1              = 5;
            long maxEventsNumberToBeAllocated_1   = 10;
            eventsFactory.addEventType(eventTypeIdentifier_1, blockEventsNumber_1, maxEventsNumberToBeAllocated_1);

            // cout << "addEventType Event type 1: " << endl;
            // cout << eventsFactory << endl;
        } 

        long eventTypeIdentifier_2            = 2;
        {
            long blockEventsNumber_2              = 2;
            long maxEventsNumberToBeAllocated_2   = 20;
            eventsFactory.addEventType(eventTypeIdentifier_2, blockEventsNumber_2, maxEventsNumberToBeAllocated_2);

            // cout << "addEventType Event type 2: " << endl;
            // cout << eventsFactory << endl;
        }

        {
            shared_ptr<Event> event = eventsFactory.getFreeEvent<Event>(eventTypeIdentifier_1);

            // cout << "getFreeEvent. event.use_count: " << event.use_count() << endl;
            // cout << eventsFactory << endl;

            eventsFactory.releaseEvent(eventTypeIdentifier_1, event);

            // cout << "releaseEvent. event.use_count: " << event.use_count() << endl;
            // cout << eventsFactory << endl;
        }

        // cout << "-------------------" << endl;

        {
            shared_ptr<GetDataEvent> event = eventsFactory.getFreeEvent<GetDataEvent>(eventTypeIdentifier_2);

            // cout << "getDataEvent: " << *event << endl;

            // cout << "getFreeEvent. event.use_count: " << event.use_count() << endl;
            // cout << eventsFactory << endl;

            eventsFactory.releaseEvent<GetDataEvent>(eventTypeIdentifier_2, event);

            // cout << "releaseEvent. event.use_count: " << event.use_count() << endl;
            // cout << eventsFactory << endl;
        }
    }
    */


    return 0;
}

