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

#include <thread>
#include <chrono>

#include "Scheduler2.h"



Scheduler2:: Scheduler2 (unsigned long ulThreadSleepInMilliSecs)
{
    _ulThreadSleepInMilliSecs		= ulThreadSleepInMilliSecs;

    _schedulerStatus			= SCHEDULER_INITIALIZED;
}


Scheduler2:: ~Scheduler2 (void)
{

}

void Scheduler2:: suspend (void)
{

    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (_schedulerStatus != SCHEDULER_STARTED)
    {
        throw runtime_error(string("Scheduler is not started"));
    }

    _schedulerStatus			= SCHEDULER_SUSPENDED;
}

void Scheduler2:: resume (void)
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (_schedulerStatus != SCHEDULER_SUSPENDED)
    {
        throw runtime_error(string("Scheduler is not suspended"));
    }

    _schedulerStatus		= SCHEDULER_STARTED;
}

void Scheduler2:: cancel (void)
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (_schedulerStatus == SCHEDULER_INITIALIZED)
    {
        throw runtime_error(string("Scheduler is already stopped"));
    }

    _schedulerStatus			= SCHEDULER_INITIALIZED;
}

void Scheduler2::operator()()
{

    SchedulerStatus_t		ssSchedulerStatus;


    {
        lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

        _schedulerStatus        = SCHEDULER_STARTED;

        ssSchedulerStatus		= _schedulerStatus;
    }

    while (ssSchedulerStatus == SCHEDULER_STARTED ||
            ssSchedulerStatus == SCHEDULER_SUSPENDED)
    {
        if (ssSchedulerStatus == SCHEDULER_STARTED)
        {
            handleTimes();
        }

        this_thread::sleep_for(_ulThreadSleepInMilliSecs * 1ms);

        {
            lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

            ssSchedulerStatus		= _schedulerStatus;
        }
    }
}

void Scheduler2:: handleTimes (void)
{

    long			lTimesPointerIndex;
    Boolean_t			bIsTimesDeactived;
    shared_ptr<Times2>		ptTimes;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    for (lTimesPointerIndex = 0; lTimesPointerIndex < _timesList.size(); lTimesPointerIndex++)
    {
        bIsTimesDeactived		= false;

        ptTimes					= _timesList[lTimesPointerIndex];

        // cout << "lTimesPointerIndex: " << lTimesPointerIndex << endl;
        if (ptTimes -> isStarted ())
        {
            if (ptTimes -> isExpiredTime ())
            {
                bool lastTimeout;
                
                if (ptTimes -> updateNextExpirationDateTime (lastTimeout) != errNoError)
                {
                    throw runtime_error(string("updateNextExpirationDateTime failed"));
                }

                if (lastTimeout)
                {
                    bIsTimesDeactived		= true;

                    deactiveTimes (lTimesPointerIndex);
                }

                try
                {
                    ptTimes -> handleTimeOut ();
                }
                catch(...)
                {
                    if (bIsTimesDeactived == false)
                    {
                        bIsTimesDeactived		= true;

                        deactiveTimes (lTimesPointerIndex);
                    }
                }
            }
        }

        if (bIsTimesDeactived)
            lTimesPointerIndex--;
    }

}


Error Scheduler2:: activeTimes (shared_ptr<Times2> pTimes)
{

    long		lTimesPointerIndex;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    try
    {
        lTimesPointerIndex = getTimesPointerIndex (pTimes);
    }
    catch(const runtime_error &ia)
    {
        // we should have this exception in the 'normal scenario'
    }

    _timesList.push_back(pTimes);

    
    return errNoError;
}


Error Scheduler2:: deactiveTimes (shared_ptr<Times2> pTimes)
{

    long					lTimesPointerIndex;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);


    lTimesPointerIndex      = getTimesPointerIndex (pTimes);

    deactiveTimes (lTimesPointerIndex);


    return errNoError;
}


void Scheduler2:: deactiveTimes (long lTimesPointerIndex)
{

    if (lTimesPointerIndex < 0 ||
            lTimesPointerIndex >= _timesList.size())
    {
        throw runtime_error(string("Wrong parameter")
                + ", lTimesPointerIndex: " + to_string(lTimesPointerIndex)
                + ", _timesList.size: " + to_string(_timesList.size())
                );
    }

    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    vector<shared_ptr<Times2>>::iterator itTimes = _timesList.begin() + lTimesPointerIndex;

    _timesList.erase(itTimes);
}


unsigned long Scheduler2:: getTimesNumber ()
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    return _timesList.size();
}


shared_ptr<Times2> Scheduler2:: getTimes (unsigned long ulTimesIndex)
{

    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);


    if (ulTimesIndex >= _timesList.size())
    {
        throw runtime_error(string("Wrong parameter")
                + ", ulTimesIndex: " + to_string(ulTimesIndex)
                );
    }

    return _timesList[ulTimesIndex];
}


long Scheduler2:: getTimesPointerIndex (shared_ptr<Times2> pTimes)
{

    long lTimesPointerIndex;
    
    for (lTimesPointerIndex = 0; lTimesPointerIndex < _timesList.size(); lTimesPointerIndex++)
    {
        if (_timesList[lTimesPointerIndex] == pTimes)
            break;
    }
	
    if (lTimesPointerIndex == _timesList.size())
    {
        throw runtime_error(string("Wrong parameter, pTimes was not found")
                );
    }


    return lTimesPointerIndex;
}
