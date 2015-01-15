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


#ifndef ClientSocket_h
	#define ClientSocket_h


	#include "Socket.h"


	/**
		This class implements client sockets.
		A socket is an endpoint for communication between two machines.
		The actual work of the socket is performed by an instance of the
		SocketImpl class. An application, by changing the socket
		implementation,
		can configure itself to create sockets appropriate
		to the local firewall.
	*/
	typedef class ClientSocket: public Socket

	{
		public:
			typedef enum ClientSocketStatus {
				SCK_CLIENTSOCKET_BUILDED,
				SCK_CLIENTSOCKET_INITIALIZED
			} ClientSocketStatus_t, *ClientSocketStatus_p;

		private:
			Boolean_t				_bSocketImplCreated;
			ClientSocketStatus_t	_sClientSocketStatus;

			time_t					_tConnectUTCStartTimeInSeconds;

			SocketImpl:: SocketType_t		_stSocketType;
			unsigned long			_ulReceivingTimeoutInSeconds;
			unsigned long			_ulReceivingAdditionalTimeoutInMicroSeconds;
			unsigned long			_ulSendingTimeoutInSeconds;
			unsigned long			_ulSendingAdditionalTimeoutInMicroSeconds;
			unsigned long	_ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS;
			Boolean_t				_bReuseAddr;
			char					_pLocalAddress [
				SCK_MAXHOSTNAMELENGTH];
			char					_pRemoteAddress [
				SCK_MAXHOSTNAMELENGTH];
			long					_lRemotePort;

		protected:
			ClientSocket (const ClientSocket &t);

			friend class ServerSocket;

		public:
			/**
				Creates an unconnected socket
			*/
			ClientSocket (void);

			/**
				Destroy the socket
			*/
			virtual ~ClientSocket (void);

			/**
				Creates a client socket empty
				Parameters: 
			*/
			Error init (unsigned long ulIdentifier = 0);

			/**
				Creates a stream socket and connects it to the specified
				port number
				on the named host.
				Parameters: 
					stSocketType - Socket type (STREAM or DGRAM)
					ulReceivingTimeoutIn* - Specify the receiving timeout
						until reporting an error. 0 to disable the timeout
					ulSendingTimeoutIn* - Specify the sending timeout
						until reporting an error. 0 to disable the timeout
					pLocalAddress - local address to handle this traffic.
						If it is NULL or "" it will be used the default local
						address
					pRemoteServer - the IP address or host name.
						It is not used if stSocketType is DGRAM.
					lRemotePort - the port number. It is not used if
						stSocketType is DGRAM.

					Remark: The SocketImpl:: setBlocking works only if the
						Receiving parameter is set to 0.
			*/
			Error init (SocketImpl:: SocketType_t stSocketType,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
				unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS,
				Boolean_t bReuseAddr, const char *pLocalAddress,
				const char *pRemoteServer = (const char *) NULL,
				long lRemotePort = -1, unsigned long ulIdentifier = 0);

			/*
			 * See the previous init
			*/
			Error init (SocketImpl:: SocketType_t stSocketType,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
				unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS,
				Boolean_t bReuseAddr, const char *pLocalAddress,
				struct sockaddr_in *psckServerAddr,
				unsigned long ulIdentifier = 0);

			/**
				Closes this socket.
			*/
			virtual Error finish (void);

			ClientSocketStatus_t getClientSocketStatus (void);

			Error reinit (void);

			time_t getConnectUTCStartTimeInSeconds (void);

	} ClientSocket_t, *ClientSocket_p;

#endif

