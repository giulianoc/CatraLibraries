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

#include "TracingThread.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#ifdef WIN32
#else
#endif
#include <iostream>
#include "Tracer.h"
#include "Buffer.h"


extern Tracer_t			gtTracer;


#ifdef WIN32
	TracingThread:: TracingThread (void): WinThread ()
#else
	TracingThread:: TracingThread (void): PosixThread ()
#endif

{

}


TracingThread:: ~TracingThread (void)

{

}


Error TracingThread:: init (void)

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


Error TracingThread:: run (void)

{

	long				lIndex;
	unsigned long		ulThreadIdentifier;
	Buffer_t			bMessage;

	
	if (bMessage. init () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		
		return _erThreadReturn;
	}

	#ifdef WIN32
		if (WinThread:: getThreadIdentifier (&ulThreadIdentifier) != errNoError)
	#else
		if (PosixThread:: getThreadIdentifier (&ulThreadIdentifier) != errNoError)
	#endif
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);

		return _erThreadReturn;
	}

	lIndex			= 0;

	while (lIndex < 1000000)
	{
		// "abcdefghil"    10 chars
		// if (bMessage. append ("abcdefghil") != errNoError)
		if (bMessage. setBuffer ("abcdefghil") != errNoError)
		{
			_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);
		
			return _erThreadReturn;
		}

		bMessage		+= lIndex;

		lIndex++;
		std:: cout << lIndex << std:: endl;

		gtTracer. trace (Tracer:: TRACER_LDBG1, (const char *) bMessage,
			__FILE__, __LINE__, ulThreadIdentifier);

		/*
		#ifdef WIN32
			if (WinThread:: getSleep (1, 10000) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 10000) != errNoError)
		#endif
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);

			return _erThreadReturn;
		}
		*/
	}


	return _erThreadReturn;
}


Error TracingThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
