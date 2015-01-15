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

#include "WebUtility.h"
#include "StringTokenizer.h"
#include "Encrypt.h"
#include <stdio.h>
#include <stdlib.h>


Error WebUtility:: parseURL (const char *pURL,
	char *pServer, unsigned long ulServerBufferLength,
	long *plPort,
	Buffer_p pbRelativePathWithoutParameters,
	Buffer_p pbURLParameters)

{

	StringTokenizer_t				stURLTokenizer;
	Error							errNextToken;
	const char						*pToken;
	const char						*pColon;


	if (pServer == (char *) NULL ||
		plPort == (long *) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (stURLTokenizer. init (pURL, -1, "/") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_INIT_FAILED);

		return err;
	}

	if ((errNextToken = stURLTokenizer. nextToken (&pToken)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

		if (stURLTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		return err;
	}

	// Server[:port]
	{
		if ((errNextToken = stURLTokenizer. nextToken (&pToken)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

			if (stURLTokenizer. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_FINISH_FAILED);
			}

			return err;
		}

		if ((pColon = strchr (pToken, ':')) != (char *) NULL)
		{
			if (pColon - pToken + 1 > (long) ulServerBufferLength)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_ACTIVATION_WRONG);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}

			strncpy (pServer, pToken, pColon - pToken);
			pServer [pColon - pToken]		= '\0';

			*plPort			= atol (pColon + 1);
		}
		else
		{
			if (strlen (pToken) + 1 > ulServerBufferLength)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_ACTIVATION_WRONG);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}

			strcpy (pServer, pToken);

			*plPort			= -1;
		}
	}

	// <relative path>
	{
		if ((errNextToken = stURLTokenizer. nextToken (&pToken, "?")) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

			if (stURLTokenizer. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_FINISH_FAILED);
			}

			return err;
		}

		if (pbRelativePathWithoutParameters != (Buffer_p) NULL)
		{
			if (pbRelativePathWithoutParameters -> setBuffer ("/") !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}

			if (pbRelativePathWithoutParameters -> append (pToken) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}
		}
	}

	// <parameters>
	if (pbURLParameters != (Buffer_p) NULL)
	{
		if ((errNextToken = stURLTokenizer. nextToken (&pToken, "\0")) !=
			errNoError)
		{
			if ((long) errNextToken != TOOLS_STRINGTOKENIZER_NOMORETOKEN)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}
		}

		if (errNextToken == errNoError)
		{
			if (pbURLParameters -> setBuffer ("?") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}

			if (pbURLParameters -> append (pToken) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}
		}
		else
		{
			if (pbURLParameters -> setBuffer ("") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				if (stURLTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}
		}
	}

	if (stURLTokenizer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error WebUtility:: getURLParameterValue (const char *pURL,
	const char *pURLParameterName,
	char *pURLParameterValue, unsigned long ulBufferLength,
	Boolean_t bParameterToBeDecrypted)

{

	const char					*pBeginParameterName;
	const char					*pBeginParameterValue;
	const char					*pEndParameterValue;
	Boolean_t					bParameterFound;
	const char					*pLocalURL;
	char						*pLocalURLParameterValue;
	Error_t						errDecodeURL;


	if (pURL == (const char *) NULL ||
		pURLParameterName == (const char *) NULL ||
		pURLParameterValue == (char *) NULL ||
		ulBufferLength == 0)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	pLocalURL			= pURL;
	bParameterFound		= false;

	// look for the parameter
	while (!bParameterFound)
	{
		if ((pBeginParameterName = strstr (pLocalURL, pURLParameterName)) ==
			(char *) NULL)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_ACTIVATION_WRONG);

			return err;
		}

		pBeginParameterValue		=
			pBeginParameterName + strlen (pURLParameterName);

		// if *pBeginParameterValue is '\0' or '&' we are in the scenario of
		// a parameter without any value:
		// 		rtsp://.../jjj?Param1
		// 		rtsp://.../jjj?Param1&Param2=Value
		// where the Param1 does not have a value
		if (*pBeginParameterValue == '=' || *pBeginParameterValue == '\0' ||
			*pBeginParameterValue == '&')
		{
			if (pBeginParameterName != pLocalURL &&
				(*(pBeginParameterName - 1) == '?' ||
				*(pBeginParameterName - 1) == '&'))
			{
				bParameterFound		= true;
			}
		}

		pLocalURL			= pBeginParameterValue;
	}

	if (*pBeginParameterValue != '=')	// it will be '\0' or '&'
	{
		// Parameter without any value (see before)
		strcpy (pURLParameterValue, "");


		return errNoError;
	}

	// retrieve the value of the parameter and decode it
	if ((pLocalURLParameterValue = new char [ulBufferLength]) ==
		(char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_NEW_FAILED);

		return err;
	}

	pBeginParameterValue			+= 1;

	if ((pEndParameterValue = strchr (pBeginParameterValue, '&')) ==
		(char *) NULL)
	{
		if (strlen (pBeginParameterValue) >= ulBufferLength)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
				2, ulBufferLength,
				(unsigned long) strlen (pBeginParameterValue));

			delete [] pLocalURLParameterValue;

			return err;
		}

		strcpy (pLocalURLParameterValue, pBeginParameterValue);
	}
	else
	{
		if (pEndParameterValue - pBeginParameterValue >= (long) ulBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
				2, ulBufferLength,
				(unsigned long) (pEndParameterValue - pBeginParameterValue));

			delete [] pLocalURLParameterValue;

			return err;
		}

		strncpy (pLocalURLParameterValue, pBeginParameterValue,
			pEndParameterValue - pBeginParameterValue);
		pLocalURLParameterValue [
			pEndParameterValue - pBeginParameterValue]		= '\0';
	}

	if ((errDecodeURL = WebUtility:: decodeURL (pLocalURLParameterValue,
		pURLParameterValue, ulBufferLength)) != errNoError)
	{
		// Error err = ToolsErrors (__FILE__, __LINE__,
		// 	WEBTOOLS_WEBUTILITY_DECODEURL_FAILED);

		delete [] pLocalURLParameterValue;

		// return err;
		return errDecodeURL;
	}

	if (bParameterToBeDecrypted)
	{
		long			lDecryptedBufferLength;


		if ((lDecryptedBufferLength = Encrypt:: getDecryptedBufferLength (
			pURLParameterValue)) < 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ENCRYPT_GETDECRYPTEDBUFFERLENGTH_FAILED);

			delete [] pLocalURLParameterValue;

			return err;
		}

		if (lDecryptedBufferLength + 1 > ulBufferLength)
		{
			delete [] pLocalURLParameterValue;

			if ((pLocalURLParameterValue = new char [
				lDecryptedBufferLength + 1]) == (char *) NULL)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_NEW_FAILED);

				return err;
			}

			lDecryptedBufferLength++;
		}
		else
		{
			lDecryptedBufferLength		= ulBufferLength;
		}

		strcpy (pLocalURLParameterValue, "");
		if (Encrypt:: decrypt (pURLParameterValue, pLocalURLParameterValue,
			lDecryptedBufferLength) != 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ENCRYPT_DECRYPT_FAILED);

			delete [] pLocalURLParameterValue;

			return err;
		}

		if (strlen (pLocalURLParameterValue) >= ulBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
				2, ulBufferLength,
				(unsigned long) strlen (pLocalURLParameterValue));

			delete [] pLocalURLParameterValue;

			return err;
		}

		strcpy (pURLParameterValue, pLocalURLParameterValue);
	}

	delete [] pLocalURLParameterValue;


	return errNoError;
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	const char *pURLParameterValue,
	unsigned long ulTruncateEncodedValueIfBiggerThen)

{

	Buffer_t		bEncodedValue;
	Error_t			errEncodeURL;


	if (strchr ((const char *) (*pbURLParameters), '?') == (char *) NULL)
	{
		if (pbURLParameters -> append ("?") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			return err;
		}
	}
	else
	{
		if (pbURLParameters -> append ("&") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			return err;
		}
	}

	if (pbURLParameters -> append (pURLParameterName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}

	if (pbURLParameters -> append ("=") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}

	if (bEncodedValue. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if ((errEncodeURL = WebUtility:: encodeURL (
		pURLParameterValue, &bEncodedValue,
		ulTruncateEncodedValueIfBiggerThen)) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_ENCODEURL_FAILED,
			2, pURLParameterValue, (const char *) errEncodeURL);

		return err;
	}

	if (pbURLParameters -> append ((const char *) bEncodedValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	long lURLParameterValue)

{

	char			pURLParameterValue [
		WEBTOOLS_MAXENCODEDBUFFERLENGTH];


	sprintf (pURLParameterValue, "%ld", lURLParameterValue);

	return WebUtility:: addURLParameter (pbURLParameters,
		pURLParameterName, pURLParameterValue);
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	unsigned long ulURLParameterValue)

{

	char			pURLParameterValue [
		WEBTOOLS_MAXENCODEDBUFFERLENGTH];


	sprintf (pURLParameterValue, "%lu", ulURLParameterValue);

	return WebUtility:: addURLParameter (pbURLParameters,
		pURLParameterName, pURLParameterValue);
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	long long llURLParameterValue)

{

	char			pURLParameterValue [
		WEBTOOLS_MAXENCODEDBUFFERLENGTH];


	sprintf (pURLParameterValue, "%lld", llURLParameterValue);

	return WebUtility:: addURLParameter (pbURLParameters,
		pURLParameterName, pURLParameterValue);
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	unsigned long long ullURLParameterValue)

{

	char			pURLParameterValue [
		WEBTOOLS_MAXENCODEDBUFFERLENGTH];


	sprintf (pURLParameterValue, "%llu", ullURLParameterValue);

	return WebUtility:: addURLParameter (pbURLParameters,
		pURLParameterName, pURLParameterValue);
}


Error WebUtility:: addURLParameter (Buffer_p pbURLParameters,
	const char *pURLParameterName,
	float fURLParameterValue)

{

	char			pURLParameterValue [
		WEBTOOLS_MAXENCODEDBUFFERLENGTH];


	sprintf (pURLParameterValue, "%f", fURLParameterValue);

	return WebUtility:: addURLParameter (pbURLParameters,
		pURLParameterName, pURLParameterValue);
}


Error WebUtility:: readHttpHeaderAndBody (
	SocketImpl_p pClientSocketImpl,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingTimeoutInMicroSeconds,
	Buffer_p pbHttpHeader,
	Buffer_p pbHttpBody,
	HttpMethod_p phmHttpMethod,
	Buffer_p pbURL,
	Buffer_p pbHeaders,
	Buffer_p pbRequestUserAgent,
	Buffer_p pbCookie,
	Buffer_p pbSetCookie,
	Buffer_p pbLocation,
	chunkReadFunction chunkRead,
	void *pvObject)

{

	Error_t				errRead;
	Error_t				errParse;
	// char				pLocalHttpResponse [WEBTOOLS_MAXHTTPRESPONSELENGTH];
	char				pLocalHttpHeaderResponse [
		WEBTOOLS_MAXHTTPHEADERBUFFERSIZE];
	char				*pLocalHttpBodyResponse;
	unsigned long		ulBodyBufferSize;
	unsigned long		ulHTTPResponseHeaderLength;
	unsigned long		ulBufferLength;
	Boolean_t			bIsHeaderFinished;
	Boolean_t			bIsBodyFinished;
	#ifdef WIN32
		__int64					ullContentLength;
		__int64					ullCurrentBodyLength;
	#else
		unsigned long long		ullContentLength;
		unsigned long long		ullCurrentBodyLength;
	#endif
	Boolean_t			bContentLengthInitialized;
	Boolean_t			bReadingCheckToBePerformed;
	char				pRemoteAddress [SCK_MAXIPADDRESSLENGTH];
	long				lRemotePort;
	char				pLocalAddress [SCK_MAXIPADDRESSLENGTH];
	unsigned long		ulChunkReadIndex;
	Buffer_t			bContentType;



	if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
		SCK_MAXIPADDRESSLENGTH) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);

		return err;
	}

	if (pClientSocketImpl -> getRemotePort (&lRemotePort) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);

		return err;
	}

	if (pClientSocketImpl -> getLocalAddress (pLocalAddress,
		SCK_MAXIPADDRESSLENGTH) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETLOCALADDRESS_FAILED);

		return err;
	}

	if (pbHttpHeader -> setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	if (pbHttpBody != (Buffer_p) NULL)
	{
		if (pbHttpBody -> setBuffer ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	if (ulReceivingTimeoutInSeconds == 0 &&
		ulReceivingTimeoutInMicroSeconds == 0)
		bReadingCheckToBePerformed			= false;
	else
		bReadingCheckToBePerformed			= true;

	// read the header
	bIsHeaderFinished							= false;

	while (!bIsHeaderFinished)
	{
		ulBufferLength			= WEBTOOLS_MAXHTTPHEADERBUFFERSIZE;

		// peek the first junk
		if ((errRead = pClientSocketImpl -> read (
			pLocalHttpHeaderResponse, &ulBufferLength,
			bReadingCheckToBePerformed,
			ulReceivingTimeoutInSeconds,
			ulReceivingTimeoutInMicroSeconds,
			true, false)) != errNoError)
		{
			if ((long) errRead == SCK_NOTHINGTOREAD)
			{
				unsigned long	ulLocalReceivingTimeoutInSeconds;
				unsigned long	ulLocalReceivingTimeoutInMicroSeconds;


				if (ulReceivingTimeoutInSeconds == 0 &&
					ulReceivingTimeoutInMicroSeconds == 0)
				{
					if (pClientSocketImpl -> getReceivingTimeouts (
						&ulLocalReceivingTimeoutInSeconds,
						&ulLocalReceivingTimeoutInMicroSeconds) != errNoError)
					{
						// Error err = SocketErrors (__FILE__, __LINE__,
						// 	SCK_SOCKETIMPL_GETLOCALADDRESS_FAILED);

						// return err;
					}
				}
				else
				{
					ulLocalReceivingTimeoutInSeconds			=
						ulReceivingTimeoutInSeconds;
					ulLocalReceivingTimeoutInMicroSeconds		=
						ulReceivingTimeoutInMicroSeconds;
				}

				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED,
					5, ulLocalReceivingTimeoutInSeconds,
					ulLocalReceivingTimeoutInMicroSeconds,
					strcmp (pLocalAddress, "") ? pLocalAddress :
						"<Local IP address decided by the OS>",
					pRemoteAddress, lRemotePort);

				return err;
			}
			else
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_SOCKETIMPL_READ_FAILED);

				return errRead;
			}
		}

		{
			char				*pEndHeader;


			if ((pEndHeader = strstr (pLocalHttpHeaderResponse, "\r\n\r\n")) !=
				(char *) NULL)
			{
				ulHTTPResponseHeaderLength	=
					pEndHeader - pLocalHttpHeaderResponse;
				bIsHeaderFinished			= true;
			}
			else
			{
				ulHTTPResponseHeaderLength	= ulBufferLength;
			}
		}

		if (bIsHeaderFinished)
		{
			if (pbHttpHeader -> append (pLocalHttpHeaderResponse,
				ulHTTPResponseHeaderLength + 4) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			// vacuum just the header from the socket
			ulBufferLength		= ulHTTPResponseHeaderLength + 4;	// \r\n\r\n
			if ((errRead = pClientSocketImpl -> read (
				pLocalHttpHeaderResponse, &ulBufferLength, false, 0, 0,
				true, true)) != errNoError)
			{
				/*
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);
				*/

				return errRead;
			}
		}
		else
		{
			if (pbHttpHeader -> append (pLocalHttpHeaderResponse,
				ulHTTPResponseHeaderLength) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			// vacuum just the header from the socket
			ulBufferLength		= ulHTTPResponseHeaderLength;
			if ((errRead = pClientSocketImpl -> read (
				pLocalHttpHeaderResponse, &ulBufferLength, false, 0, 0,
				true, true)) != errNoError)
			{
				/*
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);
				*/

				return errRead;
			}
		}
	}

	// Note: pbHeaders could be also NULL because
	// it is not a mandatory parameter
	if ((errParse = WebUtility:: parseHttpHeader (
		pbHttpHeader,
		phmHttpMethod, pbURL, pbHeaders)) != errNoError)
	{
		/*
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);
		*/

		return errParse;
	}

	if (pbRequestUserAgent != (Buffer_p) NULL)
	{
		Error_t				errGetHeaderValue;


		if ((errGetHeaderValue = getHttpHeaderValue (
			(const char *) (*pbHttpHeader),
			"User-Agent", pbRequestUserAgent)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

			// return errGetHeaderValue;
		}
	}

	if (pbCookie != (Buffer_p) NULL)
	{
		Error_t				errGetHeaderValue;


		if ((errGetHeaderValue = getHttpHeaderValue (
			(const char *) (*pbHttpHeader),
			"Cookie", pbCookie)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

			// return errGetHeaderValue;
		}
	}

	if (pbSetCookie != (Buffer_p) NULL)
	{
		Error_t				errGetHeaderValue;
		Error_t				errNextToken;
		Buffer_t			bLocalSetCookie;
		StringTokenizer_t	stStringTokenizer;
		const char			*pToken;


		if (pbSetCookie -> setBuffer ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}

		if ((errGetHeaderValue = getHttpHeaderValue (
			(const char *) (*pbHttpHeader),
			"Set-Cookie", &bLocalSetCookie)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

			// return errGetHeaderValue;
		}
		else
		{
			if (stStringTokenizer. init ((const char *) bLocalSetCookie,
				-1, "; ") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_INIT_FAILED);

				return err;
			}

			do
			{
				if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
					errNoError)
				{
					if ((long) errNextToken ==
						TOOLS_STRINGTOKENIZER_NOMORETOKEN)
						continue;
					else
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

						if (stStringTokenizer. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_STRINGTOKENIZER_FINISH_FAILED);
						}

						return err;
					}
				}

				if (pbSetCookie -> append (pToken) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}

				if (pbSetCookie -> append ("; ") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			while (errNextToken == errNoError);

			if (stStringTokenizer. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_FINISH_FAILED);

				return err;
			}
		}
	}

	if (pbLocation != (Buffer_p) NULL)
	{
		Error_t				errGetHeaderValue;


		if ((errGetHeaderValue = getHttpHeaderValue (
			(const char *) (*pbHttpHeader),
			"Location", pbLocation)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

			// return errGetHeaderValue;
		}
	}

	{
		Error_t				errGetHeaderValue;


		if ((errGetHeaderValue = getHttpHeaderValue (
			(const char *) (*pbHttpHeader),
			"Content-Type", &bContentType)) != errNoError)
		{
			// Error err = WebToolsErrors (__FILE__, __LINE__,
			// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

			// return errGetHeaderValue;
		}
	}

	// Since a HTTP request could not have a body, this method try
	// to read it only if the pbHttpBody parameter is not NULL
	if ((pbHttpBody == (Buffer_p) NULL &&
		chunkRead == (chunkReadFunction) NULL) ||
		*phmHttpMethod == WEBTOOLS_WEBUTILITY_GET)
	{

		return errNoError;
	}

	// verify if the 'Content-Length' field is present.
	// Syntax: Content-Length: XXX
	{
		Buffer_t			bContentLength;


		if (bContentLength. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}

		if (getHttpHeaderValue ((const char *) (*pbHttpHeader),
			"Content-Length", &bContentLength) != errNoError)
		{
			bContentLengthInitialized		= false;
		}
		else
		{
			#ifdef WIN32
				ullContentLength			= _atoi64 (
					(const char *) bContentLength);
			#else
				ullContentLength			= atoll (
					(const char *) bContentLength);
			#endif

			bContentLengthInitialized		= true;
		}

		if (bContentLength. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);

			return err;
		}

		/*
		const char				*pBeginContentLengthField;
		unsigned long			ulStringLength;


		if ((pBeginContentLengthField = strstr (
			(const char *) (*pbHttpHeader), "Content-Length: ")) !=
			(char *) NULL)
		{
			char					pContentLength [WEBTOOLS_MAXLONGLENGTH];

			pBeginContentLengthField		= pBeginContentLengthField +
				strlen ("Content-Length: ");

			#ifdef WIN32
				// we will have for sure a '\r char
				ulStringLength		=
					strchr (pBeginContentLengthField, '\r') -
					pBeginContentLengthField;
			#else
				const char					*pEndContentLengthField;

				pEndContentLengthField			= pBeginContentLengthField;
				while (isdigit (*pEndContentLengthField))
					pEndContentLengthField++;
				ulStringLength		=
					pEndContentLengthField - pBeginContentLengthField;
			#endif

			if (ulStringLength >= WEBTOOLS_MAXLONGLENGTH)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_CONTENTLENGTHTOOLONG,
					1, (long) (ulStringLength));

				return err;
			}

			strncpy (pContentLength, pBeginContentLengthField,
				ulStringLength);
			pContentLength [ulStringLength]		= '\0';

			#ifdef WIN32
				ullContentLength				= _atoi64 (pContentLength);
			#else
				ullContentLength				= atoll (pContentLength);
			#endif

			bContentLengthInitialized		= true;
		}
		else
		{
			bContentLengthInitialized		= false;
		}
		*/
	}

	// read the body
	bIsBodyFinished							= false;

	ullCurrentBodyLength					= 0;

	// allocate the buffer for the Body using the ContentLength if present
	// and not overcome WEBTOOLS_MAXHTTPBODYBUFFERSIZE
	{
		ulBodyBufferSize			= 0;

		if (bContentLengthInitialized)
		{
			if (ullContentLength <= WEBTOOLS_MAXHTTPBODYBUFFERSIZE)
				ulBodyBufferSize		= ullContentLength + 1;
			else
				ulBodyBufferSize		= WEBTOOLS_MAXHTTPBODYBUFFERSIZE;
		}
		else
		{
			ulBodyBufferSize		= WEBTOOLS_MAXHTTPBODYBUFFERSIZE;
		}

		if ((pLocalHttpBodyResponse = new char [ulBodyBufferSize]) ==
			(char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_NEW_FAILED);

			return err;
		}
	}

	ulChunkReadIndex			= 0;

	while (!bIsBodyFinished)
	{
		if (bContentLengthInitialized)
		{
			if (ullCurrentBodyLength < ullContentLength)
			{
				ulBufferLength		= (unsigned long)
					(ullContentLength - ullCurrentBodyLength >
					ulBodyBufferSize ? ulBodyBufferSize :
					ullContentLength - ullCurrentBodyLength);
			}
			else
			{
				bIsBodyFinished		= true;

				continue;
			}
		}
		else
		{
			ulBufferLength			= ulBodyBufferSize;
		}

		if ((errRead = pClientSocketImpl -> read (
			pLocalHttpBodyResponse, &ulBufferLength,
			bReadingCheckToBePerformed, ulReceivingTimeoutInSeconds,
			ulReceivingTimeoutInMicroSeconds,
			true, true)) != errNoError)
		{
			if ((long) errRead == SCK_NOTHINGTOREAD)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_BODYTIMEOUTEXPIRED,
					5, ulReceivingTimeoutInSeconds,
					ulReceivingTimeoutInMicroSeconds,
					pLocalAddress, pRemoteAddress, lRemotePort);

				delete [] pLocalHttpBodyResponse;

				return err;
			}
			else if ((long) errRead == SCK_READ_EOFREACHED)
			{
				// in this case it returns EOFREACHED and we will have data
				// inside the buffer. At least we will have the header.

				delete [] pLocalHttpBodyResponse;

				return errRead;
			}
			else
			{
				/*
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);
				*/

				delete [] pLocalHttpBodyResponse;

				return errRead;
			}
		}

		if (chunkRead != (chunkReadFunction) NULL)
		{
			if ((*chunkRead) (pvObject, ulChunkReadIndex++,
				(long long) (bContentLengthInitialized ? ullContentLength : -1),
				(const char *) bContentType,
				(unsigned char *) pLocalHttpBodyResponse, ulBufferLength) !=
				errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_WEBUTILITY_CHUNKREADFAILED);

				delete [] pLocalHttpBodyResponse;

				return err;
			}
		}
		else if (pbHttpBody != (Buffer_p) NULL)
		{
			if (pbHttpBody -> append (pLocalHttpBodyResponse,
				ulBufferLength) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				delete [] pLocalHttpBodyResponse;

				return err;
			}
		}

		ullCurrentBodyLength				=
			ullCurrentBodyLength + ulBufferLength;
	}

	delete [] pLocalHttpBodyResponse;


	return errNoError;
}


/*
Error WebUtility:: readHttpHeaderAndBody (
	SocketImpl_p pClientSocketImpl,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingTimeoutInMicroSeconds,
	Buffer_p pbHttpHeader,
	Buffer_p pbHttpBody,
	HttpMethod_p phmHttpMethod,
	Buffer_p pbURL,
	Buffer_p pbHeaders)

{

	Error_t				errRead;
	Error_t				errParse;
	char				pLocalHttpResponse [WEBTOOLS_MAXHTTPRESPONSELENGTH];
	unsigned long		ulHTTPResponseHeaderLength;
	unsigned long		ulBufferLength;
	Boolean_t			bIsBodyFinished;
	#ifdef WIN32
		__int64					ullContentLength;
		__int64					ullCurrentBodyLength;
	#else
		unsigned long long		ullContentLength;
		unsigned long long		ullCurrentBodyLength;
	#endif
	Boolean_t			bContentLengthInitialized;


	if (pbHttpHeader -> setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	if (pbHttpBody != (Buffer_p) NULL)
	{
		if (pbHttpBody -> setBuffer ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}

	ulBufferLength			= WEBTOOLS_MAXHTTPRESPONSELENGTH;

	// peek the first junk
	if ((errRead = pClientSocketImpl -> read (
		pLocalHttpResponse, &ulBufferLength, true,
		ulReceivingTimeoutInSeconds,
		ulReceivingTimeoutInMicroSeconds,
		true, false)) != errNoError)
	{
		if ((long) errRead == SCK_NOTHINGTOREAD)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED,
				2, ulReceivingTimeoutInSeconds,
				ulReceivingTimeoutInMicroSeconds);

			return err;
		}
		else
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			// 	SCK_SOCKETIMPL_READ_FAILED);

			return errRead;
		}
	}

	{
		char				*pEndHeader;


		if ((pEndHeader = strstr (pLocalHttpResponse, "\r\n\r\n")) !=
			(char *) NULL)
		{
			ulHTTPResponseHeaderLength		= pEndHeader - pLocalHttpResponse;
		}
		else
		{
			ulHTTPResponseHeaderLength		= ulBufferLength;
		}
	}

	if (pbHttpHeader -> setBuffer (pLocalHttpResponse,
		ulHTTPResponseHeaderLength + 4) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	// vacuum just the header from the socket
	ulBufferLength			= ulHTTPResponseHeaderLength + 4;	// \r\n\r\n
	if ((errRead = pClientSocketImpl -> read (
		pLocalHttpResponse, &ulBufferLength, false, 0, 0,
		true, true)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		// 	SCK_SOCKETIMPL_READ_FAILED);

		return errRead;
	}

	if ((errParse = WebUtility:: parseHttpHeader (
		pbHttpHeader,
		phmHttpMethod, pbURL, pbHeaders)) != errNoError)
	{
		// Error err = WebToolsErrors (__FILE__, __LINE__,
		// 	WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED);

		return errParse;
	}

	// Since a HTTP request could not have a body, this method try
	// to read it only if the pbHttpBody parameter is not NULL
	if (pbHttpBody == (Buffer_p) NULL ||
		*phmHttpMethod == WEBTOOLS_WEBUTILITY_GET)
	{

		return errNoError;
	}

	// verify if the 'Content-Length' field is present.
	// Syntax: Content-Length: XXX
	{
		char					*pBeginContentLengthField;
		char					*pEndContentLengthField;


		if ((pBeginContentLengthField = strstr (
			(const char *) (*pbHttpHeader), "Content-Length: ")) !=
			(char *) NULL)
		{
			char			pContentLength [WEBTOOLS_MAXLONGLENGTH];


			pBeginContentLengthField		+=
				strlen ("Content-Length: ");

			pEndContentLengthField			= pBeginContentLengthField;
			while (isdigit (*pEndContentLengthField))
				pEndContentLengthField++;

			if (pEndContentLengthField - pBeginContentLengthField >=
				WEBTOOLS_MAXLONGLENGTH)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_CONTENTLENGTHTOOLONG,
					1,
					(long) (pEndContentLengthField - pBeginContentLengthField));

				return err;
			}

			strncpy (pContentLength, pBeginContentLengthField,
				pEndContentLengthField - pBeginContentLengthField);
			pContentLength [
				pEndContentLengthField - pBeginContentLengthField]		= '\0';

			#ifdef WIN32
				ullContentLength				= _atoi64 (pContentLength);
			#else
				ullContentLength				= atoll (pContentLength);
			#endif

			bContentLengthInitialized		= true;
		}
		else
		{
			bContentLengthInitialized		= false;
		}
	}

	// read the body
	bIsBodyFinished							= false;

	ullCurrentBodyLength					= 0;

	while (!bIsBodyFinished)
	{
		if (bContentLengthInitialized)
		{
			if (ullCurrentBodyLength < ullContentLength)
			{
				ulBufferLength		= (unsigned long)
					(ullContentLength - ullCurrentBodyLength >
					WEBTOOLS_MAXHTTPRESPONSELENGTH ?
					WEBTOOLS_MAXHTTPRESPONSELENGTH :
					ullContentLength - ullCurrentBodyLength);
			}
			else
			{
				bIsBodyFinished		= true;

				continue;
			}
		}
		else
		{
			ulBufferLength			= WEBTOOLS_MAXHTTPRESPONSELENGTH;
		}

		if ((errRead = pClientSocketImpl -> read (
			pLocalHttpResponse, &ulBufferLength,
			true, ulReceivingTimeoutInSeconds,
			ulReceivingTimeoutInMicroSeconds,
			true, true)) != errNoError)
		{
			if ((long) errRead == SCK_NOTHINGTOREAD)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_BODYTIMEOUTEXPIRED,
					2, ulReceivingTimeoutInSeconds,
					ulReceivingTimeoutInMicroSeconds);

				return err;
			}
			else if ((long) errRead == SCK_READ_EOFREACHED)
			{
				// in this case it returns EOFREACHED and we will have data
				// inside the buffer. At least we will have the header.

				return errRead;
			}
			else
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				// 	SCK_SOCKETIMPL_READ_FAILED);

				return errRead;
			}
		}

		if (pbHttpBody -> append (pLocalHttpResponse,
			ulBufferLength) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			return err;
		}

		ullCurrentBodyLength				=
			ullCurrentBodyLength + ulBufferLength;
	}


	return errNoError;
}
*/


Error WebUtility:: parseHttpHeader (
	Buffer_p pbHttpHeaderRequest,
	HttpMethod_p phmHttpMethod,
	Buffer_p pbURL,
	Buffer_p pbHeaders)

{
	const char				*pHeadersBegin;


	// The HTTP Header could be a request:
	// GET <URI> HTTP/1.1
	// <headers>
	//
	// or an HTTP response:
	// HTTP/1.1 200 OK
	// <headers>
	//
	// or an HTTP response:
	// RTSP/1.0 200 OK
	// <headers>
	//
	// Of course the pbURL and phmHttpMethod input parameter have sense
	// only in case it is an HTTP request

	if (strncmp ((const char *) (*pbHttpHeaderRequest),
		"HTTP/", 5) != 0 &&	// it is not HTTP/1.1 200 OK
		strncmp ((const char *) (*pbHttpHeaderRequest),
		"RTSP/", 5) != 0)	// it is not RTSP/1.0 200 OK
	{
		// The HTTP Header is a request

		const char				*pURLBegin;
		const char				*pURLEnd;

		// pURLEnd is found looking at " HTTP/1.1"
		if ((pURLBegin = strchr ((const char *) (*pbHttpHeaderRequest), ' ')) ==
			(char *) NULL ||
			(pURLEnd = strstr (pURLBegin + 1, " HTTP/")) == (char *) NULL)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPSERVER_WRONGHTTPHEADER,
				1, (const char *) (*pbHttpHeaderRequest));

			return err;
		}

		if (strncmp ((const char *) (*pbHttpHeaderRequest),
			WEBTOOLS_WEBUTILITY_GETSTRING,
			pURLBegin - (const char *) (*pbHttpHeaderRequest) - 1) == 0)
			*phmHttpMethod			= WEBTOOLS_WEBUTILITY_GET;
		else if (strncmp ((const char *) (*pbHttpHeaderRequest),
			WEBTOOLS_WEBUTILITY_POSTSTRING,
			pURLBegin - (const char *) (*pbHttpHeaderRequest) - 1) == 0)
			*phmHttpMethod			= WEBTOOLS_WEBUTILITY_POST;
		else
			*phmHttpMethod			= WEBTOOLS_WEBUTILITY_UNDEFINED;

		if (pbURL != (Buffer_p) NULL)
		{
			if (pbURL -> setBuffer (pURLBegin + 1,
				pURLEnd - pURLBegin - 1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}
		}
	}
	else
	{
		*phmHttpMethod			= WEBTOOLS_WEBUTILITY_UNDEFINED;
	}

	if (pbHeaders != (Buffer_p) NULL)
	{
		if ((pHeadersBegin = strchr ((const char *) (*pbHttpHeaderRequest),
			'\n')) == (char *) NULL)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_HTTPSERVER_WRONGHTTPHEADER,
				1, (const char *) (*pbHttpHeaderRequest));

			return err;
		}

		if (pbHeaders -> setBuffer (pHeadersBegin + 1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error WebUtility:: getHttpHeaderValue (
	const char *pHeaders,
	const char *pHeaderName,
	Buffer_p pbHeaderValue)

{

	StringTokenizer_t			stStringTokenizer;
	Error						errNextToken;
	const char					*pToken;
	unsigned long				ulHeaderNameLength;
	Boolean_t					bIsHeaderNameFound;


	if (pHeaders == (const char *) NULL ||
		pHeaderName == (const char *) NULL ||
		pbHeaderValue == (Buffer_p) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	bIsHeaderNameFound				= false;

	if (pbHeaderValue -> setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	ulHeaderNameLength			= strlen (pHeaderName);

	if (stStringTokenizer. init (pHeaders, -1, "\n") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_INIT_FAILED);

		return err;
	}

	do
	{
		if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
			errNoError)
		{
			if ((long) errNextToken == TOOLS_STRINGTOKENIZER_NOMORETOKEN)
				continue;
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stStringTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}
		}

		if (strlen (pToken) >= ulHeaderNameLength + 2 &&
			!strncmp (pToken, pHeaderName, ulHeaderNameLength) &&
			pToken [ulHeaderNameLength] == ':' &&
			pToken [ulHeaderNameLength + 1] == ' ')
		{
			long				lLength;
			const char			*pValue;


			pValue				= pToken + ulHeaderNameLength + 2;

			lLength				= strlen (pValue);
			if (lLength > 0 && pValue [lLength - 1] == '\r')
				lLength--;

			if (pbHeaderValue -> setBuffer (pValue, lLength) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				if (stStringTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				return err;
			}

			bIsHeaderNameFound				= true;

			break;
		}
	}
	while (errNextToken == errNoError);

	if (stStringTokenizer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_FINISH_FAILED);

		return err;
	}

	if (!bIsHeaderNameFound)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_HEADERNAMENOTFOUND,
			1, pHeaderName);

		return err;
	} 


	return errNoError;
}


Error WebUtility:: encodeURL (const char *pURL,
	Buffer_p pbEncodedURL,
	unsigned long ulTruncateEncodedValueIfBiggerThen,
	Boolean_t bAsciiInCaseOfSpace)

{

	unsigned long			ulURLLength;
	unsigned long			ulURLIndex;
	// unsigned long			ulURLEncodedIndex;
	char					pEncodedChar [64];


	if (pURL == (const char *) NULL ||
		pbEncodedURL == (Buffer_p) NULL)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (pbEncodedURL -> setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	ulURLLength					= strlen (pURL);
	// ulURLEncodedIndex			= 0;

	for (ulURLIndex = 0; ulURLIndex < ulURLLength; ulURLIndex++)
	{
		if (isalnum (pURL [ulURLIndex]) ||
			pURL [ulURLIndex] == '.' ||
			pURL [ulURLIndex] == '-' ||
			pURL [ulURLIndex] == '/' ||
			pURL [ulURLIndex] == '\\' ||
			pURL [ulURLIndex] == '_' ||
			pURL [ulURLIndex] == ':')
		{
			// valid character

			if (ulTruncateEncodedValueIfBiggerThen != -1 &&
				((unsigned long) (*pbEncodedURL)) + 1 >
					ulTruncateEncodedValueIfBiggerThen)
				break;

			if (pbEncodedURL -> append (pURL [ulURLIndex]) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}
		}
		else if (pURL [ulURLIndex] == ' ')
		{
			// The ' ' (space), according the rfc, can be converted as %20
			// as well as '+'

			if (ulTruncateEncodedValueIfBiggerThen != -1 &&
				((unsigned long) (*pbEncodedURL)) + 1 >
					ulTruncateEncodedValueIfBiggerThen)
				break;

			if (bAsciiInCaseOfSpace)
			{
				if (pbEncodedURL -> append ("%20") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else
			{
				if (pbEncodedURL -> append ('+') != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
		}
		else
		{
			/*
			pEncodedURL [ulURLEncodedIndex++]		= '%';
			sprintf (&(pEncodedURL [ulURLEncodedIndex]), "%02X",
				pURL [ulURLIndex]);
			ulURLEncodedIndex			+= 2;
			*/

			if (ulTruncateEncodedValueIfBiggerThen != -1 &&
				((unsigned long) (*pbEncodedURL)) + 3 >
					ulTruncateEncodedValueIfBiggerThen)
				break;

			sprintf (pEncodedChar, "%%%02X", pURL [ulURLIndex]);

			// std::cout << pEncodedChar << std::endl;

			// the char Ç generates in pEncodedChar
			// 	%FFFFFFCC and %FFFFFFA7
			// but the encoding should be just: %CC%A7
			// (verified using wget)
			// That should be a generic rule, I tested and it works also
			// for ö and ü
			// So:
			if (strlen (pEncodedChar) > 7 &&
				!strncmp (pEncodedChar, "%FFFFFF", 7))
			{
				int			iCurrentIndex;
				for (iCurrentIndex = 1;
					iCurrentIndex + 7 - 1 < strlen (pEncodedChar);
					iCurrentIndex++)
					pEncodedChar [iCurrentIndex]	=
						pEncodedChar [iCurrentIndex + 7 - 1];
				pEncodedChar [iCurrentIndex]		= '\0';
			}

			// std::cout << pEncodedChar << std::endl;

			if (pbEncodedURL -> append (pEncodedChar) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error WebUtility:: decodeURL (const char *pURL,
	char *pDecodedURL, unsigned long ulDecodedURLBufferLength,
	Buffer_p pbDecodedURL)

{

	const char				*pRemainingURLToDecode;
	const char				*pEncodedChar;
	long					lCurrentDecodedLength;
	char					pEncodedValue [3];
	long					lEncodedValue;
	char					pDecodedChar [3];



	if (pURL == (const char *) NULL ||
		(pDecodedURL == (char *) NULL && pbDecodedURL == (Buffer_p) NULL))
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (pbDecodedURL != (Buffer_p) NULL)
	{
		if (pbDecodedURL -> setBuffer ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}
	else
	{
		memset ((void *) pDecodedURL, 0, ulDecodedURLBufferLength);
	}

	pRemainingURLToDecode			= pURL;

	while ((pEncodedChar = strchr (pRemainingURLToDecode, '%')) !=
		(char *) NULL)
	{
		if (pbDecodedURL != (Buffer_p) NULL)
		{
			if (pbDecodedURL -> append (pRemainingURLToDecode,
				pEncodedChar - pRemainingURLToDecode) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}
		}
		else
		{
			if (pEncodedChar - pRemainingURLToDecode >=
				ulDecodedURLBufferLength - strlen (pDecodedURL))
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
					2, ulDecodedURLBufferLength,
					(unsigned long) (pEncodedChar - pRemainingURLToDecode));

				return err;
			}

			lCurrentDecodedLength		= strlen (pDecodedURL);

			strncat (pDecodedURL, pRemainingURLToDecode,
				pEncodedChar - pRemainingURLToDecode);
			pDecodedURL [lCurrentDecodedLength +
				(pEncodedChar - pRemainingURLToDecode)]		= '\0';
		}

		if (strlen (pEncodedChar) >= 2 &&
			isxdigit (pEncodedChar [1]) &&
			isxdigit (pEncodedChar [2]))
		{
			pEncodedValue [0]			= pEncodedChar [1];
			pEncodedValue [1]			= pEncodedChar [2];
			pEncodedValue [2]			= '\0';
			sscanf (pEncodedValue, "%lX", &lEncodedValue);

			pDecodedChar [0]		= (char) lEncodedValue;
			pDecodedChar [1]		= '\0';

			if (pbDecodedURL != (Buffer_p) NULL)
			{
				if (pbDecodedURL -> append (pDecodedChar) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					return err;
				}
			}
			else
			{
				if (strlen (pDecodedChar) >=
					ulDecodedURLBufferLength - strlen (pDecodedURL))
				{
					Error err = WebToolsErrors (__FILE__, __LINE__,
						WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
						2, ulDecodedURLBufferLength,
						(unsigned long) strlen (pDecodedChar));

					return err;
				}

				strcat (pDecodedURL, pDecodedChar);
			}

			pRemainingURLToDecode			= pEncodedChar + 3;
		}
		else
		{
			// we should never find a '%' that it is not followed by two digits.
			// Also if the char is %, it will be encoded in %25
			// See the http://netzreport.googlepages.com/online_tool_for_url_en_decoding.html
			// site for encoding/decoding

			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_WRONGFORMATFORURLTOBEDECODED,
				1, pURL);

			return err;
		}
	}

	if (pbDecodedURL != (Buffer_p) NULL)
	{
		if (pbDecodedURL -> append (pRemainingURLToDecode) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			return err;
		}
	}
	else
	{
		if (strlen (pRemainingURLToDecode) >=
			ulDecodedURLBufferLength - strlen (pDecodedURL))
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
				2, ulDecodedURLBufferLength,
				(unsigned long) strlen (pRemainingURLToDecode));

			return err;
		}

		strcat (pDecodedURL, pRemainingURLToDecode);
	}

	// In case there was a '+', it was not touched by the decode
	// but it has to be changed to ' ' (the encodeURL does the opposite)
	if (pbDecodedURL != (Buffer_p) NULL)
	{
		if (pbDecodedURL -> substitute ("+", " ") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);

			return err;
		}
	}
	else
	{
		char			*pPlusChar;


		pPlusChar			= pDecodedURL;

		while ((pPlusChar = strchr (pPlusChar, '+')) != (char *) NULL)
			*pPlusChar		= ' ';
	}
	

	return errNoError;
}

