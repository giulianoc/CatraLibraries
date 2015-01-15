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

#include "FileIO.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <stdlib.h>


int main (int iArgc, char *pArgv [])

{

	const char					*pSrcPathName;
	const char					*pDestPathName;
	Boolean_t					bIsFileExisting;
	unsigned long				ulBufferSizeToBeUsed;


	if (iArgc != 3 && iArgc != 4)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <src file> <dest file> [<BufferSizeToBeUsed>]"
			<< std:: endl;

		return 1;
	}

	pSrcPathName				= pArgv [1];
	pDestPathName				= pArgv [2];
	if (iArgc == 4)
		ulBufferSizeToBeUsed		= atol (pArgv [3]);
	else
		ulBufferSizeToBeUsed		= 0;

	if (FileIO:: isFileExisting (pSrcPathName, &bIsFileExisting) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (!bIsFileExisting)
	{
		std:: cerr << "The " << pSrcPathName << " does not exist." << std:: endl;

		return 1;
	}

	if (FileIO:: copyFile (pSrcPathName, pDestPathName,
		ulBufferSizeToBeUsed) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_COPYFILE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

