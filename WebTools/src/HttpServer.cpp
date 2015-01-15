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

#include "HttpServer.h"
#include "ServerSocket.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>


#ifdef WIN32
	HttpServer:: HttpServer (void): WinThread ()
#else
	HttpServer:: HttpServer (void): PosixThread ()
#endif

{

}


HttpServer:: ~HttpServer (void)

{

}


Error HttpServer:: init (
	const char *pServerLocalIpAddress,
	long lServerPort,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingTimeoutInMicroSeconds,
	unsigned long ulMaxClients)

{

	if (lServerPort <= 0)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (pServerLocalIpAddress == (const char *) NULL)
		strcpy (_pServerLocalIpAddress, "");
	else
		strcpy (_pServerLocalIpAddress, pServerLocalIpAddress);
	_lServerPort			= lServerPort;

	_ulReceivingTimeoutInSeconds		= ulReceivingTimeoutInSeconds;
	_ulReceivingTimeoutInMicroSeconds	= ulReceivingTimeoutInMicroSeconds;

	_ulMaxClients						= ulMaxClients;

	if (_mtHttpServer. init (PMutex:: MUTEX_RECURSIVE) !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		return err;
	}

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);

		if (_mtHttpServer. finish () != errNoError)
		{
			// Error err = PThreadErrors (__FILE__, __LINE__,
			// 	THREADLIB_PMUTEX_FINISH_FAILED);
		}

		return err;
	}


	return errNoError;
}


Error HttpServer:: finish (void)

{

	if (_mtHttpServer. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);

		// return err;
	}

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);

		// return err;
	}


	return errNoError;
}


Error HttpServer:: run (void)

{

	Boolean_t					bIsShutdown;
	Error_t						errGeneric;
	Error						errAcceptConnection;
	ServerSocket_t				ssServerSocket;
	ClientSocket_t				csClientSocket;
	SocketImpl_p				pServerSocketImpl;
	char						pRemoteAddress [SCK_MAXIPADDRESSLENGTH];
	long						lRemotePort;
	SocketImpl_p				pClientSocketImpl;
	Buffer_t					bHttpHeaderRequest;
	WebUtility:: HttpMethod_t	hmHttpMethod;
	Buffer_t					bHttpBodyRequest;
	Buffer_t					bCookie;



	if ((_erThreadReturn = ssServerSocket. init (
		!strcmp (_pServerLocalIpAddress, "") ?
		(const char *) NULL : _pServerLocalIpAddress, _lServerPort, true,
		SocketImpl:: STREAM,
		_ulReceivingTimeoutInSeconds,
		_ulReceivingTimeoutInMicroSeconds,
		_ulMaxClients)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//		SCK_SERVERSOCKET_INIT_FAILED);

		return _erThreadReturn;
	}

	if (ssServerSocket. getSocketImpl (&pServerSocketImpl) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (pServerSocketImpl -> setBlocking (false) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETBLOCKING_FAILED);

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (bHttpHeaderRequest. init () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (bHttpBodyRequest. init () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bHttpHeaderRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (bCookie. init () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bHttpBodyRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bHttpHeaderRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	bIsShutdown				= false;

	if (setShutdown (bIsShutdown) != errNoError)
	{
		_erThreadReturn = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPSERVER_SETSHUTDOWN_FAILED);

		if (bCookie. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bHttpBodyRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bHttpHeaderRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	while (!bIsShutdown)
	{
		if ((_erThreadReturn = csClientSocket. init ()) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			// 	SCK_CLIENTSOCKET_INIT_FAILED);

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if ((errAcceptConnection = ssServerSocket. acceptConnection (
			&csClientSocket)) != errNoError)
		{
			Boolean_t				bIsThereError;


			bIsThereError			= false;

			if ((long) errAcceptConnection == SCK_ACCEPT_FAILED)
			{
				int					iErrno;
				unsigned long		ulUserDataBytes;

				errAcceptConnection. getUserData (&iErrno, &ulUserDataBytes);
				#ifdef WIN32
					if (iErrno == WSAEWOULDBLOCK)
				#else
					if (iErrno == EAGAIN)
				#endif
				{
					// std:: cout << "No connection to accept" << std:: endl;
					#ifdef WIN32
						if (WinThread:: getSleep (0, 500000) != errNoError)
					#else
						if (PosixThread:: getSleep (0, 500000) != errNoError)
					#endif
					{
						_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PTHREAD_GETSLEEP_FAILED);

						if (csClientSocket. finish () != errNoError)
						{
							// Error err = SocketErrors (__FILE__, __LINE__,
							// 	SCK_CLIENTSOCKET_FINISH_FAILED);
						}

						if (bCookie. finish () != errNoError)
						{
							// Error err = ToolsErrors (__FILE__, __LINE__,
							// 	TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bHttpBodyRequest. finish () != errNoError)
						{
							// Error err = ToolsErrors (__FILE__, __LINE__,
							// 	TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bHttpHeaderRequest. finish () != errNoError)
						{
							// Error err = ToolsErrors (__FILE__, __LINE__,
							// 	TOOLS_BUFFER_FINISH_FAILED);
						}

						if (ssServerSocket. finish () != errNoError)
						{
							// Error err = SocketErrors (__FILE__, __LINE__,
							//	SCK_SERVERSOCKET_FINISH_FAILED);
						}

						return _erThreadReturn;
					}
				}
				else
					bIsThereError			= true;
			}
			else
				bIsThereError			= true;
			
			if (bIsThereError)
			{
				// _erThreadReturn = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
				_erThreadReturn			= errAcceptConnection;

				if (csClientSocket. finish () != errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					// 	SCK_CLIENTSOCKET_FINISH_FAILED);
				}

				if (bCookie. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpBodyRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpHeaderRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (ssServerSocket. finish () != errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					//	SCK_SERVERSOCKET_FINISH_FAILED);
				}

				return _erThreadReturn;
			}

			if (csClientSocket. finish () != errNoError)
			{
				_erThreadReturn = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);

				if (bCookie. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpBodyRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpHeaderRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (ssServerSocket. finish () != errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					//	SCK_SERVERSOCKET_FINISH_FAILED);
				}

				return _erThreadReturn;
			}

			if (getShutdown (&bIsShutdown) != errNoError)
			{
				_erThreadReturn = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPSERVER_GETSHUTDOWN_FAILED);

				if (csClientSocket. finish () != errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					// 	SCK_CLIENTSOCKET_FINISH_FAILED);
				}

				if (bCookie. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpBodyRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bHttpHeaderRequest. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				if (ssServerSocket. finish () != errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					//	SCK_SERVERSOCKET_FINISH_FAILED);
				}

				return _erThreadReturn;
			}

			continue;
		}

		if (csClientSocket. getSocketImpl (&pClientSocketImpl) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);

			if (csClientSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
			SCK_MAXIPADDRESSLENGTH) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);

			if (csClientSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if (pClientSocketImpl -> getRemotePort (&lRemotePort) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);

			if (csClientSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if ((_erThreadReturn = WebUtility:: readHttpHeaderAndBody (
			pClientSocketImpl,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingTimeoutInMicroSeconds,
			&bHttpHeaderRequest,
			&bHttpBodyRequest,
			&hmHttpMethod, (Buffer_p) NULL, (Buffer_p) NULL, (Buffer_p) NULL,
			&bCookie, (Buffer_p) NULL, (Buffer_p) NULL)) != errNoError)
		{
			/*
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED);
			*/

			if (csClientSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if ((_erThreadReturn = httpReceived (pRemoteAddress, lRemotePort,
			hmHttpMethod, &bHttpHeaderRequest, &bHttpBodyRequest,
			&bCookie, pClientSocketImpl)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_HTTPSERVER_HTTPRECEIVED_FAILED);

			if (csClientSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if (csClientSocket. finish () != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}

		if (getShutdown (&bIsShutdown) != errNoError)
		{
			_erThreadReturn = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPSERVER_GETSHUTDOWN_FAILED);

			if (bCookie. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpBodyRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bHttpHeaderRequest. finish () != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_FINISH_FAILED);
			}

			if (ssServerSocket. finish () != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SERVERSOCKET_FINISH_FAILED);
			}

			return _erThreadReturn;
		}
	}

	if (bCookie. finish () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bHttpBodyRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bHttpHeaderRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (bHttpBodyRequest. finish () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bHttpHeaderRequest. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (bHttpHeaderRequest. finish () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (ssServerSocket. finish () != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SERVERSOCKET_FINISH_FAILED);
		}

		return _erThreadReturn;
	}

	if (ssServerSocket. finish () != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);

		return _erThreadReturn;
	}


	return _erThreadReturn;
}


Error HttpServer:: httpReceived (
	const char *pRemoteAddress,
	long lRemotePort,
	WebUtility:: HttpMethod_t hmHttpMethod,
	Buffer_p pbHttpHeaderRequest,
	Buffer_p pbHttpBodyRequest,
	Buffer_p pbCookie,
	SocketImpl_p pClientSocketImpl)

{

	Buffer_t			bResponseHeader;
	Buffer_t			bResponseBody;


	std:: cout << "\t\t" << "HEADER: " << (const char *) (*pbHttpHeaderRequest)
		<< std:: endl;
	std:: cout << "\t\t" << "BODY: " << (const char *) (*pbHttpBodyRequest)
		<< std:: endl;
	std:: cout << "\t\t" << "COOKIE: " << (const char *) (*pbCookie)
		<< std:: endl;
	std:: cout << " arrived from "
		<< pRemoteAddress << ":" << lRemotePort << std:: endl;

	if (bResponseHeader. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (bResponseBody. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseBody. append (
		"This is what I received. URL: "
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseBody. append (
		(const char *) (*pbHttpBodyRequest)
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseHeader. append (
		"HTTP/1.1 200 OK" "\r\n"
		"Server: HTTP Catra Libraries Server" "\r\n"
		"Content-Length: "
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseHeader. append (
		(unsigned long) bResponseBody
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseHeader. append (
		"\r\n"
		"Connection: close" "\r\n"
		"Content-Type: text/plain; charset=UTF-8" "\r\n"
		"\r\n"
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pClientSocketImpl -> writeString (
		(const char *) bResponseHeader, true, 2, 0) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pClientSocketImpl -> writeString (
		(const char *) bResponseBody, true, 2, 0) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);

		if (bResponseBody. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseBody. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bResponseHeader. finish () != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bResponseHeader. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error HttpServer:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t	stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif


	if (setShutdown (true) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPSERVER_SETSHUTDOWN_FAILED);

		return err;
	}

	tUTCNow					= time (NULL);

	do
	{
		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);

			return err;
		}

		if (getThreadState (&stThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);

			return err;
		}
	}
	while (stThreadState == THREADLIB_STARTED &&
		time (NULL) - tUTCNow < 5);

	if (stThreadState == THREADLIB_STARTED)
	{
		#ifdef WIN32
			// no cancel available for Windows
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);

				return err;
			}
		#endif
	}


	return errNoError;
}


Error HttpServer:: getShutdown (
	Boolean_p pbIsShutdown)

{

	if (_mtHttpServer. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	*pbIsShutdown		= _bIsShutdown;

	if (_mtHttpServer. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error HttpServer:: setShutdown (
	Boolean_t bIsShutdown)

{

	if (_mtHttpServer. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	_bIsShutdown		= bIsShutdown;

	if (_mtHttpServer. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}

