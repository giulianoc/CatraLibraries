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

#include "HttpServer.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


int main (int iArgc, char **pArgv)

{

	HttpServer_t				thHttpServer;
	long						lServerPort;
	Error_t						errReturn;
	Error_t						errJoin;


	if (iArgc != 2)
	{
		std:: cout << "Usage: " << pArgv [0]
			<< " ServerPort"
			<< std:: endl
			<< "i.e.: 8081"
			<< std:: endl;


		return 1;
	}

	lServerPort						= atol (pArgv [1]);

	if (thHttpServer. init (
		(const char *) NULL,
		lServerPort, 5, 0, 10) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPSERVER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (thHttpServer. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (thHttpServer. finish () != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPSERVER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	if ((errReturn = thHttpServer. join (&errJoin)) != errNoError)
	{
		std:: cout << (const char *) errReturn << std:: endl;

		if (errJoin != errNoError)
		{
			std:: cout << (const char *) errJoin << std:: endl;

			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_JOIN_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		if (thHttpServer. finish () != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPSERVER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (errJoin != errNoError)
	{
		std:: cout << (const char *) errJoin << std:: endl;

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		std:: cout << (const char *) err << std:: endl;
	}

	if (thHttpServer. finish () != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPSERVER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}



	return 0;
}


