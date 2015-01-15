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
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "CoreHandler.h"



#ifdef WIN32
	AThread:: AThread (void): WinThread ()
#else
	AThread:: AThread (void): PosixThread ()
#endif

{

}


AThread:: ~AThread (void)

{

}


Error AThread:: init (Boolean_t bCore)

{

	Error		errInit;

	#ifdef WIN32
		if ((errInit = WinThread:: init ()) != errNoError)
	#else
		if ((errInit = PosixThread:: init ()) != errNoError)
	#endif
	{
		std:: cout << (const char *) errInit << std:: endl;

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return err;
	}

	_bCore			= bCore;


	return errNoError;
}


Error AThread:: run (void)

{

	{
		unsigned long			ulThreadIdentifier;

		if (getThreadIdentifier (&ulThreadIdentifier) != errNoError)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		std:: cout << "Id: " << ulThreadIdentifier << std:: endl;

		std:: cout << "Id: " << (long) (*this) << std:: endl;
	}

//	signal(SIGSEGV, handler);
//	signal(SIGBUS, handler);

	#ifdef WIN32
		if (WinThread:: getSleep (_bCore ? 10 : 20, 0) != errNoError)
	#else
		if (PosixThread:: getSleep (_bCore ? 10 : 20, 0) != errNoError)
	#endif
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETSLEEP_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	std:: cout << "Core?????" << std:: endl;

	if (_bCore)
	{
		char			*ptr			= NULL;


		*ptr		= 'a';

	}

	#ifdef WIN32
		if (WinThread:: getSleep (10, 0) != errNoError)
	#else
		if (PosixThread:: getSleep (10, 0) != errNoError)
	#endif
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETSLEEP_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}


	return _erThreadReturn;
}


Error AThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
