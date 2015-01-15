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
#include <time.h>
#include "TelnetClient.h"
#ifdef WIN32
#else
	#include <unistd.h>
#endif

#define TLN_MAXBUFFERLENGTH				1024 + 1


int main (int argc, char **argv)

{

	TelnetClient_t				tcTelnetClient;
	char						*pIpAddress;
	char						*pUser;
	char						*pPassword;
	char						pMessage [TLN_MAXBUFFERLENGTH];
	long						lRemotePort;
	SocketImpl_p				pSocketImpl;
	char						pBuffer [TLN_MAXBUFFERLENGTH];
	Error						errReadLineByTelnet;
	// long						lIndex;
	Error 						errTelnetInit;
	Error 						errTelnetFinish;
	time_t						tStartTime;
	Error						errIsReady;
	Boolean_t					bIsReadyForReading;
	unsigned long				ulBufferLength;
	TelnetClient:: TelnetServerType_t			tstTelnetServerType;
	Boolean_t					bDebug		= false;


	if (argc != 6)
	{
		std:: cerr << "Usage: ./client <ip address> <port number> <user> <password> <message>" << std:: endl;

		return 1;
	}

	pIpAddress			= argv [1];
	lRemotePort			= atoi (argv [2]);
	pUser				= argv [3];
	pPassword			= argv [4];
	strcpy (pMessage, argv [5]);

	tstTelnetServerType		= TelnetClient:: SCK_UNIX_TELNETSERVER;

	// for (lIndex = 0; lIndex < 10; lIndex++)
	{
		// std:: cout << "lIndex: " << lIndex << std:: endl;

	if (bDebug)
		std:: cout << "initialization..." << std:: endl;

	if ((errTelnetInit = tcTelnetClient. init ((const char *) NULL,
		pIpAddress, lRemotePort, 2, 0, 2, 0, true, pUser,
		pPassword, 10, 0, tstTelnetServerType, "\n")) !=
		errNoError)
	{
		std:: cerr << (const char *) errTelnetInit << std:: endl;
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_TELNETCLIENT_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (tcTelnetClient. getSocketImpl (&pSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (tcTelnetClient. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_TELNETCLIENT_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bDebug)
		std:: cout << "vacuumByTelnet..." << std:: endl;

	if (pSocketImpl -> vacuumByTelnet (2, 0) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_TELNETCLIENT_FINISH_FAILED);
			// SCK_SOCKETIMPL_VACUUMBYTELNET_FAILED);

		if (tcTelnetClient. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_TELNETCLIENT_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	// std:: cout << pMessage << std:: endl;

	if (tstTelnetServerType == TelnetClient:: SCK_WINDOWS_TELNETSERVER)
		strcat (pMessage, "\r\n");
	else
		strcat (pMessage, "\n");

	if (bDebug)
		std:: cout << "writing..." << std:: endl;

	if (pSocketImpl -> writeString (pMessage, true, 2, 0) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (tcTelnetClient. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_TELNETCLIENT_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	tStartTime			= time (NULL);

	// while (time (NULL) - tStartTime < 5)
	{
		if (bDebug)
			std:: cout << "checking if ready for reading..." << std:: endl;

		if ((errIsReady = pSocketImpl -> isReadyForReading (
			&bIsReadyForReading, 1, 0)) != errNoError)
		{
			std:: cerr << (const char *) errIsReady << std:: endl;
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			if (tcTelnetClient. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_TELNETCLIENT_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bIsReadyForReading)
		{
			ulBufferLength	= TLN_MAXBUFFERLENGTH - 1;

			if (bDebug)
				std:: cout << "reading..." << std:: endl;

			if ((errReadLineByTelnet = pSocketImpl -> readLineByTelnet (
				pBuffer, ulBufferLength, 2, 0)) !=
				errNoError)
			{
				std:: cerr << (const char *) errReadLineByTelnet << std:: endl;
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_READLINEBYTELNET_FAILED);

				if (tcTelnetClient. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_TELNETCLIENT_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return 1;
			}

			/*
			if (pBuffer [0] == '$')
				std:: cout << (long) pBuffer [0] << " " << (long) pBuffer [1]
				<< " " << (long) pBuffer [2] << " "
				<< (long) pBuffer [3] << " ";
			*/

			// the first read is just the command sent
			// std:: cout << "First read: " << pBuffer << std:: endl;
		}

		if (bDebug)
			std:: cout << "checking if ready for reading..." << std:: endl;

		if ((errIsReady = pSocketImpl -> isReadyForReading (
			&bIsReadyForReading, 1, 0)) != errNoError)
		{
			std:: cerr << (const char *) errIsReady << std:: endl;
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			if (tcTelnetClient. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_TELNETCLIENT_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bIsReadyForReading)
		{
			ulBufferLength	= TLN_MAXBUFFERLENGTH - 1;

			if (bDebug)
				std:: cout << "reading..." << std:: endl;

			if ((errReadLineByTelnet = pSocketImpl -> readLineByTelnet (
				pBuffer, ulBufferLength, 2, 0)) !=
				errNoError)
			{
				std:: cerr << (const char *) errReadLineByTelnet << std:: endl;
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_READLINEBYTELNET_FAILED);

				if (tcTelnetClient. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_TELNETCLIENT_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return 1;
			}

			/*
			if (pBuffer [0] == '$')
				std:: cout << (long) pBuffer [0] << " " << (long) pBuffer [1]
				<< " " << (long) pBuffer [2] << " "
				<< (long) pBuffer [3] << " ";
			*/

			std:: cout << pBuffer + 2 << std:: endl;
			/*
			for (long lIndex=0; lIndex< strlen (pBuffer + 2); lIndex++)
				std:: cout << (int) pBuffer [lIndex] << ": " << "'" << pBuffer [lIndex] << "'" << std:: endl;
			*/
		}
	}

	if (bDebug)
		std:: cout << "finishing..." << std:: endl;

	if ((errTelnetFinish = tcTelnetClient. finish ()) != errNoError)
	{
		std:: cerr << (const char *) errTelnetFinish << std:: endl;
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_TELNETCLIENT_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}
	}


	return 0;
}

