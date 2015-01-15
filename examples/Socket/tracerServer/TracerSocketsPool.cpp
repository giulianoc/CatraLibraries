
#include "TracerSocketsPool.h"
#include "TracerServerMessages.h"
#include "ServerSocket.h"
#include <assert.h>



TracerSocketsPool:: TracerSocketsPool (void): SocketsPool ()

{

}


TracerSocketsPool:: ~TracerSocketsPool (void)

{

}


TracerSocketsPool:: TracerSocketsPool (
	const TracerSocketsPool &t)

{
	assert (1 == 0);

}


Error TracerSocketsPool:: init (
	Tracer_p ptTracer)

{

	_ptTracer				= ptTracer;

	if (SocketsPool:: init (TS_TRACERSOCKETSPOOL_MAXSOCKETSNUMBER,
		0, 0, true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_INIT_FAILED);
		_ptTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error TracerSocketsPool:: finish (void)

{

	if (SocketsPool:: finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FINISH_FAILED);
		_ptTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error TracerSocketsPool:: updateSocketStatus (
	Socket_p pSocket, long lSocketType,
	void *pvSocketData,
	unsigned short usSocketCheckType)

{

	// REMEMBER, you cannot call deleteSocket from the method if
	//  bAllowDeletionSocketFromUpdateMethod (parameter in init)
	//  is false

	switch (lSocketType)
	{
		case TS_TRACERSOCKETPOOL_SERVERSOCKET:
			{
				ServerSocket_p				pssServerSocket;
				ClientSocket_p				pcsClientSocket;
				Error_t						errAcceptConnection;
				SocketImpl_p				pClientSocketImpl;


				if (usSocketCheckType & SOCKETSTATUS_EXCEPTION)
				{
					Error err = TracerServerErrors (__FILE__, __LINE__,
						TS_TRACERSOCKETSPOOL_EXCEPTIONONSOCKET,
						1, lSocketType);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				pssServerSocket			= (ServerSocket_p) pSocket;

				if ((pcsClientSocket = new ClientSocket_t) ==
					(ClientSocket_p) NULL)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_NEW_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				if (pcsClientSocket -> init () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_INIT_FAILED,
						2, "<not applicable>", -1);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					delete pcsClientSocket;

					return err;
				}

				if ((errAcceptConnection = pssServerSocket -> acceptConnection (
					pcsClientSocket)) != errNoError)
				{
					/*
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
					*/

					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errAcceptConnection, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (pcsClientSocket -> finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						delete pcsClientSocket;

						return err;
					}

					delete pcsClientSocket;

					return err;
				}

				if (pcsClientSocket -> getSocketImpl (&pClientSocketImpl) !=
					errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKET_GETSOCKETIMPL_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (pcsClientSocket -> finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pcsClientSocket;

					return err;
				}

				if (readFromClientSocket (pcsClientSocket, true) != errNoError)
				{
					Error err = TracerServerErrors (__FILE__, __LINE__,
						TS_TRACERSOCKETSPOOL_READFROMCLIENTSOCKET_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (pcsClientSocket -> finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pcsClientSocket;

					return err;
				}
			}

			break;
		case TS_TRACERSOCKETPOOL_CLIENTSOCKET:
			{
				ClientSocket_p				pcsClientSocket;


				if (usSocketCheckType & SOCKETSTATUS_EXCEPTION)
				{
					Error err = TracerServerErrors (__FILE__, __LINE__,
						TS_TRACERSOCKETSPOOL_EXCEPTIONONSOCKET,
						1, lSocketType);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				pcsClientSocket			= (ClientSocket_p) pSocket;

				if (readFromClientSocket (pcsClientSocket, false) != errNoError)
				{
					Error err = TracerServerErrors (__FILE__, __LINE__,
						TS_TRACERSOCKETSPOOL_READFROMCLIENTSOCKET_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
			}

			break;
		default:
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_WRONGSOCKETTYPE,
					1, lSocketType);
				_ptTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
	}


	return errNoError;
}


Error TracerSocketsPool:: readFromClientSocket (
	ClientSocket_p pcsClientSocket,
	Boolean_t bClientSocketToBeAddedInSocketsPool)

{

	SocketImpl_p				pClientSocketImpl;
	char						pRemoteAddress [
		SCK_MAXIPADDRESSLENGTH];
	long						lRemotePort;
	Error_t						errReadLine;
	char						pTrace [TS_TRACERSOCKETPOOL_MAXTRACELENGTH];
	unsigned long				ulCharsRead;


	if (pcsClientSocket -> getSocketImpl (&pClientSocketImpl) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (pClientSocketImpl -> getRemoteAddress (pRemoteAddress,
		SCK_MAXIPADDRESSLENGTH) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);
		_ptTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (pClientSocketImpl -> getRemotePort (&lRemotePort) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);
		_ptTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errReadLine = pClientSocketImpl -> readLine (pTrace,
		TS_TRACERSOCKETPOOL_MAXTRACELENGTH, &ulCharsRead, 2, 0)) != errNoError)
	{
		if ((long) errReadLine == SCK_READ_EOFREACHED)
		{
			// ClientSocket to be removed by the SocketsPool

			if (!bClientSocketToBeAddedInSocketsPool)
			{
				Error_t						errDeleteSocket;


				if ((errDeleteSocket = deleteSocket (pcsClientSocket,
					(void **) NULL)) != errNoError)
				{
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errDeleteSocket, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETSPOOL_DELETESOCKET_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				{
					Message msg = TracerServerMessages (__FILE__, __LINE__,
						TS_TRACERSOCKETSPOOL_REMOVEDCLIENTCONNECTION,
						2, pRemoteAddress, lRemotePort);
					_ptTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				if (pcsClientSocket -> finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					_ptTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				delete pcsClientSocket;


				return errNoError;
			}
			else
			{
				return errReadLine;
			}
		}
		else
		{
			if ((unsigned long) errReadLine != SCK_NOTHINGTOREAD)
			{
				_ptTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errReadLine, __FILE__, __LINE__);

				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READLINE_FAILED);
				_ptTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return errReadLine;
			}
		}
	}

	{
		char					*pEndOfTheTraceLevel;
		char					pTraceLevel [
			TS_TRACERSOCKETPOOL_MAXTRACELENGTH];
		Tracer:: DefaultTraceLevel_t		tlTraceLevel;
		const char				*pTraceForTracer;


		// std:: cout << pTrace << std:: endl;
		// std:: cout. flush ();

		if ((pEndOfTheTraceLevel = strchr (pTrace, '_')) == (char *) NULL)
		{
			tlTraceLevel		= Tracer:: TRACER_LFTAL;
			pTraceForTracer		= pTrace;
		}
		else
		{
			strncpy (pTraceLevel, pTrace, pEndOfTheTraceLevel - pTrace);
			pTraceLevel [pEndOfTheTraceLevel - pTrace]	= '\0';
			tlTraceLevel		= (Tracer:: DefaultTraceLevel_t) atol (
				pTraceLevel);
			pTraceForTracer		= pEndOfTheTraceLevel + 1;
		}

		_ptTracer -> trace (tlTraceLevel, pTraceForTracer,
			__FILE__, __LINE__);
	}

	if (bClientSocketToBeAddedInSocketsPool)
	{
		if (addSocket (
			SocketsPool:: SOCKETSTATUS_READ |
			SocketsPool:: SOCKETSTATUS_EXCEPTION,
			TS_TRACERSOCKETPOOL_CLIENTSOCKET,
			pcsClientSocket, (void *) NULL) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
			_ptTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		{
			Message msg = TracerServerMessages (__FILE__, __LINE__,
				TS_TRACERSOCKETSPOOL_ADDEDCLIENTCONNECTION,
				2, pRemoteAddress, lRemotePort);
			_ptTracer -> trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	}


	return errNoError;
}

