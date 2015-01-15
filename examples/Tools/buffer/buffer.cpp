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

#include "Buffer.h"
#include <iostream>
#ifdef WIN32
	#include <time.h>
#endif

int main ()

{

	unsigned long	ulIndex;
	time_t			tStartTime;
	Buffer_t		bBuffer;
	Buffer_t		bCopyBuffer;
	Error			errReadBufferFromFile;
	Error			errStrip;


	/*
	bBuffer. init ();
	bBuffer. readBufferFromFile ("/app/2_ext3_10GB/giuliano/tmp/2009_09_16_15_55_38_0791_21Reality_0048.avi.xml");
	std:: cout << (const char *) bBuffer << std:: endl << " Length: " << (long) bBuffer << std:: endl;

	long lDirtyCharactersLength = strchr ((const char *) bBuffer, '<') -
		(const char *) bBuffer;

	std:: cout << std:: endl << "Dirty index: " << lDirtyCharactersLength
		<< std:: endl;

	if (lDirtyCharactersLength > 0)
		bBuffer. strip (Buffer:: STRIPTYPE_LEADING,
			lDirtyCharactersLength);

	std:: cout << (const char *) bBuffer << std:: endl << " Length: " << (long) bBuffer << std:: endl;

	bBuffer. writeBufferOnFile ("/app/2_ext3_10GB/giuliano/tmp/2009_09_16_15_55_38_0791_21Reality_0048.avi.xml");


	return 0;
	*/

	tStartTime			= time (NULL);

	for (ulIndex = 0; ulIndex < 1; ulIndex++)
	{

	if (bBuffer. init ("/12/12/aaa/12/12") != errNoError ||
		bCopyBuffer. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	// std:: cout << "Buffer: " << (const char *) bBuffer << std:: endl;
	// std:: cout << "Buffer length: " << (long) bBuffer << std:: endl;

	if ((errStrip = bBuffer. strip (Buffer:: STRIPTYPE_LEADING, "/12")) !=
		errNoError)
	{
		std:: cout << (const char *) errStrip << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_STRIP_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "Buffer: " << (const char *) bBuffer << std:: endl;
	// std:: cout << "Buffer length: " << (long) bBuffer << std:: endl;

/*
	if ((errReadBufferFromFile = bBuffer. readBufferFromFile (
		"./text.txt")) != errNoError)
	{
		cout << (const char *) errReadBufferFromFile << endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		cout << (const char *) err << endl;

		return 1;
	}

	cout << "Buffer: " << (const char *) bBuffer << endl;

	bCopyBuffer			= bBuffer;

	bBuffer				= "abc";

	cout << "Buffer: " << (const char *) bBuffer << endl;

	cout << "Copy Buffer: " << (const char *) bCopyBuffer << endl;

	bBuffer				+= bCopyBuffer;

	cout << "Buffer dopo +=: " << (const char *) bBuffer << endl;

	bBuffer				+= "abcdef";

	cout << "Buffer dopo +=: " << (const char *) bBuffer << endl;

	bBuffer				+= ((long) 300);

	bBuffer				= "123";
	bCopyBuffer			= "456";

	bBuffer				= bBuffer + bCopyBuffer;

	cout << "Buffer dopo +: " << (const char *) bBuffer << endl;

	bBuffer				= bBuffer + "789";

	cout << "Buffer dopo +: " << (const char *) bBuffer << endl;

	bBuffer				= "123";
	bCopyBuffer			= "123";

	if (bBuffer == bCopyBuffer)
		cout << "Buffer uguali" << endl;
	else
		cout << "Buffer non uguali" << endl;

	bBuffer				= "123";
	bCopyBuffer			= "456";

	if (bBuffer == bCopyBuffer)
		cout << "Buffer uguali" << endl;
	else
		cout << "Buffer non uguali" << endl;

	cout << "Buffer [0]:" << bBuffer [0] << endl;

	cout << "Buffer [1]:" << bBuffer [1] << endl;
	*/

	if (bBuffer. finish (true) != errNoError ||
		bCopyBuffer. finish (true) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	}

	std:: cout << "Elapsed: " << time (NULL) - tStartTime << std:: endl;


	return 0;
}
