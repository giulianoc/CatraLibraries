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

#include "Convert.h"
#include <vector>
#include <stdexcept>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


Convert:: Convert (void)

{

}


Convert:: ~Convert (void)

{

}


Convert:: Convert (const Convert &)

{

	assert (1==0);

	// to do
}


Convert &Convert:: operator = (const Convert &)

{

	assert (1==0);

	// to do

	return *this;
}

string Convert::base64_encode(const string &in) 
{

    string out;

    int val=0, valb=-6;
    for (unsigned char c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

string Convert::base64_decode(const string &in) 
{

    string out;

    vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i; 

    int val=0, valb=-8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}

void Convert:: binaryToBase16 (
	const unsigned char *pucSrcBinaryData, unsigned long ulSrcBinaryDataSize,
	char *pDestBase16Data, unsigned long ulDestBase16DataSize)

{

	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;


	if (pucSrcBinaryData == (const unsigned char *) NULL ||
		pDestBase16Data == (char *) NULL ||
		ulSrcBinaryDataSize * 2 + 1 != ulDestBase16DataSize)
	{
            throw runtime_error(string("Invalid argument"));
	}

	memset ((void *) pDestBase16Data, 0, ulDestBase16DataSize);

	ulDestIndex				= 0;

	for (ulSrcIndex = 0; ulSrcIndex < ulSrcBinaryDataSize; ulSrcIndex++)
	{
		sprintf (&(pDestBase16Data [ulDestIndex]), "%02X",
			pucSrcBinaryData [ulSrcIndex]);
		ulDestIndex				+= 2;
	}
}


void Convert:: base16ToBinary (
	const char *pSrcBase16Data, unsigned long ulSrcBase16DataSize,
	unsigned char *pucDestBinaryData, unsigned long ulDestBinaryDataSize)

{

	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;
	char						pBuffer [5];


	if (pucDestBinaryData == (unsigned char *) NULL ||
		pSrcBase16Data == (const char *) NULL ||
		ulDestBinaryDataSize * 2 + 1 != ulSrcBase16DataSize)
	{
            throw runtime_error(string("Invalid argument"));
	}

	memset ((void *) pucDestBinaryData, 0, ulDestBinaryDataSize);

	ulDestIndex				= 0;

	pBuffer [2]			= '\0';
	for (ulSrcIndex = 0; ulSrcIndex < ulSrcBase16DataSize - 1; ulSrcIndex += 2)
	{
		pBuffer [0]			= pSrcBase16Data [ulSrcIndex];
		pBuffer [1]			= pSrcBase16Data [ulSrcIndex + 1];

		pucDestBinaryData [ulDestIndex]		=
			(unsigned char) strtol (pBuffer, (char **) NULL, 16);
		ulDestIndex				+= 1;
	}
}


void Convert:: stringToBase16 (const char *pSrcStringData,
	char *pDestBase16Data, unsigned long ulDestBase16DataSize)

{

	unsigned long				ulSrcStringDataSize;
	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;


	if (pSrcStringData == (const char *) NULL ||
		pDestBase16Data == (char *) NULL)
	{
            throw runtime_error(string("Invalid argument"));
	}

	ulSrcStringDataSize			= strlen (pSrcStringData);

	if (ulDestBase16DataSize < ulSrcStringDataSize * 2 + 1)
	{
            throw runtime_error(string("Invalid argument"));
	}

	memset ((void *) pDestBase16Data, 0, ulDestBase16DataSize);

	ulDestIndex				= 0;

	for (ulSrcIndex = 0; ulSrcIndex < ulSrcStringDataSize; ulSrcIndex++)
	{
		sprintf (&(pDestBase16Data [ulDestIndex]), "%02X",
			pSrcStringData [ulSrcIndex]);
		ulDestIndex				+= 2;
	}
}


void Convert:: base16ToString (const char *pSrcBase16Data, 
	char *pDestStringData, unsigned long ulDestStringDataSize)

{

	unsigned long				ulSrcBase16DataSize;
	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;
	char						pBuffer [5];


	if (pDestStringData == (char *) NULL ||
		pSrcBase16Data == (const char *) NULL)
	{
            throw runtime_error(string("Invalid argument"));
	}

	ulSrcBase16DataSize		= strlen (pSrcBase16Data);

	if (ulSrcBase16DataSize == 0 ||
		ulDestStringDataSize < ulSrcBase16DataSize / 2 + 1)
	{
            throw runtime_error(string("Invalid argument"));
	}

	memset ((void *) pDestStringData, 0, ulDestStringDataSize);

	ulDestIndex				= 0;

	pBuffer [2]			= '\0';
	for (ulSrcIndex = 0; ulSrcIndex < ulSrcBase16DataSize - 1; ulSrcIndex += 2)
	{
		pBuffer [0]			= pSrcBase16Data [ulSrcIndex];
		pBuffer [1]			= pSrcBase16Data [ulSrcIndex + 1];

		pDestStringData [ulDestIndex]		=
			(unsigned char) strtol (pBuffer, (char **) NULL, 16);
		ulDestIndex				+= 1;
	}
}

