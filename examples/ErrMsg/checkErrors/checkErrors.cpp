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


#include "ErrorTest.h"
#include <time.h>
#include <iostream>


Error func1 (void)

{

	long	lParameter;
	float	fParameter;
	double	dParameter;
	char	cParameter;
	const char	*pParameter = "1234567890";
	unsigned long			ulParameter;
	#ifdef WIN32
		__int64				llParameter;
		__int64				ullParameter;
	#else
		long long				llParameter;
		unsigned long long		ullParameter;
	#endif

	lParameter	= 1234;
	fParameter	= 1234.5678;
	dParameter	= 9012.3456;
	cParameter	= 'A';
	ulParameter			= 123456789;
	llParameter			= -1234567890;
	ullParameter		= 1234567890;

	Error err = ErrorTest (__FILE__, __LINE__,
		ERRORTEST_FUNC1_FAILED, 9,
		lParameter, lParameter, cParameter, pParameter, fParameter, dParameter,
		ulParameter, llParameter, ullParameter);


	return err;
}


Error func2 (void)

{

	long		lReturnValue;

	Error err = ErrorTest (__FILE__, __LINE__,
		ERRORTEST_FUNC2_FAILED);

	lReturnValue	= 24;

	if (err. setUserData (&lReturnValue, sizeof (long)))
		std:: cerr << "setUserData failed" << std:: endl;


	return err;
}


Error func3 (void)

{


	return errNoError;
}


Error func4 (void)

{

	Error		errMyNoError	= errNoError;
	long		lReturnValue;

	lReturnValue	= 1024;

	if (errMyNoError. setUserData (&lReturnValue, sizeof (long)))
		std:: cerr << "setUserData failed" << std:: endl;


	return errMyNoError;
}


int main (void)

{

	Error		errMyNoError	= errNoError;
	// Error		errNotFound	= ErrorTest (__FILE__, __LINE__, 30);
	Error		errFunc1;
	Error		errFunc2;
	Error		errFunc4;
	char		pErrorClassName [1024 + 1];
	long		lIndex;
	long		lReturnValue;
	unsigned long		ulUserDataBytes;
	time_t				tTimeStart;


	tTimeStart			= time (NULL);

	for (lIndex = 0; lIndex < 1; lIndex++)
	{
		errFunc1 = func1 ();
		std:: cout << "errFunc1: " << (const char *) errFunc1 << std:: endl;
		if (errFunc1 == errNoError)
			return 0;

		errFunc2 = func2 ();
		// std:: cout << "AAAAAAA: " << (const char *) errFunc2 << std:: endl;
		errFunc2. getUserData (&lReturnValue, &ulUserDataBytes);
		std:: cout << "Return value: " << lReturnValue << std:: endl;
		// std:: cout << std:: endl;
		if (errFunc2 != errNoError)
		{
			Error err = ErrorTest (__FILE__, __LINE__,
				ERRORTEST_FUNC3_FAILED);
		}

		func3 ();

		if (func3 () != errNoError)
		{
			Error err = ErrorTest (__FILE__, __LINE__,
				ERRORTEST_FUNC3_FAILED);

			return 1;
		}

		if ((errFunc4 = func4 ()) != errNoError)
		{
			Error err = ErrorTest (__FILE__, __LINE__,
				ERRORTEST_FUNC4_FAILED);

			return 1;
		}
		errFunc4. getUserData (&lReturnValue, &ulUserDataBytes);
		/**/
		std:: cout << "Return value: " << lReturnValue << std:: endl;
		std:: cout << std:: endl;


		std:: cout << "Error message of errMyNoError: " << (const char *) errMyNoError
			<< std:: endl;
		std:: cout << std:: endl;
		std:: cout << "Error message of the error: " << (const char *) errFunc1
			<< std:: endl;
		std:: cout << std:: endl;

		// std:: cout << "Error message of errNotFound: " << (const char *) errNotFound
		//	<< std:: endl;
		//std:: cout << std:: endl;

		if (errMyNoError. getClassName (pErrorClassName))
			std:: cout << "getClassName failed" << std:: endl;
		else
			std:: cout << "ClassName of errMyNoError: " << pErrorClassName << std:: endl;
		std:: cout << std:: endl;

		if (errFunc1. getClassName (pErrorClassName))
			std:: cout << "getClassName failed" << std:: endl;
		else
			std:: cout << "ClassName of the error: " << pErrorClassName << std:: endl;
		std:: cout << std:: endl;

		//if (errNotFound. getClassName (pErrorClassName))
		//	std:: cout << "getClassName failed" << std:: endl;
		//else
		//	std:: cout << "ClassName of the errNotFound: " << pErrorClassName << std:: endl;
		//std:: cout << std:: endl;

		std:: cout << "ErrorIdentifier of errMyNoError: " << (long) errMyNoError
			<< std:: endl;
		std:: cout << std:: endl;

		std:: cout << "ErrorIdentifier of err: " << (long) errFunc1 << std:: endl;
		std:: cout << std:: endl;

		//if (errNotFound. getIdentifier (&lErrorIdentifier))
		//	std:: cout << "getIdentifier failed" << std:: endl;
		//else
		//	std:: cout << "ErrorIdentifier of errNotFound: " << lErrorIdentifier
		//		<< std:: endl;
		//std:: cout << std:: endl;
		/**/
	}

	std:: cout << "Seconds elapsed: " << (long) (time (NULL) - tTimeStart) << std:: endl;


	return 0;
}

