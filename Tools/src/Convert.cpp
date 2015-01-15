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


Error Convert:: binaryToBase16 (
	const unsigned char *pucSrcBinaryData, unsigned long ulSrcBinaryDataSize,
	char *pDestBase16Data, unsigned long ulDestBase16DataSize)

{

	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;


	if (pucSrcBinaryData == (const unsigned char *) NULL ||
		pDestBase16Data == (char *) NULL ||
		ulSrcBinaryDataSize * 2 + 1 != ulDestBase16DataSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	memset ((void *) pDestBase16Data, 0, ulDestBase16DataSize);

	ulDestIndex				= 0;

	for (ulSrcIndex = 0; ulSrcIndex < ulSrcBinaryDataSize; ulSrcIndex++)
	{
		sprintf (&(pDestBase16Data [ulDestIndex]), "%02X",
			pucSrcBinaryData [ulSrcIndex]);
		ulDestIndex				+= 2;
	}


	return errNoError;
}


Error Convert:: base16ToBinary (
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
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
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


	return errNoError;
}


Error Convert:: stringToBase16 (const char *pSrcStringData,
	char *pDestBase16Data, unsigned long ulDestBase16DataSize)

{

	unsigned long				ulSrcStringDataSize;
	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;


	if (pSrcStringData == (const char *) NULL ||
		pDestBase16Data == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	ulSrcStringDataSize			= strlen (pSrcStringData);

	if (ulDestBase16DataSize < ulSrcStringDataSize * 2 + 1)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	memset ((void *) pDestBase16Data, 0, ulDestBase16DataSize);

	ulDestIndex				= 0;

	for (ulSrcIndex = 0; ulSrcIndex < ulSrcStringDataSize; ulSrcIndex++)
	{
		sprintf (&(pDestBase16Data [ulDestIndex]), "%02X",
			pSrcStringData [ulSrcIndex]);
		ulDestIndex				+= 2;
	}


	return errNoError;
}


Error Convert:: base16ToString (const char *pSrcBase16Data, 
	char *pDestStringData, unsigned long ulDestStringDataSize)

{

	unsigned long				ulSrcBase16DataSize;
	unsigned long				ulSrcIndex;
	unsigned long				ulDestIndex;
	char						pBuffer [5];


	if (pDestStringData == (char *) NULL ||
		pSrcBase16Data == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	ulSrcBase16DataSize		= strlen (pSrcBase16Data);

	if (ulSrcBase16DataSize == 0 ||
		ulDestStringDataSize < ulSrcBase16DataSize / 2 + 1)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
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


	return errNoError;
}

