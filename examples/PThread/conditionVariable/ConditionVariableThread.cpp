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

#include "ConditionVariableThread.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
	#include <winsock2.h>
#endif
#include <iostream>


#ifdef WIN32
	ConditionVariableThread:: ConditionVariableThread (void): WinThread ()
#else
	ConditionVariableThread:: ConditionVariableThread (void): PosixThread ()
#endif

{

}


ConditionVariableThread:: ~ConditionVariableThread (void)

{

}


Error ConditionVariableThread:: init (void)

{

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return err;
	}

	if (_mtVariableMutex. init (PMutex:: MUTEX_FAST) != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	#if defined(__hpux) && defined(_CMA__HP)
		if (_cdVariableCondition. init (PCondition:: COND_DEFAULT) !=
			errNoError)
	#else	// POSIX
		#if defined(__sparc)
			if (_cdVariableCondition. init (
				PCondition:: COND_PROCESS_PRIVATE) != errNoError)
		#else		// POSIX.1-1996 standard (HPUX 11)
			if (_cdVariableCondition. init (PCondition:: COND_DEFAULT) !=
				errNoError)
		#endif
	#endif
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PCONDITION_INIT_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	_lVariable		= 0;


	return errNoError;
}


Error ConditionVariableThread:: finish (void)

{

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return err;
	}

	if (_mtVariableMutex. finish () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	if (_cdVariableCondition. finish () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PCONDITION_FINISH_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}


	return errNoError;
}


Error ConditionVariableThread:: run (void)

{

	while (1)
	{
		if (_mtVariableMutex. lock () != errNoError)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

std:: cout << "Prima: " << time (NULL) << std:: endl;
		if ((_erThreadReturn = _cdVariableCondition. timedWait (
			&_mtVariableMutex, 0, 500)) != errNoError)
		//	&_mtVariableMutex, 20, 0)) != errNoError)
		{
			int					i_errno;
			unsigned long		ulUserDataBytes;


			if ((long) _erThreadReturn == THREADLIB_COND_TIMEDWAIT_FAILED)
			{
				_erThreadReturn. getUserData (&i_errno, &ulUserDataBytes);

				#if defined(__hpux) && defined(_CMA__HP)
					if (i_errno == 11)	// EAGAIN
				#elif WIN32
					if (i_errno == WSAETIMEDOUT)
				#else
					if (i_errno == ETIMEDOUT)
				#endif
				{
					std:: cout << "time expired" << std:: endl;
					_erThreadReturn			= errNoError;
				}
				else
				{
					std:: cout << (const char *) _erThreadReturn << std:: endl;
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PCONDITION_TIMEDWAIT_FAILED);
					std:: cout << (const char *) _erThreadReturn << std:: endl;

					if (_mtVariableMutex. unLock () != errNoError)
					{
						_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cout << (const char *) _erThreadReturn
							<< std:: endl;
					}

					return _erThreadReturn;
				}
			}
			else
			{
				std:: cout << (const char *) _erThreadReturn << std:: endl;
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PCONDITION_TIMEDWAIT_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				if (_mtVariableMutex. unLock () != errNoError)
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cout << (const char *) _erThreadReturn << std:: endl;
				}

				return _erThreadReturn;
			}
		}
		std:: cout << "Dopo: " << time (NULL) << std:: endl;

		std:: cout << "Value: " << _lVariable << std:: endl;

		if (_mtVariableMutex. unLock () != errNoError)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}
	}


	return _erThreadReturn;
}


Error ConditionVariableThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}


Error ConditionVariableThread:: sendSignal (void)

{

	if (_mtVariableMutex. lock () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	_lVariable++;

	if (_cdVariableCondition. signal () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PCONDITION_SIGNAL_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	if (_mtVariableMutex. unLock () != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}


	return errNoError;
}

