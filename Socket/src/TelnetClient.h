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


#ifndef TelnetClient_h
	#define TelnetClient_h


	#include "ClientSocket.h"


	#define SCK_TELNETCLIENT_MAXBUFFERTONEGOTIALE			(1024 + 1)
	#define SCK_TELNETCLIENT_MAXCLOSECOMMANDLENGTH			(1024 + 1)
	#define SCK_TELNETCLIENT_CLOSETELNETSESSIONCOMMAND		"exit\n"


	/**
		This class implements a telnet client.
	*/
	typedef class TelnetClient: public ClientSocket

	{
		private:
			long						_lSecondsUsedToNegotiate;
			long						_lAdditionalMicroSecondsUsedToNegotiate;
			char						_pCommandToBeSentToCloseTheSession [
				SCK_TELNETCLIENT_MAXCLOSECOMMANDLENGTH];

		protected:
			TelnetClient (const TelnetClient &t);

		public:
			typedef enum TelnetServerType {
				SCK_UNIX_TELNETSERVER,
				SCK_WINDOWS_TELNETSERVER
			} TelnetServerType_t, *TelnetServerType_p;

		public:
			/**
				Creates an unconnected socket
			*/
			TelnetClient (void);

			/**
				Destroy the socket
			*/
			virtual ~TelnetClient (void);

			/**
				Initializes a telnet client.
				Parameters: 
					pLocalAddress - local address to handle this traffic.
						If it is NULL is used the default local address
					pRemoteAddress - the IP address
					lRemotePort -  the port number
					pUser - user for the telnet login autentication
					pPassword - user for the telnet login autentication
					lSecondsUsedToNegotiate - seconds used to negotiate the
						telnet connection properties
					lAdditionalMicroSecondsUsedToNegotiate - microseconds used
						to negotiate the telnet connection properties
					pSocketImpl - user-specified SocketImpl
			*/
			Error init (const char *pLocalAddress,
				const char *pRemoteAddress, long lRemotePort,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
				Boolean_t bReuseAddr,
				const char *pUser = (const char *) NULL,
				const char *pPassword = (const char *) NULL,
				long lSecondsUsedToNegotiate = 0,
				long lAdditionalMicroSecondsUsedToNegotiate = 1000,
				TelnetServerType_t tstTelnetServerType = SCK_UNIX_TELNETSERVER,
				const char *pCommandToBeSentToCloseTheSession =
					(const char *) NULL);

			/**
				Closes this telnet session
			*/
			virtual Error finish (void);

	} TelnetClient_t, *TelnetClient_p;

#endif

