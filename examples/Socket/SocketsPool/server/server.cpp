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
#include "CustomSocketsPool.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif
#include <errno.h>


#define SRV_MAXBUFFER			1024

typedef struct MyStruct
{
	long            lMyLong;
	char            pMyString [100];
} MyStruct_t, *MyStruct_p;


int main (int argc, char **argv)

{

	long						lLocalPort1;
	long						lLocalPort2;
	ServerSocket_t				ssServerSocket1;
	const char						*pServerSocket1	=
		"Server Socket 1";
	ServerSocket_t				ssServerSocket2;
	const char						*pServerSocket2	=
		"Server Socket 2";

	ClientSocket_t				csClientSocket;
	char						pRemoteAddress [SCK_MAXIPADDRESSLENGTH];
	long						lRemotePort;
	SocketImpl_p				pClientSocketImpl;
	SocketImpl_p				pServerSocketImpl;
	long						lMaxClients;
	char						pBuffer [SRV_MAXBUFFER + 1];
	Error						errAcceptConnection;
	MyStruct_t					msMyStruct;
	unsigned long				ulCharsRead;
	unsigned long				ulBufferLength;
	CustomSocketsPool_t			spSocketsPool;
	Error_t						errSocketsStatus;
	unsigned long				ulMaxSocketsNumber;



	ulMaxSocketsNumber				= 100;

	std:: cout << "FD_SETSIZE: " << (long) FD_SETSIZE << std:: endl;

	{
		const char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by CatraSoftware\nemail                : catrasoftware-support@catrasoftware.it\n***************************************************************************\n";

		std:: cout << pCopyright << std:: endl;
	}

	if (argc != 3)
	{
		std:: cout << "Usage: ./server <port number 1> <port number 2>"
			<< std:: endl;

		return 1;
	}

	lLocalPort1			= atoi (argv [1]);
	lLocalPort2			= atoi (argv [2]);

	lMaxClients			= 10;

	if (ssServerSocket1. init ((const char *) NULL, lLocalPort1, true,
		SocketImpl:: STREAM, 10, 0, lMaxClients) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket1. getSocketImpl (&pServerSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pServerSocketImpl -> setBlocking (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETBLOCKING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket2. init ((const char *) NULL, lLocalPort2, true,
		SocketImpl:: STREAM, 10, 0, lMaxClients) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket2. getSocketImpl (&pServerSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (pServerSocketImpl -> setBlocking (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETBLOCKING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (spSocketsPool. init (ulMaxSocketsNumber, 0, 0, true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (spSocketsPool. addSocket (
		SocketsPool:: SOCKETSTATUS_READ |
		SocketsPool:: SOCKETSTATUS_EXCEPTION,
		1, &ssServerSocket1, (char *) pServerSocket1) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (spSocketsPool. addSocket (
		SocketsPool:: SOCKETSTATUS_READ |
		SocketsPool:: SOCKETSTATUS_EXCEPTION,
		2, &ssServerSocket2, (char *) pServerSocket2) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "Waiting on " << lLocalPort1 << " port" << std:: endl;

	if ((errSocketsStatus = spSocketsPool. checkSocketsStatus (
		60, 0)) != errNoError)
	{
		if ((long) errSocketsStatus == SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED)
		{
			std:: cout << "No connection received" << std:: endl;

			return 1;
		}
		else
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_CHECKSOCKETSSTATUS_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}


	// server socket 2 management
	{
		if (csClientSocket. init () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if ((errAcceptConnection = ssServerSocket2. acceptConnection (
			&csClientSocket)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (csClientSocket. getSocketImpl (&pClientSocketImpl) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
			SCK_MAXIPADDRESSLENGTH) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pClientSocketImpl -> getRemotePort (&lRemotePort) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Arrived connection. " << "Remote address: "
			<< pRemoteAddress << ", remote port: " << lRemotePort
			<< std:: endl;

		{
			Boolean_t				bIsReadyToRead;


			bIsReadyToRead				= false;

			while (!bIsReadyToRead)
			{
				if (pClientSocketImpl -> isReadyForReading (&bIsReadyToRead,
					2, 0) != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
					std:: cout << (const char *) err << std:: endl;

					return 1;
				}
			}
		}

		if (pClientSocketImpl -> readLine (pBuffer,
			SRV_MAXBUFFER, &ulCharsRead, 2, 0) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_READLINE_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Message: " << pBuffer << std:: endl;

		{
			Boolean_t				bIsReadyToRead;


			bIsReadyToRead				= false;

			while (!bIsReadyToRead)
			{
				if (pClientSocketImpl -> isReadyForReading (&bIsReadyToRead,
					2, 0) != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
					std:: cout << (const char *) err << std:: endl;

					return 1;
				}
			}
		}

		ulBufferLength			= sizeof (MyStruct);

		if (pClientSocketImpl -> read (&msMyStruct,
			&ulBufferLength, false, 0, 0, true) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_READ_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "READ: " << msMyStruct. lMyLong << " "
			<< msMyStruct. pMyString << std:: endl;

		{
			char			pChar [2];
			Error			errRead;


			ulBufferLength			= 1;

			if ((errRead = pClientSocketImpl -> read (pChar,
				&ulBufferLength, true, 2, 0, true)) != errNoError)
			{
				// se il client, dopo aver scritto sul socket fa shutdown
				// la read sul socket ritorna come errore SCK_READ_EOFREACHED.
				// Da questo errore si puo' capire quindi che la connessione
				// con il client e' andata giu'
				std:: cout << (const char *) errRead << std:: endl;
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);
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
	}



	// server socket 1 management
	{
		if (csClientSocket. init () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if ((errAcceptConnection = ssServerSocket1. acceptConnection (
			&csClientSocket)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (csClientSocket. getSocketImpl (&pClientSocketImpl) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
			SCK_MAXIPADDRESSLENGTH) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (pClientSocketImpl -> getRemotePort (&lRemotePort) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Arrived connection. " << "Remote address: "
			<< pRemoteAddress << ", remote port: " << lRemotePort
			<< std:: endl;

		{
			Boolean_t				bIsReadyToRead;


			bIsReadyToRead				= false;

			while (!bIsReadyToRead)
			{
				if (pClientSocketImpl -> isReadyForReading (&bIsReadyToRead,
					2, 0) != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
					std:: cout << (const char *) err << std:: endl;

					return 1;
				}
			}
		}

		if (pClientSocketImpl -> readLine (pBuffer,
			SRV_MAXBUFFER, &ulCharsRead, 2, 0) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_READLINE_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "Message: " << pBuffer << std:: endl;

		{
			Boolean_t				bIsReadyToRead;


			bIsReadyToRead				= false;

			while (!bIsReadyToRead)
			{
				if (pClientSocketImpl -> isReadyForReading (&bIsReadyToRead,
					2, 0) != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
					std:: cout << (const char *) err << std:: endl;

					return 1;
				}
			}
		}

		ulBufferLength			= sizeof (MyStruct);

		if (pClientSocketImpl -> read (&msMyStruct,
			&ulBufferLength, false, 0, 0, true) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_READ_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		std:: cout << "READ: " << msMyStruct. lMyLong << " "
			<< msMyStruct. pMyString << std:: endl;

		{
			char			pChar [2];
			Error			errRead;


			ulBufferLength			= 1;

			if ((errRead = pClientSocketImpl -> read (pChar,
				&ulBufferLength, true, 2, 0, true)) != errNoError)
			{
				// se il client, dopo aver scritto sul socket fa shutdown
				// la read sul socket ritorna come errore SCK_READ_EOFREACHED.
				// Da questo errore si puo' capire quindi che la connessione
				// con il client e' andata giu'
				std:: cout << (const char *) errRead << std:: endl;
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);
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
	}




	if (spSocketsPool. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket2. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket1. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

