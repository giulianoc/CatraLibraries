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

	long			lBufferCryptedLength;
	char			*pBufferCrypted;


	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <string to encrypt>"
			<< std:: endl;

		return 1;
	}

	std:: cout << "'" << pArgv [1] << "'" << endl;

	// for (long lIndex = 0; lIndex < 10000; lIndex++)
	{
	lBufferCryptedLength	= Encrypt:: getCryptedBufferLength (pArgv [1]);


	if ((pBufferCrypted = new char [lBufferCryptedLength + 1]) ==
		(char *) NULL)
	{
		std:: cerr << "new failed";

		return 1;
	}

	strcpy (pBufferCrypted, "");
	if (Encrypt:: encrypt (pArgv [1], pBufferCrypted,
		lBufferCryptedLength + 1) != 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ENCRYPT_ENCRYPT_FAILED);
		std:: cerr << (const char *) err;

		delete [] pBufferCrypted;

		return 1;
	}

	// for (int index = 0; index < lBufferCryptedLength; index++)
	// 	std:: cout << (int) (pBufferCrypted[index]) << std:: endl;

	std:: cout << pBufferCrypted << std:: endl;

	delete [] pBufferCrypted;
	}


	return 0;
}

