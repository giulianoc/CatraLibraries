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


#include "WebToolsErrors.h"


ErrMsgBase:: ErrMsgsInfo WebToolsErrorsStr = {

	// HttpServer
	{ WEBTOOLS_HTTPSERVER_INIT_FAILED,
		"The init function of the HttpServer class failed" },
	{ WEBTOOLS_HTTPSERVER_FINISH_FAILED,
		"The finish function of the HttpServer class failed" },
	{ WEBTOOLS_HTTPSERVER_GETSHUTDOWN_FAILED,
		"The getShutdown function of the HttpServer class failed" },
	{ WEBTOOLS_HTTPSERVER_SETSHUTDOWN_FAILED,
		"The setShutdown function of the HttpServer class failed" },
	{ WEBTOOLS_HTTPSERVER_WRONGHTTPHEADER,
		"Wrong HTTP header: %s" },

	// HttpGetThread
	{ WEBTOOLS_HTTPGETTHREAD_INIT_FAILED,
		"The init function of the HttpGetThread class failed" },
	{ WEBTOOLS_HTTPGETTHREAD_FINISH_FAILED,
		"The finish function of the HttpGetThread class failed" },
	{ WEBTOOLS_HTTPGETTHREAD_GETHTTPRESPONSE_FAILED,
		"The getHttpResponse function of the HttpGetThread class failed" },
	{ WEBTOOLS_HTTPGETTHREAD_CLOSINGHTTPGET_FAILED,
		"The closingHTTPGet function of the HttpGetThread class failed" },
	{ WEBTOOLS_HTTPGETTHREAD_TIMEOUTEXPIRED,
		"Timeout (%lu seconds) expired to get the response" },
	{ WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED,
		"Timeout (%lu seconds, %lu micro seconds) expired to get the header response. SrcIP: %s, DestIP: %s, Dest. port: %ld" },
	{ WEBTOOLS_HTTPGETTHREAD_BODYTIMEOUTEXPIRED,
		"Timeout (%lu seconds, %lu micro seconds) expired to get the body response. SrcIP: %s, DestIP: %s, Dest. port: %ld" },
	{ WEBTOOLS_HTTPGETTHREAD_CONTENTLENGTHTOOLONG,
		"HTTP 'Content-Length' field too long (%lu)" },

	// HttpPostThread
	{ WEBTOOLS_HTTPPOSTTHREAD_INIT_FAILED,
		"The init function of the HttpPostThread class failed" },
	{ WEBTOOLS_HTTPPOSTTHREAD_FINISH_FAILED,
		"The finish function of the HttpPostThread class failed" },
	{ WEBTOOLS_HTTPPOSTTHREAD_GETHTTPRESPONSE_FAILED,
		"The getHttpResponse function of the HttpPostThread class failed" },
	{ WEBTOOLS_HTTPPOSTTHREAD_CLOSINGHTTPPOST_FAILED,
		"The closingHTTPPost function of the HttpPostThread class failed" },
	{ WEBTOOLS_HTTPPOSTTHREAD_TIMEOUTEXPIRED,
		"Timeout (%lu seconds) expired to get the response" },
	{ WEBTOOLS_HTTPPOSTTHREAD_HEADERTIMEOUTEXPIRED,
		"Timeout (%lu seconds, %lu micro seconds) expired to get the header response" },
	{ WEBTOOLS_HTTPPOSTTHREAD_BODYTIMEOUTEXPIRED,
		"Timeout (%lu seconds, %lu micro seconds) expired to get the body response" },
	{ WEBTOOLS_HTTPPOSTTHREAD_CONTENTLENGTHTOOLONG,
		"HTTP 'Content-Length' field too long (%lu)" },

	// SaveHttpGetThread
	{ WEBTOOLS_SAVEHTTPGETTHREAD_INIT_FAILED,
		"The init function of the SaveHttpGetThread class failed" },
	{ WEBTOOLS_SAVEHTTPGETTHREAD_FINISH_FAILED,
		"The finish function of the SaveHttpGetThread class failed" },
	{ WEBTOOLS_SAVEHTTPGETTHREAD_UNKNOWNCONTENTTYPE,
		"Unknown content type: %s" },

	// WebUtility
	{ WEBTOOLS_WEBUTILITY_PARSEURL_FAILED,
		"The parseURL function of the WebUtility class failed. URL: %s" },
	{ WEBTOOLS_WEBUTILITY_GETURLPARAMETERVALUE_FAILED,
		"The getURLParameterValue function of the WebUtility class failed. URL parameter name: %s. URL: %s" },
	{ WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED,
		"The readHttpHeaderAndBody function of the WebUtility class failed" },
	{ WEBTOOLS_WEBUTILITY_PARSEHTTPHEADER_FAILED,
		"The parseHttpHeader function of the WebUtility class failed" },
	{ WEBTOOLS_WEBUTILITY_GETHTTPHEADERVALUE_FAILED,
		"The getHttpHeaderValue function of the WebUtility class failed" },
	{ WEBTOOLS_WEBUTILITY_HEADERNAMENOTFOUND,
		"The HeaderName was not found. HeaderName: %s" },
	{ WEBTOOLS_WEBUTILITY_ENCODEURL_FAILED,
		"The encodeURL function of the WebUtility class failed. URL: %s, Error: %s" },
	{ WEBTOOLS_WEBUTILITY_CHUNKREADFAILED,
		"The chunkRead function failed" },
	{ WEBTOOLS_WEBUTILITY_ADDURLPARAMETER_FAILED,
		"The addURLParameter function of the WebUtility class failed" },
	{ WEBTOOLS_WEBUTILITY_DECODEURL_FAILED,
		"The decodeURL function of the WebUtility class failed" },
	{ WEBTOOLS_WEBUTILITY_BUFFERTOOSHORT,
		"The buffer is too short (%lu) to copy %lu characters" },
	{ WEBTOOLS_WEBUTILITY_WRONGFORMATFORURLTOBEDECODED,
		"Wrong format for URL to be decoded. URL to be decoded: %s" },

	// common
	{ WEBTOOLS_NEW_FAILED,
		"The new function failed" },
	{ WEBTOOLS_ACTIVATION_WRONG,
		"Activation wrong" }

	// Insert here other errors...

} ;

