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
        throw invalid_argument(string("Scheduler is not started"));
    }

    _schedulerStatus			= SCHEDULER_SUSPENDED;
}

void Scheduler2:: resume (void)
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (_schedulerStatus != SCHEDULER_SUSPENDED)
    {
        throw invalid_argument(string("Scheduler is not suspended"));
    }

    _schedulerStatus		= SCHEDULER_STARTED;
}

void Scheduler2:: cancel (void)
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (_schedulerStatus == SCHEDULER_INITIALIZED)
    {
        throw invalid_argument(string("Scheduler is already stopped"));
    }

    _schedulerStatus			= SCHEDULER_INITIALIZED;
}


void Scheduler2::operator()()

{

    SchedulerStatus_t		ssSchedulerStatus;
    // unsigned long		ulSecondsToWait;
    // unsigned long		ulAdditionalMicroSeconds;


    {
        lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

        ssSchedulerStatus		= _schedulerStatus;
    }

//    ulSecondsToWait			= _ulThreadSleepInMilliSecs / 1000;
//    ulAdditionalMicroSeconds	= (_ulThreadSleepInMilliSecs -
//            (ulSecondsToWait * 1000)) * 1000;

    while (ssSchedulerStatus == SCHEDULER_STARTED ||
            ssSchedulerStatus == SCHEDULER_SUSPENDED)
    {
        if (ssSchedulerStatus == SCHEDULER_STARTED)
        {
/*
#ifdef WIN32
__int64             ullNowLocalInMilliSecs1;
__int64             ullNowLocalInMilliSecs2;
#else
unsigned long long  ullNowLocalInMilliSecs1;
unsigned long long  ullNowLocalInMilliSecs2;
#endif
static long lHandleTimesElapsed = 0;
DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs1);
*/
            if (handleTimes () != errNoError)
            {
                throw invalid_argument(string("handleTimes failed"));
            }
/*
DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs2);
// if ((long) (ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1) >
// 	lHandleTimesElapsed)
{
    lHandleTimesElapsed =
            (long) (ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);
char pBuffer [1024];
sprintf (pBuffer,
"SCHEDULER. max handleTimes elapsed (millisecs): %ld\n",
    lHandleTimesElapsed);
#ifdef WIN32
FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
#else
FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
#endif
}
*/
        }

        this_thread::sleep_for(_ulThreadSleepInMilliSecs * 1ms);

        {
            lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

            ssSchedulerStatus		= _schedulerStatus;
        }
    }
}


Error Scheduler2:: handleTimes (void)

{

    long			lTimesPointerIndex;
    Boolean_t			bIsTimesDeactived;
    shared_ptr<Times2>		ptTimes;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    for (lTimesPointerIndex = 0; lTimesPointerIndex < _timesList.size(); lTimesPointerIndex++)
    {
        bIsTimesDeactived		= false;

        ptTimes					= _timesList[lTimesPointerIndex];

        if (ptTimes -> isStarted ())
        {
            if (ptTimes -> isExpiredTime ())
            {
                Error			errUpdateNextExpirationDateTime;


                if ((errUpdateNextExpirationDateTime =
                    ptTimes -> updateNextExpirationDateTime ()) != errNoError)
                {
                    if ((long) errUpdateNextExpirationDateTime ==
                            SCH_REACHED_LASTTIMEOUT)
                    {
                        bIsTimesDeactived		= true;

                        if (deactiveTimes (lTimesPointerIndex) != errNoError)
                        {
                                Error err = SchedulerErrors (__FILE__, __LINE__,
                                        SCH_SCHEDULER_DEACTIVETIMES_FAILED);

                                return err;
                        }
                    }
                    else
                    {
                        Error err = SchedulerErrors (__FILE__, __LINE__,
                                SCH_TIMES_UPDATENEXTEXPIRATIONDATETIME_FAILED);

                        return err;
                    }
                }

/*
char aaa [1024];
unsigned long long ullLocalExpirationLocalDateTimeInMilliSecs1;
unsigned long long ullLocalExpirationLocalDateTimeInMilliSecs2;
DateTime:: nowLocalInMilliSecs (&ullLocalExpirationLocalDateTimeInMilliSecs1);
*/

                if (ptTimes -> handleTimeOut () != errNoError)
                {
                    Error err = SchedulerErrors (__FILE__, __LINE__,
                        SCH_TIMES_HANDLETIMEOUT_FAILED);

                    if (bIsTimesDeactived == false)
                    {
                        bIsTimesDeactived		= true;

                        if (deactiveTimes (lTimesPointerIndex) != errNoError)
                        {
                            Error err = SchedulerErrors (__FILE__, __LINE__,
                                    SCH_SCHEDULER_DEACTIVETIMES_FAILED);

                            return err;
                        }
                    }
                }
/*
DateTime:: nowLocalInMilliSecs (&ullLocalExpirationLocalDateTimeInMilliSecs2);
sprintf (aaa, "Index: %ld, %ld\n",
lTimesPointerIndex, (long) (ullLocalExpirationLocalDateTimeInMilliSecs2 - ullLocalExpirationLocalDateTimeInMilliSecs1));
FileIO:: appendBuffer ("/tmp/elapsed.txt", aaa, false);
*/
            }
        }

        if (bIsTimesDeactived)
            lTimesPointerIndex--;
    }


    return errNoError;
}


Error Scheduler2:: activeTimes (shared_ptr<Times2> pTimes)

{

    long		lTimesPointerIndex;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    if (getTimesPointerIndex (pTimes, &lTimesPointerIndex) == errNoError)
    {
        Error err = SchedulerErrors (__FILE__, __LINE__,
                SCH_SCHEDULER_TIMESALREADYACTIVE);

        return err;
    }

    _timesList.push_back(pTimes);


    return errNoError;
}


Error Scheduler2:: deactiveTimes (shared_ptr<Times2> pTimes)

{

    long					lTimesPointerIndex;


    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);


    if (getTimesPointerIndex (pTimes, &lTimesPointerIndex) != errNoError)
    {
        Error err = SchedulerErrors (__FILE__, __LINE__,
                SCH_SCHEDULER_GETTIMESPOINTERINDEX_FAILED);

        return err;
    }

    if (deactiveTimes (lTimesPointerIndex) != errNoError)
    {
        Error err = SchedulerErrors (__FILE__, __LINE__,
                SCH_SCHEDULER_DEACTIVETIMES_FAILED);

        return err;
    }


    return errNoError;
}


Error Scheduler2:: deactiveTimes (long lTimesPointerIndex)

{

    if (lTimesPointerIndex < 0 ||
            lTimesPointerIndex >= _timesList.size())
    {
            Error err = SchedulerErrors (__FILE__, __LINE__,
                    SCH_ACTIVATION_WRONG);

            return err;
    }

    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    vector<shared_ptr<Times2>>::iterator itTimes = _timesList.begin() + lTimesPointerIndex;

    _timesList.erase(itTimes);

    
    return errNoError;
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
        throw invalid_argument(string("Wrong parameter")
                + ", ulTimesIndex: " + to_string(ulTimesIndex)
                );
    }

    return _timesList[ulTimesIndex];
}

/*
SchedulerStatus_t Scheduler2:: getSchedulerStatus ()
{
    lock_guard<recursive_mutex>     locker(_mtSchedulerMutex);

    return _schedulerStatus;
}
 */

Error Scheduler2:: getTimesPointerIndex (shared_ptr<Times2> pTimes,
	long *plTimesPointerIndex)

{

    for (*plTimesPointerIndex = 0;
            *plTimesPointerIndex < _timesList.size();
            (*plTimesPointerIndex)++)
    {
        if (_timesList[*plTimesPointerIndex] == pTimes)
            break;
    }
	
    if (*plTimesPointerIndex == _timesList.size())
    {
        Error err = SchedulerErrors (__FILE__, __LINE__,
                SCH_TIMES_NOTFOUND);

        return err;
    }


    return errNoError;
}

