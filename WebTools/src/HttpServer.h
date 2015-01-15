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

#ifndef HttpServer_h
	#define HttpServer_h

	#include "ClientSocket.h"
	#include "WebUtility.h"
	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "PMutex.h"
	#include "Buffer.h"
	#include "WebToolsErrors.h"


	#define WEBTOOLS_HTTPSERVER_MAXHTTPREQUESTLENGTH		(1024 * 1)


	/**
		This class represent an HttpServer.
	*/
	#ifdef WIN32
		typedef class HttpServer: public WinThread {
	#else
		typedef class HttpServer: public PosixThread {
	#endif

		private:
			char						_pServerLocalIpAddress [
				SCK_MAXIPADDRESSLENGTH];
			long						_lServerPort;

			unsigned long				_ulReceivingTimeoutInSeconds;
			unsigned long				_ulReceivingTimeoutInMicroSeconds;
			unsigned long				_ulSendingTimeoutInSeconds;
			unsigned long				_ulSendingTimeoutInMicroSeconds;

			unsigned long				_ulMaxClients;

			PMutex_t					_mtHttpServer;
			unsigned long				_bIsShutdown;


		protected:
			virtual Error run (void);

			Error getShutdown (Boolean_p pbIsShutdown);

			Error setShutdown (Boolean_t bIsShutdown);

			/**
				httpReceived is a virtual method that is
				called when an HTTP request is received by the server.

				If this method returns an error, the HTTP server will go down.
			*/
			virtual Error httpReceived (
				const char *pRemoteAddress,
				long lRemotePort,
				WebUtility:: HttpMethod_t hmHttpMethod,
				Buffer_p pbHttpHeaderRequest,
				Buffer_p pbHttpBodyRequest,
				Buffer_p pbCookie,
				SocketImpl_p pClientSocketImpl);

		public:
			HttpServer (void);

			~HttpServer (void);
 
			/**
				Parameters:
					pServerLocalIpAddress - Local IP address to handle
						this traffic. If it is NULL is used
						the default local address
					ulServerPort - Port where the server is listening
					ulReceivingTimeoutInSeconds -
					ulReceivingTimeoutInMicroSeconds -
					ulMaxClients -
			*/
			Error init (
				const char *pServerLocalIpAddress,
				long lServerPort,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingTimeoutInMicroSeconds,
				unsigned long ulMaxClients);

			virtual Error finish (void);

			virtual Error cancel (void);

	} HttpServer_t, *HttpServer_p;

#endif

