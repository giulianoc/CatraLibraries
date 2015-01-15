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

#include "AThread.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


#define ITERATIONS_NUMBER		2


int main ()

{

	AThread_t					thThread1;
	AThread_t					thThread2;
	PMutex_t					mtMutex;
	long						lIndex;
	Error_t						errReturn;
	Error_t						errJoin;


	for (lIndex = 0; lIndex < ITERATIONS_NUMBER; lIndex++)
	{
		std:: cout << "mtMutex. init" << std:: endl;

		#if defined(__hpux) && defined(_CMA__HP)
			if ((errReturn = mtMutex. init (
				PMutex:: MUTEX_FAST)) !=
		#else	// POSIX
			#if defined(__sparc)
				if ((errReturn = mtMutex. init (
					PMutex:: MUTEX_PROCESS_PRIVATE)) !=
			#elif defined(__CYGWIN__)
				if ((errReturn = mtMutex. init (
					PMutex:: MUTEX_NONRECURSIVE)) !=
			#else		// POSIX.1-1996 standard (HPUX 11)
				if ((errReturn = mtMutex. init (
					PMutex:: MUTEX_FAST)) !=
			#endif
		#endif
			errNoError)
		{
			std:: cout << (const char *) errReturn << std:: endl;
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread1. init" << std:: endl;

		if (thThread1. init (&mtMutex, "Thread 1") != errNoError)
		{
			std:: cout << "ThreadsAndMutex:: init failed" << std:: endl;

			return 1;
		}

		std:: cout << "thThread2. init" << std:: endl;

		if (thThread2. init (&mtMutex, "Thread 2") != errNoError)
		{
			std:: cout << "ThreadsAndMutex:: init failed" << std:: endl;

			return 1;
		}

		std:: cout << "thThread1. start" << std:: endl;

		if (thThread1. start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "PThread:: getSleep 1 sec" << std:: endl;

		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread2. start" << std:: endl;

		if (thThread2. start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread1. join" << std:: endl;

		if (thThread1. join (&errJoin) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_JOIN_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread2. join" << std:: endl;

		if (thThread2. join (&errJoin) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_JOIN_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread2. finish" << std:: endl;

		if (thThread2. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "thThread1. finish" << std:: endl;

		if (thThread1. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "mtMutex. finish" << std:: endl;

		if (mtMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}


	return 0;
}


