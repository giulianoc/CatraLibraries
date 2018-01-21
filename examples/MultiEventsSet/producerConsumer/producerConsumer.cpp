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
#include <chrono>
#include <memory>
#include <vector>

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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

    friend ostream& operator << (ostream& os, GetDataEvent &event)
    {
        time_t tExpirationTimePoint = chrono::system_clock::to_time_t(event.getExpirationTimePoint());
        time_t tStartProcessingTime = chrono::system_clock::to_time_t(event.getStartProcessingTime());

        cout
            << "Event. "
            << "type identifier: " << event.getEventKey().first
            << ", identifier: " << event.getEventKey().second
            << ", source: " << event.getSource()
            << ", destination: " << event.getDestination()
            << ", expiration time point: " << ctime(&tExpirationTimePoint)
            << ", start processing time: " << ctime(&tStartProcessingTime)
            << ", dataId: " << event.dataId 
            << endl;
        
        return os;
    }
};

class EndEvent: public Event {
private:

    friend ostream& operator << (ostream& os, EndEvent &event)
    {
        time_t tExpirationTimePoint = chrono::system_clock::to_time_t(event.getExpirationTimePoint());
        time_t tStartProcessingTime = chrono::system_clock::to_time_t(event.getStartProcessingTime());

        cout
            << "Event. "
            << "type identifier: " << event.getEventKey().first
            << ", identifier: " << event.getEventKey().second
            << ", source: " << event.getSource()
            << ", destination: " << event.getDestination()
            << ", expiration time point: " << ctime(&tExpirationTimePoint)
            << ", start processing time: " << ctime(&tStartProcessingTime)
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
            // cout << "Calling getAndRemoveFirstEvent" << endl;
            shared_ptr<Event> event = multiEventsSet.getAndRemoveFirstEvent(DESTINATION, blocking, milliSecondsToBlock);
            if (event == nullptr)
            {
                cout << "No event found or event not yet expired" << endl;
                
                continue;
            }
            
            // cout << "event->getTypeIdentifier(): " << event->getTypeIdentifier() << endl;
            switch(event->getEventKey().first)
            {
                case GETDATAEVENT_TYPE:
                {                    
                    shared_ptr<GetDataEvent>    getDataEvent = dynamic_pointer_cast<GetDataEvent>(event);
                    cout << "getAndRemoveFirstEvent: GETDATAEVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << "): " << getDataEvent->getDataId() << endl << endl;
                    multiEventsSet.getEventsFactory()->releaseEvent<GetDataEvent>(getDataEvent);
                }
                break;
                case ENDEVENT_TYPE:
                {
                    cout << "getAndRemoveFirstEvent: ENDEVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << ")" << endl << endl;
                    endEvent = true;
                    
                    shared_ptr<EndEvent>    endEvent = dynamic_pointer_cast<EndEvent>(event);
                    multiEventsSet.getEventsFactory()->releaseEvent<EndEvent>(endEvent);
                }
                break;
                case EVENT_TYPE:
                {
                    cout << "getAndRemoveFirstEvent: EVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << ")" << endl << endl;
                    
                    multiEventsSet.getEventsFactory()->releaseEvent<Event>(event);

                }
                break;
                default:
                    throw invalid_argument(string("Event type identifier not managed")
                            + to_string(event->getEventKey().first));
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

        for (long counter = 0; counter < 200; counter++)
        {
            switch(counter % 2)
            {
                case GETDATAEVENT_TYPE:
                {
                   shared_ptr<GetDataEvent>    getDataEvent =
                            multiEventsSet.getEventsFactory()->getFreeEvent<GetDataEvent>(GETDATAEVENT_TYPE);
                    
                    getDataEvent->setDestination(DESTINATION);
                    getDataEvent->setExpirationTimePoint(chrono::system_clock::now() + chrono::minutes(1));
                    getDataEvent->setSource(SOURCE);
                    getDataEvent->setDataId(counter);
                    
                    shared_ptr<Event>    event = dynamic_pointer_cast<Event>(getDataEvent);
                    cout << "addEvent: GETDATAEVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << ")" << endl;
                    multiEventsSet.addEvent(event);
                }
                break;
                case EVENT_TYPE:
                {
                    shared_ptr<Event>    event = multiEventsSet.getEventsFactory()->getFreeEvent<Event>(EVENT_TYPE);
                    
                    event->setDestination(DESTINATION);
                    event->setExpirationTimePoint(chrono::system_clock::now() + chrono::minutes(1));
                    event->setSource(SOURCE);
                    
                    cout << "addEvent: EVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << ")" << endl;
                    multiEventsSet.addEvent(event);                    
                }
                break;
            }
        
            // this_thread::sleep_for(chrono::milliseconds(100));
        }
        
        this_thread::sleep_for(chrono::seconds(1));
        
        {
            shared_ptr<EndEvent>    endEvent =
                    multiEventsSet.getEventsFactory()->getFreeEvent<EndEvent>(ENDEVENT_TYPE);

            endEvent->setDestination(DESTINATION);
            endEvent->setExpirationTimePoint(chrono::system_clock::now() + chrono::minutes(1));
            endEvent->setSource(SOURCE);

            shared_ptr<Event>    event = dynamic_pointer_cast<Event>(endEvent);
            multiEventsSet.addEvent(event);

            cout << "addEvent: ENDEVENT_TYPE (" << event->getEventKey().first << ", " << event->getEventKey().second << ")" << endl;
        }

        cout << "producer thread terminated" << endl;
    }
};

int parseLine(char* line){
    // This assumes that a digit will be found and the line ends in " Kb".
    int i = strlen(line);
    const char* p = line;
    while (*p <'0' || *p > '9') p++;
    line[i-3] = '\0';
    i = atoi(p);
    return i;
}

pair<long,long> getMemoryInfo(){ //Note: this value is in KB!
    FILE* file = fopen("/proc/self/status", "r");
    char line[128];
    int virtualMemoryCurrentlyUsedByCurrentProcess = -1;
    int physicalMemoryCurrentlyUsedByCurrentProcess = -1;

    while (fgets(line, 128, file) != NULL){
        if (strncmp(line, "VmSize:", 7) == 0){
            virtualMemoryCurrentlyUsedByCurrentProcess = parseLine(line);
        }
        else if (strncmp(line, "VmRSS:", 6) == 0){
            physicalMemoryCurrentlyUsedByCurrentProcess = parseLine(line);
        }
        
        if (virtualMemoryCurrentlyUsedByCurrentProcess != -1 && physicalMemoryCurrentlyUsedByCurrentProcess != -1)
            break;
    }
    fclose(file);

    return make_pair(virtualMemoryCurrentlyUsedByCurrentProcess, physicalMemoryCurrentlyUsedByCurrentProcess);
}


int main ()
{
    vector<pair<long,long>> results;
    
    for (int counter = 0; counter < 1000000; counter++)
    {
        MultiEventsSet multiEventsSet;
        Consumer consumer;
        Producer producer;

        multiEventsSet.addDestination(DESTINATION);

        thread consumerThread(consumer, ref(multiEventsSet));
        thread producerThread(producer, ref(multiEventsSet));
        producerThread.join();
        consumerThread.join();
        
        cout << endl << endl;
        results.push_back(getMemoryInfo());

        for (pair<long,long> result: results)
            cout << "VirtualMemory: " << result.first << "KB, PhysicalMemory: " << result.second << "KB" << endl;

        this_thread::sleep_for(chrono::seconds(10));
    }


    return 0;
}

