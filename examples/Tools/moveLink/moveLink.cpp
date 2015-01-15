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


int main (int iArgc, char *pArgv [])

{

	const char					*pSrcPathName;
	const char					*pDestPathName;
	Error_t						errFileIO;


	if (iArgc != 3)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <src link> <dest link or dir>"
			<< std:: endl;

		return 1;
	}

	pSrcPathName				= pArgv [1];
	pDestPathName				= pArgv [2];


	if ((errFileIO = FileIO:: moveLink (pSrcPathName,
		pDestPathName)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_MOVELINK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

