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
#include <iostream>


int main (int iArgc, char *pArgv [])

{

	Boolean_t					bIsExist;
	Error_t						errFileIO;


	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <path file name>"
			<< std:: endl;

		return 1;
	}

	long maxMillisecondsToWait = 20000;
	long milliSecondsWaitingBetweenChecks = 50;

	if ((errFileIO = FileIO:: isFileExisting (pArgv [1], &bIsExist,
					maxMillisecondsToWait, milliSecondsWaitingBetweenChecks)) !=
		errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (bIsExist)
		std:: cerr << "The file does exist" << std:: endl;
	else
		std:: cerr << "The file does not exist" << std:: endl;


	return 0;
}

