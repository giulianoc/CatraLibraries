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

#include <stdlib.h>
#if defined(WIN32)
#else
	#include <unistd.h>
#endif
#include <iostream>
#include "ServerSocket.h"
#include "ClientSocket.h"


int main (int argc, char **argv)

{

	long						lRemotePort;
	char						*pTracerIpAddress;
	char						*pCommand;
	char						*pParameters;
	ClientSocket_t				csClientSocket;
	SocketImpl_p				pSocketImpl;


	{
		const char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by Giuliano Catrambone\nemail                : giuliano.catrambone@catrasoftware.it\n***************************************************************************\n";

		std:: cout << pCopyright << std:: endl;
	}

	if (argc < 4)
	{
		std:: cout << "Usage: " << argv [0]
			<< " <tracer ip address> <tracer port number> <command> [<parameters>]"
			<< std:: endl
			<< "command: flushTraceFileCache           parameters: " << std:: endl
			<< "command: setMaxTraceFileSize           parameters: <lMaxTraceFileSize>" << std:: endl
			<< "command: setCompressedTraceFile        parameters: [0 | 1]" << std:: endl
			<< "command: setTraceFilesNumberToMaintain parameters: <lTraceFilesNumberToMaintain>" << std:: endl
			<< "command: setTraceOnFile                parameters: [0 | 1]" << std:: endl
			<< "command: setTraceOnTTY                 parameters: [0 | 1]" << std:: endl
			<< "command: setTraceLevel                 parameters: <lTraceLevel>" << std:: endl;

		return 1;
	}

	pTracerIpAddress	= argv [1];
	lRemotePort			= atoi (argv [2]);
	pCommand			= argv [3];

	if (argc == 4)
		pParameters			= (char *) NULL;
	else
		pParameters			= argv [4];

	if (csClientSocket. init (SocketImpl:: STREAM, 2, 0, 2, 0, 2, true,
		"127.0.0.1", pTracerIpAddress, lRemotePort) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED,
			2, pTracerIpAddress, lRemotePort);
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

	if (pSocketImpl -> writeString (pCommand,
		true, 2, 0) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pSocketImpl -> writeString ("\n", true, 2, 0) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pParameters != (char *) NULL)
	{
		if (pSocketImpl -> writeString (pParameters,
			true, 2, 0) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pSocketImpl -> writeString ("\n", true, 2, 0) !=
			errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (csClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

