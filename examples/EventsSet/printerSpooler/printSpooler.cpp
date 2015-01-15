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

#include "PrintRequester.h"
#include "PrinterSpooler.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


#define CONSUMERTHREADS		5


int main ()

{

	PrintRequester_t			thPrintRequester;
	PrinterSpooler_t			thPrinterSpooler [CONSUMERTHREADS];
	EventsSet_t					esEventsSet;
	Error_t						errJoin;
	unsigned long				ulThreadIndex;



	if (esEventsSet. init (true, 1) != errNoError)
	{
		std:: cout << "esEventsSet:: init failed" << std:: endl;

		return 1;
	}

	for (ulThreadIndex = 0; ulThreadIndex < CONSUMERTHREADS; ulThreadIndex++)
	{
		if ((thPrinterSpooler [ulThreadIndex]). init (&esEventsSet, ulThreadIndex) != errNoError)
		{
			std:: cout << "thPrinterSpooler:: init failed" << std:: endl;

			return 1;
		}
	}

	if (thPrintRequester. init (&esEventsSet) != errNoError)
	{
		std:: cout << "thPrintRequester:: init failed" << std:: endl;

		return 1;
	}

	for (ulThreadIndex = 0; ulThreadIndex < CONSUMERTHREADS; ulThreadIndex++)
	{
		if (thPrinterSpooler [ulThreadIndex]. start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (thPrintRequester. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thPrintRequester. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	for (ulThreadIndex = 0; ulThreadIndex < CONSUMERTHREADS; ulThreadIndex++)
	{
		if ((thPrinterSpooler [ulThreadIndex]). join (&errJoin) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_JOIN_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	for (ulThreadIndex = 0; ulThreadIndex < CONSUMERTHREADS; ulThreadIndex++)
	{
		if ((thPrinterSpooler [ulThreadIndex]). finish () != errNoError)
		{
			std:: cout << "thPrinterSpooler:: finish failed" << std:: endl;

			return 1;
		}
	}

	if (thPrintRequester. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (esEventsSet. finish () != errNoError)
	{
		std:: cout << "esEventsSet:: finish failed" << std:: endl;

		return 1;
	}


	return 0;
}

