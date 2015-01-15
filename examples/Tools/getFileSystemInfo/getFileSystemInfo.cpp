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

	const char					*pPathName;
	unsigned long long			ullUsedInKB;
	unsigned long long			ullAvailableInKB;
	long						lPercentUsed;
	Error_t						errFileIO;


	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <pathname of any file within the mounted filesystem>"
			<< std:: endl;

		return 1;
	}

	pPathName			= pArgv [1];

	if ((errFileIO = FileIO:: getFileSystemInfo (pPathName,
		&ullUsedInKB, &ullAvailableInKB, &lPercentUsed)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_GETFILESYSTEMINFO_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "PathName: " << pPathName
		<< std:: endl << "UsedInKB: " << ullUsedInKB
		<< std:: endl << "AvailableInKB: " << ullAvailableInKB
		<< std:: endl << "PercentUsed: " << lPercentUsed << "%"
		<< std:: endl;


	return 0;
}

