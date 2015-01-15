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
#include <errno.h>


#define SRV_MAXBUFFER			1024

typedef struct MyStruct
{
	long            lMyLong;
	char            pMyString [100];
} MyStruct_t, *MyStruct_p;


int main (int argc, char **argv)

{

	long						lLocalPort;
	ServerSocket_t				ssServerSocket;
	ClientSocket_t				csClientSocket;
	char						pRemoteAddress [SCK_MAXIPADDRESSLENGTH];
	long						lRemotePort;
	SocketImpl_p				pClientSocketImpl;
	SocketImpl_p				pServerSocketImpl;
	long						lMaxClients;
	char						pBuffer [SRV_MAXBUFFER + 1];
	Boolean_t					bIsEnd;
	Error						errAcceptConnection;
	MyStruct_t					msMyStruct;
	unsigned long				ulCharsRead;
	unsigned long				ulBufferLength;
	#ifdef WIN32
	#else
		Boolean_t				bBlocking;
	#endif
	Error_t						errReadLine;



	{
		const char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by CatraSoftware\nemail                : catrasoftware-support@catrasoftware.it\n***************************************************************************\n";

		std:: cout << pCopyright << std:: endl;
	}

	if (argc != 2)
	{
		std:: cout << "Usage: ./server <port number>" << std:: endl;

		return 1;
	}

	lLocalPort			= atoi (argv [1]);

	lMaxClients			= 10;

	if (ssServerSocket. init ((const char *) NULL, lLocalPort, true,
		SocketImpl:: STREAM, 10, 0, lMaxClients) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ssServerSocket. getSocketImpl (&pServerSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	#ifdef WIN32
	#else
		if (pServerSocketImpl -> getBlocking (&bBlocking) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETBLOCKING_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}
	#endif

	if (pServerSocketImpl -> setBlocking (false) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETBLOCKING_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	#ifdef WIN32
	#else
		if (pServerSocketImpl -> getBlocking (&bBlocking) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETBLOCKING_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}
	#endif

	bIsEnd					= false;
	while (!bIsEnd)
	{
		std:: cout << "Waiting on " << lLocalPort << " port" << std:: endl;

		if (csClientSocket. init () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			std:: cout << (const char *) err << std:: endl;

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}

		if ((errAcceptConnection = ssServerSocket. acceptConnection (
			&csClientSocket)) != errNoError)
		{
			Boolean_t				bIsThereError;


			bIsThereError			= false;

			if ((long) errAcceptConnection == SCK_ACCEPT_FAILED)
			{
				int					iErrno;
				unsigned long		ulUserDataBytes;

				errAcceptConnection. getUserData (&iErrno, &ulUserDataBytes);
				#ifdef WIN32
					if (iErrno == WSAEWOULDBLOCK)
				#else
					if (iErrno == EAGAIN)
				#endif
				{
					std:: cout << "No connection to accept" << std:: endl;
					#if defined(WIN32)
						Sleep (1000);
					#else
						sleep (1);
					#endif
				}
				else
					bIsThereError			= true;
			}
			else
				bIsThereError			= true;
			
			if (bIsThereError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
				std:: cout << (const char *) err << std:: endl;

				if (csClientSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				return 1;
			}

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;

				return 1;
			}

			continue;
		}

		if (csClientSocket. getSocketImpl (&pClientSocketImpl) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
			SCK_MAXIPADDRESSLENGTH) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (pClientSocketImpl -> getRemotePort (&lRemotePort) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

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

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cout << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cout << (const char *) err << std:: endl;
					}

					return 1;
				}
			}
		}

		if ((errReadLine = pClientSocketImpl -> readLine (pBuffer,
			SRV_MAXBUFFER, &ulCharsRead, 2, 0)) != errNoError)
		{
			std:: cout << (const char *) errReadLine << std:: endl;

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_READLINE_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}

		std:: cout << "Message: '" << pBuffer << "'" << std:: endl;

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

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cout << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cout << (const char *) err << std:: endl;
					}

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

			if (csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

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

				if (csClientSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				return 1;
			}
		}

		if (csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			if (ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				std:: cout << (const char *) err << std:: endl;
			}

			return 1;
		}
	}

	if (ssServerSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

