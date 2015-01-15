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

#ifndef CacheSaveHttpGetThread_h
	#define CacheSaveHttpGetThread_h

	#include "SaveHttpGetThread.h"
	#include "Tracer.h"


	typedef class CacheSaveHttpGetThread: public SaveHttpGetThread {

		private:
			long long			_llBufferDataSaved;
			unsigned long		_ulIdentifier;
			Tracer_p			_ptSystemTracer;


		public:
			CacheSaveHttpGetThread (void);

			~CacheSaveHttpGetThread (void);
 
			Error init (
				unsigned long ulIdentifier,
				Tracer_p ptSystemTracer,
				Buffer_p pbDestinationPathName,
				Boolean_t bExtensionToBeAdded,
				const char *pWebServer,
				long lWebServerPort,
				const char *pURI,
				const char *pURLParameters = (const char *) NULL,
				const char *pCookie = (const char *) NULL,
				const char *pUserAgent = (const char *) NULL,
				unsigned long ulReceivingTimeoutInSeconds = 15,
				unsigned long ulReceivingTimeoutInMicroSeconds = 0,
				unsigned long ulSendingTimeoutInSeconds = 15,
				unsigned long ulSendingTimeoutInMicroSeconds = 0,
				const char *pLocalIPAddress = (const char *) NULL,
				const char *pProxyIpAddress = (const char *) NULL,
				long lProxyPort = 8888);

			virtual Error_t chunkRead (unsigned long ulChunkReadIndex,
				long long llTotalContentLength, const char *pContentType,
				unsigned char *pucBuffer, unsigned long ulBufferDataSize);

	} CacheSaveHttpGetThread_t, *CacheSaveHttpGetThread_p;

#endif

