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
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


typedef struct MyStruct
{
	long			lMyLong;
	char			pMyString [100];
} MyStruct_t, *MyStruct_p;


int main (int argc, char **argv)

{

	ClientSocket_t				csClientSocket;
	char						*pRemoteIpAddress;
	char						*pMessage;
	long						lRemotePort;
	SocketImpl_p				pSocketImpl;
	MyStruct_t					msMyStruct;
	unsigned long				ulClientIndex;
	unsigned long				ulClientsNumber;




	{
		const char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by CatraSoftware\nemail                : catrasoftware-support@catrasoftware.it\n***************************************************************************\n";

		std:: cout << pCopyright << std:: endl;
	}

	if (argc != 4)
	{
		std:: cout << "Usage: ./client <ip address> <port number> <message>" << std:: endl;

		return 1;
	}

	pRemoteIpAddress			= argv [1];
	lRemotePort			= atoi (argv [2]);
	pMessage			= argv [3];

	ulClientsNumber				= 2;

	for (ulClientIndex = 0; ulClientIndex < ulClientsNumber;
		ulClientIndex++)
	{
		std:: cout << "Starting of connection no.: " << (ulClientIndex + 1) << std:: endl;

		if (csClientSocket. init (SocketImpl:: STREAM, 2, 0, 2, 0, 2, true,
			(const char *) NULL, pRemoteIpAddress, lRemotePort) !=
			errNoError)
		{
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

		strcat (pMessage, "\n");

		std:: cout << "Write message: " << pMessage << std:: endl;

		if (pSocketImpl -> writeString (pMessage, true, 2, 0) !=
			errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		msMyStruct. lMyLong				= 120;
		strcpy (msMyStruct. pMyString, "pippo");

		std:: cout << "Write structure: " << msMyStruct. lMyLong
			<< " - " << msMyStruct. pMyString << std:: endl;

		if (pSocketImpl -> write (&msMyStruct, sizeof (MyStruct_t),
			true, 2, 0) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITE_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		/*
		std:: cout << "inizio sleep" << std:: endl;
		#if defined(WIN32)
			Sleep (60 * 1000);
		#else
			sleep (60);
		#endif
		std:: cout << "fine sleep" << std:: endl;
		*/

		std:: cout << "Finishing of connection no.: " << (ulClientIndex + 1) << std:: endl;

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	std:: cout << "inizio sleep" << std:: endl;
	#if defined(WIN32)
		Sleep (60 * 1000);
	#else
		sleep (60);
	#endif
	std:: cout << "fine sleep" << std:: endl;


	return 0;
}

