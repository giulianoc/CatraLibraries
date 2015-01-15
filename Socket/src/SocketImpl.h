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


#ifndef SocketImpl_h
	#define SocketImpl_h

	#include "SocketErrors.h"
	#include "vector"

	#ifndef Boolean_t
		typedef long	Boolean_t;
	#endif

	#ifndef Boolean_p
		typedef long	*Boolean_p;
	#endif

	#ifndef false
		#define false	0L
	#endif

	#ifndef true
		#define true	1L
	#endif


	#define SCK_MAXIPADDRESSLENGTH			(15 + 1)
	#define SCK_MAXHOSTNAMELENGTH			(128 + 1)
	#define SCK_MAXADDRESSBUFFERSIZE		(1024 * 4)


	// do not use big number (on hpux we had an overflow (negative number)
	//	because this number is multiplied per 1000 in source code)
	#define SCK_MAXTIMEOUTINSECONDS			1000000

	//	TELNET SESSION PROCESSING

	#define SCK_TN_MAXRESPONSELENGTH			(512 + 1)

	//	Telnet command:
	//	Data Byte 255 (IAC: Interpret as Command)
	#define	SCK_TN_IAC			255
	//	Indicates the request that the other party perform, or confirmation
	//	that you are expecting the other party to perform, the indicated option
	#define SCK_TN_DO			253
	//	Indicates the demand that the other party stop performing, or
	//	confirmation that you are no longer expecting the other party
	//	to perform, the indicated option
	#define SCK_TN_DONT			254
	//	Indicates the desire to begin performing, or confirmation that
	//	you are now performing, the indicated option
	#define SCK_TN_WILL 		251
	//	Indicates the refusal to perform, or continue performing, the
	//	indicated option
	#define SCK_TN_WONT 		252
	//	Indicates that what follows is subnegotiation of the indicated option
	#define SCK_TN_SB			250
	// End of subnegotiation parameters
	#define SCK_TN_SE			240
	#define SCK_TN_TERMTYPE		24
	#define SCK_TN_SEND			1
	#define SCK_TN_IS			0

	//	Absorb CR/LF as LF
	#define SCK_TN_CR			0x0D // decimal 13 carriage return
	#define SCK_TN_LF			0x0A // decimal 10 new line feed

	//	defines used by the algorithm to interpret the telnet command
	#define SCK_IAC_RESET		0
	#define SCK_IAC_OPCODE		1
	#define SCK_IAC_WILL		2
	#define SCK_IAC_DO			3

	//	This sequence requests termtype info
	#define SCK_IAC_SB			4
	#define SCK_IAC_TERMTYPE	5
	#define SCK_IAC_SEND		6
	#define SCK_IAC_IAC			7
	#define SCK_IAC_SE			8


	/**
		The class SocketImpl is a common superclass of all classes that
		actually implement sockets.
		It is used to create both client and server sockets.
		A "plain" socket implements these methods exactly as described,
		without attempting to go through a firewall or proxy.
	*/
	typedef class SocketImpl

	{

		public:
			typedef enum SocketType {
				STREAM,
				DGRAM
			} SocketType_t, *SocketType_p;

			typedef struct IPAddress {
				char			pIPAddress [SCK_MAXIPADDRESSLENGTH];
				char			pHostName [SCK_MAXHOSTNAMELENGTH];
			} IPAddress_t, *IPAddress_p;

		private:
			int					_iFd;
			char				_pRemoteAddress [SCK_MAXIPADDRESSLENGTH];
			long				_lRemotePort;
			long				_lLocalPort;
			char				_pLocalAddress [SCK_MAXIPADDRESSLENGTH];
			SocketType_t		_stSocketType;
			Boolean_t			_bIsConnectionRealized;

			unsigned long		_ulReceivingTimeoutInSeconds;
			unsigned long		_ulReceivingAdditionalTimeoutInMicroSeconds;
			unsigned long		_ulSendingTimeoutInSeconds;
			unsigned long		_ulSendingAdditionalTimeoutInMicroSeconds;


			/**
				used to perform the telnet protocol
			*/
			long				_lIAC_State;

			/**
				This routine processes data taken from the socket and munches on
				telnet IAC sequences, in a passive way.  Allows initial terminal
				type negotiation to occur, all other options are standard NVT.
				The buffer processed length is always minor than buffer
				to process length
			*/
			Error telnetDecoder (
				unsigned char *pucBuffer, long lBufferToProcessLength);


		protected:
			SocketImpl (const SocketImpl &t);

			friend class SocketsPool;

		public:
			SocketImpl (void);

			~SocketImpl (void);        

			/**
				Creates either a stream or a datagram socket. 
				Parameters: 
					stSocketType - socket type
					ulReceivingTimeoutIn* - Specify the receiving timeout
						until reporting an error. 0 to disable the timeout
					ulSendingTimeoutIn* - Specify the sending timeout
						until reporting an error. 0 to disable the timeout
					bReuseAddr - specifies the reuse of the address

					Remark: The SocketImpl:: setBlocking works only if the
						Receiving parameter is set to 0.
			*/
			Error create (SocketType_t stSocketType,
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
				Boolean_t bReuseAddr = false);

			/**
				Connects this socket to the specified port on the named host. 
				Parameters: 
					pRemoteAddress - the name of the remote host. 
					lRemotePort - the port number.
					timeouts parameters: see the comments inside the connect
			*/
			Error connect (const char *pRemoteAddress, long lRemotePort,
				unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS);

			/**
				Connects this socket to the specified sockaddr. 
				Parameters: 
					psckServerAddr - Socket address
			*/
			Error connect (struct sockaddr_in *psckServerAddr,
				unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS);

			/**
				Binds this socket to the specified port number. 
					Parameters: 
						pLocalAddress - local address to handle this traffic.
							If it is NULL or "" it will be used
								the default local address
						lLocalPort - the port number.
			*/
			Error bind (const char *pLocalAddress, long lLocalPort);

			/**
				Sets the maximum queue length for incoming connection
				indications
				(a request to connect) to the count argument.
				If a connection indication arrives when the
				queue is full, the connection is refused. 
				Parameters: 
					lClientsQueueLength - the maximum length of the queue.
			*/
			Error listen (long lClientsQueueLength);

			/**
				Accepts a connection.
				Parameters:
					pClientSocket - the accepted connection.
			*/
			Error acceptConnection (SocketImpl *pClientSocket);

			/**
				Closes this socket.
			*/
			Error close (void);

			/**
				Sets the blocking flag.
				Parameters:
					bBlocking - true for blocking some methods (for example
						read or acceptConnection) 
						false otherwise

				Remark: The SocketImpl:: setBlocking works only if the
					Receiving parameter (ServerSocket:: init or
					ClientSocket:: init) is set to 0.
			*/
			Error setBlocking (Boolean_t bBlocking);

			#ifdef WIN32
			#else
				/**
					Gets the blocking flag.
					Parameters:
						pbBlocking -
				*/
				Error getBlocking (Boolean_p pbBlocking);
			#endif

			/**
				Sets the  receiving timeout of the  socket
				Parameters:
					ulReceivingTimeoutInSeconds -
					ulReceivingAdditionalTimeoutInMicroSeconds -
			*/
			Error setReceivingTimeout (
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds);

			/**
				Sets the  sending timeout of the  socket
				Parameters:
					ulSendingTimeoutInSeconds -
					ulSendingAdditionalTimeoutInMicroSeconds -
			*/
			Error setSendingTimeout (
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds);

			/**
				Sets the  maximum  socket send buffer in bytes
				Parameters:
					ulMaxReceiveBuffer -
			*/
			Error setMaxSendBuffer (unsigned long ulMaxSendBuffer);

			/**
				Sets the  maximum  socket receive buffer in bytes
				Parameters:
					ulMaxReceiveBuffer -
			*/
			Error setMaxReceiveBuffer (unsigned long ulMaxReceiveBuffer);

			/**
			  	When the close or shutdown is called, the OS does not make
			  	the socket available but it sets the socket state to TIME_WAIT.
			  	OS will maintain this state for a time between 1 and 4 minutes
			  	before to make the socket available. From Internet:

			  	"Because of these potential problems with TIME_WAIT
				assassinations, one should not avoid the TIME_WAIT state
				by setting the SO_LINGER option to send an RST instead
				of the normal TCP connection termination (FIN/ACK/FIN/ACK).
				The TIME_WAIT state is there for a reason; it's
			  	your friend and it's there to help you :-)"

				At the same time, if a connection (socket) is open and closed
				continuously (stress test), the number of TIME_WAIT
				will increase and we could reach easily
				the max number of sockets.

				setLinger, when  enabled,  a  close(2) or shutdown(2) will not
				return until all queued messages for the socket
				have been  successfully  sent or  the  linger  timeout 
				has been reached.  Otherwise, the call
				returns immediately and the closing is done in  the  background.
				When  the socket is closed as part of exit(2), it always lingers
				in the background.
			*/
			Error setLinger (
				Boolean_t bLingerOnOff,
				unsigned long ulManySecondsToLingerFor);

			/**
				Enable sending of keep-alive messages on connection-oriented
				sockets.
				Questa opzione abilita o disabilita l'invio di pacchetti
				di tipo keepalives su una socket di tipo stream permettendo
				di rilevare eventuali problemi su una connessione su cui non si
				stanno trasmettendo dati. E' disabilitata di default.
				I keepalives sono pacchetti che non contengono dati ma che
				costringono lo scack TCP-IP che li riceve a inviare un
				riscontro al mittente. Inviare periodicamente tali pacchetti
				su una connessione momentaneamente inutilizzata (su cui non si
				sta ne trasmettendo ne ricevendo) permette di verificarne
				la validità. Il rovescio della medaglia è ovviamente la
				larghezza di banda sprecata per implementare tale meccanismo.
				In caso di errore (ad es. nessuno dei keepalives trasmessi è
				stato riscontrato) verrà notificato un evento FD_CLOSE
				contenete l'errore (WSAECONNABORTED in questo caso) alla
				procedura di finestra dell'applicazione.
				Parameters:
					bKeepAlive -
			*/
			Error setKeepAlive (Boolean_t bKeepAlive);

			/**
				Enable TCP_NODELAY on connection-oriented sockets.
				Questa opzione abilita o disabilita l'algoritmo di Nagle:
				quando è disabilitata (il suo valore è false) tale algoritmo è
				abilitato.
				L'algoritmo di Nagle è usato per ridurre il traffico della rete
				senza ridurre le prestazioni. Opera evitando di trasmettere i
				dati non appena questi si rendono disponibili ma aspettando di
				riempire il pacchetto prima di spedirlo. Disabilitare tale
				algoritmo potrebbe avere un impatto negativo sulle prestazioni
				della rete per cui dovrebbe essere fatto solo nel caso in cui
				il ritardo introdotto da esso nell'inoltro dei dati penalizzi
				eccessivamente le prestazioni della propria applicazione.

				TCP connections run with Nagle's algorithm enabled by default.
				This is a cooperative way of limiting bandwidth consumption,
				by having the sender only send at a rate the recipient
				can handle. The algorithm adapts, but it does ramp up slowly.
				Set TCP_NODELAY on a socket if you do not want it.

				Parameters:
					bTCPNodelay -
			*/
			Error setNoDelay (Boolean_t bNoDelay);

			/**
				Return true if in the socket there is something to read
				false otherwise
				Parameters:
					pbIsReadyForReading -
					ulSecondsToWait -
					ulAdditionalMicrosecondsToWait -
			*/
			Error isReadyForReading (Boolean_p pbIsReadyForReading,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait);

			/**
				Return true if in the socket there is something to write
				false otherwise
				Parameters:
					pbIsReadyForWriting -
					ulSecondsToWait -
					ulAdditionalMicrosecondsToWait -
			*/
			Error isReadyForWriting (Boolean_p pbIsReadyForWriting,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait);

			/**
				Return true if in the socket there is exception
				false otherwise
				Parameters:
					pbIsThereException -
					ulSecondsToWait -
					ulAdditionalMicrosecondsToWait -
			*/
			Error isThereException (Boolean_p pbIsThereException,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait);

			/**
				Wait for the appearance of the specified string in the data
				stream
				Return errNoError if the string is detected, the
				SCK_SOCKETIMPL_TIMEOUTEXPIRED error if timeout expired
				Parameters:
					pBufferToWait -
					lTimeoutInSeconds - seconds available to wait
						the buffer
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to read a line
			*/
			Error waitForBuffer (const char *pBufferToWait,
				long lTimeoutInSeconds, long lAdditionalTimeoutInMicroSeconds);

			/**
				This method is used to vacuum unwanted data from the incomming
				data stream and drop it.
				Parameters:
					lTimeoutInSeconds - seconds available to vacuum
						the chars read
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to vacuum the chars read
			*/
			Error vacuum (long lTimeoutInSeconds,
				long lAdditionalTimeoutInMicroSeconds);

			/**
				This method is used to vacuum unwanted data from the incomming
				data stream using the telnet protocol and drop it.
				Parameters:
					lTimeoutInSeconds - seconds available to vacuum
						the chars read
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to vacuum the chars read
			*/
			Error vacuumByTelnet (long lTimeoutInSeconds,
				long lAdditionalTimeoutInMicroSeconds);

			/**
				Reads data.
				Parameters:
					pvBuffer - buffer to read
					pulBufferLength - the buffer length
					bReadingCheckToBePerformed - indicates if must be call
						internally isReadyForReading before to read
					ulSecondsToWait and ulAdditionalMicrosecondsToWait -
						are parameters for isReadyForReading. Not used if
						bReadingCheckToBePerformed is false
					bOneShotRead - can it perform only one read
				If this method returns:
					SCK_READ_EOFREACHED error if the socket connection is down
					SCK_NOTHINGTOREAD error if there is nothing to read
						(only if bReadingCheckToBePerformed is true)
			*/
			Error read (void *pvBuffer, unsigned long *pulBufferLength,
				Boolean_t bReadingCheckToBePerformed,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait,
				Boolean_t bOneShotRead = false,
				Boolean_t bRemoveDataFromSocket = true);

			/**
				Reads a line of data.
				Parameters:
					pBuffer - buffer cointaining the line and finishing
						with the '\0' character
					ulBufferLength - the buffer length
					pulCharsRead	- chars read
					ulTimeoutInSeconds - seconds available to read a line
					ulAdditionalTimeoutInMicroSeconds - microseconds available
						to read a line
				If this method returns the SCK_READ_EOFREACHED error,
				that means the socket connection is down

				If ulTimeoutInSeconds and
				ulAdditionalTimeoutInMicroSeconds are both zero,
				the internal SocketImpl:: read method will not perform any
				check if there is something to read before reading.
			*/
			Error readLine (char *pBuffer,
				unsigned long ulBufferLength,
				unsigned long *pulCharsRead, unsigned long ulTimeoutInSeconds,
				unsigned long ulAdditionalTimeoutInMicroSeconds);

			/**
				Eliminata perchè la readLines è una read.

				Reads lines of data until there are lines in the
				socket or the buffer is filled completely.
				Parameters:
					pBuffer - buffer cointaining the lines
					lBufferLength - the buffer length
					plCharsRead	- chars read
					lTimeoutInSeconds - seconds available to read the lines
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to read the lines
					pNewLine - this is what the method use to separate
						the lines. Examples are "\n", "\r\n", ...
				If this method returns the SCK_READ_EOFREACHED error,
				that means the socket connection is down
			Error readLines (char *pBuffer, long lBufferLength,
				long *plCharsRead, long lTimeoutInSeconds,
				long lAdditionalTimeoutInMicroSeconds,
				const char *pNewLine = "\n");
			*/

			/**
				Reads a line of data using the telnet protocol.
				Parameters:
					pBuffer - buffer cointaining the line
					lBufferLength - the buffer length
					lTimeoutInSeconds - seconds available to read a line
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to read a line
			*/
			Error readLineByTelnet (char *pBuffer,
				long lBufferLength, long lTimeoutInSeconds = -1,
				long lAdditionalTimeoutInMicroSeconds = 0);

			/**
				Reads data for a specified period using the telnet
				protocol.
				Parameters:
					pBuffer - buffer cointaining the line
					lBufferLength - the buffer length
					lTimeoutInSeconds - seconds available to read a line
					lAdditionalTimeoutInMicroSeconds - microseconds available
						to read a line
			*/
			Error readByTelnet (char *pBuffer,
				unsigned long *pulBufferLength, Boolean_t bOneShotRead);

			/**
				Reads a JAVA line of data. In Java each char is 2 bytes.
				Parameters:
					pBuffer - buffer cointaining the line
					lBufferLength - the buffer length
				If this method returns the SCK_READ_EOFREACHED error,
				that means the socket connection is down
			*/
			Error readLineFromJava (char *pBuffer, long lBufferLength);

			/**
				Writes a string of data.
				Parameters:
					pString - string to be written
					bWritingCheckToBePerformed - indicates if must be call
						internally isReadyForWriting before to write
					ulSecondsToWait and ulAdditionalMicrosecondsToWait -
						are parameters for isReadyForWriting. Not used if
						bWritingCheckToBePerformed is false
					pRemoteAddress - it is necessary if we have a DGRAM socket
					lRemotePort - it is necessary if we have a DGRAM socket
			*/
			Error writeString (const char *pString,
				Boolean_t bWritingCheckToBePerformed,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait,
				const char *pRemoteAddress = (const char *) NULL,
				long lRemotePort = -1);

			/**
				Writes a buffer of data for STREAM socket.
				Parameters:
					pvBuffer - buffer to be written
					lBufferLength - the buffer length
					bWritingCheckToBePerformed - indicates if must be call
						internally isReadyForWriting before to write
					ulSecondsToWait and ulAdditionalMicrosecondsToWait -
						are parameters for isReadyForWriting. Not used if
						bWritingCheckToBePerformed is false
					pRemoteAddress - it is necessary if we have a DGRAM socket
					lRemotePort - it is necessary if we have a DGRAM socket
			*/
			Error write (void *pvBuffer, long lBufferLength,
				Boolean_t bWritingCheckToBePerformed,
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMicrosecondsToWait,
				const char *pRemoteAddress = (const char *) NULL,
				long lRemotePort = -1);

			Error getRemoteAddress (char *pRemoteAddress,
				unsigned long ulBufferLength);

			Error getRemotePort (long *plRemotePort);

			Error getLocalAddress (char *pLocalAddress,
				unsigned long ulBufferLength);

			Error getLocalPort (long *plLocalPort);

			Error getReceivingTimeouts (
				unsigned long *pulReceivingTimeoutInSeconds,
				unsigned long *pulReceivingAdditionalTimeoutInMicroSeconds);

			Error getSendingTimeouts (
				unsigned long *pulSendingTimeoutInSeconds,
				unsigned long *pulSendingAdditionalTimeoutInMicroSeconds);

			static Error getIPAddressesList (
				std:: vector<IPAddress_t> *pvIPAddresses);

			/**
				The pNetworkIdentifier depends from the OS.
				For Windows this parameter is not used and the first
					card he retrieved is used.
				For unix environment this parameter must be filled with
					the string identifying the network (i.e.: "eth0")
			*/
			static Error getMACAddress (
				const char *pNetworkIdentifier,
				unsigned char pucMACAddress [6]);

	} SocketImpl_t, *SocketImpl_p;

#endif

