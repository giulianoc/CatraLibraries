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
#include <stdlib.h>
#include <time.h>
#include <iostream>


int main ()

{

	DetachedThread_p		pthThread;
	long					lAttemptIndex;


	for (lAttemptIndex = 0; lAttemptIndex < 200; lAttemptIndex++)
	{
		if ((pthThread = new DetachedThread_t) == (DetachedThread_p) NULL)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_NEW_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pthThread -> init () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pthThread -> start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pthThread -> detach () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_DETACH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Sleep main thread (" << lAttemptIndex << ")"
			<< std:: endl;

		#ifdef WIN32
			if (WinThread:: getSleep (2, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (2, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	while (1)
	{
		std:: cout << "Sleep main thread" << std:: endl;

		#ifdef WIN32
			if (WinThread:: getSleep (10, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (10, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	return 0;
}


