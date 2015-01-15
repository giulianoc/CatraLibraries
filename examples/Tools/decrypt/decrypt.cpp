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

#include <iostream>
#include <string.h>
#include "Encrypt.h"
#include "ToolsErrors.h"


int main (int iArgc, char **pArgv)

{

	char			*pDecryptedBuffer;
	long			lDecryptedBufferLength;


	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <string to decrypt>"
			<< std:: endl;

		return 1;
	}

	// for (long lIndex = 0; lIndex < 10000; lIndex++)
	{
	if ((lDecryptedBufferLength = Encrypt:: getDecryptedBufferLength (pArgv [1])) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ENCRYPT_GETDECRYPTEDBUFFERLENGTH_FAILED);
		std:: cerr << (const char *) err;

		return 1;
	}

	if ((pDecryptedBuffer = new char [lDecryptedBufferLength + 1]) ==
		(char *) NULL)
	{
		std:: cerr << "new failed";

		return 1;
	}

	strcpy (pDecryptedBuffer, "");
	if (Encrypt:: decrypt (pArgv [1], pDecryptedBuffer,
		lDecryptedBufferLength + 1) != 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ENCRYPT_DECRYPT_FAILED);
		std:: cerr << (const char *) err;

		delete [] pDecryptedBuffer;

		return 1;
	}

	std:: cout << pDecryptedBuffer << std:: endl;

	delete [] pDecryptedBuffer;
	}


	return 0;
}

