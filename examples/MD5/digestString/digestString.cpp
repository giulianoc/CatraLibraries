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
// #include <openssl/md5.h>
#include <openssl/evp.h>



int main (int iArgc, char *pArgv [])

{

	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <string to digest>"
			<< std:: endl;

		return 1;
	}

	std::string str(pArgv [1]);

	// unsigned char digest[MD5_DIGEST_LENGTH];
	// MD5((unsigned char*)str.c_str(), str.size(), digest);

	unsigned char *md5_digest;
	unsigned int md5_digest_len = EVP_MD_size(EVP_md5());


	EVP_MD_CTX *mdctx;

	// MD5_Init
	mdctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mdctx, EVP_md5(), NULL);

	// MD5_Update
	EVP_DigestUpdate(mdctx, (unsigned char*) str.c_str(), str.size());

	// MD5_Final
	md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_len);
	EVP_DigestFinal_ex(mdctx, md5_digest, &md5_digest_len);

	std::ostringstream sout;
	sout<<std::hex<<std::setfill('0');
	for(int i = 0; i < md5_digest_len; i++)
	{
		long long c = md5_digest[i];

		sout<<std::setw(2)<<(long long)c;
	}

// linux command in case for example of "apple": printf "%s" "apple" | md5sum
	std::cout << str << " hex ---> " << sout.str() << std::endl;
	std::cout << str << " bin ---> " << md5_digest << std::endl;

	OPENSSL_free(md5_digest);
	EVP_MD_CTX_free(mdctx);

	return 0;
}

