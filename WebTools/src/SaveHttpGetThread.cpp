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

#include "SaveHttpGetThread.h"
#include "WebUtility.h"
#include "DateTime.h"
#include "FileIO.h"
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>


SaveHttpGetThread:: SaveHttpGetThread (void): HttpGetThread ()

{

	_sStatus			= WEBTOOLS_SAVEHTTP_BUILDED;
}


SaveHttpGetThread:: ~SaveHttpGetThread (void)

{
	if (_sStatus == WEBTOOLS_SAVEHTTP_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_SAVEHTTPGETTHREAD_FINISH_FAILED);
		}
	}
}


Error SaveHttpGetThread:: init (
	Buffer_p pbDestinationPathName,
	Boolean_t bExtensionToBeAdded,
	const char *pWebServer,
	long lWebServerPort,
	const char *pURI,
	const char *pURLParameters,
	const char *pCookie,
	const char *pUserAgent,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingTimeoutInMicroSeconds,
	const char *pLocalIPAddress,
	const char *pProxyIpAddress,
	long lProxyPort)

{

	Error_t					errIO;


	if (_sStatus == WEBTOOLS_SAVEHTTP_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_SAVEHTTPGETTHREAD_FINISH_FAILED);
		}
	}

	if (!strcmp ((const char *) (*pbDestinationPathName), ""))
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	_bExtensionToBeAdded			= bExtensionToBeAdded;
	_iDestFileDescriptor			= -1;

	if (_bDestinationPathName. setBuffer (
		(const char *) (*pbDestinationPathName)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	if ((errIO = HttpGetThread:: init (
		pWebServer,
		lWebServerPort,
		pURI,
		pURLParameters,
		pCookie,
		pUserAgent,
		ulReceivingTimeoutInSeconds,
		ulReceivingTimeoutInMicroSeconds,
		ulSendingTimeoutInSeconds,
		ulSendingTimeoutInMicroSeconds,
		pLocalIPAddress,
		pProxyIpAddress,
		lProxyPort)) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPGETTHREAD_INIT_FAILED);

		return errIO;
	}

	_sStatus		= WEBTOOLS_SAVEHTTP_INITIALIZED;


	return errNoError;
}


Error SaveHttpGetThread:: finish (void)

{

	Error_t					errIO;


	if (_sStatus != WEBTOOLS_SAVEHTTP_INITIALIZED)
	{

		return errNoError;
	}

	if (HttpGetThread:: finish () != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);

		return err;
	}

	if (_bDestinationPathName. setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	_sStatus		= WEBTOOLS_SAVEHTTP_BUILDED;


	return errNoError;
}


Error_t SaveHttpGetThread:: chunkRead (unsigned long ulChunkReadIndex,
	long long llTotalContentLength, const char *pContentType,
	unsigned char *pucBuffer, unsigned long ulBufferDataSize)

{

	long long				llBytesWritten;
	Error_t					errIO;


	if (ulChunkReadIndex == 0)
	{
		if (_bExtensionToBeAdded)
		{
			unsigned long		ulPathNameLength;
			const char			*pPathName;


			ulPathNameLength		= (unsigned long) _bDestinationPathName;
			pPathName				= (const char *) _bDestinationPathName;

			if (pPathName [ulPathNameLength - 1] != '.')
			{
				if (_bDestinationPathName. append (".") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}

			if (strstr (pContentType, "image/x-png") != (char *) NULL ||
				strstr (pContentType, "image/png") != (char *) NULL
				)
			{
				if (_bDestinationPathName. append ("png") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else if (strstr (pContentType, "image/jpeg") != (char *) NULL)
			{
				if (_bDestinationPathName. append ("jpg") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else if (strstr (pContentType, "image/gif") != (char *) NULL)
			{
				if (_bDestinationPathName. append ("gif") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else if (strstr (pContentType, "video/3gpp") != (char *) NULL)
			{
				if (_bDestinationPathName. append ("3gp") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_SAVEHTTPGETTHREAD_UNKNOWNCONTENTTYPE,
					1, pContentType);

				return err;
			}
		}

		#ifdef WIN32
   			if ((errIO = FileIO:: open (
				(const char *) _bDestinationPathName,
				O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE,
				&_iDestFileDescriptor)) != errNoError)
		#else
   			if ((errIO = FileIO:: open (
				(const char *) _bDestinationPathName,
				O_WRONLY | O_TRUNC | O_CREAT,
				S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
				&_iDestFileDescriptor)) != errNoError)
		#endif
   		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED,
				1, (const char *) _bDestinationPathName);

			return errIO;
		}
	}

	if ((errIO = FileIO:: writeBytes (_iDestFileDescriptor,
		pucBuffer, ulBufferDataSize, &llBytesWritten)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITEBYTES_FAILED);

		return errIO;
	}


	return errNoError;
}


Error SaveHttpGetThread:: closingHttpGet (
	Error_p perr)

{

	Error_t				errIO;


	if (_iDestFileDescriptor != -1)
	{
		if ((errIO = FileIO:: close (_iDestFileDescriptor)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);

			return errIO;
		}

		_iDestFileDescriptor			= -1;
	}

	if (*perr != errNoError)
	{
		Boolean_t			bExist;


		if ((errIO = FileIO:: isFileExisting (
			(const char *) _bDestinationPathName, &bExist)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_ISFILEEXISTING_FAILED,
				1, (const char *) _bDestinationPathName);

			return err;
		}

		if (bExist)
		{
			if ((errIO = FileIO:: remove (
				(const char *) _bDestinationPathName)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_REMOVE_FAILED,
					1, (const char *) _bDestinationPathName);

				return err;
			}
		}
	}


	return errNoError;
}

