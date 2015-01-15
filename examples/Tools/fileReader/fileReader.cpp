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

#include "FileReader.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


int main (int iArgc, char *pArgv [])

{

	const char					*pSrcPathName;
	const char					*pBufferToSearch;
	FileReader_t				frFileReader;
	Error_t						errFileReader;
	unsigned long				ulBufferSize = 1024 * 10;
	char						pLine [1024];
	unsigned long long			ullCharsRead;
	long long					llCurrentPosition;


	if (iArgc != 3)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <src file> <buffer to search>"
			<< std:: endl;

		return 1;
	}

	pSrcPathName				= pArgv [1];
	pBufferToSearch				= pArgv [2];

	if (frFileReader. init (pSrcPathName, ulBufferSize) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_INIT_FAILED,
			1, pSrcPathName);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	while ((errFileReader = frFileReader. readLine (pLine, 1024,
		&ullCharsRead)) == errNoError)
	{
		std:: cout << pLine << std:: endl;
	}

	std:: cout << "File finished to read" << std:: endl;

	if ((long) errFileReader != TOOLS_FILEREADER_REACHEDENDOFFILE)
	{
		std:: cerr << (const char *) errFileReader << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READLINE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (frFileReader. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	std:: cout << "Searching " << pBufferToSearch << " ..." << std:: endl;

	if ((errFileReader = frFileReader. seekBySearch (
		(unsigned char *) pBufferToSearch,
		strlen (pBufferToSearch)/*, ulBufferSize */)) != errNoError)
	{
		std:: cerr << (const char *) errFileReader << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_SEEKBYSEARCH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (frFileReader. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	std:: cout << pBufferToSearch << " was found" << std:: endl;

	if (frFileReader. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

