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
#include <assert.h>
#include <iostream>


int main (int iArgc, char *pArgv [])

{

	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <file path name>"
			<< std:: endl;

		return 1;
	}

	FileReader		frFileReader;
	char			pLine [1024];
	#ifdef WIN32
		__int64		ullCharsRead;
	#else
		unsigned long long		ullCharsRead;
	#endif
	Error_t			errRead;


	if (frFileReader. init (pArgv [1], 1024) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_INIT_FAILED,
			1, pArgv [1]);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	while ((errRead = frFileReader. readLine (pLine, 1024,
		&ullCharsRead)) == errNoError)
	{
		std:: cout << "Line: " << pLine << std:: endl;
	}

	if ((long) errRead != TOOLS_FILEREADER_REACHEDENDOFFILE)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READLINE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		frFileReader. finish ();

		return 1;
	}

	if (frFileReader. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

