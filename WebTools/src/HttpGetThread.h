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

#ifndef HttpGetThread_h
	#define HttpGetThread_h

	#include "ClientSocket.h"
	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "Buffer.h"
	#include "WebUtility.h"
	#include "WebToolsErrors.h"


	/**
		This class represent a thread performing an HTTP GET.

		It is possible to specify a reference to a Proxy to be used
		to perform the HTTP request.
	*/
	#ifdef WIN32
		typedef class HttpGetThread: public WinThread {
	#else
		typedef class HttpGetThread: public PosixThread {
	#endif

		protected:
			char						_pWebServer[
				SCK_MAXHOSTNAMELENGTH];
			long						_lWebServerPort;
			char						_pLocalIPAddress [
				SCK_MAXIPADDRESSLENGTH];
			char						_pProxyIpAddress [
				SCK_MAXIPADDRESSLENGTH];
			long						_lProxyPort;
			Buffer_t					_bHttpURI;
			Buffer_t					_bHttpGetRequest;
			Buffer_t					_bHttpGetHeaderResponse;
			Buffer_t					_bHttpGetBodyResponse;
			Buffer_t					_bHttpCookieHeaderResponse;
			Buffer_t					_bHttpSetCookieHeaderResponse;
			Buffer_t					_bHttpLocationHeaderResponse;
			unsigned long				_ulReceivingTimeoutInSeconds;
			unsigned long				_ulReceivingTimeoutInMicroSeconds;
			unsigned long				_ulSendingTimeoutInSeconds;
			unsigned long				_ulSendingTimeoutInMicroSeconds;
			unsigned long				_ulResponseTimeInMilliSecs;


			/**
				This method is not declared protected because in some cases
				is useful to call it directly when it is necessary to do
				an HTTP GET without starting a thread.
			*/
			virtual Error run (void);

			/**
				The closingHttpGet is a virtual method that is
				called when the HTTP GET request is finished.
				This method is called also in case of a failure
				or an error during the request.
			*/
			virtual Error closingHttpGet (Error_p perr);

		public:
			HttpGetThread (void);

			~HttpGetThread (void);
 
			/**
				Parameters:
					pWebServer - IP address or hostname of the WEB Server
					pURI - Relative path
						rapresenting the HTTP request without any parameters
						(it must starts with the '/' character)
					pURLParameters - Parameters of the URL according the
						URL syntax. It must starts with a '?' character and
						the parameter must be separated using the '&' character
					ulReceivingTimeoutInSeconds -
					ulReceivingTimeoutInMicroSeconds -
					ulSendingTimeoutInSeconds -
					ulSendingTimeoutInMicroSeconds -
					pLocalIPAddress - local address to handle this traffic.
						If it is NULL is used the default local address
					pProxyIpAddress - IP address of a proxy in case
						the HTTP request must go through a proxy
					lProxyPort - Port where a proxy is in listen in case
						the HTTP request must go through a proxy
			*/
			Error init (
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

			Error init (
				const char *pURL,
				const char *pCookie = (const char *) NULL,
				const char *pUserAgent = (const char *) NULL,
				unsigned long ulReceivingTimeoutInSeconds = 15,
				unsigned long ulReceivingTimeoutInMicroSeconds = 0,
				unsigned long ulSendingTimeoutInSeconds = 15,
				unsigned long ulSendingTimeoutInMicroSeconds = 0,
				const char *pLocalIPAddress = (const char *) NULL,
				const char *pProxyIpAddress = (const char *) NULL,
				long lProxyPort = 8888);

			virtual Error finish (void);

			Error getInputDetails (
				char *pWebServer, unsigned long ulWebServerBufferLength,
				long *plWebServerPort, Buffer_p pbURI);

			virtual Error cancel (void);

			/**
				This method is useful to call it directly
				when it is necessary to do an HTTP GET without
				starting a thread.

				If a parameter is set to NULL, it will not be initialized.
			*/
			Error run (
				Buffer_p pbHttpGetHeaderResponse,
				Buffer_p pbHttpGetBodyResponse,
				Buffer_p pbHttpCookieHeaderResponse,
				Buffer_p pbHttpSetCookieHeaderResponse,
				Buffer_p pbHttpLocationHeaderResponse);

			/**
				Return the HTTP response.
				If a parameter is set to NULL, it will not be initialized.
			*/
			Error getHttpResponse (
				Buffer_p pbHttpGetHeaderResponse,
				Buffer_p pbHttpGetBodyResponse,
				Buffer_p pbHttpCookieHeaderResponse);

			/*
			 *
			 * This method has not to be called directly but
			 * it could be redefined creating a class herediting from this class
			 * to manage the downloded content
			 */
			virtual Error_t chunkRead (unsigned long ulChunkReadIndex,
				long long llTotalContentLength, const char *pContentType,
				unsigned char *pucBuffer, unsigned long ulBufferDataSize);

			operator unsigned long (void) const;

	} HttpGetThread_t, *HttpGetThread_p;

#endif

