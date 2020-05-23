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
#include <iomanip>
#include <fstream>
#include <sstream>
// md5 function seems to be defined in openssl/md5.h and implemented in crypto lib
#include <openssl/md5.h>

#define BUFFSIZE 16384

int main (int iArgc, char *pArgv [])

{

	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <path file name to digest>"
			<< std:: endl;

		return 1;
	}

	std::string pathFileName(pArgv [1]);

	char buffer[BUFFSIZE];
	unsigned char digest[MD5_DIGEST_LENGTH];
	std::stringstream ss;

	std::ifstream ifs(pathFileName, std::ifstream::binary);
	MD5_CTX md5Context;
	MD5_Init(&md5Context);
	while (ifs.good()) 
	{
		ifs.read(buffer, BUFFSIZE);
		MD5_Update(&md5Context, buffer, ifs.gcount()); 
	}
	ifs.close();
	int res = MD5_Final(digest, &md5Context);

	if( res == 0 ) // hash failed
	{
		std::cout << "MD5 failed" << std::endl;

		return 1;
	}

	// set up stringstream format
	ss << std::hex << std::uppercase << std::setfill('0');
	for(unsigned char uc: digest)
		ss << std::setw(2) << (int)uc;

	std::cout << pathFileName << " bin ---> " << digest << std::endl;
	std::cout << pathFileName << " hex ---> " << ss.str() << std::endl;

	return 0;
}

