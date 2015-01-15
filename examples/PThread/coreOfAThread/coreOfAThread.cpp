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
#include "CoreHandler.h"
#include "signal.h"


int main ()

{

	AThread_t					thThread1;
	AThread_t					thThread2;
	Error_t						errJoin;


	signal(SIGSEGV, handler);
	signal(SIGINT, handler);
	#ifdef WIN32
	#else
		signal(SIGBUS, handler);
	#endif


	#ifdef WIN32
	#else
	{
		pthread_t		tThread;
		unsigned long			ulThreadIdentifier;


		if (PosixThread:: getCurrentThread (tThread) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETCURRENTTHREAD_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (PosixThread:: getThreadIdentifier (&tThread, &ulThreadIdentifier) !=
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Id: " << ulThreadIdentifier << std:: endl;
	}
	#endif

	if (thThread1. init (false) != errNoError)
	{
		std:: cout << "ThreadsAndMutex:: init failed" << std:: endl;

		return 1;
	}

	if (thThread2. init (true) != errNoError)
	{
		std:: cout << "ThreadsAndMutex:: init failed" << std:: endl;

		return 1;
	}

	if (thThread1. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thThread2. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	#ifdef WIN32
		WinThread:: getSleep (2, 0);
	#else
		PosixThread:: getSleep (2, 0);
	#endif

	{
		char			*ptr			= NULL;


		*ptr		= 'a';

	}

	if (thThread1. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thThread2. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thThread2. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thThread1. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}


