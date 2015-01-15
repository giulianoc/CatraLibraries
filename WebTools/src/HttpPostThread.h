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

#ifndef HttpPostThread_h
	#define HttpPostThread_h

	#include "ClientSocket.h"
	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "Buffer.h"
	#include "WebToolsErrors.h"


	/**
		This class represent a thread performing an HTTP POST.

		It is possible to specify a reference to a Proxy to be used
		to perform the HTTP request.
	*/
	#ifdef WIN32
		typedef class HttpPostThread: public WinThread {
	#else
		typedef class HttpPostThread: public PosixThread {
	#endif

		protected:
			ClientSocket_t					_csClientSocket;
			Boolean_t						_bUseInternalClientSocket;

			ClientSocket_p					_pcsUsedClientSocket;
			SocketImpl_p					_pClientSocketImpl;
			unsigned long					_ulReceivingTimeoutInSeconds;
			unsigned long					_ulReceivingTimeoutInMicroSeconds;

			/*
			char						_pWebServerIpAddress [
				SCK_MAXIPADDRESSLENGTH];
			long						_lWebServerPort;
			char						_pLocalIPAddress [
				SCK_MAXIPADDRESSLENGTH];
			char						_pProxyIpAddress [
				SCK_MAXIPADDRESSLENGTH];
			long						_lProxyPort;
			unsigned long				_ulReceivingTimeoutInSeconds;
			unsigned long				_ulReceivingTimeoutInMicroSeconds;
			unsigned long				_ulSendingTimeoutInSeconds;
			unsigned long				_ulSendingTimeoutInMicroSeconds;
			*/
			Buffer_t					_bHttpPostHeaderRequest;
			Buffer_t					_bHttpPostBodyRequest;
			Buffer_t					_bHttpPostHeaderResponse;
			Buffer_t					_bHttpPostBodyResponse;
			Buffer_t					_bHttpCookieHeaderResponse;
			unsigned long				_ulResponseTimeInMilliSecs;


			/**
				This method is not declared protected because in some cases
				is useful to call it directly when it is necessary to do
				an HTTP GET without starting a thread.
			*/
			virtual Error run (void);

			/**
				The closingHttpPost is a virtual method that is
				called when the HTTP POST request is finished.
				This method is called also in case of a failure
				or an error during the request.
			*/
			virtual Error closingHttpPost (Error_p perr);

		public:
			HttpPostThread (void);

			~HttpPostThread (void);
 
			/**
				Parameters:
					pWebServerIpAddress - IP address of the WEB Server
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
				const char *pWebServerIpAddress,
				long lWebServerPort,
				const char *pURI,
				const char *pHttpPostBodyRequest,
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
				ClientSocket_p pcsWebServerSocket,
				const char *pWebServerIpAddress,
				long lWebServerPort,
				const char *pURI,
				const char *pHttpPostBodyRequest,
				const char *pURLParameters,
				const char *pCookie,
				const char *pUserAgent,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingTimeoutInMicroSeconds);

			virtual Error finish (void);

			virtual Error cancel (void);

			/**
				This method is useful to call it directly
				when it is necessary to do an HTTP GET without
				starting a thread.

				If a parameter is set to NULL, it will not be initialized.
			*/
			Error run (
				Buffer_p pbHttpPostHeaderResponse,
				Buffer_p pbHttpPostBodyResponse,
				Buffer_p pbHttpCookieHeaderResponse);

			/**
				Return the HTTP response.
				If a parameter is set to NULL, it will not be initialized.
			*/
			Error getHttpResponse (
				Buffer_p pbHttpPostHeaderResponse,
				Buffer_p pbHttpPostBodyResponse,
				Buffer_p pbHttpCookieHeaderResponse);

			operator unsigned long (void) const;

	} HttpPostThread_t, *HttpPostThread_p;

#endif

