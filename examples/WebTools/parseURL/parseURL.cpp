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

#include "WebUtility.h"
#include <stdlib.h>
#include <time.h>
#include <iostream>

#define MAXIPADDRESSLENGTH			15 + 1


int main (int iArgc, char **pArgv)

{

	Error_t						errParseURL;
	char						*pURLToBeParsed;
	char						pHost [
		SCK_MAXHOSTNAMELENGTH];
	long						lPort;
	Buffer_t					bRelativePathWithoutParameters;
	Buffer_t					bURLParameters;


	if (iArgc != 2)
	{
		std:: cout << "Usage: " << pArgv [0]
			<< " URLToBeParsed"
			<< std:: endl
			<< "i.e.: rtsp://10.213.44.55:7777/aaa/bbb/ccc?aaa=bbb&ccc=ddd"
			<< std:: endl;


		return 1;
	}

	pURLToBeParsed					= pArgv [1];

	if (bRelativePathWithoutParameters. init () != errNoError ||
		bURLParameters. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if ((errParseURL = WebUtility:: parseURL (pURLToBeParsed,
		pHost, SCK_MAXHOSTNAMELENGTH, &lPort,
		&bRelativePathWithoutParameters, &bURLParameters)) != errNoError)
	{
		std:: cout << (const char *) errParseURL << std:: endl;
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_PARSEURL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bRelativePathWithoutParameters. finish () != errNoError ||
			bURLParameters. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	std:: cout << "URL to be parsed: " << pURLToBeParsed
		<< std:: endl << std:: endl;
	std:: cout << "Host: " << pHost << std:: endl;
	std:: cout << "Port: " << lPort << std:: endl;
	std:: cout << "Relative Path Without Parameters: " << (const char *) bRelativePathWithoutParameters << std:: endl;
	std:: cout << "URL Parameters: " << (const char *) bURLParameters << std:: endl;

	if (bRelativePathWithoutParameters. finish () != errNoError ||
		bURLParameters. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}


