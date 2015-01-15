
#ifdef WIN32
	#include <winsock2.h>
#else
	#include <unistd.h>
#endif
#include "HTTPProxyThread.h"
#include "StringTokenizer.h"
#include "ConnectionsManagerMessages.h"
#include "DateTime.h"
#include "WebUtility.h"


HTTPProxyThread:: HTTPProxyThread (void):
	HttpGetThread ()

{

}


HTTPProxyThread:: ~HTTPProxyThread (void)

{

}


Error HTTPProxyThread:: init (
	Session_p psSession,
	time_t tUTCStartConnectionTime,
	const char *pWebServerIPAddress,
	unsigned long ulWebServerPort,
	const char *pURLWithoutParameters,
	const char *pURLParameters,
	unsigned long ulTimeoutInSecs,
	const char *pLocalIPAddress,
	Tracer_p ptSystemTracer)

{

	_psSession						= psSession;
	_tUTCStartConnectionTime		= tUTCStartConnectionTime;
	_ulHeaderDataSizeSentToClient	= 0;
	_ullBodyDataSizeSentToClient	= 0;
	_ptSystemTracer					= ptSystemTracer;


	return HttpGetThread:: init (
		pWebServerIPAddress,
		ulWebServerPort,
		pURLWithoutParameters,
		pURLParameters,
		(const char *) NULL,	// cookie
		(const char *) NULL,	// user agent
		ulTimeoutInSecs,
		0,
		ulTimeoutInSecs,
		0,
		pLocalIPAddress);
}


Error HTTPProxyThread:: closingHttpGet (
	Error_p perr)

{

	if (*perr != errNoError &&
		(unsigned long) (*perr) != SCK_READ_EOFREACHED &&
		_ulHeaderDataSizeSentToClient == 0)
	{
		SocketImpl_p		psiClientSocketImpl;
		Error_t				errWrite;


		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_HTTPPROXYTHREAD_HTTPGETINTERNALERROR,
			1, (const char *) (*perr));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bHttpGetHeaderResponse. setBuffer (
			"HTTP/1.1 200 OK"
			"\r\n"
			"Content-Type: text/xml; charset=utf-8"
			"\r\n"
			"Content-Length: "
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetBodyResponse. setBuffer (
			"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
			"\n"
			"<Status><![CDATA[FAILURE: ") !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetBodyResponse. append ((const char *) (*perr)) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetBodyResponse. append ("]]></Status>") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetHeaderResponse. append (
			(unsigned long) _bHttpGetBodyResponse
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		_ulHeaderDataSizeSentToClient			=
			(unsigned long) _bHttpGetHeaderResponse;

		_ullBodyDataSizeSentToClient			=
			(unsigned long) _bHttpGetBodyResponse;

		if (_bHttpGetHeaderResponse. append (
			"\r\n"
			"\r\n"
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if ((errWrite = _psSession -> writeResponse (
			&_tUTCStartConnectionTime,
			&_bHttpGetHeaderResponse, &_bHttpGetBodyResponse, true)) !=
			errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errWrite, __FILE__, __LINE__);

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_WRITERESPONSE_FAILED,
				1, _psSession -> getConnectionIdentifier ());
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			return err;
		}
	}


	return errNoError;
}


Error_t HTTPProxyThread:: chunkRead (unsigned long ulChunkReadIndex,
	long long llTotalContentLength, const char *pContentType,
	unsigned char *pucBuffer, unsigned long ulBufferDataSize)

{

	SocketImpl_p		psiClientSocketImpl;
	Error_t				errWrite;


	if (ulChunkReadIndex == 0)
	{
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_CMPROCESSOR_HTTPGETPROXYRESPONSETIME,
				2,
				"HTTPProxyThread response time: ",
				_ulResponseTimeInMilliSecs);
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6, (const char *) msg,
				__FILE__, __LINE__);
		}

		if (_bHttpGetHeaderResponse. setBuffer (
			"HTTP/1.1 200 OK"
			"\r\n"
			"Content-Type: text/xml; charset=utf-8"
			"\r\n"
			"Content-Length: "
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetHeaderResponse. append (llTotalContentLength) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		_ulHeaderDataSizeSentToClient	=
			(unsigned long) _bHttpGetHeaderResponse;

		if (_bHttpGetHeaderResponse. append (
			"\r\n"
			"\r\n"
		) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (_bHttpGetBodyResponse. setBuffer (
			(char *) pucBuffer, ulBufferDataSize) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		_ullBodyDataSizeSentToClient			= ulBufferDataSize;

		if ((errWrite = _psSession -> writeResponse (
			&_tUTCStartConnectionTime, &_bHttpGetHeaderResponse,
			&_bHttpGetBodyResponse,
			_ullBodyDataSizeSentToClient >= llTotalContentLength ? true : false
			)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errWrite, __FILE__, __LINE__);

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_WRITERESPONSE_FAILED,
				1, _psSession -> getConnectionIdentifier ());
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			return err;
		}
	}
	else
	{
		if (_bHttpGetBodyResponse. setBuffer (
			(char *) pucBuffer, ulBufferDataSize) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		_ullBodyDataSizeSentToClient			+= ulBufferDataSize;

		if ((errWrite = _psSession -> writeResponse (
			&_tUTCStartConnectionTime, (Buffer_p) NULL,
			&_bHttpGetBodyResponse,
			_ullBodyDataSizeSentToClient >= llTotalContentLength ? true : false
			)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errWrite, __FILE__, __LINE__);

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_WRITERESPONSE_FAILED,
				1, _psSession -> getConnectionIdentifier ());
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			return err;
		}
	}


	return errNoError;
}


Error HTTPProxyThread:: sendErrorToHTTPClient (
	Session_p psSession, time_t tUTCStartConnectionTime, Error_p perr,
	Tracer_p ptSystemTracer)

{

	Buffer_t			bHTTPHeaderResponse;
	Buffer_t			bHTTPBodyResponse;
	Error_t				errWrite;
	const char			*pStartFailureMessage;
	const char			*pEndFailureMessage;


	if (bHTTPHeaderResponse. init (
		"HTTP/1.1 200 OK"
		"\r\n"
		"Content-Type: text/xml; charset=utf-8"
		"\r\n"
		"Content-Length: "
		) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (bHTTPBodyResponse. init (
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"\n"
		"<CMS> <Status><![CDATA[FAILURE: ") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	pStartFailureMessage			= strstr (
		(const char *) (*perr), "FAILURE: ");

	if (pStartFailureMessage != (const char *) NULL)
	{
		pStartFailureMessage			+= strlen ("FAILURE: ");

		pEndFailureMessage		= strstr (pStartFailureMessage, "]]");

		if (pEndFailureMessage != (const char *) NULL)
		{
			if (bHTTPBodyResponse. append (pStartFailureMessage,
				pEndFailureMessage - pStartFailureMessage) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}
		else
		{
			if (bHTTPBodyResponse. append (pStartFailureMessage) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}
	}
	else
	{
		if (bHTTPBodyResponse. append ((const char *) (*perr)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}

	if (bHTTPBodyResponse. append ("]]></Status> </CMS>") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (bHTTPBodyResponse. append (
		"\n") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (bHTTPHeaderResponse. append (
		(unsigned long) bHTTPBodyResponse) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (bHTTPHeaderResponse. append (
		"\r\n"
		"\r\n") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errWrite = psSession -> writeResponse (
		&tUTCStartConnectionTime, &bHTTPHeaderResponse, &bHTTPBodyResponse,
		true)) != errNoError)
	{
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errWrite, __FILE__, __LINE__);

		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_SESSION_WRITERESPONSE_FAILED,
			1, psSession -> getConnectionIdentifier ());
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


