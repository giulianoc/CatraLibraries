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


#include "PCondition.h"
#include "DateTime.h"
#include <errno.h>


#if defined(__hpux) && defined(_CMA__HP)
	extern int errno;
#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
#endif



PCondition:: PCondition (void)

{ 

}


PCondition:: ~PCondition (void)

{ 

}


PCondition:: PCondition (const PCondition &c)
{

	*this = c;

}


/*
PCondition &PCondition:: operator = (const PCondition &)

{

	return *this;
}
*/


Error PCondition:: init (PConditionType_t pConditionType)

{ 

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_condattr_create (&_pConditionAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CONDATTR_CREATE_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_condattr_init (
				&_pConditionAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CONDATTR_CREATE_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
	#else	// POSIX
		#if defined(__sparc)				// SunOs
			{
				long			lErrorNumber;


				if ((lErrorNumber = pthread_condattr_setpshared (
					&_pConditionAttribute, (int) pConditionType)) != 0)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_CONDATTR_SETATTRIBUTE_FAILED,
						1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));

					return err;
				}
			}
		#else	// POSIX.1-1996 standard (HPUX 11)
		#endif
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_init (&_pCondition, _pConditionAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_INIT_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			if (pthread_condattr_delete (&_pConditionAttribute))
			{
			}

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_cond_init (&_pCondition,
				&_pConditionAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_COND_INIT_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				if (pthread_condattr_destroy (&_pConditionAttribute))
				{
				}

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PCondition:: finish (void)

{ 

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_destroy (&_pCondition))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_DESTROY_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_cond_destroy (&_pCondition)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_COND_DESTROY_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_condattr_delete (&_pConditionAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CONDATTR_DELETE_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));
		
			if (pthread_cond_init (&_pCondition, _pConditionAttribute))
			{
			}

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long				lErrorNumber;


			if ((lErrorNumber = pthread_condattr_destroy (
				&_pConditionAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CONDATTR_DELETE_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));
		
				if (pthread_cond_init (&_pCondition, &_pConditionAttribute))
				{
				}

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PCondition:: wait (PMutex_p pMutex)

{

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_wait (&_pCondition, (pthread_mutex_t *) pMutex))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_WAIT_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long				lErrorNumber;


			if ((lErrorNumber = pthread_cond_wait (&_pCondition,
				(pthread_mutex_t *) pMutex)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_COND_WAIT_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PCondition:: timedWait (PMutex_p pMutex, unsigned long ulSeconds,
	unsigned long ulAdditionalMilliSeconds)

{

	/*
	#ifdef WIN32
		__int64				ullNowUTCInSecs;
	#else
	*/
		unsigned long long	ullNowUTCInSecs;
	// #endif
	unsigned long			ulLocalAdditionalMilliSecs;
	struct timespec			waitTime;


	if (DateTime:: nowUTCInMilliSecs (
		&ullNowUTCInSecs,
		&ulLocalAdditionalMilliSecs,
		(long *) NULL) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

		return err;
	}

	if (ulAdditionalMilliSeconds + ulLocalAdditionalMilliSecs >= 1000)
	{
		waitTime. tv_sec		= ullNowUTCInSecs + ulSeconds + 1;
		waitTime. tv_nsec		=
			(ulAdditionalMilliSeconds + ulLocalAdditionalMilliSecs - 1000) *
			1000000;
	}
	else
	{
		waitTime. tv_sec		= ullNowUTCInSecs + ulSeconds;
		waitTime. tv_nsec		=
			(ulAdditionalMilliSeconds + ulLocalAdditionalMilliSecs) *
			1000000;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_timedwait (&_pCondition, (pthread_mutex_t *) pMutex,
			&waitTime) == -1)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_TIMEDWAIT_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_cond_timedwait (&_pCondition,
				(pthread_mutex_t *) pMutex, &waitTime)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_COND_TIMEDWAIT_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PCondition:: signal (void)

{ 

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_signal (&_pCondition))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_SIGNAL_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_cond_signal (&_pCondition)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_COND_SIGNAL_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PCondition:: broadcast (void)

{ 

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_cond_broadcast (&_pCondition))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_BROADCAST_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
	{
		long			lErrorNumber;


		if ((lErrorNumber = pthread_cond_broadcast (&_pCondition)) != 0)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_COND_BROADCAST_FAILED, 1, lErrorNumber);
			err. setUserData (&lErrorNumber, sizeof (long));

			return err;
		}
	}
	#endif


	return errNoError;
}


PCondition:: operator pthread_cond_t *()

{ 

	return &_pCondition; 

}

