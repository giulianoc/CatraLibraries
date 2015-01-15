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

#include "ConditionVariableThread.h"
#include <iostream>
#include <stdlib.h>


int main (void)

{

	ConditionVariableThread_t		cvtConditionVariableThread;
	Boolean_t						bEnd;
	char							pInputBuffer [1024];


	if (cvtConditionVariableThread. init () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (cvtConditionVariableThread. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	#ifdef WIN32
	#else
		if (cvtConditionVariableThread. setCancel (
			PosixThread:: THREADLIB_CANCEL_ON) !=
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);

			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	#endif

	bEnd = false;
	while (!bEnd)
	{
		std:: cout << std:: endl << std:: endl;
		std:: cout << "1. sendSignal" << std:: endl;
		std:: cout << "2. cancel (Exit)" << std:: endl;

		std:: cin >> pInputBuffer;

		switch (atoi (pInputBuffer))
		{
			case 1:	
				if (cvtConditionVariableThread. sendSignal () != errNoError)
				{
					std:: cout << "sendSignal failed" << std:: endl;
				}

				break;
			case 2:
				if (cvtConditionVariableThread. cancel () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_CANCEL_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				bEnd = true;


				break;
		}
	}

	if (cvtConditionVariableThread. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

