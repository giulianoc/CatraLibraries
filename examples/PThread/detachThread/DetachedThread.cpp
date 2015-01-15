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

	// _pBuffer2			= new char [1024 * 10];

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		// delete [] _pBuffer2;

		return err;
	}


	return errNoError;
}


Error DetachedThread:: finish (void)

{

	Error_t				errFinish;


	std:: cout << "FINISH CALLED" << std:: endl;

	#ifdef WIN32
		if ((errFinish = WinThread:: finish ()) != errNoError)
	#else
		if ((errFinish = PosixThread:: finish ()) != errNoError)
	#endif
	{
		std:: cout << (const char *) errFinish << std:: endl;

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		// return err;
	}

	// delete [] _pBuffer2;


	return errNoError;
}


Error DetachedThread:: run (void)

{

	// char		*pBuffer;
	long		lAttemptsNumber;


	lAttemptsNumber		= 0;

	// while (lAttemptsNumber < 1000)
	while (lAttemptsNumber < 1)
	{
		/*
		if ((pBuffer = new char [1024]) == (char *) NULL)
		{
			std:: cout << "new failed" << std:: endl;
		}
		*/

		std:: cout << "Sleep detached thread" << std:: endl;

		#ifdef WIN32
			if (WinThread:: getSleep (0, 300000) != errNoError)
		#else
			if (PosixThread:: getSleep (0, 300000) != errNoError)
		#endif
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		// delete [] pBuffer;

		lAttemptsNumber++;
	}


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
