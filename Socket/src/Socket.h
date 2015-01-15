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


#ifndef Socket_h
	#define Socket_h


	#include "SocketErrors.h"
	#include "SocketImpl.h"


	/**

		La libreria Socket permette di gestire connessioni tramite socket.

		E' costituita dalla classe SocketImpl che incapsula le API socket
		fornite dal sistema operativo e fornisce una interfaccia indipendente
		dallo stesso.

		Le classi ServerSocket e ClientSocket rappresentano rispettivamente
		il server ed il client della connessione.

		Il processo client deve:

		1. istanziare un client socket (ClientSocket csClientSocket)
		2. inizializzare il socket indicando l'ip address della macchina
			dove e' in running il processo server su cui ci si vuole connettere
			e la porta su cui lo stesso processo server sta in ascolto
			(csClientSocket. init (pIpAddress, lRemotePort))
		3. recuperare l'oggetto SocketImpl che e' associato al ClientSocket
			(csClientSocket. getSocketImpl (&pSocketImpl))
		4. scrivere sul socket (pSocketImpl -> writeString (pBuffer))
		5. eseguire la finish dell'oggetto socket client
			(csClientSocket. finish ())

		Il processo server deve:

		1. istanziare un server socket (ServerSocket ssServerSocket)
		2. inizializzare il socket indicando la porta su cui deve attendere
			connessioni ed il numero massimo di client su cui puo' accettare
			connessioni (ssServerSocket. init (lLocalPort, lMaxClients))
		3. inizializzare il client (csClientSocket. init ())
		4. attendere che un client si connetta (pSocketImpl -> setBlocking
			(true); ssServerSocket. acceptConnection (&csClientSocket))
			o verificare se un client abbia richiesto una connessione
			(pSocketImpl -> setBlocking (false); ssServerSocket.
			acceptConnection (&csClientSocket))
		5. una volta che una connessione con un client sia stata instaurata,
			recuperare l'oggetto SocketImpl che e' associato al ClientSocket
			(csClientSocket. getSocketImpl (&pSocketImpl))
		6. e' possibile sapere da quale ip address e da quale porta il client
			si e' connesso (pSocketImpl -> getRemoteAddress (pRemoteAddress);
			pSocketImpl -> getRemotePort (&lRemotePort))
		7. il processo server quindi puo' leggere cio' che scrive il client
			(pSocketImpl -> readLine (pBuffer, SRV_MAXBUFFER))
		8. eseguire la finish del client socket (csClientSocket. finish ())
		9. eseguire la finish del server socket (ssServerSocket. finish ())

		Un particolare ClientSocket e' rappresentato dalla classe TelnetClient
		che incapsula sia la negoziazione delle proprieta' tra client e server
		che la connessione telnet deve rispettare e sia la gestione
		di user e password.
		Una connessione telnet e' una connessione TCP usata per trasmettere
		dati intrisi di caratteri di controllo in accordo
		con il protocollo TELNET.
		Per ulteriori informazioni vi rimando  alle due raccomandazioni,
		RFC764 e RFC884, che definiscono il protocollo TELNET.

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.
	*/
	typedef class Socket

	{
		protected:
			unsigned long			_ulIdentifier;


			Socket (const Socket &t);

			SocketImpl_p		_pSocketImpl;

		public:
			/**
				Creates an unconnected socket
			*/
			Socket (void);

			/**
				Destroy the socket
			*/
			virtual ~Socket (void);

			Error init (unsigned long ulIdentifier = 0);

			Error finish (void);

			/**
				Returns the socketImpl instance of this Socket.
			*/
			Error getSocketImpl (SocketImpl_p *pSocketImpl);

			Error setIdentifier (unsigned long ulIdentifier);

			unsigned long getIdentifier (void);

	} Socket_t, *Socket_p;

#endif

