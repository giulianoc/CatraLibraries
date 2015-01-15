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


#include "PMutex.h"
#include <errno.h>


#if defined(__hpux) && defined(_CMA__HP)
	extern int errno;
#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
#endif



PMutex:: PMutex (void)

{

	_stPMutexStatus		= THREADLIB_BUILDED;
}


PMutex:: ~PMutex (void)

{ 

}


PMutex:: PMutex (const PMutex &m)

{


	_stPMutexStatus		= THREADLIB_BUILDED;
	*this = m;
}


/*
PMutex &PMutex:: operator = (const PMutex &)

{

	return *this;
}
*/


Error PMutex:: init (PMutexType_t pMutexType)

{

	if (_stPMutexStatus != THREADLIB_BUILDED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutexattr_create (&_pMutexAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEXATTR_CREATE_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long				lErrorNumber;


			if ((lErrorNumber = pthread_mutexattr_init (
				&_pMutexAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEXATTR_CREATE_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutexattr_setkind_np (&_pMutexAttribute,
			(int) pMutexType))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEXATTR_SETTYPE_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			if (pthread_mutexattr_delete (&_pMutexAttribute))
			{
			}

			return err;
		}
	#else	// POSIX
		/*
		#if defined(__sparc)				// SunOs (one of the first version)
			{
				long			lErrorNumber;


				if ((lErrorNumber = pthread_mutexattr_setpshared (
					&_pMutexAttribute, (int) pMutexType)) != 0)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_MUTEXATTR_SETTYPE_FAILED, 1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));

					if (pthread_mutexattr_destroy (&_pMutexAttribute))
					{
					}

					return err;
				}
			}
		#else	// POSIX.1-1996 standard (HPUX 11)
		*/
			// POSIX.1-1996 standard (HPUX 11)
			/*
			if ((errno = pthread_mutexattr_setpshared (&_pMutexAttribute,
				PTHREAD_PROCESS_PRIVATE)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEXATTR_SETTYPE_FAILED, 1, errno);
				err. setUserData (&errno, sizeof (int));

				if (pthread_mutexattr_destroy (&_pMutexAttribute))
				{
				}

				return err;
			}
			*/

			{
				long			lErrorNumber;


				// pthread_mutexattr_settype is X/Open standard
				if ((lErrorNumber = pthread_mutexattr_settype (
					&_pMutexAttribute, (int) pMutexType)) != 0)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_MUTEXATTR_SETTYPE_FAILED, 1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));

					if (pthread_mutexattr_destroy (&_pMutexAttribute))
					{
					}

					return err;
				}
			}
		// #endif
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutex_init (&_pMutex, _pMutexAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEX_INIT_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));
		
			if (pthread_mutexattr_delete (&_pMutexAttribute))
			{
			}

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_mutex_init (&_pMutex,
				&_pMutexAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEX_INIT_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));
		
				if (pthread_mutexattr_destroy (&_pMutexAttribute))
				{
				}

				return err;
			}
		}
	#endif

	_stPMutexStatus		= THREADLIB_INITIALIZED;


	return errNoError;
}


Error PMutex:: finish (void)

{

	if (_stPMutexStatus != THREADLIB_INITIALIZED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutex_destroy (&_pMutex))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEX_DESTROY_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_mutex_destroy (&_pMutex)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEX_DESTROY_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutexattr_delete (&_pMutexAttribute))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEXATTR_DELETE_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));
		
			if (pthread_mutex_init (&_pMutex, _pMutexAttribute))
			{
			}

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_mutexattr_destroy (
				&_pMutexAttribute)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEXATTR_DELETE_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));
		
				if (pthread_mutex_init (&_pMutex, &_pMutexAttribute))
				{
				}

				return err;
			}
		}
	#endif

	_stPMutexStatus		= THREADLIB_BUILDED;


	return errNoError;
}


Error PMutex:: lock (void)

{ 

	if (_stPMutexStatus != THREADLIB_INITIALIZED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutex_lock (&_pMutex))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEX_LOCK_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_mutex_lock (&_pMutex)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEX_LOCK_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));

				return err;
			}
		}
	#endif


	return errNoError;
}


Error PMutex:: unLock (void)

{

	if (_stPMutexStatus != THREADLIB_INITIALIZED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (pthread_mutex_unlock (&_pMutex))
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_MUTEX_UNLOCK_FAILED, 1, errno);
			err. setUserData (&errno, sizeof (int));

			return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long			lErrorNumber;


			if ((lErrorNumber = pthread_mutex_unlock (&_pMutex)) != 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEX_UNLOCK_FAILED, 1, lErrorNumber);
				err. setUserData (&lErrorNumber, sizeof (long));
		
				return err;
			}
		}
	#endif


	return errNoError;

}


Error PMutex:: tryLock (Boolean_p pbIsLockable)

{

	if (_stPMutexStatus != THREADLIB_INITIALIZED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return err;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		switch (pthread_mutex_trylock (&_pMutex))
		{
			case 1:
				*pbIsLockable		= true;

				break;
			case 0:
				*pbIsLockable		= false;

				break;
			case -1:
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_MUTEX_TRYLOCK_FAILED, 1, errno);
				err. setUserData (&errno, sizeof (int));

				return err;
		}
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		{
			long				lErrorNumber;


			switch ((lErrorNumber = pthread_mutex_trylock (&_pMutex)))
			{
				case 0:
					*pbIsLockable		= true;

					break;
				case EBUSY:
					*pbIsLockable		= false;

					break;
				default:
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_MUTEX_TRYLOCK_FAILED, 1, lErrorNumber);
					err. setUserData (&lErrorNumber, sizeof (long));

					return err;
			}
		}
	#endif


	return errNoError;
}


PMutex:: operator pthread_mutex_t *()

{ 

	if (_stPMutexStatus != THREADLIB_INITIALIZED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPMutexStatus);

		return ((pthread_mutex_t *) NULL);
	}

	return &_pMutex; 

}

