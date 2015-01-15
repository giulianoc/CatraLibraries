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

#include "CacheSaveHttpGetThread.h"
#include "HTTPCacheManagerMessages.h"
#include "FileIO.h"
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>


CacheSaveHttpGetThread:: CacheSaveHttpGetThread (void): SaveHttpGetThread ()

{

}


CacheSaveHttpGetThread:: ~CacheSaveHttpGetThread (void)

{
}


Error CacheSaveHttpGetThread:: init (
	unsigned long ulIdentifier,
	Tracer_p ptSystemTracer,
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

	_ulIdentifier				= ulIdentifier;
	_ptSystemTracer				= ptSystemTracer;
	_llBufferDataSaved			= 0;

	return SaveHttpGetThread:: init (
		pbDestinationPathName,
		bExtensionToBeAdded,
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
		lProxyPort);
}


Error_t CacheSaveHttpGetThread:: chunkRead (unsigned long ulChunkReadIndex,
	long long llTotalContentLength, const char *pContentType,
	unsigned char *pucBuffer, unsigned long ulBufferDataSize)

{
	Error_t				errChunkRead;
	

	if ((errChunkRead = SaveHttpGetThread::chunkRead (ulChunkReadIndex,
		llTotalContentLength, pContentType, pucBuffer, ulBufferDataSize)) !=
		errNoError)
	{
		return errChunkRead;
	}

	_llBufferDataSaved			+= ulBufferDataSize;

	if (_llBufferDataSaved == llTotalContentLength)
	{
		Buffer_t			bCompletedHTTPLocalPathName;


		{
			Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
				HCM_CACHESAVEHTTPGETTHREAD_HTTPREQUESTFINISHED,
				4,
				_ulIdentifier,
				_pWebServer,
				_lWebServerPort,
				_bHttpURI. str());
			_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}

		if (bCompletedHTTPLocalPathName. setBuffer (
			(const char *) _bDestinationPathName) != errNoError ||
			bCompletedHTTPLocalPathName. append (
			".complete") != errNoError
			)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}

		if (FileIO:: touchFile ((const char *) bCompletedHTTPLocalPathName) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_TOUCHFILE_FAILED);

			return err;
		}
	}


	return errNoError;
}

