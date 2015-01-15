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

#include "HttpGetThread.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>


int main (int iArgc, char **pArgv)

{

	char						*pWebServerIpAddress;
	char						*pRelativePathWithoutParameters;
	long						lWebServerPort;
	char						*pURLParameters;
	char						*pCookie;
	HttpGetThread_p				pthHttpGetThread;
	Buffer_t					bHttpGetHeaderResponse;
	Buffer_t					bHttpGetBodyResponse;
	// Buffer_t					bHttpCookieHeaderResponse;
	long						lIndex;
	Error_t						errReturn;
	Error_t						errRun;


	if (iArgc != 6)
	{
		std:: cout << "Usage: " << pArgv [0]
			<< " WebServerIpAddress WebServerPort RelativePathWithoutParameters URLParameters Cookie"
			<< std:: endl
			<< "i.e.: 10.214.138.5 8081 /index.html \"\" \"\""
			<< std:: endl;


		return 1;
	}

	pWebServerIpAddress					= pArgv [1];
	lWebServerPort						= atol (pArgv [2]);
	pRelativePathWithoutParameters		= pArgv [3];
	pURLParameters						= pArgv [4];
	pCookie								= pArgv [5];

	for (lIndex = 0; lIndex < 1; lIndex++)
	{
		std:: cout << "Index: " << lIndex << std:: endl;

	if ((pthHttpGetThread = new HttpGetThread_t) == (HttpGetThread_p) NULL)
	{
		std:: cout << "thread new failed" << std:: endl;

		return 1;
	}

	if (pthHttpGetThread -> init (
		pWebServerIpAddress,
		lWebServerPort,
		pRelativePathWithoutParameters,
		pURLParameters, pCookie) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPGETTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		delete pthHttpGetThread;

		return 1;
	}

#ifdef OPZIONE_1
	if (pthHttpGetThread -> start (true) != errNoError)
	// if (pthHttpGetThread -> start (false) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	#ifdef WIN32
		WinThread:: getSleep (0, 500000);
	#else
		PosixThread:: getSleep (0, 500000);
	#endif
#else

	if (bHttpGetHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	if (bHttpGetBodyResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	if ((errRun = pthHttpGetThread -> run (
		&bHttpGetHeaderResponse, &bHttpGetBodyResponse,
		(Buffer_p) NULL, (Buffer_p) NULL, (Buffer_p) NULL)) != errNoError)
	{
		std:: cout << (const char *) errRun << std:: endl;

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_RUN_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bHttpGetBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		if (bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	std:: cout << (const char *) bHttpGetHeaderResponse
		<< std:: endl << std:: endl;
	std:: cout << (const char *) bHttpGetBodyResponse;
	// std:: cout << (long) bHttpGetResponse << std:: endl;

	// if (bHttpGetResponse. writeBufferOnFile ("./buff.txt") != errNoError)
	//{
	//	Error err = ToolsErrors (__FILE__, __LINE__,
	//		TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED);
	//	std:: cout << (const char *) err << std:: endl;

//		if (pthHttpGetThread -> finish () != errNoError)
//		{
//			Error err = PThreadErrors (__FILE__, __LINE__,
//				THREADLIB_PTHREAD_FINISH_FAILED);
//			std:: cout << (const char *) err << std:: endl;
//		}

//		delete pthHttpGetThread;

//		return 1;
//	}

	if (bHttpGetBodyResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	if (bHttpGetHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (pthHttpGetThread -> finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		delete pthHttpGetThread;

		return 1;
	}

	if (pthHttpGetThread -> finish () != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPGETTHREAD_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	delete pthHttpGetThread;
#endif
	}

#ifdef OPZIONE_1
	while (1)
	{
		std:: cout << "FINISHED" << std:: endl;
		#ifdef WIN32
			WinThread:: getSleep (60, 0);
		#else
			PosixThread:: getSleep (60, 0);
		#endif
	}
#endif

	return 0;
}


