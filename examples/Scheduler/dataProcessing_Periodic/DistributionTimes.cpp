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

#include "FileIO.h"
#include "DistributionTimes.h"
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <iostream>



DistributionTimes:: DistributionTimes (void): Times ()

{

}


DistributionTimes:: ~DistributionTimes (void)

{

}


/*
DistributionTimes:: DistributionTimes (const DistributionTimes &t)

{

	assert (1==0);

	*this = t;
}
*/


/*
Error DistributionTimes:: handleTimeOut (void)

{

	Error_t			errFileIO;
	int				iFileDescriptor;
	#ifdef WIN32
		__int64		llBytesWritten;
	#else
		long long	llBytesWritten;
	#endif


	std:: cout << "Time out for distribution: " << _pNextExpirationDateTime << std:: endl;
	return errNoError;

	#ifdef WIN32
		if ((errFileIO = FileIO:: open ("./processing.txt",
			O_CREAT | O_WRONLY | O_APPEND,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor)) != errNoError)
	#else
		if ((errFileIO = FileIO:: open ("./processing.txt",
			O_CREAT | O_WRONLY | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO,
			&iFileDescriptor)) != errNoError)
	#endif
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	if ((errFileIO = FileIO:: writeChars (iFileDescriptor,
		"Time out for distribution: ",
		strlen ("Time out for distribution: "),
		&llBytesWritten)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	if ((errFileIO = FileIO:: writeChars (iFileDescriptor,
		_pNextExpirationDateTime,
		strlen (_pNextExpirationDateTime),
		&llBytesWritten)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	if ((errFileIO = FileIO:: writeChars (iFileDescriptor,
		"\n", strlen ("\n"),
		&llBytesWritten)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	if ((errFileIO = FileIO:: close (iFileDescriptor)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}


	return errNoError;
}
*/

Error DistributionTimes:: handleTimeOut (void)

{

	Error_t			errFileIO;
	int				iFileDescriptor;
	#ifdef WIN32
		__int64		llBytesWritten;
	#else
		long long	llBytesWritten;
	#endif
	char			pBuffer [1024];


	#ifdef WIN32
		if ((errFileIO = FileIO:: open ("./distribution.txt",
			O_CREAT | O_WRONLY | O_APPEND,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor)) != errNoError)
	#else
		if ((errFileIO = FileIO:: open ("./distribution.txt",
			O_CREAT | O_WRONLY | O_APPEND,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IRWXO,
			&iFileDescriptor)) != errNoError)
	#endif
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	sprintf (pBuffer,
		"Time: %s --> %s, Daylight Saving Time: %ld --> %ld\n",
		_pCurrentExpirationDateTime, _pNextExpirationDateTime,
		_bCurrentDaylightSavingTime, _bNextDaylightSavingTime);

	if ((errFileIO = FileIO:: writeChars (iFileDescriptor,
		pBuffer, strlen (pBuffer), &llBytesWritten)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}

	if ((errFileIO = FileIO:: close (iFileDescriptor)) != errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		return errFileIO;
	}


	return errNoError;
}

