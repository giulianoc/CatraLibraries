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

#include "HttpGetThread.h"
#include "WebUtility.h"
#include "DateTime.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>


Error_t chunkReadHttpGet (void *pvObject, unsigned long ulChunkReadIndex,
	long long llTotalContentLength, const char *pContentType,
	unsigned char *pucBuffer, unsigned long ulBufferDataSize)

{

	HttpGetThread_p					phgtHttpGetThread			=
		(HttpGetThread_p) pvObject;

	if (phgtHttpGetThread == (HttpGetThread_p) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}


	return phgtHttpGetThread -> chunkRead (ulChunkReadIndex,
		llTotalContentLength, pContentType, pucBuffer, ulBufferDataSize);

}


#ifdef WIN32
	HttpGetThread:: HttpGetThread (void): WinThread ()
#else
	HttpGetThread:: HttpGetThread (void): PosixThread ()
#endif

{
	strcpy (_pWebServer, "");
	_lWebServerPort							= -1;
	strcpy (_pLocalIPAddress, "");
	strcpy (_pProxyIpAddress, "");
	_lProxyPort								= -1;
	_ulReceivingTimeoutInSeconds			= 0;
	_ulReceivingTimeoutInMicroSeconds		= 0;
	_ulSendingTimeoutInSeconds				= 0;
	_ulSendingTimeoutInMicroSeconds			= 0;
	_ulResponseTimeInMilliSecs				= 0;

}


HttpGetThread:: ~HttpGetThread (void)

{

}


Error HttpGetThread:: init (
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

	if (pWebServer== (const char *) NULL ||
		strlen (pWebServer) >= SCK_MAXHOSTNAMELENGTH ||
		lWebServerPort <= 0 ||
		pURI == (const char *) NULL ||
		lProxyPort <= 0)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	strcpy (_pWebServer, pWebServer);
	_lWebServerPort			= lWebServerPort;

	_ulReceivingTimeoutInSeconds		= ulReceivingTimeoutInSeconds;
	_ulReceivingTimeoutInMicroSeconds	= ulReceivingTimeoutInMicroSeconds;
	_ulSendingTimeoutInSeconds			= ulSendingTimeoutInSeconds;
	_ulSendingTimeoutInMicroSeconds		= ulSendingTimeoutInMicroSeconds;

	_ulResponseTimeInMilliSecs			= 0;

	if (pLocalIPAddress == (const char *) NULL)
		strcpy (_pLocalIPAddress, "");
	else
		strcpy (_pLocalIPAddress, pLocalIPAddress);

	if (pProxyIpAddress == (const char *) NULL)
		strcpy (_pProxyIpAddress, "");
	else
		strcpy (_pProxyIpAddress, pProxyIpAddress);

	_lProxyPort				= lProxyPort;

	if (_bHttpURI. init (pURI) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (_bHttpGetRequest. init ("GET ") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetRequest. append (pURI) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pURLParameters != (const char *) NULL)
	{
		if (_bHttpGetRequest. append (pURLParameters) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpGetRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bHttpURI. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (_bHttpGetRequest. append (
		" HTTP/1.0\r\nAccept: */*\r\nHost: ") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetRequest. append (_pWebServer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetRequest. append (":") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetRequest. append (_lWebServerPort) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pCookie != (const char *) NULL &&
		strcmp (pCookie, ""))
	{
		if (_bHttpGetRequest. append ("\r\nCookie: ") !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpGetRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bHttpURI. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (_bHttpGetRequest. append (pCookie) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpGetRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bHttpURI. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	// User-Agent
	{
		if (_bHttpGetRequest. append ("\r\nUser-Agent: ") !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpGetRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bHttpURI. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (pUserAgent != (const char *) NULL &&
			strcmp (pUserAgent, ""))
		{
			if (_bHttpGetRequest. append (pUserAgent) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (_bHttpGetRequest. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (_bHttpURI. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
		else
		{
			if (_bHttpGetRequest. append ("CatraGetHttp") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (_bHttpGetRequest. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (_bHttpURI. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	}

	if (_bHttpGetRequest. append ("\r\n\r\n") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpGetBodyResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpCookieHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpGetBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpSetCookieHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpLocationHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpSetCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

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

		if (_bHttpLocationHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpSetCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpGetRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpURI. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}


	return errNoError;
}


Error HttpGetThread:: init (
	const char *pURL,
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

	char				pWebServer[SCK_MAXHOSTNAMELENGTH];
	long				lWebServerPort;
	Buffer_t			bURI;
	Buffer_t			bURLParameters;


	if (pURL== (const char *) NULL ||
		lProxyPort <= 0)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (WebUtility:: parseURL (pURL, pWebServer, SCK_MAXHOSTNAMELENGTH,
		&lWebServerPort, &bURI, &bURLParameters) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_PARSEURL_FAILED,
			1, pURL);

		return err;
	}


	return HttpGetThread:: init (
		pWebServer,
		lWebServerPort,
		bURI. str (),
		bURLParameters. str (),
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


Error HttpGetThread:: finish (void)

{

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);

		return err;
	}

	if (_bHttpLocationHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpSetCookieHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpCookieHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpGetBodyResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpGetHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpGetRequest. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpURI. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}


	return errNoError;
}


Error HttpGetThread:: getInputDetails (
	char *pWebServer, unsigned long ulWebServerBufferLength,
	long *plWebServerPort, Buffer_p pbURI)

{

	if (pWebServer == (char *) NULL ||
		strlen (_pWebServer) >= ulWebServerBufferLength ||
		plWebServerPort == (long *) NULL ||
		pbURI == (Buffer_p) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	strcpy (pWebServer, _pWebServer);
	*plWebServerPort			= _lWebServerPort;

	if (pbURI -> setBuffer ((const char *) _bHttpURI) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}


Error HttpGetThread:: run (
	Buffer_p pbHttpGetHeaderResponse,
	Buffer_p pbHttpGetBodyResponse,
	Buffer_p pbHttpCookieHeaderResponse,
	Buffer_p pbHttpSetCookieHeaderResponse,
	Buffer_p pbHttpLocationHeaderResponse)

{

	Error_t						errRun;


	if ((errRun = run ()) != errNoError)
	{
		/*
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_RUN_FAILED);
		*/

		return errRun;
	}

	if (pbHttpGetHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpGetHeaderResponse -> setBuffer (
			(const char *) _bHttpGetHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpGetBodyResponse != (Buffer_p) NULL)
	{
		if (pbHttpGetBodyResponse -> setBuffer (
			(const char *) _bHttpGetBodyResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpCookieHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpCookieHeaderResponse -> setBuffer (
			(const char *) _bHttpCookieHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpSetCookieHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpSetCookieHeaderResponse -> setBuffer (
			(const char *) _bHttpSetCookieHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpLocationHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpLocationHeaderResponse -> setBuffer (
			(const char *) _bHttpLocationHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error HttpGetThread:: run (void)

{

	ClientSocket_t					csClientSocket;
	SocketImpl_p					pClientSocketImpl;
	// Error_t							errSocket;
	WebUtility:: HttpMethod_t		hmHttpMethod;
	/*
	#ifdef WIN32
		__int64					ullLocalDateTimeInMilliSecs1;
		__int64					ullLocalDateTimeInMilliSecs2;
	#else
	*/
		unsigned long long		ullLocalDateTimeInMilliSecs1;
		unsigned long long		ullLocalDateTimeInMilliSecs2;
	// #endif

	_ulResponseTimeInMilliSecs			= 0;

	if (!strcmp (_pProxyIpAddress, ""))
	{
		if ((_erThreadReturn = csClientSocket. init (SocketImpl:: STREAM,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingTimeoutInMicroSeconds,
			_ulSendingTimeoutInSeconds,
			_ulSendingTimeoutInMicroSeconds,
			_ulReceivingTimeoutInSeconds,
			true, !strcmp (_pLocalIPAddress, "") ? (const char *) NULL :
			_pLocalIPAddress,
			_pWebServer, _lWebServerPort)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, _pWebServer, _lWebServerPort);

			if (closingHttpGet (&_erThreadReturn) !=
				errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
			}

			return _erThreadReturn;
		}
	}
	else
	{
		if ((_erThreadReturn = csClientSocket. init (SocketImpl:: STREAM,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingTimeoutInMicroSeconds,
			_ulSendingTimeoutInSeconds,
			_ulSendingTimeoutInMicroSeconds,
			_ulReceivingTimeoutInSeconds,
			true, !strcmp (_pLocalIPAddress, "") ? (const char *) NULL :
			_pLocalIPAddress,
			_pProxyIpAddress, _lProxyPort)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, _pProxyIpAddress, _lProxyPort);

			if (closingHttpGet (&_erThreadReturn) !=
				errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
			}

			return _erThreadReturn;
		}
	}

	if (csClientSocket. getSocketImpl (&pClientSocketImpl) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	#ifdef WIN32
		if (pClientSocketImpl -> setBlocking (false) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETBLOCKING_FAILED);

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			if (closingHttpGet (&_erThreadReturn) !=
				errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
			}

			return _erThreadReturn;
		}
	#endif

	if (pClientSocketImpl -> setReceivingTimeout (
		_ulReceivingTimeoutInSeconds,
		_ulReceivingTimeoutInMicroSeconds) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETRECEIVINGTIMEOUT_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	// setNoDelay important to call (see documentation in SocketImpl.h)
	if (pClientSocketImpl -> setNoDelay (true) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETNODELAY_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	// setKeepAlive important to call (see documentation in SocketImpl.h)
	if ((_erThreadReturn = pClientSocketImpl -> setKeepAlive (true)) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETKEEPALIVE_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if (pClientSocketImpl -> setMaxSendBuffer (
		15 * 1024) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXSENDBUFFER_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if (pClientSocketImpl -> setMaxReceiveBuffer (
		15 * 1024) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if ((_erThreadReturn = pClientSocketImpl -> writeString (
		(const char *) _bHttpGetRequest, true, 0, 1000)) != errNoError)
	{
		// _erThreadReturn = SocketErrors (__FILE__, __LINE__,
		// 	SCK_SOCKETIMPL_WRITESTRING_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if (DateTime:: nowLocalInMilliSecs (
		&ullLocalDateTimeInMilliSecs1) != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	#ifdef WIN32
		if ((_erThreadReturn = WebUtility:: readHttpHeaderAndBody (
			pClientSocketImpl,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingTimeoutInMicroSeconds,
			&_bHttpGetHeaderResponse,
			(Buffer_p) NULL,
			&hmHttpMethod,
			(Buffer_p) NULL,
			(Buffer_p) NULL,
			(Buffer_p) NULL,
			&_bHttpCookieHeaderResponse,
			&_bHttpSetCookieHeaderResponse,
			&_bHttpLocationHeaderResponse,
			chunkReadHttpGet,
			this)) != errNoError)
	#else
		if ((_erThreadReturn = WebUtility:: readHttpHeaderAndBody (
			pClientSocketImpl,
			0,	// timeout already initialized before
			0,
			&_bHttpGetHeaderResponse,
			(Buffer_p) NULL,
			&hmHttpMethod,
			(Buffer_p) NULL,
			(Buffer_p) NULL,
			(Buffer_p) NULL,
			&_bHttpCookieHeaderResponse,
			&_bHttpSetCookieHeaderResponse,
			&_bHttpLocationHeaderResponse,
			chunkReadHttpGet,
			this)) != errNoError)
	#endif
	{
		// Error err = WebToolsErrors (__FILE__, __LINE__,
		// 	WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
		}

		_ulResponseTimeInMilliSecs			= (unsigned long)
			(ullLocalDateTimeInMilliSecs2 -
			ullLocalDateTimeInMilliSecs1);

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if (DateTime:: nowLocalInMilliSecs (
		&ullLocalDateTimeInMilliSecs2) != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	_ulResponseTimeInMilliSecs			= (unsigned long)
		(ullLocalDateTimeInMilliSecs2 -
		ullLocalDateTimeInMilliSecs1);

	if (csClientSocket. finish () != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);

		if (closingHttpGet (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	{
		Error_t			err;

		err			= errNoError;

		if (closingHttpGet (&err) != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);

			return err;
		}
	}


	return _erThreadReturn;
}


Error HttpGetThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}


Error HttpGetThread:: getHttpResponse (
	Buffer_p pbHttpGetHeaderResponse,
	Buffer_p pbHttpGetBodyResponse,
	Buffer_p pbHttpCookieHeaderResponse)

{

	#ifdef WIN32
		WinThread:: PThreadStatus_t	stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif


	if (getThreadState (&stThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);

		return err;
	}

	#ifdef WIN32
		if (stThreadState != WinThread:: THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	#else
		if (stThreadState != PosixThread:: THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	#endif
	{
		// the thread is not finished
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED,
			1, (long) stThreadState);

		return err;
	}

	if (pbHttpGetHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpGetHeaderResponse -> setBuffer (
			(const char *) _bHttpGetHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpGetBodyResponse != (Buffer_p) NULL)
	{
		if (pbHttpGetBodyResponse -> setBuffer (
			(const char *) _bHttpGetBodyResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpCookieHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpCookieHeaderResponse -> setBuffer (
			(const char *) _bHttpCookieHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error_t HttpGetThread:: chunkRead (unsigned long ulChunkReadIndex,
	long long llTotalContentLength, const char *pContentType,
	unsigned char *pucBuffer, unsigned long ulBufferDataSize)

{

	if (ulChunkReadIndex == 0)
	{
		if (_bHttpGetBodyResponse. setBuffer ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (_bHttpGetBodyResponse. append (
		(char *) pucBuffer, ulBufferDataSize) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error HttpGetThread:: closingHttpGet (
	Error_p perr)

{

	return errNoError;
}


HttpGetThread:: operator unsigned long (void) const

{

	return _ulResponseTimeInMilliSecs;
}

