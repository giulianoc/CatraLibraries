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
#ifdef WIN32
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
	SocketImpl_p				pServerSocketImpl;
	char						pBuffer [SRV_MAXBUFFER + 1];
	MyStruct_t					msMyStruct;
	unsigned long				ulBufferLength;



	std:: cout << std:: endl;

	if (argc != 2)
	{
		std:: cout << "Usage: ./server <port number>" << std:: endl;

		return 1;
	}

	lLocalPort			= atoi (argv [1]);

	if (ssServerSocket. init ((const char *) NULL, lLocalPort, true,
		SocketImpl:: DGRAM, 10, 0, 120) != errNoError)
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

	{
		Boolean_t				bIsReadyToRead;


		bIsReadyToRead				= false;

		while (!bIsReadyToRead)
		{
			if (pServerSocketImpl -> isReadyForReading (&bIsReadyToRead,
				2, 0) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
				std:: cout << (const char *) err << std:: endl;

				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cout << (const char *) err << std:: endl;
				}

				return 1;
			}

			std:: cout << "Waiting to read" << std:: endl;
		}
	}

	ulBufferLength			= SRV_MAXBUFFER;

	if (pServerSocketImpl -> read (pBuffer,
		&ulBufferLength, false, 0, 0, true, false) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_READ_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	std:: cout << "Message: " << pBuffer << std:: endl;

	if (pServerSocketImpl -> read (pBuffer,
		&ulBufferLength, true, 2, 0, true, true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_READ_FAILED);
		std:: cout << (const char *) err << std:: endl;

		if (ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;
		}

		return 1;
	}

	std:: cout << "Message: " << pBuffer << std:: endl;

	{
		Boolean_t				bIsReadyToRead;


		bIsReadyToRead				= false;

		while (!bIsReadyToRead)
		{
			if (pServerSocketImpl -> isReadyForReading (&bIsReadyToRead,
				2, 0) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
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
	}

	ulBufferLength				= sizeof (MyStruct);

	if (pServerSocketImpl -> read (&msMyStruct,
		&ulBufferLength, false, 0, 0, true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_READ_FAILED);
		std:: cout << (const char *) err << std:: endl;

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

		if ((errRead = pServerSocketImpl -> read (pChar,
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

