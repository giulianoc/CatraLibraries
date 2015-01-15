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


#include "FileReader.h"
#include "FileIO.h"
#ifndef WIN32
	#include <unistd.h>
	#include <sys/time.h>
#endif
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>



FileReader:: FileReader (void)

{

	_frFileReaderStatus		= TOOLS_FILEREADER_BUILDED;
}


FileReader:: ~FileReader (void)

{

	if (_frFileReaderStatus == TOOLS_FILEREADER_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
		}
	}

}



FileReader:: FileReader (const FileReader &)

{

	assert (1==0);

	// to do

}


FileReader &FileReader:: operator = (const FileReader &)

{

	assert (1==0);

	// to do

	return *this;

}


Error FileReader:: init (const char *pFilePath, unsigned long ulMaxCacheSize)

{

	long long				llCurrentPosition;
	Error						errOpen;


	if (pFilePath == (char *) NULL ||
		ulMaxCacheSize <= TOOLS_FILEREADER_MINIMUMCACHESIZE)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_frFileReaderStatus != TOOLS_FILEREADER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if ((_pFilePath = new char [strlen (pFilePath) + 1]) == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}
	strcpy (_pFilePath, pFilePath);

	_ulCacheSize			= ulMaxCacheSize;

	_ullCurrentFileOffset		= 0;
	_ullCacheFileOffset			= (unsigned long long) -1;

	#ifdef WIN32
		if ((errOpen = FileIO:: open (_pFilePath, O_RDONLY | O_BINARY,
			&_iFileDescriptor)) != errNoError)
	#else
		if ((errOpen = FileIO:: open (_pFilePath, O_RDONLY,
			&_iFileDescriptor)) != errNoError)
	#endif
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED);
		*/
		delete [] _pFilePath;
		_pFilePath			= (char *) NULL;


		// return err;
		return errOpen;
	}

	if (FileIO:: seek (_iFileDescriptor, 0,
		SEEK_END, &llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_SEEK_FAILED);

		if (FileIO:: close (_iFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
		}

		delete [] _pFilePath;
		_pFilePath			= (char *) NULL;


		return err;
	}

	_ullFileSize				= llCurrentPosition;

	if (_ulCacheSize > _ullFileSize)
		_ulCacheSize		= (unsigned long) _ullFileSize;

	if ((_pucCache = new unsigned char [(unsigned int) _ulCacheSize]) ==
		(unsigned char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		if (FileIO:: close (_iFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
		}

		delete [] _pFilePath;
		_pFilePath			= (char *) NULL;


		return err;
	}

	_frFileReaderStatus		= TOOLS_FILEREADER_INITIALIZED;


	return errNoError;
}


Error FileReader:: finish (void)

{

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		// Error err = ToolsErrors (__FILE__, __LINE__,
		// 	TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		// return err;
		return errNoError;
	}

	delete [] _pucCache;
	_pucCache			= (unsigned char *) NULL;

	if (FileIO:: close (_iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);
	}

	delete [] _pFilePath;
	_pFilePath			= (char *) NULL;

	_frFileReaderStatus		= TOOLS_FILEREADER_BUILDED;


	return errNoError;
}


FileReader:: operator unsigned long long (void) const

{

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		// Error err = ToolsErrors (__FILE__, __LINE__,
		// 	TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		// return err;
		return 0;
	}

	return _ullFileSize;
}


FileReader:: operator const char * (void) const

{
	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		// Error err = ToolsErrors (__FILE__, __LINE__,
		// 	TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		// return err;
		return (const char *) NULL;
	}

	return _pFilePath;
}


Error FileReader:: seek (long long llBytes, int iWhence,
	long long *pllCurrentPosition)

{

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullFileSize == 0)
	{
		*pllCurrentPosition			= 0;
	}
	else
	{
		switch (iWhence)
		{
			case SEEK_SET:
				if (llBytes <= 0)
				{
					_ullCurrentFileOffset		= 0;
				}
				else
				{
					if (llBytes >= _ullFileSize)
					{
						_ullCurrentFileOffset		= _ullFileSize;
					}
					else
					{
						_ullCurrentFileOffset		= llBytes;
					}
				}


				break;
			case SEEK_CUR:
				if (_ullCurrentFileOffset + llBytes >=
					_ullFileSize)
				{
					_ullCurrentFileOffset		= _ullFileSize;
				}
				else if (((long long) _ullCurrentFileOffset) + llBytes < 0)
				{
					_ullCurrentFileOffset		= 0;
				}
				else
				{
					_ullCurrentFileOffset		=
						_ullCurrentFileOffset + llBytes;
				}


				break;
			case SEEK_END:
				if (llBytes >= 0)
				{
					_ullCurrentFileOffset			= _ullFileSize;
				}
				else
				{
					if (llBytes * -1 >= _ullFileSize)
					{
						_ullCurrentFileOffset		= 0;
					}
					else
					{
						// llBytes is negative
						_ullCurrentFileOffset		= _ullFileSize + llBytes;
					}
				}


				break;
		}
	}

	*pllCurrentPosition			= _ullCurrentFileOffset;


	return errNoError;
}


Error FileReader:: seekBySearch (const unsigned char *pucBufferToSearch,
	unsigned long ulBufferToSearchLength, unsigned long ulBufferSizeToBeUsed)

{

	long long			llCurrentPosition;
	unsigned long long	ullCharsRead;
	unsigned char			*pucBuffer;
	Error_t					errRead;
	Boolean_t				bIsFound;
	unsigned long			ulBufferSizeIndex;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (seek (0, SEEK_SET, &llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_SEEK_FAILED);

		return err;
	}

	if ((pucBuffer = new unsigned char [ulBufferSizeToBeUsed]) ==
		(unsigned char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}

	bIsFound		= false;

	while ((errRead = readBytes (pucBuffer, ulBufferSizeToBeUsed,
		false, &ullCharsRead)) == errNoError)
	{
		for (ulBufferSizeIndex = 0; ulBufferSizeIndex < ullCharsRead;
			ulBufferSizeIndex++)
		{
			if (ullCharsRead - ulBufferSizeIndex >= ulBufferToSearchLength &&
				memcmp (pucBuffer + ulBufferSizeIndex, pucBufferToSearch,
				ulBufferToSearchLength) == 0)
			{
				bIsFound		= true;

				break;
			}
		}

		if (bIsFound)
			break;

		// that seek is to manage the case in which the buffer to search is
		// in the middle of two blocks
		if (_ullCurrentFileOffset < _ullFileSize && // the file is not finished
			_ullCurrentFileOffset >= ulBufferToSearchLength - 1)
		{
			if (seek (-1 * (ulBufferToSearchLength - 1), SEEK_CUR,
				&llCurrentPosition) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_SEEK_FAILED);

				delete [] pucBuffer;

				return err;
			}
		}
	}

	if (!bIsFound)
	{
		delete [] pucBuffer;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_BUFFERNOTFOUND);

		return err;
	}

	if (seek (-1 * (ullCharsRead - ulBufferSizeIndex),
		SEEK_CUR, &llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_SEEK_FAILED);

		delete [] pucBuffer;

		return err;
	}

	delete [] pucBuffer;


	return errNoError;
}


Error FileReader:: readChars (char *pBuffer,
	unsigned long long ullCharsNumber,
	Boolean_t bErrorIfAllBytesNotAvailable,
	unsigned long long *pullCharsRead)

{

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (bErrorIfAllBytesNotAvailable)
	{
		if (_ullCurrentFileOffset + ullCharsNumber > _ullFileSize)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		*pullCharsRead		= ullCharsNumber;
	}
	else
	{
		if (_ullCurrentFileOffset + ullCharsNumber > _ullFileSize)
		{
			if (_ullFileSize - _ullCurrentFileOffset <= 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_REACHEDENDOFFILE);

				return err;
			}

			*pullCharsRead			= _ullFileSize - _ullCurrentFileOffset;
		}
		else
		{
			*pullCharsRead			= ullCharsNumber;
		}
	}

	if ((*pullCharsRead) > _ulCacheSize)
	{
		if (readMoreThanCache ((unsigned char *) pBuffer, *pullCharsRead) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_READMORETHANCACHE_FAILED);

			return err;
		}
	}
	else
	{
		Error				errRefresh;

		if ((errRefresh = refreshCacheIfNecessary (*pullCharsRead)) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

			return errRefresh;
		}

		memcpy (pBuffer,
			_pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
			(size_t) (*pullCharsRead));
	}

	_ullCurrentFileOffset			+= (*pullCharsRead);


	return errNoError;
}


Error FileReader:: readLine (char *pBuffer,
	unsigned long long ullBufferLength,
	unsigned long long *pullCharsRead)

{

	Error_t						errRead;
	unsigned long long		ullLocalCharsRead;


	if (pBuffer == (char *) NULL ||
		ullBufferLength < 1 ||
		pullCharsRead == (unsigned long long *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	pBuffer [0]				= '\0';
	*pullCharsRead			= 0;

	while ((errRead = readChars (&(pBuffer [*pullCharsRead]), 1, true,
		&ullLocalCharsRead)) == errNoError)
	{
		if (pBuffer [*pullCharsRead] == '\n')
		{
			if (*pullCharsRead > 0 && pBuffer [(*pullCharsRead) - 1] == '\r')
			{
				pBuffer [(*pullCharsRead) - 1]	= '\0';

				(*pullCharsRead)		-= 1;
			}
			else
				pBuffer [*pullCharsRead]	= '\0';

			break;
		}

		(*pullCharsRead)		+= 1;

		if (*pullCharsRead >= ullBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_BUFFERTOOSMALL);

			(*pullCharsRead)			-= 1;

			pBuffer [*pullCharsRead]	= '\0';

			return err;
		}
	}

	if (errRead != errNoError)
	{
		if ((long) errRead == TOOLS_ACTIVATION_WRONG)
		{
			if (*pullCharsRead > 0)
			{
				// we reached the end of file but we read some chars. In this
				// scenario we will return NoError and next time this method
				// is called, the return will be REACHEDENDOFFILE

				pBuffer [*pullCharsRead]	= '\0';

				return errNoError;
			}
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_REACHEDENDOFFILE);

				pBuffer [*pullCharsRead]	= '\0';

				return err;
			}
		}
		else
			return errRead;
	}


	return errNoError;
}


Error FileReader:: readBytes (unsigned char *pucValue,
	unsigned long long ullBytesNumber,
	Boolean_t bErrorIfAllBytesNotAvailable,
	unsigned long long *pullBytesRead)

{

	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (bErrorIfAllBytesNotAvailable)
	{
		if (_ullCurrentFileOffset + ullBytesNumber > _ullFileSize)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		*pullBytesRead		= ullBytesNumber;
	}
	else
	{
		if (_ullCurrentFileOffset + ullBytesNumber > _ullFileSize)
		{
			if (_ullFileSize - _ullCurrentFileOffset <= 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_REACHEDENDOFFILE);

				return err;
			}

			*pullBytesRead			= _ullFileSize - _ullCurrentFileOffset;
		}
		else
		{
			*pullBytesRead		= ullBytesNumber;
		}
	}

	if ((*pullBytesRead) > _ulCacheSize)
	{
		if (readMoreThanCache (pucValue, *pullBytesRead) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_READMORETHANCACHE_FAILED);

			return err;
		}
	}
	else
	{
		Error		errRefresh;

		if ((errRefresh = refreshCacheIfNecessary (*pullBytesRead)) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

			return errRefresh;
		}

		memcpy (pucValue,
			_pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
			(size_t) (*pullBytesRead));
	}

	_ullCurrentFileOffset			+= (*pullBytesRead);


	return errNoError;
}


Error FileReader:: readNetUnsignedInt8Bit (unsigned long *pulValue)

{

	unsigned char					pucValue [1];
	Error							errRefresh;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullCurrentFileOffset + 1 > _ullFileSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errRefresh = refreshCacheIfNecessary (1)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

		return errRefresh;
	}

	memcpy (pucValue, _pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
		1);

	*pulValue			= (
		pucValue [0]);

	_ullCurrentFileOffset			+= 1;


	return errNoError;
}


Error FileReader:: readNetUnsignedInt16Bit (unsigned long *pulValue)

{

	unsigned char					pucValue [2];
	Error							errRefresh;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullCurrentFileOffset + 2 > _ullFileSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errRefresh = refreshCacheIfNecessary (2)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

		return errRefresh;
	}

	memcpy (pucValue, _pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
		2);

	*pulValue			= (
		(pucValue [0] << 8) |
		pucValue [1]);

	_ullCurrentFileOffset			+= 2;


	return errNoError;
}


Error FileReader:: readNetUnsignedInt24Bit (unsigned long *pulValue)

{

	unsigned char					pucValue [3];
	Error							errRefresh;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullCurrentFileOffset + 3 > _ullFileSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errRefresh = refreshCacheIfNecessary (3)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

		return errRefresh;
	}

	memcpy (pucValue, _pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
		3);

	*pulValue			= (
		(pucValue [0] << 16) |
		(pucValue [1] << 8) |
		pucValue [2]);

	_ullCurrentFileOffset			+= 3;


	return errNoError;
}


Error FileReader:: readNetUnsignedInt32Bit (unsigned long *pulValue)

{

	unsigned char					pucValue [4];
	Error							errRefresh;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullCurrentFileOffset + 4 > _ullFileSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errRefresh = refreshCacheIfNecessary (4)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

		return errRefresh;
	}

	memcpy (pucValue, _pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
		4);

	*pulValue			= (
		(pucValue [0] << 24) |
		(pucValue [1] << 16) |
		(pucValue [2] << 8) |
		pucValue [3]);

	_ullCurrentFileOffset			+= 4;


	return errNoError;
}

Error FileReader:: readNetUnsignedInt64Bit (unsigned long long *pullValue)
{

	unsigned char					pucValue [8];
	Error							errRefresh;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (_ullCurrentFileOffset + 8 > _ullFileSize)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errRefresh = refreshCacheIfNecessary (8)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED);

		return errRefresh;
	}

	memcpy (pucValue, _pucCache + (_ullCurrentFileOffset - _ullCacheFileOffset),
		8);

	*pullValue			= (
		(pucValue [0] << 56) |
		(pucValue [1] << 48) |
		(pucValue [2] << 40) |
		(pucValue [3] << 32) |
		(pucValue [4] << 24) |
		(pucValue [5] << 16) |
		(pucValue [6] << 8) |
		pucValue [7]);

	_ullCurrentFileOffset			+= 8;


	return errNoError;
}


Error FileReader:: readNetFloat16Bit (float *pfValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (readNetUnsignedInt8Bit (&ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READNETUNSIGNEDINT8BIT_FAILED);

		return err;
	}

	if (readNetUnsignedInt8Bit (&ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READNETUNSIGNEDINT8BIT_FAILED);

		return err;
	}

	*pfValue			= (ulIntegerPart + (((float) ulFloatPart) / 0x100));


	return errNoError;
}


Error FileReader:: readNetFloat32Bit (float *pfValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	if (readNetUnsignedInt16Bit (&ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READNETUNSIGNEDINT16BIT_FAILED);

		return err;
	}

	if (readNetUnsignedInt16Bit (&ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEREADER_READNETUNSIGNEDINT16BIT_FAILED);

		return err;
	}

	*pfValue			= (ulIntegerPart + (((float) ulFloatPart) / 0x10000));


	return errNoError;
}


Error FileReader:: readMP4DescriptorSize (unsigned long *pulSize,
	unsigned char *pucNumBytes)

{

	unsigned long			ulTmp;
	unsigned char			ucTmp;


	if (_frFileReaderStatus != TOOLS_FILEREADER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _frFileReaderStatus);

		return err;
	}

	*pulSize			= 0;
	*pucNumBytes		= 0;

	do
	{
		if (readNetUnsignedInt8Bit (&ulTmp) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_READNETUNSIGNEDINT8BIT_FAILED);

			return err;
		}
		ucTmp				= (unsigned char) ulTmp;

		*pulSize			= (*pulSize << 7) | (ucTmp & 0x7F);

		(*pucNumBytes)++;
	}
	while ((ucTmp & 0x80) && *pucNumBytes < 4);


	return errNoError;
}


Error FileReader:: refreshCacheIfNecessary (
	unsigned long long ullBytesToRead)

{

	// _ullCurrentFileOffset could be in any place due to a seek previous call.
	//	The conditions below are:
	//		- buffer no initialized
	//		- current file offset before our cache
	//		- current file offset after our cache
	//		- current file offset inside our cache
	if (_ullCacheFileOffset == (unsigned long long) -1 ||
		_ullCurrentFileOffset < _ullCacheFileOffset ||
		_ullCurrentFileOffset > _ullCacheFileOffset + _ulCacheSize ||
		_ullCurrentFileOffset - _ullCacheFileOffset + ullBytesToRead > _ulCacheSize)
	{
		long long				llCurrentPosition;
		unsigned long long		ullBytesToReadForCache;
		long long				llBytesRead;


		if (ullBytesToRead > _ulCacheSize)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_CACHESIZETOOLOW,
				1, _ulCacheSize);

			return err;
		}

		if (FileIO:: seek (_iFileDescriptor, _ullCurrentFileOffset,
			SEEK_SET, &llCurrentPosition) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_SEEK_FAILED);

			return err;
		}

		if (_ullCacheFileOffset == (unsigned long long) -1)
		{
			if (_ullFileSize > _ulCacheSize)
				ullBytesToReadForCache			= _ulCacheSize;
			else
				ullBytesToReadForCache			= _ullFileSize;
		}
		else if (_ullCurrentFileOffset < _ullCacheFileOffset)
			ullBytesToReadForCache			= _ulCacheSize;
		else if (_ullCurrentFileOffset > _ullCacheFileOffset + _ulCacheSize)
		{
			if (_ullFileSize - _ullCurrentFileOffset > _ulCacheSize)
				ullBytesToReadForCache			= _ulCacheSize;
			else
				ullBytesToReadForCache			= _ullFileSize - _ullCurrentFileOffset;
		}
		else
		{
			if (_ullFileSize - _ullCurrentFileOffset > _ulCacheSize)
				ullBytesToReadForCache			= _ulCacheSize;
			else
				ullBytesToReadForCache			= _ullFileSize - _ullCurrentFileOffset;
		}

		_ullCacheFileOffset			= _ullCurrentFileOffset;

		if (FileIO:: readBytes (_iFileDescriptor, _pucCache,
			ullBytesToReadForCache, &llBytesRead) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READBYTES_FAILED);

			return err;
		}
	}



	return errNoError;
}


Error FileReader:: readMoreThanCache (
	unsigned char *pucBuffer,
	unsigned long long ullBytesToRead)

{

	long long				llCurrentPosition;
	long long				llBytesRead;


	if (FileIO:: seek (_iFileDescriptor, _ullCurrentFileOffset,
		SEEK_SET, &llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_SEEK_FAILED);

		return err;
	}

	if (FileIO:: readBytes (_iFileDescriptor, pucBuffer,
		ullBytesToRead, &llBytesRead) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READBYTES_FAILED);

		return err;
	}


	return errNoError;
}

