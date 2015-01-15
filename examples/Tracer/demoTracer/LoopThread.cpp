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

#include "LoopThread.h"
#include "Buffer.h"
#include "Tracer.h"
#include <fcntl.h>
#ifdef WIN32
#else
	#include <unistd.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <signal.h>


extern Tracer_t			gtTracer;


void loopHandler (int sig)

{

	std:: cout << "LOOPHANDLER: Received " << sig << " signal" << std:: endl;

	#ifdef WIN32
	#else
		std:: cout << "Signal return: " << signal (SIGUSR1, loopHandler) << std:: endl;
	#endif


	return;
}


#ifdef WIN32
	LoopThread:: LoopThread (void): WinThread ()
#else
	LoopThread:: LoopThread (void): PosixThread ()
#endif

{

}


LoopThread:: ~LoopThread (void)

{

}


Error LoopThread:: init ()

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


	return errNoError;
}


Error LoopThread:: run (void)

{

	long				lIndex;
	unsigned long		ulThreadIdentifier;
	Buffer_t			bMessage;
	long			lRepeatIndex;
	long			lSleepsNumber;

	time_t			tStartTime;
	time_t			tEndTime;


	
	if (bMessage. init () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;
		
		return _erThreadReturn;
	}

	#ifdef WIN32
		WinThread:: getSleep (1, 0);
	#else
		PosixThread:: getSleep (1, 0);
	#endif
	// PosixThread:: getSleep (60 * 10, 0);

	#ifdef WIN32
		if ((_erThreadReturn = WinThread:: getThreadIdentifier (
			&ulThreadIdentifier)) != errNoError)
	#else
		if ((_erThreadReturn = PosixThread:: getThreadIdentifier (
			&ulThreadIdentifier)) != errNoError)
	#endif
	{
		// _erThreadReturn = PThreadErrors (__FILE__, __LINE__,
		// 	THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}


//	std:: cout << "Signal return: " << signal (SIGUSR1, loopHandler) << std:: endl;

	tStartTime			= time (NULL);

	lSleepsNumber		= 0;

	for (lRepeatIndex = 0; lRepeatIndex < 100; lRepeatIndex++)
	{
		for (lIndex = 0; lIndex < 1000; lIndex++)
		{
			/*
			#ifdef WIN32
				WinThread:: getSleep (1, 0);
			#else
				PosixThread:: getSleep (1, 0);
			#endif
			*/

			// if (bMessage. append ("1234567890") != errNoError)
			if (bMessage. setBuffer ("1234567890") != errNoError)
			{
				_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;
		
				return _erThreadReturn;
			}

			bMessage		+= lIndex;
			// std:: cout << lIndex << std:: endl;

			if (true)
			{
				gtTracer. trace (Tracer:: TRACER_LDBG1, (const char *) bMessage,
					__FILE__, __LINE__, ulThreadIdentifier);
			}

			/*
			if (lIndex % 10000 == 0)
			{
				lSleepsNumber++;
				// std:: cout << PThread:: getCurrentThreadIdentifier ()
				// 	<< " Sleeping" << std:: endl;
				#ifdef WIN32
					if (WinThread:: getSleep (1, 0) != errNoError)
				#else
					if (PosixThread:: getSleep (1, 0) != errNoError)
				#endif
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);
					std:: cout << (const char *) _erThreadReturn << std:: endl;

					return _erThreadReturn;
				}
			}
			*/
		}
	}

	tEndTime			= time (NULL);

	std:: cout << "Time elapsed: " << tEndTime - tStartTime << " sec." << std:: endl;
	std:: cout << "Totale sleep: " << lSleepsNumber * 5 << " sec." << std:: endl;
	std:: cout << "Time for the loop: "
		<< (tEndTime - tStartTime) - (lSleepsNumber * 5) << std:: endl;


	return _erThreadReturn;
}


Error LoopThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
