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

#include "HttpPostThread.h"
#include "WebUtility.h"
#include "DateTime.h"
// #include "FileIO.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>


#ifdef WIN32
	HttpPostThread:: HttpPostThread (void): WinThread ()
#else
	HttpPostThread:: HttpPostThread (void): PosixThread ()
#endif

{

	_bUseInternalClientSocket			= false;
}


HttpPostThread:: ~HttpPostThread (void)

{

}


Error HttpPostThread:: init (
	const char *pWebServerIpAddress,
	long lWebServerPort,
	const char *pURI,
	const char *pHttpPostBodyRequest,
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

	Error_t				errClientSocketInit;


	if (pWebServerIpAddress == (const char *) NULL ||
		lWebServerPort <= 0 ||
		pURI == (const char *) NULL ||
		pHttpPostBodyRequest == (const char *) NULL ||
		lProxyPort <= 0)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	_ulReceivingTimeoutInSeconds		= ulReceivingTimeoutInSeconds;
	_ulReceivingTimeoutInMicroSeconds	= ulReceivingTimeoutInMicroSeconds;

	/*
	strcpy (_pWebServerIpAddress, pWebServerIpAddress);
	_lWebServerPort			= lWebServerPort;

	_ulSendingTimeoutInSeconds			= ulSendingTimeoutInSeconds;
	_ulSendingTimeoutInMicroSeconds		= ulSendingTimeoutInMicroSeconds;

	if (pLocalIPAddress == (const char *) NULL)
		strcpy (_pLocalIPAddress, "");
	else
		strcpy (_pLocalIPAddress, pLocalIPAddress);

	if (pProxyIpAddress == (const char *) NULL)
		strcpy (_pProxyIpAddress, "");
	else
		strcpy (_pProxyIpAddress, pProxyIpAddress);

	_lProxyPort				= lProxyPort;
	*/

	if (pProxyIpAddress == (const char *) NULL)
	{
		if ((errClientSocketInit = _csClientSocket. init (SocketImpl:: STREAM,
			ulReceivingTimeoutInSeconds,
			ulReceivingTimeoutInMicroSeconds,
			ulSendingTimeoutInSeconds,
			ulSendingTimeoutInMicroSeconds,
			ulReceivingTimeoutInSeconds,
			true, !strcmp (pLocalIPAddress, "") ? (const char *) NULL :
			pLocalIPAddress,
			pWebServerIpAddress, lWebServerPort)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, pWebServerIpAddress, lWebServerPort);

			return errClientSocketInit;
		}
	}
	else
	{
		if ((errClientSocketInit = _csClientSocket. init (SocketImpl:: STREAM,
			ulReceivingTimeoutInSeconds,
			ulReceivingTimeoutInMicroSeconds,
			ulSendingTimeoutInSeconds,
			ulSendingTimeoutInMicroSeconds,
			ulReceivingTimeoutInSeconds,
			true, !strcmp (pLocalIPAddress, "") ? (const char *) NULL :
				pLocalIPAddress,
			pProxyIpAddress, lProxyPort)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, pProxyIpAddress, lProxyPort);

			return errClientSocketInit;
		}
	}

	if (init (&_csClientSocket,
		pWebServerIpAddress,
		lWebServerPort,
		pURI,
		pHttpPostBodyRequest,
		pURLParameters,
		pCookie,
		pUserAgent,
		ulReceivingTimeoutInSeconds,
		ulReceivingTimeoutInMicroSeconds) != errNoError)
	{
		Error_t err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_HTTPPOSTTHREAD_INIT_FAILED);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return err;
	}

	_bUseInternalClientSocket			= true;


	return errNoError;
}


Error HttpPostThread:: init (
	ClientSocket_p pcsWebServerSocket,
	const char *pWebServerIpAddress,
	long lWebServerPort,
	const char *pURI,
	const char *pHttpPostBodyRequest,
	const char *pURLParameters,
	const char *pCookie,
	const char *pUserAgent,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingTimeoutInMicroSeconds
	)

{

	if (pURI == (const char *) NULL ||
		pHttpPostBodyRequest == (const char *) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	_bUseInternalClientSocket			= false;

	_ulResponseTimeInMilliSecs			= 0;

	_ulReceivingTimeoutInSeconds		= ulReceivingTimeoutInSeconds;
	_ulReceivingTimeoutInMicroSeconds	= ulReceivingTimeoutInMicroSeconds;

	_pcsUsedClientSocket			= pcsWebServerSocket;

	if (_pcsUsedClientSocket -> getSocketImpl (&_pClientSocketImpl) !=
		errNoError)
	{
		Error_t err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		return err;
	}

	if (_bHttpPostHeaderRequest. init ("POST ") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (_bHttpPostHeaderRequest. append (pURI) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pURLParameters != (const char *) NULL)
	{
		if (_bHttpPostHeaderRequest. append (pURLParameters) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpPostHeaderRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (_bHttpPostHeaderRequest. append (
		" HTTP/1.0\r\nConnection: close") != errNoError
	)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (pCookie != (const char *) NULL &&
		strcmp (pCookie, ""))
	{
		if (_bHttpPostHeaderRequest. append (
			"\r\nCookie: ") != errNoError ||
			_bHttpPostHeaderRequest. append (pCookie) != errNoError
		)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpPostHeaderRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (pUserAgent != (const char *) NULL &&
		strcmp (pUserAgent, ""))
	{
		if (_bHttpPostHeaderRequest. append (
			"\r\nUser-Agent: ") != errNoError ||
			_bHttpPostHeaderRequest. append (pUserAgent) != errNoError
		)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (_bHttpPostHeaderRequest. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (_bHttpPostHeaderRequest. append ("\r\nAccept: */*\r\nHost: ") !=
			errNoError ||
		_bHttpPostHeaderRequest. append (pWebServerIpAddress) != errNoError ||
		_bHttpPostHeaderRequest. append (":") != errNoError ||
		_bHttpPostHeaderRequest. append (lWebServerPort) != errNoError ||
		_bHttpPostHeaderRequest. append ("\r\nContent-Length: ") !=
			errNoError ||
		_bHttpPostHeaderRequest. append (
			(long) strlen (pHttpPostBodyRequest)) != errNoError ||
		_bHttpPostHeaderRequest. append ("\r\n\r\n") != errNoError
	)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpPostBodyRequest. init (pHttpPostBodyRequest) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpPostHeaderResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpPostBodyRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bHttpPostBodyResponse. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bHttpPostHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostBodyRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderRequest. finish () != errNoError)
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

		if (_bHttpPostBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostBodyRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderRequest. finish () != errNoError)
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

		if (_bHttpCookieHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostBodyResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderResponse. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostBodyRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bHttpPostHeaderRequest. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}


	return errNoError;
}


Error HttpPostThread:: finish (void)

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

	if (_bHttpCookieHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpPostBodyResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpPostHeaderResponse. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpPostBodyRequest. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bHttpPostHeaderRequest. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bUseInternalClientSocket)
	{
		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}
	}


	return errNoError;
}


Error HttpPostThread:: run (
	Buffer_p pbHttpPostHeaderResponse,
	Buffer_p pbHttpPostBodyResponse,
	Buffer_p pbHttpCookieHeaderResponse)

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

	if (pbHttpPostHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpPostHeaderResponse -> setBuffer (
			(const char *) _bHttpPostHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpPostBodyResponse != (Buffer_p) NULL)
	{
		if (pbHttpPostBodyResponse -> setBuffer (
			(const char *) _bHttpPostBodyResponse) != errNoError)
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


Error HttpPostThread:: run (void)

{

	Error_t							errRead;
	Error_t							errWrite;
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

	if ((errWrite = _pClientSocketImpl -> writeString (
		(const char *) _bHttpPostHeaderRequest, true, 0, 1000)) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);

		if (closingHttpPost (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if ((errWrite = _pClientSocketImpl -> writeString (
		(const char *) _bHttpPostBodyRequest, true, 0, 1000)) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);

		if (closingHttpPost (&_erThreadReturn) !=
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

		if (closingHttpPost (&_erThreadReturn) !=
			errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);
		}

		return _erThreadReturn;
	}

	if ((_erThreadReturn = WebUtility:: readHttpHeaderAndBody (
		_pClientSocketImpl,
		_ulReceivingTimeoutInSeconds,
		_ulReceivingTimeoutInMicroSeconds,
		&_bHttpPostHeaderResponse,
		&_bHttpPostBodyResponse,
		&hmHttpMethod,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		&_bHttpCookieHeaderResponse,
		(Buffer_p) NULL, (Buffer_p) NULL)) != errNoError)
	{
		// Error err = WebToolsErrors (__FILE__, __LINE__,
		// 	WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED);

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
		}

		_ulResponseTimeInMilliSecs			= (unsigned long)
			(ullLocalDateTimeInMilliSecs2 -
			ullLocalDateTimeInMilliSecs1);

		if (closingHttpPost (&_erThreadReturn) !=
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

		if (closingHttpPost (&_erThreadReturn) !=
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

	{
		Error_t			err;

		err			= errNoError;

		if (closingHttpPost (&err) != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED);

			return err;
		}
	}


	return _erThreadReturn;
}


Error HttpPostThread:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}


Error HttpPostThread:: getHttpResponse (
	Buffer_p pbHttpPostHeaderResponse,
	Buffer_p pbHttpPostBodyResponse,
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

	if (pbHttpPostHeaderResponse != (Buffer_p) NULL)
	{
		if (pbHttpPostHeaderResponse -> setBuffer (
			(const char *) _bHttpPostHeaderResponse) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (pbHttpPostBodyResponse != (Buffer_p) NULL)
	{
		if (pbHttpPostBodyResponse -> setBuffer (
			(const char *) _bHttpPostBodyResponse) != errNoError)
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


Error HttpPostThread:: closingHttpPost (
	Error_p perr)

{

	return errNoError;
}


HttpPostThread:: operator unsigned long (void) const

{

	return _ulResponseTimeInMilliSecs;
}

