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
#include "Compressor.h"


int main (int iArgc, char **pArgv)

{

	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <string to compress>"
			<< std:: endl;

		return 1;
	}

	string toBeCompressed = pArgv [1];

	// for (long lIndex = 0; lIndex < 10000; lIndex++)
	{
		string compressed = Compressor::compress_string(toBeCompressed);
		std:: cout << "compressed: " << compressed << std:: endl;

		string unCompressed = Compressor::decompress_string(compressed);
		std:: cout << "unCompressed: " << unCompressed << std:: endl;
	}


	return 0;
}

