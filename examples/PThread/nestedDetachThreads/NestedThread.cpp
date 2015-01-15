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

#include "NestedThread.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>


#ifdef WIN32
	NestedThread:: NestedThread (void): WinThread ()
#else
	NestedThread:: NestedThread (void): PosixThread ()
#endif

{

}


NestedThread:: ~NestedThread (void)

{

}


Error NestedThread:: init ()

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


Error NestedThread:: run (void)

{

	#ifdef WIN32
		if (WinThread:: getSleep (5, 0) != errNoError)
	#else
		if (PosixThread:: getSleep (5, 0) != errNoError)
	#endif
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETSLEEP_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}


	return _erThreadReturn;
}


Error NestedThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
