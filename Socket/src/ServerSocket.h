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


#ifndef ServerSocket_h
	#define ServerSocket_h


	#include "SocketErrors.h"
	#include "SocketImpl.h"
	#include "ClientSocket.h"


	/**
		This class implements server sockets.
		A server socket waits for requests to come in over the network.
		It performs some operation based on that request, and then
		possibly returns a result to the requester.
		The actual work of the socket is performed by an instance of the
		SocketImpl class. An application, by changing the socket
		implementation,
		can configure itself to create sockets appropriate
		to the local firewall.
	*/
	typedef class ServerSocket: public Socket

	{
		private:
			typedef enum ServerSocketStatus {
				SCK_SERVERSOCKET_BUILDED,
				SCK_SERVERSOCKET_INITIALIZED
			} ServerSocketStatus_t, *ServerSocketStatus_p;

		private:
			ServerSocketStatus_t	_sServerSocketStatus;

		protected:
			ServerSocket (const ServerSocket &t);

		public:
			/**
				Creates a server socket.
			*/
			ServerSocket (void);

			/**
				Destroy a server socket.
			*/
			~ServerSocket (void);        

			/**
				Initializes a server socket and binds it to the specified
				local port number.
				The maximum queue length for incoming connection indications
				(a request to connect) is set to the lClientsQueueLength
				parameter.
				If a connection indication arrives when the queue is full,
				the connection is refused.
				Parameters: 
					pLocalAddress - local address to handle this traffic.
						If it is NULL, he will listen on all the local
						ip adresses
					lRemotePort - the specified port.
					bReuseAddr - specifies the reuse of the address
					stSocketType - socket type (STREAM or DGRAM)
					ulReceivingTimeoutIn* - Specify the receiving timeout
						until reporting an error. 0 to disable the timeout
					lClientsQueueLength - the maximum length of the queue.
						It is not used on DGRAM socket.

					Remark: The SocketImpl:: setBlocking works only if the
						Receiving parameter is set to 0.
			*/
			Error init (const char *pLocalAddress, long lLocalPort,
				Boolean_t bReuseAddr,
				SocketImpl:: SocketType_t stSocketType,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				long lClientsQueueLength = -1);

			/**
				Listens for a connection to be made to this socket and accepts
				it.
				If the SocketImpl:: setBlocking is called with true as parameter
				this method will be blocking until a connection is performed,
				otherwise this method returns an error if the connection
				is not performed.

				Remarks that the SocketImpl:: setBlocking method works
				only if the Receiving parameters (init method) are passed as 0.

				The pClientSocket parameter must be initialized with the
				init method.
				Parameters:
					pClientSocket - the accepted connection.
			*/
			Error acceptConnection (ClientSocket_p pClientSocket);

			/**
				Closes this socket.
			*/
			Error finish (void);

	} ServerSocket_t, *ServerSocket_p;

#endif

