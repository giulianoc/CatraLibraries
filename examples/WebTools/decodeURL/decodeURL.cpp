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

	Error_t						errDecodeURL;
	char						*pURLToBeDecoded;
	char						pURLDecoded [1024];



	if (iArgc != 2)
	{
		std:: cout << "Usage: " << pArgv [0]
			<< " URLToBeDecoded"
			<< std:: endl
			<< "i.e.: rtsp://10.213.44.55:7777/aaa/bbb/ccc?aaa=bbb&ccc=ddd"
			<< std:: endl;


		return 1;
	}

	pURLToBeDecoded					= pArgv [1];

	if ((errDecodeURL = WebUtility:: decodeURL (pURLToBeDecoded,
		pURLDecoded, 1024)) != errNoError)
	{
		std:: cout << (const char *) errDecodeURL << std:: endl;

		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_DECODEURL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "URL to be decoded: '" << pURLToBeDecoded
		<< "'" << std:: endl << std:: endl;
	std:: cout << "URL decoded: '" << pURLDecoded
		<< "'" << std:: endl << std:: endl;


	return 0;
}

