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

#include "StringTokenizer.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


int main ()

{

	StringTokenizer_t			stStringTokenizer;
	Error						errRead;
	Error						errNextToken;
	const char					*pToken;
	long						lIndex;
	Buffer_t					bBuffer;


	if (bBuffer. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if ((errRead = bBuffer. readBufferFromFile ("./buff.txt")) != errNoError)
	{
		std:: cout << (const char *) errRead << std:: endl;
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bBuffer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	for (lIndex = 0; lIndex < 1; lIndex++)
	{
	if (stStringTokenizer. init ((char *) ((const char *) bBuffer), -1, "\n") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bBuffer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	do
	{
		if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
			errNoError)
		{
			if ((long) errNextToken == TOOLS_STRINGTOKENIZER_NOMORETOKEN)
				continue;
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);
				std:: cout << (const char *) err << std:: endl;

				if (stStringTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				if (bBuffer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				return 1;
			}
		}

		std:: cout << "Token: " << pToken << std:: endl << std:: endl;
	}
	while (errNextToken == errNoError);

	if (stStringTokenizer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (bBuffer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}
	#ifdef WIN32
		Sleep (1000 * 1);
	#else
		sleep (1);
	#endif
	}

	if (bBuffer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}
