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

#include "MessageTest.h"
#include <time.h>
#include <iostream>


Message func1 (void)

{

	long	lParameter;
	float	fParameter;
	double	dParameter;
	char	cParameter;
	const char	*pParameter = "1234567890";

	lParameter	= 1234;
	fParameter	= 1234.5678;
	dParameter	= 9012.3456;
	cParameter	= 'A';

	Message msg = MessageTest (__FILE__, __LINE__,
		MESSAGETEST_MSG1, 6,
		lParameter, lParameter, cParameter, pParameter, fParameter, dParameter);


	return msg;
}


Message func2 (void)

{

	long		lInfo;

	Message msg = MessageTest (__FILE__, __LINE__,
		MESSAGETEST_MSG2);

	lInfo	= 24;

	if (msg. setUserData (&lInfo, sizeof (long)))
		std:: cerr << "setUserData failed" << std:: endl;


	return msg;
}


Message func3 (void)

{

	
	return msgNoMessage;
}


Message func4 (void)

{

	Message		msgMyNoMessage	= msgNoMessage;
	long		lInfo;

	lInfo		= 1024;

	if (msgMyNoMessage. setUserData (&lInfo, sizeof (long)))
		std:: cerr << "setUserData failed" << std:: endl;

	return msgMyNoMessage;
}



int main (void)

{

	Message		msgMyNoMessage	= msgNoMessage;
	Message		msgNotFound	= MessageTest (__FILE__, __LINE__, 30);
	Message		msgFunc1;
	Message		msgFunc2;
	Message		msgFunc4;
	long		lIndex;
	long		lInfo;
	unsigned long		ulUserDataBytes;
	time_t				tTimeStart;


	tTimeStart			= time (NULL);

	for (lIndex = 0; lIndex < 1000000; lIndex++)
	{
		msgFunc1 = func1 ();
		if (msgFunc1 == msgNoMessage)
			return 0;

		msgFunc2 = func2 ();
		msgFunc2. getUserData (&lInfo, &ulUserDataBytes);
		// std:: cout << "Info value: " << lInfo << std:: endl;
		if (msgFunc2 != msgNoMessage)
		{
		}

		if (func3 () != msgNoMessage)
		{
			return 1;
		}

		if ((msgFunc4 = func4 ()) != msgNoMessage)
		{
			return 1;
		}
		msgFunc4. getUserData (&lInfo, &ulUserDataBytes);
		/*
		std:: cout << "Info value: " << lInfo << std:: endl;
		std:: cout << std:: endl;


		std:: cout << "Message of msgMyNoMessage: " << (const char *) msgMyNoMessage
			<< std:: endl;
		std:: cout << std:: endl;
		std:: cout << (const char *) msgFunc1 << std:: endl;
		std:: cout << std:: endl;
		std:: cout << "Message of msgNotFound: " << (const char *) msgNotFound
			<< std:: endl;
		std:: cout << std:: endl;
		if (msgMyNoMessage. getClassName (pMessageClassName))
			std:: cout << "getClassName failed" << std:: endl;
		else
			std:: cout << "ClassName of MyNoMessage: " << pMessageClassName << std:: endl;
		std:: cout << std:: endl;

		if (msgFunc1. getClassName (pMessageClassName))
			std:: cout << "getClassName failed" << std:: endl;
		else
			std:: cout << "ClassName of the message: " << pMessageClassName << std:: endl;
		std:: cout << std:: endl;

		if (msgNotFound. getClassName (pMessageClassName))
			std:: cout << "getClassName failed" << std:: endl;
		else
			std:: cout << "ClassName of the msgNotFound: " << pMessageClassName
				<< std:: endl;
		std:: cout << std:: endl;

		std:: cout << "MessageIdentifier of MyNoMessage: " << (long) msgMyNoMessage
			<< std:: endl;
		std:: cout << std:: endl;

		std:: cout << "MessageIdentifier of msg: " << (long) msgFunc1 << std:: endl;
		std:: cout << std:: endl;

		if (msgNotFound. getIdentifier (&lMessageIdentifier))
			std:: cout << "getIdentifier failed" << std:: endl;
		else
			std:: cout << "MessageIdentifier of msgNotFound: " << lMessageIdentifier
				<< std:: endl;
		std:: cout << std:: endl;
		std:: cout << std:: endl;
		*/
	}

	std:: cout << "Seconds elapsed: " << (long) (time (NULL) - tTimeStart) << std:: endl;

	return 0;
}

