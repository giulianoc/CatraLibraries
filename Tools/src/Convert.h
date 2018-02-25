
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


#ifndef Convert_h
#define Convert_h

#include <string>

using namespace std;

typedef class Convert {

private:
    Convert (const Convert &);

    Convert &operator = (const Convert &);

public:
    Convert ();

    ~Convert ();

    static string base64_encode(const string &in);

    static string base64_decode(const string &in);

    /**
            pDestBase16Data must be already allocated:
            pDestBase16Data = new unsigned char [ulSrcBinaryDataSize * 2 + 1]
            ulSrcBinaryDataSize does not include '\0'
            ulDestBase16DataSize includes the '\0'
    */
    static void binaryToBase16 (
            const unsigned char *pucSrcBinaryData,
            unsigned long ulSrcBinaryDataSize,
            char *pDestBase16Data, unsigned long ulDestBase16DataSize);

    /**
            pucDestBinaryData must be already allocated:
            pucDestBinaryData = new unsigned char [ulSrcBase16DataSize / 2]
            ulSrcBase16DataSize includes the '\0'
            ulDestBinaryDataSize does not include '\0'
    */
    static void base16ToBinary (
            const char *pSrcBase16Data, unsigned long ulSrcBase16DataSize,
            unsigned char *pucDestBinaryData,
            unsigned long ulDestBinaryDataSize);

    /**
            pDestBase16Data must be already allocated:
            the size of pDestBase16Data must be at least
                    (SizeOfSrc * 2 + 1)
    */
    static void stringToBase16 (const char *pSrcStringData,
            char *pDestBase16Data, unsigned long ulDestBase16DataSize);

    /**
            pDestStringData must be already allocated:
            the size of pDestStringData must be at least
                    (SizeOfSrc / 2 + 1) characters
    */
    static void base16ToString (const char *pSrcBase16Data,
            char *pDestStringData, unsigned long ulDestStringDataSize);

} Convert_t, *Convert_p;

#endif

