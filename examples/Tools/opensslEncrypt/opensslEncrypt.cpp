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

	int max = 100000000;
	for (long lIndex = 0; lIndex < max; lIndex++)
	{
	// A 256 bit key
	// unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
	unsigned char *key =   (unsigned char *) "r12Dd5678h012_45.7890*534D6l/9R1";

	// A 128 bit IV
	// unsigned char *iv = (unsigned char *)"0123456789012345";
	unsigned char *iv =   (unsigned char *) "0/2xd5678GA12*_5";

	// Message to be encrypted
	string plaintext = pArgv [1];

	string base64Encoded = Encrypt::opensslEncrypt(key, iv, plaintext);
	std::cout << lIndex << "/" << max << ". base64Encoded: " << base64Encoded << std::endl;
	}


	return 0;
}

