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

#include <iostream>
#include <stdlib.h>
#include "ServerSocket.h"
#include "ClientSocket.h"
#include "Buffer.h"
#include "WebUtility.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


int main (int argc, char **argv)

{

	ClientSocket_t				csClientSocket;
	char						*pRemoteIpAddress;
	long						lRemotePort;
	char						*pHeaderPathName;
	Buffer_t					bHeader;
	Buffer_t					bBody;
	char						*pBodyPathName;
	SocketImpl_p				pSocketImpl;
	Error_t						errInit;
	Error_t						errRead;
	WebUtility:: HttpMethod_t		hmHttpMethod;



	{
		// char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by CatraSoftware\nemail                : catrasoftware-support@catrasoftware.it\n***************************************************************************\n";

		// std:: cout << pCopyright << std:: endl;
	}

	if (argc != 5)
	{
		std:: cout << "Usage: " << argv [0] << " <ip address> <port number> <headerPathName> <bodyPathName>" << std:: endl;

		return 1;
	}

	pRemoteIpAddress			= argv [1];
	lRemotePort					= atoi (argv [2]);
	pHeaderPathName				= argv [3];
	pBodyPathName				= argv [4];


	if (bHeader. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bBody. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bHeader. readBufferFromFile (pHeaderPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (strstr ((const char *) bHeader, "\r\n") == (char *) NULL)
	{
		if (bHeader. substitute ("\n", "\r\n") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (bBody. readBufferFromFile (pBodyPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (bHeader. substitute ("@BODYLENGTH@",
		((unsigned long) bBody) - 1) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if ((errInit = csClientSocket. init (SocketImpl:: STREAM,
		2, 0, 2, 0, 2, true,
		(const char *) NULL,
		pRemoteIpAddress, lRemotePort)) != errNoError)
	{
		std:: cout << (const char *) errInit << std:: endl;
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED,
			2, pRemoteIpAddress, lRemotePort);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (csClientSocket. getSocketImpl (&pSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pSocketImpl -> writeString ((const char *) bHeader, true, 2, 0) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pSocketImpl -> writeString ((const char *) bBody, true, 2, 0) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if ((errRead = WebUtility:: readHttpHeaderAndBody (
		pSocketImpl,
		30, // _ulReceivingTimeoutInSeconds,
		0, // _ulReceivingTimeoutInMS,
		&bHeader,
		&bBody,
		&hmHttpMethod,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		(Buffer_p) NULL,
		(Buffer_p) NULL		// Location
	)) != errNoError)
	{
		if ((unsigned long) errRead != SCK_READ_EOFREACHED)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << (const char *) errRead << std:: endl;
	}

	std:: cout << "HEADER: '"
		<< (const char *) bHeader << "'" << std:: endl << std:: endl;

	std:: cout << "BODY: '"
		<< (const char *) bBody << "'" << std:: endl << std:: endl;

	if (csClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

