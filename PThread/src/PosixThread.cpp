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


#include "PosixThread.h"
#include "DateTime.h"
#include "FileIO.h"
#include "PCondition.h"
#include "PMutex.h"
#include <errno.h>

#if defined(__hpux) && defined(_CMA__HP)
#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
	#ifdef WIN32
		#include <windows.h>
	#else
		#include <unistd.h>		// for the sleep function
	#endif
#endif

#if defined(__hpux) && defined(_CMA__HP)
	extern int errno;
#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
#endif



void *runFunction (void *pvPosixThread)

{

	PosixThread_p					pPosixThread			=
		(PosixThread_p) pvPosixThread;


	if (pPosixThread == (PosixThread_p) NULL)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPosixThread");

		return (void *) NULL;
	}

	pPosixThread -> _erThreadReturn = pPosixThread -> run ();

	if (pPosixThread -> _stPThreadStatus ==
		PosixThread:: THREADLIB_DETACHED)
	{
		pPosixThread -> finish ();

		delete (pPosixThread);
	}
	else
	{
		// the state is THREADLIB_STARTED or THREADLIB_STARTED_AND_JOINED
		if (pPosixThread -> _stPThreadStatus ==
			PosixThread:: THREADLIB_STARTED)
		{
			long                lErrorNumber;


			// It is necessary to detach the thread to allow the
			// operative system to free the resources
			if ((lErrorNumber = pthread_detach (
				pPosixThread -> _pThread)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_DETACH_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));
			}
		}

		pPosixThread -> _stPThreadStatus		=
			PosixThread:: THREADLIB_INITIALIZEDAGAINAFTERRUNNING;
	}


	return (void *) NULL;

}


PosixThread:: PosixThread (void)

{

	strcpy (_pPThreadName, "");
	_erThreadReturn			= errNoError;
	_stPThreadStatus		= THREADLIB_BUILDED;
}


PosixThread:: ~PosixThread (void)

{

}


PosixThread:: PosixThread (const PosixThread &t)

{

	strcpy (_pPThreadName, "");
	_erThreadReturn			= errNoError;
	_stPThreadStatus		= THREADLIB_BUILDED;
	*this = t;
}


/*
PosixThread &PosixThread:: operator = (const PosixThread &)

{

	return *this;
}
*/


Error PosixThread:: init (const char *pPThreadName)

{

	if (_stPThreadStatus != THREADLIB_BUILDED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (pPThreadName == (const char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	if (setPThreadName (pPThreadName) != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_SETPTHREADNAME_FAILED);
		_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));
		
		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_attr_create (&_pThreadAttribute) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_CREATE_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_init (&_pThreadAttribute)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_CREATE_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif

	_stPThreadStatus		= THREADLIB_INITIALIZED;


	return errNoError;
}


Error PosixThread:: finish (void)

{

	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING &&
		_stPThreadStatus != THREADLIB_DETACHED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_attr_delete (&_pThreadAttribute) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_DELETE_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));
		
			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_destroy (&_pThreadAttribute)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_DELETE_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif

	_stPThreadStatus		= THREADLIB_BUILDED;


	return errNoError;
}


Error PosixThread:: getThreadState (PThreadStatus_p pstThreadState)

{

	if (pstThreadState == (PThreadStatus_p) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pstThreadState");

		return _erThreadReturn;
	}

	*pstThreadState		= _stPThreadStatus;


	return errNoError;
}


PosixThread::PThreadStatus_t PosixThread:: getThreadState (void)

{

	return _stPThreadStatus;
}


Error PosixThread:: setPriority (long lNewPriority)

{

	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_attr_setprio (&_pThreadAttribute, (int) lNewPriority) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_SETPRIO_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			struct sched_param		spSchedParam;
			long					lErrorNumber;


			#if !defined(__APPLE__)
				if (lNewPriority == THREADLIB_PRIORITY_MIN)
				{
					spSchedParam. sched_priority		=
						sched_get_priority_min (
							sched_getscheduler(getpid ()));
				}
				else if (lNewPriority == THREADLIB_PRIORITY_MEDIUM)
				{
					spSchedParam. sched_priority		=
						(
						sched_get_priority_max (
							sched_getscheduler(getpid ())) -
						sched_get_priority_min (
							sched_getscheduler(getpid ()))
						) / 2;
				}
				else if (lNewPriority == THREADLIB_PRIORITY_MAX)
				{
					spSchedParam. sched_priority		=
						sched_get_priority_max (
							sched_getscheduler(getpid ()));
				}
				else
				{
					spSchedParam. sched_priority		= (int) lNewPriority;
				}
			#else
				spSchedParam. sched_priority		= (int) lNewPriority;
			#endif

			if ((lErrorNumber = pthread_attr_setschedparam (&_pThreadAttribute,
				&spSchedParam)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_SETPRIO_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: getPriority (long *plPriority)

{

	if (_stPThreadStatus == THREADLIB_BUILDED ||
		_stPThreadStatus == THREADLIB_DETACHED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (plPriority == (long *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "plPriority");

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if ((*plPriority = pthread_attr_getprio (_pThreadAttribute)) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_GETPRIO_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			struct sched_param		spSchedParam;
			long					lErrorNumber;


			if ((lErrorNumber = pthread_attr_getschedparam (&_pThreadAttribute,
				&spSchedParam)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_GETPRIO_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}

			*plPriority		= spSchedParam. sched_priority;
		}
	#endif


	return errNoError;
}


#if __QTCOMPILER__
#else
	Error PosixThread:: setInheritScheduling (
		PThreadInheritScheduling_t isNewInheritScheduling)

	{

		if (_stPThreadStatus != THREADLIB_INITIALIZED &&
			_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

			return _erThreadReturn;
		}

		#if defined(__hpux) && defined(_CMA__HP)
			if (pthread_attr_setinheritsched (&_pThreadAttribute,
				(int) isNewInheritScheduling) == -1)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_SETINHERITSCHED_FAILED, 1, errno);
				_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

				return _erThreadReturn;
			}
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			{
				long			lErrorNumber;


				if ((lErrorNumber = pthread_attr_setinheritsched (
					&_pThreadAttribute, (int) isNewInheritScheduling)) != 0)
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_ATTR_SETINHERITSCHED_FAILED, 1, lErrorNumber);
					_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

					return _erThreadReturn;
				}
			}
		#endif


		return errNoError;
	}
#endif


#if __QTCOMPILER__
#else
	Error PosixThread:: getInheritScheduling (
		PThreadInheritScheduling_p pisInheritScheduling)

	{

		if (_stPThreadStatus == THREADLIB_BUILDED ||
			_stPThreadStatus == THREADLIB_DETACHED)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

			return _erThreadReturn;
		}

		if (pisInheritScheduling == (PThreadInheritScheduling_p) NULL)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PARAMETER_WRONG, 1, "pisInheritScheduling");

			return _erThreadReturn;
		}

		#if defined(__hpux) && defined(_CMA__HP)
			if ((*pisInheritScheduling =
				(PThreadInheritScheduling_t) pthread_attr_getinheritsched (
				_pThreadAttribute)) == -1)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_GETINHERITSCHED_FAILED, 1, errno);
				_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

				return _erThreadReturn;
			}
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			{
				long			lErrorNumber;


				if ((lErrorNumber = pthread_attr_getinheritsched (
					&_pThreadAttribute, (int *) pisInheritScheduling)) != 0)
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_ATTR_GETINHERITSCHED_FAILED, 1, lErrorNumber);
					_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

					return _erThreadReturn;
				}
			}
		#endif


		return errNoError;
	}
#endif


Error PosixThread:: setSchedulingPolicy (
	PThreadSchedulingPolicy_t spNewSchedulingPolicy)

{

	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_attr_setsched (&_pThreadAttribute,
			(int) spNewSchedulingPolicy) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_SETSCHED_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_setschedpolicy (&_pThreadAttribute,
				(int) spNewSchedulingPolicy)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_SETSCHED_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: getSchedulingPolicy (
	PThreadSchedulingPolicy_p pspSchedulingPolicy)

{

	if (_stPThreadStatus == THREADLIB_BUILDED ||
		_stPThreadStatus == THREADLIB_DETACHED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (pspSchedulingPolicy == (PThreadSchedulingPolicy_p) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pspSchedulingPolicy");

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if ((*pspSchedulingPolicy =
			(PThreadSchedulingPolicy_t) pthread_attr_getsched (
			_pThreadAttribute)) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_GETSCHED_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_getschedpolicy (&_pThreadAttribute,
				(int *) pspSchedulingPolicy)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_GETSCHED_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: setStackSize (long lNewStackSize)

{

	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_attr_setstacksize (&_pThreadAttribute,
			(int) lNewStackSize) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_SETSTACKSIZE_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_setstacksize (&_pThreadAttribute,
				(size_t) lNewStackSize)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_SETSTACKSIZE_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: getStackSize (long *plStackSize)

{

	if (_stPThreadStatus == THREADLIB_BUILDED ||
		_stPThreadStatus == THREADLIB_DETACHED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (plStackSize == (long *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "plStackSize");

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if ((*plStackSize = pthread_attr_getstacksize (_pThreadAttribute)) ==
			-1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_GETSTACKSIZE_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_attr_getstacksize (&_pThreadAttribute,
				(size_t *) plStackSize)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_ATTR_GETSTACKSIZE_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: setPThreadName (const char *pPThreadName)

{

	if (pPThreadName == (const char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	if (strlen (pPThreadName) > MAX_PTHREADNAMELENGTH - 1)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_SETPTHREADNAME_FAILED);

		return _erThreadReturn;
	}

	strcpy (_pPThreadName, pPThreadName);

	
	return errNoError;
}


Error PosixThread:: getPThreadName (char *pPThreadName)

{

	if (pPThreadName == (char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	strcpy (pPThreadName, _pPThreadName);


	return errNoError;
}


Error PosixThread:: start (Boolean_t bToBeDetached)

{

	long					lErrorNumber;


	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_erThreadReturn		= errNoError;

	// E' necessario inizializzare lo stato a STARTED prima perche'
	// nel caso in cui il thread e' molto veloce e finisce prima
	// che la pthread_create ritorna, lo stato viene messo a STARTED
	// con il thread concluso (cioe' viene sovrascritta l'inizializzazione
	// a INITIALIZEDAGAINAFTERRUNNING che il thread finito
	// nella runFunction setta)
	if (bToBeDetached)
	{
		if ((lErrorNumber = pthread_attr_setdetachstate (&_pThreadAttribute,
			PTHREAD_CREATE_DETACHED)) != 0)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_SETDETACHSTATE_FAILED, 1, lErrorNumber);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (long));

			return _erThreadReturn;
		}

		_stPThreadStatus		= THREADLIB_DETACHED;
	}
	else
	{
		if ((lErrorNumber = pthread_attr_setdetachstate (&_pThreadAttribute,
			PTHREAD_CREATE_JOINABLE)) != 0)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_ATTR_SETDETACHSTATE_FAILED, 1, lErrorNumber);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (long));

			return _erThreadReturn;
		}

		_stPThreadStatus		= THREADLIB_STARTED;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_create (&_pThread, _pThreadAttribute, runFunction,
			this) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CREATE_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));
		
			_stPThreadStatus		= THREADLIB_INITIALIZED;

			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_create (&_pThread, &_pThreadAttribute,
				runFunction, this)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CREATE_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));
		
				_stPThreadStatus		= THREADLIB_INITIALIZED;

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: getThreadIdentifier (unsigned long *pulThreadIdentifier) const

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return err;
	}

	if (pulThreadIdentifier == (unsigned long *) NULL)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "plThreadIdentifier");

		return err;
	}

	#if defined(WIN32)
		*pulThreadIdentifier			= GetCurrentThreadId ();
	#else
		if (PosixThread:: getThreadIdentifier (
			&_pThread, pulThreadIdentifier) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


PosixThread:: operator long (void) const

{

	unsigned long			ulThreadIdentifier;


	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return -1;
	}

	#if defined(WIN32)
		ulThreadIdentifier			= GetCurrentThreadId ();
	#else
		if (PosixThread:: getThreadIdentifier (
			&_pThread, &ulThreadIdentifier) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);

			return -1;
		}
	#endif


	return ulThreadIdentifier;
}


Error PosixThread:: getThreadIdentifier (const pthread_t *pThread,
	unsigned long *pulThreadIdentifier)

{

	if (pThread == (pthread_t *) NULL)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pThread");

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		*pulThreadIdentifier		= pthread_getunique_np (pThread);
	#elif defined(__CYGWIN__)
		*pulThreadIdentifier			= 1;
	#elif defined(WIN32)
		*pulThreadIdentifier			= 1;
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		#ifdef __APPLE__
			*pulThreadIdentifier		= 1;
		#else
			*pulThreadIdentifier		= *pThread;
		#endif
	#endif


	return errNoError;
}


unsigned long PosixThread:: getCurrentThreadIdentifier (void)

{

	#if defined(WIN32)
	#else
		pthread_t			pThread;
	#endif
	unsigned long			ulThreadIdentifier;


	#if defined(WIN32)
		ulThreadIdentifier			= GetCurrentThreadId ();
	#else
		if (getCurrentThread (pThread) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETCURRENTTHREAD_FAILED);

			return 0;
		}

		if (getThreadIdentifier (&pThread, &ulThreadIdentifier) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);

			return 0;
		}
	#endif


	return ulThreadIdentifier;
}


Error PosixThread:: getCurrentThread (pthread_t &pThread)

{

	pThread				= pthread_self ();


	return errNoError;
}


Error PosixThread:: setCurrentPriority (long lNewCurrentPriority)

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_setprio (_pThread, (int) lNewCurrentPriority) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_SETPRIO_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#elif defined(__CYGWIN__) || defined(WIN32)
	#elif defined(__APPLE__)
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			struct sched_param		spSchedParam;


			spSchedParam. sched_priority		= (int) lNewCurrentPriority;

			if (sched_setparam (_pThread, &spSchedParam) == -1)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_SETPRIO_FAILED, 1, errno);
				_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

				return _erThreadReturn;
			}
		}
	#endif


	return errNoError;
}


Error PosixThread:: getCurrentPriority (long *plCurrentPriority)

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (plCurrentPriority == (long *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "plCurrentPriority");

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if ((*plCurrentPriority = pthread_getprio (_pThread)) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_GETPRIO_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}
	#elif defined(__CYGWIN__) || defined (WIN32)
		*plCurrentPriority				= 1;
	#elif defined(__APPLE__)
		*plCurrentPriority				= 1;
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		struct sched_param		spSchedParam;


		if (sched_getparam (_pThread, &spSchedParam) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_SETPRIO_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));

			return _erThreadReturn;
		}

		*plCurrentPriority		= spSchedParam. sched_priority;
	#endif


	return errNoError;
}


Error PosixThread:: yield (void)

{

	#if defined(__hpux) && defined(_CMA__HP)
		pthread_yield ();
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		if (sched_yield () == -1)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_YIELD_FAILED, 1, errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}
	#endif



	return errNoError;
}


Error PosixThread:: getSleep (unsigned long ulSeconds,
	unsigned long ulAdditionalMicroSeconds)

{

	#if defined(__hpux) && defined(_CMA__HP)
		{
			struct timespec sleepTime;

			sleepTime. tv_sec		= (int) ulSeconds;
			sleepTime. tv_nsec		= ulAdditionalMicroSeconds * 1000;

			if (pthread_delay_np (&sleepTime))
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_DELAY_NP_FAILED, 1, errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}
	#elif defined(WIN32)
		#define SIMPLE_SLEEP 1
		#ifdef SIMPLE_SLEEP
		{
// __int64 ullNowLocalInMilliSecs1;
// __int64 ullNowLocalInMilliSecs2;
// long lTimeDifference;
// unsigned long ulMillisecondsToWait;

		if (ulSeconds > 0)
		{
// DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs1);
			Sleep (ulSeconds * 1000);
// DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs2);
// lTimeDifference		= (long)
// 	(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);
// if (lTimeDifference < (long) (ulSeconds * 1000))
// {
// 	ulMillisecondsToWait			=
// 		((long) (ulSeconds * 1000)) -
// 		lTimeDifference;
// char pBuffer [1024];
// sprintf (pBuffer,
// 	"SLEEP. SECS. Difference to wait: %lu Input was: %lu\n",
// 	ulMillisecondsToWait,
// 	(unsigned long) (ulSeconds * 1000));
// FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
// }
		}

		if (ulAdditionalMicroSeconds > 0)
		{
// DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs1);
			Sleep (ulAdditionalMicroSeconds / 1000);
// DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs2);
// lTimeDifference		= (long)
//	(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);
//if (lTimeDifference < (long) (ulAdditionalMicroSeconds / 1000))
//{
//	ulMillisecondsToWait			=
//		((long) (ulAdditionalMicroSeconds / 1000)) -
//		lTimeDifference;
//char pBuffer [1024];
//sprintf (pBuffer,
//	"SLEEP. MILLISECS. Difference to wait: %lu Input was: %lu\n",
//	ulMillisecondsToWait,
//	(unsigned long) (ulAdditionalMicroSeconds / 1000));
//FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
//}
		}
		}
		#elif SLEEP_REPEATED
		{
		__int64						ullNowLocalInMilliSecs1;
		__int64						ullNowLocalInMilliSecs2;
		Boolean_t					bIsTimeExpired;
		unsigned long				ulMillisecondsToWait;
		long						lTimeDifference;

		if (ulSeconds > 0)
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullNowLocalInMilliSecs1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

				return err;
			}

			ulMillisecondsToWait			= ulSeconds * 1000;

			bIsTimeExpired				= false;

			while (!bIsTimeExpired)
			{
				Sleep (ulMillisecondsToWait);

				if (DateTime:: nowLocalInMilliSecs (
					&ullNowLocalInMilliSecs2) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

					return err;
				}

				lTimeDifference		= (long)
					(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);

				if (lTimeDifference < (long) (ulSeconds * 1000))
				{
					ulMillisecondsToWait			=
						((long) (ulSeconds * 1000)) -
						lTimeDifference;
// char pBuffer [1024];
// sprintf (pBuffer,
// 	"SLEEP. SECS. Difference to wait: %lu Input was: %lu\n",
// 	ulMillisecondsToWait,
// 	ulSeconds * 1000);
// FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
				}
				else
				{
					bIsTimeExpired			= true;
				}
			}
		}

		if (ulAdditionalMicroSeconds > 0)
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullNowLocalInMilliSecs1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

				return err;
			}

			ulMillisecondsToWait			=
				ulAdditionalMicroSeconds / 1000;

			bIsTimeExpired				= false;

			while (!bIsTimeExpired)
			{
				Sleep (ulMillisecondsToWait);

				if (DateTime:: nowLocalInMilliSecs (
					&ullNowLocalInMilliSecs2) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

					return err;
				}

				lTimeDifference		= (long)
					(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);

				if (lTimeDifference <
					(long) (ulAdditionalMicroSeconds / 1000))
				{
					ulMillisecondsToWait			=
						((long) (ulAdditionalMicroSeconds / 1000)) -
						lTimeDifference;
// char pBuffer [1024];
// sprintf (pBuffer,
// 	"SLEEP. MILLISECS. Difference to wait: %lu Input was: %lu\n",
// 	ulMillisecondsToWait,
// 	ulAdditionalMicroSeconds / 1000);
// FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
				}
				else
				{
					bIsTimeExpired			= true;
				}
			}
		}
		}
		#elif COND_VAR
		{
		__int64				ullInitialLocalInMilliSecs;
		__int64				ullNowLocalInMilliSecs;
		PCondition_t		cCondition;
		PMutex_t			mtMutex;
		unsigned long		ulRemainingSecondsToWait;
		unsigned long		ulRemainingMilliSecondsToWait;
		Boolean_t			bSignalReceivedButEventNotPresent;
		long				lRemainingTimeInMilliSecs;
		unsigned long		ulMilliSecondsToWait;
		Error_t				errTimedWait;


		if (DateTime:: nowLocalInMilliSecs (
			&ullInitialLocalInMilliSecs) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

			return err;
		}

		if (mtMutex. init (PMutex:: MUTEX_FAST) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			return err;
		}

		if (mtMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			if (mtMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}

			return err;
		}

		if (cCondition. init (PCondition:: COND_DEFAULT) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PCONDITION_INIT_FAILED);

			if (mtMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			if (mtMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}

			return err;
		}

		ulRemainingSecondsToWait		= ulSeconds;

		ulRemainingMilliSecondsToWait	= ulAdditionalMicroSeconds / 1000;

		ulMilliSecondsToWait		=
			(ulSeconds * 1000) + ulRemainingMilliSecondsToWait;

		bSignalReceivedButEventNotPresent				= true;

		while (bSignalReceivedButEventNotPresent)
		{
			// the signal API (addEvent method) could unblock more than one
			// threads. In this case the event cannot be found because the
			// previous thread took it
			if ((errTimedWait = cCondition. timedWait (
				&mtMutex, ulRemainingSecondsToWait,
				ulRemainingMilliSecondsToWait)) != errNoError)
			{
				int					i_errno;
				unsigned long		ulUserDataBytes;


				if ((long) errTimedWait == THREADLIB_COND_TIMEDWAIT_FAILED)
				{
					errTimedWait. getUserData (&i_errno, &ulUserDataBytes);

					#if defined(__hpux) && defined(_CMA__HP)
						if (i_errno == 11)	// EAGAIN
					#elif WIN32
						if (i_errno == WSAETIMEDOUT)
					#else
						if (i_errno == ETIMEDOUT)
					#endif
					{
						// blocking time expired

						// bSignalReceivedButEventNotPresent			= false;

						// continue;
					}
					else
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

						if (cCondition. finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PCONDITION_FINISH_FAILED);
						}

						if (mtMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}

						if (mtMutex. finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
						}

						return errTimedWait;
					}
				}
				else
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

					if (cCondition. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PCONDITION_FINISH_FAILED);
					}

					if (mtMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}

					if (mtMutex. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
					}

					return errTimedWait;
				}
			}

			if (DateTime:: nowLocalInMilliSecs (
				&ullNowLocalInMilliSecs) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

				if (cCondition. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PCONDITION_FINISH_FAILED);
				}

				if (mtMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				if (mtMutex. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
				}

				return err;
			}

			lRemainingTimeInMilliSecs			= (long) (
				ulMilliSecondsToWait -
				((unsigned long) (ullNowLocalInMilliSecs -
				ullInitialLocalInMilliSecs)));

			if (lRemainingTimeInMilliSecs <= 0)
			{
				// blocking time expired

				bSignalReceivedButEventNotPresent				= false;
			}
			else
			{
				ulRemainingSecondsToWait		= (unsigned long)
					(lRemainingTimeInMilliSecs / 1000);

				ulRemainingMilliSecondsToWait	=
					lRemainingTimeInMilliSecs -
					ulRemainingSecondsToWait * 1000;
// char		pBuffer [1024];
// sprintf (pBuffer,
// 	"SLEEP Remaining secs-millisecs: %lu-%lu, Input secs-millisecs: %lu-%lu\n",
//	ulRemainingSecondsToWait, ulRemainingMilliSecondsToWait,
//	ulSeconds, (unsigned long) (ulAdditionalMicroSeconds / 1000));
//FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
			}
		}

//			{
//char		pBuffer [1024];
//DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs);
//lRemainingTimeInMilliSecs			= (long) (
//	ulMilliSecondsToWait -
//	((unsigned long) (ullNowLocalInMilliSecs -
//	ullInitialLocalInMilliSecs)));
//sprintf (pBuffer, "SLEEP remaining time: %ld\n", lRemainingTimeInMilliSecs);
//FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
//			}
		if (cCondition. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PCONDITION_FINISH_FAILED);

			if (mtMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			if (mtMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}

			return err;
		}

		if (mtMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			if (mtMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}

			return err;
		}

		if (mtMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);

			return err;
		}
		}
		#endif
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		//	The current process is  suspended  from  execution  for  the
		//	number  of  seconds  specified  by the argument.
		//	In a multithreaded program,  only  the  invoking  thread  is
		//	suspended from execution.
		//	If a SIGALRM is generated for a multi-threaded process,
		//	it may not be delivered to a thread currently in sleep().

		if (ulSeconds > 0)
		{
//unsigned long long ullNowLocalInMilliSecs1;
//unsigned long long ullNowLocalInMilliSecs2;
//long lTimeDifference;
//unsigned long ulMillisecondsToWait;
//DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs1);
			sleep (ulSeconds);
//DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs2);
//lTimeDifference		= (long)
//	(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);
//if (lTimeDifference < (long) (ulSeconds * 1000))
//{
//	ulMillisecondsToWait			=
//		((long) (ulSeconds * 1000)) -
//		lTimeDifference;
//char pBuffer [1024];
//sprintf (pBuffer,
//	"SLEEP. SECS. Difference to wait: %lu Input was: %lu\n",
//	ulMillisecondsToWait,
//	(unsigned long) (ulSeconds * 1000));
//FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
//}
		}

		if (ulAdditionalMicroSeconds > 0)
		{
//unsigned long long ullNowLocalInMilliSecs1;
//unsigned long long ullNowLocalInMilliSecs2;
//long lTimeDifference;
//unsigned long ulMillisecondsToWait;
//DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs1);
			usleep (ulAdditionalMicroSeconds);
//DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs2);
//lTimeDifference		= (long)
//	(ullNowLocalInMilliSecs2 - ullNowLocalInMilliSecs1);
//if (lTimeDifference < (long) (ulAdditionalMicroSeconds / 1000))
//{
//	ulMillisecondsToWait			=
//		((long) (ulAdditionalMicroSeconds / 1000)) -
//		lTimeDifference;
//char pBuffer [1024];
//sprintf (pBuffer,
//	"SLEEP. MILLISECS. Difference to wait: %lu Input was: %lu\n",
//	ulMillisecondsToWait,
//	(unsigned long) (ulAdditionalMicroSeconds / 1000));
// FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
//}
		}
	#endif


	return errNoError;
}


Error PosixThread:: exit (void)

{

	#if defined(__hpux) && defined(_CMA__HP)
		pthread_exit ((pthread_addr_t) NULL);
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		pthread_exit ((void *) NULL);
	#endif


	return errNoError;
}


Error PosixThread:: cancel (void)

{

	#ifdef __QTCOMPILER__
		// pthread_cancel is not supported by Android
        return errNoError;
    #else
		if (_stPThreadStatus != THREADLIB_STARTED)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

			return _erThreadReturn;
		}

		#if defined(__hpux) && defined(_CMA__HP)
			if (pthread_cancel (_pThread) == -1)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CANCEL_FAILED, 1, errno);
				_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));
			
				return _erThreadReturn;
			}
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			{
				long			lErrorNumber;


				if ((lErrorNumber = pthread_cancel (_pThread)) != 0)
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_CANCEL_FAILED, 1, lErrorNumber);
					_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));

					return _erThreadReturn;
				}
			}
		#endif

	
		return errNoError;
	#endif
}


#ifdef __QTCOMPILER__
#else
	Error PosixThread:: testCancel (void)

	{

		pthread_testcancel ();


		return errNoError;
	}
#endif


#ifdef __QTCOMPILER__
#else
	Error PosixThread:: setCancel (PThreadCancellationState_t csCancelability)

	{

		#if defined(__hpux) && defined(_CMA__HP)
			if (pthread_setcancel (csCancelability) == -1)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_SETCANCEL_FAILED, 1, errno);
				err. setUserData ((void *) (&errno), sizeof (int));
		
				return err;
			}
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			{
				int			iOldCancelability;
				long		lErrorNumber;


				if ((lErrorNumber = pthread_setcancelstate (
					(int) csCancelability, &iOldCancelability)) != 0)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_SETCANCEL_FAILED, 1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));
		
					return err;
				}
			}
		#endif


		return errNoError;
	}
#endif


#ifdef __QTCOMPILER__
#else
	Error PosixThread:: setCancelType (PThreadCancellationType_t ctCancelType)

	{

		#if defined(__hpux) && defined(_CMA__HP)
			if (pthread_setasynccancel (ctCancelType) == -1)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_SETCANCELTYPE_FAILED, 1, errno);
				err. setUserData ((void *) (&errno), sizeof (int));
		
				return err;
			}
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			{
				int			iOldCancelType;
				long		lErrorNumber;


				if ((lErrorNumber = pthread_setcanceltype ((int) ctCancelType,
					&iOldCancelType)) != 0)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_SETCANCELTYPE_FAILED, 1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));
		
					return err;
				}
			}
		#endif


		return errNoError;
	}
#endif


Error PosixThread:: getThreadError (Error *perrThreadError)

{

	if (perrThreadError == (Error *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "perrThreadError");

		return _erThreadReturn;
	}

	*perrThreadError	= _erThreadReturn;


	return errNoError;
}


Error PosixThread:: join (Error_p perrJoinError)

{


	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		*perrJoinError = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_stPThreadStatus		= THREADLIB_STARTED_AND_JOINED;

	*perrJoinError			= errNoError;

	#if defined(__hpux) && defined(_CMA__HP)
		{
			pthread_addr_t	paReturnValue;

			if (pthread_join (_pThread, &paReturnValue) == -1)
			{
				*perrJoinError = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_JOIN_FAILED, 1, errno);
				perrJoinError -> setUserData ((void *) (&errno), sizeof (int));

				_stPThreadStatus		= THREADLIB_STARTED;

				return _erThreadReturn;
			}
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			void		*pvStatus;
			long		lErrorNumber;


			if ((lErrorNumber = pthread_join (_pThread, &pvStatus)) != 0)
			{
				*perrJoinError = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_JOIN_FAILED, 1, lErrorNumber);
				perrJoinError -> setUserData (&lErrorNumber, sizeof (long));

				_stPThreadStatus		= THREADLIB_STARTED;

				return _erThreadReturn;
			}
		}
	#endif


	return _erThreadReturn;
}


Error PosixThread:: detach (void)

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_detach (&_pThread) == -1)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_DETACH_FAILED, 1, errno);
			_erThreadReturn. setUserData ((void *) (&errno), sizeof (int));
		
			return _erThreadReturn;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_detach (_pThread)) != 0)
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_DETACH_FAILED, 1, lErrorNumber);
				_erThreadReturn. setUserData (&lErrorNumber, sizeof (long));
		
				return _erThreadReturn;
			}
		}
	#endif

	_stPThreadStatus		= THREADLIB_DETACHED;


	return errNoError;

}


