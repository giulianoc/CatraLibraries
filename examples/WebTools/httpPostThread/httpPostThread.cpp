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

#include "HttpPostThread.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


int main (int iArgc, char **pArgv)

{

	char						*pWebServerIpAddress;
	char						*pRelativePathWithoutParameters;
	long						lWebServerPort;
	char						*pURLParameters;
	char						*pBodyPathName;
	HttpPostThread_p				pthHttpPostThread;
	Buffer_t					bHttpPostHeaderResponse;
	Buffer_t					bHttpPostBodyResponse;
	long						lIndex;
	Error_t						errReturn;
	Error_t						errJoin;
	Buffer_t					bBodyForThePost;


	if (iArgc != 6)
	{
		std:: cout << "Usage: " << pArgv [0]
			<< " WebServerIpAddress WebServerPort RelativePathWithoutParameters URLParameters BodyPathName"
			<< std:: endl
			<< "i.e.: 10.214.138.5 8081 /index.html \"\""
			<< std:: endl;


		return 1;
	}

	pWebServerIpAddress					= pArgv [1];
	lWebServerPort						= atol (pArgv [2]);
	pRelativePathWithoutParameters		= pArgv [3];
	pURLParameters						= pArgv [4];
	pBodyPathName						= pArgv [5];

	// for (lIndex = 0; lIndex < 10000; lIndex++)
	{
	//	std:: cout << "Index: " << lIndex << std:: endl;

	if (bBodyForThePost. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bBodyForThePost. readBufferFromFile (pBodyPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bHttpPostHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bHttpPostBodyResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if ((pthHttpPostThread = new HttpPostThread_t) == (HttpPostThread_p) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pthHttpPostThread -> init (
		pWebServerIpAddress,
		lWebServerPort,
		pRelativePathWithoutParameters,
		pURLParameters,
		(const char *) bBodyForThePost) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPPOSTTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pthHttpPostThread -> run (
		&bHttpPostHeaderResponse, &bHttpPostBodyResponse,
		(Buffer_p) NULL) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_RUN_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << (const char *) bHttpPostHeaderResponse
		<< std:: endl << std:: endl;
	std:: cout << (const char *) bHttpPostBodyResponse;

	if (bHttpPostBodyResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bHttpPostHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pthHttpPostThread -> finish () != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPGETTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	delete pthHttpPostThread;
	}

	return 0;
}


