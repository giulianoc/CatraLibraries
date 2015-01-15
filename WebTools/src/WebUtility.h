
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


#ifndef WebUtility_h
	#define WebUtility_h

	#include "Buffer.h"
	#include "WebToolsErrors.h"
	#include "SocketImpl.h"

	// #define WEBTOOLS_MAXHTTPRESPONSELENGTH		(1024 * 30)
	#define WEBTOOLS_MAXHTTPHEADERBUFFERSIZE		(1024 * 2)
	#define WEBTOOLS_MAXHTTPBODYBUFFERSIZE			(1024 * 4)
	#define WEBTOOLS_MAXLONGLENGTH					64
	#define WEBTOOLS_WEBUTILITY_GETSTRING			"GET"
	#define WEBTOOLS_WEBUTILITY_POSTSTRING			"POST"
	#define WEBTOOLS_MAXENCODEDBUFFERLENGTH			(1024 * 1)

	typedef class WebUtility {

		public:
			typedef enum HttpMethod {
				WEBTOOLS_WEBUTILITY_GET,
				WEBTOOLS_WEBUTILITY_POST,
				WEBTOOLS_WEBUTILITY_UNDEFINED
			} HttpMethod_t, *HttpMethod_p;

			typedef Error_t (*chunkReadFunction) (void *pvObject,
				unsigned long ulChunkReadIndex, 
				long long llContentLength, const char *pContentType,
				unsigned char *pucBuffer, unsigned long ulBufferDataSize);

			/**
				This method parses the pURL parameter and returns
				the IP address, the port (if it is specified, -1 otherwise),
				the relative path without parameters and
				the parameters.
				The pbRelativePathWithoutParameters and
				pbURLParameters parameters could be NULL and in this case
				the method will not initialize them. In case the Buffer are not 
				NULL, they have to be already initialized.
			*/
			static Error parseURL (const char *pURL,
				char *pServer, unsigned long ulServerBufferLength,
				long *plRTSPPort,
				Buffer_p pbRelativePathWithoutParameters = (Buffer_p) NULL,
				Buffer_p pbURLParameters = (Buffer_p) NULL);

			/**
				This method retrieves the value of a parameter from an URL
			*/
			static Error getURLParameterValue (const char *pURL,
				const char *pURLParameterName,
				char *pURLParameterValue, unsigned long ulBufferLength,
				Boolean_t bParameterToBeDecrypted = false);

			/**
				This method add a parameter inside the URL.

				ulTruncateEncodedValueIfBiggerThen:
				The value of the encoded pURLParameterValue could be
				up to the double of the pURLParameterValue length.
				ulTruncateEncodedValueIfBiggerThen is used in case we want
				to set a cap to this length
				-1 means no cap

			*/
			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				const char *pURLParameterValue,
				unsigned long ulTruncateEncodedValueIfBiggerThen = -1);

			/**
				This method add a parameter inside the URL
			*/
			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				long lURLParameterValue);

			/**
				This method add a parameter inside the URL
			*/
			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				unsigned long ulURLParameterValue);

			/**
				This method add a parameter inside the URL
			*/
			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				long long llURLParameterValue);

			/**
				This method add a parameter inside the URL
			*/
			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				unsigned long long ullURLParameterValue);

			static Error addURLParameter (Buffer_p pbURLParameters,
				const char *pURLParameterName,
				float fURLParameterValue);

			/**
				Read an HTTP header and body and initialize
				the buffers pbHttpHeader and pbHttpBody.

				Since a HTTP request could not have a body, this method try
				to read it only if the pbHttpBody parameter
				is not NULL. Therefore the pbHttpBody parameter
				could be NULL.
				Besides, pbURL and pbHeaders could be NULL and in this case
				will not be initialized.

				if the chunkRead function parameter is not null,
					this function is called every time a piece of body is read
					and, in this case, pbHttpBody is not used

				If ulReceivingTimeoutInSeconds and
				ulReceivingTimeoutInMicroSeconds are both zero,
				the internal SocketImpl:: read method will not perform any
				check if there is something to read before reading.

				REMARK: if this method returns the EOFREACHED error and
					the 'Content-Length' HTTP header is not present,
					it does not mean necessarily that it does not read any data
			*/
			static Error readHttpHeaderAndBody (
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
				chunkReadFunction chunkRead = (chunkReadFunction) NULL,
				void *pvObject = (void *) NULL);

			/**
				This method parses the HTTP header and returns
				the method (GET, POST, UNDEFINED), URL and headers

				The pbURL and pbHeaders parameters could be NULL and
				in this case the method will not initialize them
			*/
			static Error parseHttpHeader (
				Buffer_p pbHttpHeaderRequest,
				HttpMethod_p phmHttpMethod,
				Buffer_p pbURL,
				Buffer_p pbHeaders);

			static Error getHttpHeaderValue (
				const char *pHeaders,
				const char *pHeaderName,
				Buffer_p pbHeaderValue);

			/**
				This method encodes an URL in order to be passed as a parameter
				value of a URL.
				If you use the java language, the encoded buffer
				have to be decoded using the following java code:
				sDecodedURL = java.net.URLDecoder.decode(sEncodedURL, "UTF-8");
			*/
			static Error encodeURL (const char *pURL,
				Buffer_p pbEncodedURL,
				unsigned long ulTruncateEncodedValueIfBiggerThen = -1,
				Boolean_t bAsciiInCaseOfSpace = false);

			/**
			 * This method decodes a URL filling pbDecodedURL if not null
			 * or pDecodedURL otherwise
			*/
			static Error decodeURL (const char *pURL,
				char *pDecodedURL, unsigned long ulDecodedURLBufferLength,
				Buffer_p pbDecodedURL = (Buffer_p) NULL);

	} WebUtility_t, *WebUtility_p;

#endif

