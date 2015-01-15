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


#include "Scheduler.h"
#include "DateTime.h"
#include "FileIO.h"
#include <assert.h>



#ifdef WIN32
	Scheduler:: Scheduler (void): WinThread ()
#else
	Scheduler:: Scheduler (void): PosixThread ()
#endif

{

	_schedulerStatus			= SCHEDULER_BUILDED;

}


Scheduler:: ~Scheduler (void)

{

}


Scheduler:: Scheduler (const Scheduler &)

{

	assert (1==0);

	// to do

}


Error Scheduler:: init (
	unsigned long ulThreadSleepInMilliSecs,
	unsigned long ulTimesPointerToAllocOnOverflow)

{

	if (_schedulerStatus != SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		return err;
	}

	if (ulTimesPointerToAllocOnOverflow < 1)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);

		return err;
	}

	//	Il mutex e' necessario che sia MUTEX_RECURSIVE perche' all'interno
	//	del metodo handleTimes, una volta eseguito un primo lock sul mutex,
	//	viene chiamato il metodo deactiveTimes che a sua volta eseguira'
	//	il lock dello stesso mutex.
	if (_mtSchedulerMutex. init (PMutex:: MUTEX_RECURSIVE) !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
		}

		return err;
	}

	_ulThreadSleepInMilliSecs			= ulThreadSleepInMilliSecs;
	_ulTimesPointerToAllocOnOverflow	= ulTimesPointerToAllocOnOverflow;
	_ulAllocatedTimesPointerNumber		= ulTimesPointerToAllocOnOverflow;
	_ulTimesPointerNumber				= 0;

	if ((_pTimesList = new Times_p [ulTimesPointerToAllocOnOverflow]) ==
		(Times_p *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_NEW_FAILED);

		if (_mtSchedulerMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
		}

		return err;
	}

	_schedulerStatus			= SCHEDULER_INITIALIZED;



	return errNoError;
}


Error Scheduler:: finish (Boolean_t bDestroyTimes)

{

	if (_schedulerStatus != SCHEDULER_INITIALIZED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		return err;
	}

	if (bDestroyTimes)
	{
		long						lTimesPointerIndex;
		Times_p						ptTimes;


		if (_mtSchedulerMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
		}

		for (lTimesPointerIndex = 0;
			lTimesPointerIndex < (long) _ulTimesPointerNumber;
			lTimesPointerIndex++)
		{
			ptTimes					= _pTimesList [lTimesPointerIndex];

			if (ptTimes -> finish () != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_TIMES_FINISH_FAILED);
			}

			delete ptTimes;
		}

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}
	}

	delete [] (_pTimesList);
	_pTimesList		= (Times_p *) NULL;

	if (_mtSchedulerMutex. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
	}

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
	}

	_schedulerStatus			= SCHEDULER_BUILDED;


	return errNoError;
}


Error Scheduler:: start (void)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus != SCHEDULER_INITIALIZED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);
	
		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}


		return err;
	}

	_schedulerStatus		= SCHEDULER_STARTED;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	#ifdef WIN32
		return (WinThread:: start ());
	#else
		return (PosixThread:: start ());
	#endif
}


Error Scheduler:: suspend (void)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus != SCHEDULER_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	_schedulerStatus			= SCHEDULER_SUSPENDED;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}

	
	return errNoError;
}


Error Scheduler:: resume (void)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus != SCHEDULER_SUSPENDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	_schedulerStatus		= SCHEDULER_STARTED;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: cancel (void)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED ||
		_schedulerStatus == SCHEDULER_INITIALIZED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	_schedulerStatus			= SCHEDULER_INITIALIZED;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: run (void)

{

	SchedulerStatus_t		ssSchedulerStatus;
	unsigned long			ulSecondsToWait;
	unsigned long			ulAdditionalMicroSeconds;


	if (_mtSchedulerMutex. lock () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return _erThreadReturn;
	}

	ssSchedulerStatus		= _schedulerStatus;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return _erThreadReturn;
	}

	ulSecondsToWait				= _ulThreadSleepInMilliSecs / 1000;
	ulAdditionalMicroSeconds	= (_ulThreadSleepInMilliSecs -
		(ulSecondsToWait * 1000)) * 1000;

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
				_erThreadReturn = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_HANDLETIMES_FAILED);

				return _erThreadReturn;
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

		#ifdef WIN32
			if (WinThread:: getSleep (
				ulSecondsToWait, ulAdditionalMicroSeconds) != errNoError)
		#else
			if (PosixThread:: getSleep (
				ulSecondsToWait, ulAdditionalMicroSeconds) != errNoError)
		#endif
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);

			return _erThreadReturn;
		}

		if (_mtSchedulerMutex. lock () != errNoError)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return _erThreadReturn;
		}

		ssSchedulerStatus		= _schedulerStatus;

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return _erThreadReturn;
		}
	}


	return _erThreadReturn;
}


Error Scheduler:: handleTimes (void)

{

	long				lTimesPointerIndex;
	Boolean_t			bIsExpired;
	Boolean_t			bIsStarted;
	Boolean_t			bIsTimesDeactived;
	Times_p				ptTimes;


	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	for (lTimesPointerIndex = 0;
		lTimesPointerIndex < (long) _ulTimesPointerNumber;
		lTimesPointerIndex++)
	{
		bIsTimesDeactived		= false;

		ptTimes					= _pTimesList [lTimesPointerIndex];

		if (ptTimes -> isStarted (&bIsStarted) != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_TIMES_ISSTARTED_FAILED);

			if (_mtSchedulerMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (bIsStarted)
		{
			if (ptTimes -> isExpiredTime (&bIsExpired) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_TIMES_ISEXPIREDTIME_FAILED);

				if (_mtSchedulerMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}

			if (bIsExpired)
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

							if (_mtSchedulerMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}
					}
					else
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_TIMES_UPDATENEXTEXPIRATIONDATETIME_FAILED);

						if (_mtSchedulerMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}

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

							if (_mtSchedulerMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

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

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: activeTimes (Times_p pTimes)

{

	long					lTimesPointerIndex;


	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (pTimes == (Times_p) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (getTimesPointerIndex (pTimes, &lTimesPointerIndex) == errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_TIMESALREADYACTIVE);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (_ulTimesPointerNumber == _ulAllocatedTimesPointerNumber)
	{
		Times_p					*pTimesList;


		pTimesList		= _pTimesList;

		_ulAllocatedTimesPointerNumber	+= _ulTimesPointerToAllocOnOverflow;

		if ((_pTimesList = new Times_p [_ulAllocatedTimesPointerNumber]) ==
			(Times_p *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_NEW_FAILED);
			_pTimesList		= pTimesList;

			if (_mtSchedulerMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		for (lTimesPointerIndex = 0;
			lTimesPointerIndex < (long) _ulTimesPointerNumber;
			lTimesPointerIndex++)
			_pTimesList [lTimesPointerIndex] = pTimesList [lTimesPointerIndex];
		
		delete [] (pTimesList);
		pTimesList		= (Times_p *) NULL;
	}

	_pTimesList [_ulTimesPointerNumber]		= pTimes;
	_ulTimesPointerNumber++;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: deactiveTimes (Times_p pTimes)

{

	long					lTimesPointerIndex;


	if (pTimes == (Times_p) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (getTimesPointerIndex (pTimes, &lTimesPointerIndex) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_GETTIMESPOINTERINDEX_FAILED);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (deactiveTimes (lTimesPointerIndex) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_DEACTIVETIMES_FAILED);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: deactiveTimes (long lTimesPointerIndex)

{

	if (lTimesPointerIndex < 0 ||
		lTimesPointerIndex >= (long) _ulTimesPointerNumber)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	for (lTimesPointerIndex++;
		lTimesPointerIndex < (long) _ulTimesPointerNumber;
		lTimesPointerIndex++)
	{
		_pTimesList [lTimesPointerIndex - 1] = _pTimesList [lTimesPointerIndex];
	}

	_ulTimesPointerNumber--;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: getTimesNumber (unsigned long *pulTimesNumber)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (pulTimesNumber == (unsigned long *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	*pulTimesNumber		= _ulTimesPointerNumber;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: getTimes (unsigned long ulTimesIndex,
	Times_p *pTimes)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (ulTimesIndex >= _ulTimesPointerNumber ||
		pTimes == (Times_p *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtSchedulerMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	*pTimes		= _pTimesList [ulTimesIndex];

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: getSchedulerStatus (
	SchedulerStatus_p pssSchedulerStatus)

{

	if (_mtSchedulerMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	*pssSchedulerStatus			= _schedulerStatus;

	if (_mtSchedulerMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Scheduler:: getTimesPointerIndex (Times_p pTimes,
	long *plTimesPointerIndex)

{

	if (_schedulerStatus == SCHEDULER_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schedulerStatus);

		return err;
	}

	if (pTimes == (Times_p) NULL ||
		plTimesPointerIndex == (long *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		return err;
	}

	for (*plTimesPointerIndex = 0;
		*plTimesPointerIndex < (long) _ulTimesPointerNumber;
		(*plTimesPointerIndex)++)
	{
		if (_pTimesList [*plTimesPointerIndex] == pTimes)
			break;
	}
	
	if (*plTimesPointerIndex == _ulTimesPointerNumber)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_NOTFOUND);

		return err;
	}


	return errNoError;
}

