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

#include "DetachedThread.h"
#include "NestedThread.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>


#ifdef WIN32
	DetachedThread:: DetachedThread (void): WinThread ()
#else
	DetachedThread:: DetachedThread (void): PosixThread ()
#endif

{

}


DetachedThread:: ~DetachedThread (void)

{

}


Error DetachedThread:: init ()

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


Error DetachedThread:: run (void)

{

	NestedThread_p		pNestedThread [100];
	long				lAttemptIndex;
	long				lAttemptsNumber;
	#ifdef WIN32
	#else
		long				lStackSize;
	#endif

	
	lAttemptsNumber			= 30;

	for (lAttemptIndex = 0; lAttemptIndex < lAttemptsNumber; lAttemptIndex++)
	{
		std:: cout << "	NEW: " << lAttemptIndex << std:: endl;

		if ((pNestedThread [lAttemptIndex] = new NestedThread_t) ==
			(NestedThread_p) NULL)
		{
			std:: cout << "new failed" << std:: endl;
		}

		if ((pNestedThread [lAttemptIndex]) -> init () != errNoError)
		{
			std:: cout << "init failed" << std:: endl;
		}

		#ifdef WIN32
		#else
			lStackSize				= 61440 * 2;
			if ((pNestedThread [lAttemptIndex]) -> setStackSize (lStackSize) !=
				errNoError)
			{
				std:: cout << "setStackSize failed" << std:: endl;
			}
			std:: cout << "setStackSize: " << lStackSize << std:: endl;
		#endif

		if ((pNestedThread [lAttemptIndex]) -> start () != errNoError)
		{
			std:: cout << "start failed" << std:: endl;
		}

		#ifdef WIN32
		#else
			if ((pNestedThread [lAttemptIndex]) -> getStackSize (&lStackSize) !=
				errNoError)
			{
				std:: cout << "getStackSize failed" << std:: endl;
			}

			std:: cout << "getStackSize: " << lStackSize << std:: endl;
		#endif

		if ((pNestedThread [lAttemptIndex]) -> detach () != errNoError)
		{
			std:: cout << "start failed" << std:: endl;
		}
	}

/*
	for (lAttemptIndex = 0; lAttemptIndex < lAttemptsNumber; lAttemptIndex++)
	{
		std:: cout << "DELETE: " << lAttemptIndex << std:: endl;

		if ((pNestedThread [lAttemptIndex]) -> join () != errNoError)
		{
			std:: cout << "join failed" << std:: endl;
		}

		if ((pNestedThread [lAttemptIndex]) -> finish () != errNoError)
		{
			std:: cout << "finish failed" << std:: endl;
		}

		delete (pNestedThread [lAttemptIndex]);
	}
*/


	return _erThreadReturn;
}


Error DetachedThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}


