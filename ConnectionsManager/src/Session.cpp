
#include "Session.h"
#include "ConnectionsManagerMessages.h"
#include "ConnectionEvent.h"
#include "WebUtility.h"
#include <assert.h>


Session:: Session (void)

{

}


Session:: ~Session (void)

{

}


Session:: Session (
	const Session &t)

{

	assert (1==0);

	// to do

	*this = t;
}


Error Session:: init (
	unsigned long ulConnectionIdentifier,
	CMSocketsPool_p pspSocketsPool,
	CMEventsSet_p pesEventsSet,
	unsigned long ulTimeoutToWaitRequestInSeconds,
	unsigned long ulAdditionalMicrosecondsToWait,
	Tracer_p ptTracer)

{
	Error_t					errSocket;


	// _ulConnectionIdentifier				= ulConnectionIdentifier;
	_ulTimeoutToWaitRequestInSeconds	= ulTimeoutToWaitRequestInSeconds;
	_ulAdditionalMicrosecondsToWait		= ulAdditionalMicrosecondsToWait;
	_pspSocketsPool						= pspSocketsPool;
	_pesEventsSet						= pesEventsSet;
	_ptSystemTracer						= ptTracer;

	// _tUTCStartConnectionTime			= 0;
	strcpy (_pClientIPAddress, "");
	_lClientPort						= 0;

	_ciConnectionInformation. _psSession		= this;
	_ciConnectionInformation. _ulConnectionIdentifier	=
		ulConnectionIdentifier;
	_ciConnectionInformation. _tUTCStartConnectionTime	= 0;
	_ciConnectionInformation. _tUTCLastRead				= 0;

	#if defined(__CYGWIN__)
		if (_mtSession. init (PMutex:: MUTEX_RECURSIVE) !=
			errNoError)
	#else							// POSIX.1-1996 standard (HPUX 11)
		if (_mtSession. init (PMutex:: MUTEX_RECURSIVE) !=
			errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errSocket = _csClientSocket. init (ulConnectionIdentifier)) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED,
			2, "<not applicable>", -1);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	return errNoError;
}


Error Session:: finish (void)

{

	if (_csClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_mtSession. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error Session:: reset (void)

{

	_ciConnectionInformation. _tUTCStartConnectionTime	= 0;
	_ciConnectionInformation. _tUTCLastRead				= 0;
	/*
	if (finish () != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_SESSION_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (init (_ciConnectionInformation. _ulConnectionIdentifier,
		_pspSocketsPool, _pesEventsSet,
		_ulTimeoutToWaitRequestInSeconds, _ulAdditionalMicrosecondsToWait,
		_ptSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_SESSION_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}
	*/


	return errNoError;
}


Error Session:: acceptConnection (
	ServerSocket_p pssServerSocket,
	ConnectionInformation_p *pciConnectionInformation)

{

	Error_t							errAcceptConnection;
	SocketImpl_p					psiClientSocketImpl;
	Error_t							errAddSocket;
	Error_t							errSocket;


	if (_mtSession. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errAcceptConnection = pssServerSocket -> acceptConnection (
		&_csClientSocket)) != errNoError)
	{
		Boolean_t				bIsThereError;
		// ConnectionInformation_p	pciConnectionInformation;


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
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_SESSION_NOTCONNECTIONTOACCEPT_FAILED);
				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				//	(const char *) err, __FILE__, __LINE__);

				if (_mtSession. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
			else
				bIsThereError			= true;
		}
		else
			bIsThereError			= true;

		if (bIsThereError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errAcceptConnection,
				__FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	_ciConnectionInformation. _tUTCStartConnectionTime		= time (NULL);
	_ciConnectionInformation. _tUTCLastRead		=
		_ciConnectionInformation. _tUTCStartConnectionTime;

	if (_csClientSocket. getSocketImpl (
		&psiClientSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (psiClientSocketImpl -> getRemoteAddress (
		_pClientIPAddress, SCK_MAXIPADDRESSLENGTH) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (psiClientSocketImpl -> getRemotePort (
		&_lClientPort) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETREMOTEPORT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	#ifdef WIN32
		if (psiClientSocketImpl -> setBlocking (false) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETBLOCKING_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_csClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if ((errSocket = _csClientSocket. init ()) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_INIT_FAILED,
					2, "<not applicable>", -1);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
			_ciConnectionInformation. _tUTCLastRead					= 0;

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	#endif

	if (psiClientSocketImpl -> setReceivingTimeout (
		_ulTimeoutToWaitRequestInSeconds,
		_ulAdditionalMicrosecondsToWait) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETRECEIVINGTIMEOUT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// setNoDelay important to call (see documentation in SocketImpl.h
	if (psiClientSocketImpl -> setNoDelay (true) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETNODELAY_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// setKeepAlive important to call (see documentation in SocketImpl.h
	if (psiClientSocketImpl -> setKeepAlive (true) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETKEEPALIVE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (psiClientSocketImpl -> setMaxSendBuffer (15 * 1024) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXSENDBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (psiClientSocketImpl -> setMaxReceiveBuffer (15 * 1024) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((errSocket = _csClientSocket. init ()) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	/*
	if ((errAddSocket = _pspSocketsPool -> addSocket (
		SocketsPool:: SOCKETSTATUS_READ |
		SocketsPool:: SOCKETSTATUS_EXCEPTION,
		CM_SBSOCKETPOOL_SOCKET,
		&_csClientSocket,
		&_ciConnectionInformation)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errAddSocket, __FILE__, __LINE__);

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (_csClientSocket. init () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, "<not applicable>", -1);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		_ciConnectionInformation. _tUTCStartConnectionTime		= 0;
		_ciConnectionInformation. _tUTCLastRead					= 0;

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}
	*/

	*pciConnectionInformation		= &_ciConnectionInformation;

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_SESSION_CONNECTIONARRIVED,
			4, _ciConnectionInformation. _ulConnectionIdentifier,
			_pClientIPAddress, _lClientPort,
			(unsigned long) _ciConnectionInformation. _tUTCStartConnectionTime);
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5, (const char *) msg,
			__FILE__, __LINE__);
	}

	if (_mtSession. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error Session:: releaseConnection (
	Boolean_t bDeleteFromSocketsPool)

{

	Error_t						errDeleteSocket;
	Error_t						errSocket;


	if (_mtSession. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (bDeleteFromSocketsPool)
	{
		{
			/*
			SocketImpl_p					psiClientSocketImpl;
			_csClientSocket. getSocketImpl (&psiClientSocketImpl);
			*/
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_SESSION_DELETESOCKET,
				2, _ciConnectionInformation. _ulConnectionIdentifier,
				0);
				// (long) (psiClientSocketImpl -> _iFd));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);
		}

		if ((errDeleteSocket = _pspSocketsPool -> deleteSocket (
			&_csClientSocket, (void **) NULL)) != errNoError)
		{
			// The scenario that could cause this error will be:
			//	THREAD 1 (socket to be read):
			//	1. socketpool wakes up a socket because there is something to
			//   	read
			//	2. the socket is removed from the socketspool
			//	3. readRequest
			//		3.1 if EOF
			//			3.1.1 the sockets is closed
			//			3.1.2 the socket is added to the free vector
			//		3.2 if request to read
			//			3.2.1 read
			//			3.2.2 ....
			//			3.2.3 write and add the socket to the sockets pool
			// THREAD 2 (unused sockets):
			//	1. the same socket is old (opened from a long time)
			//	2. the socket is removed from the socketspool (this method)
			//		If it fails returns an error and a next Session
			//		is considered (this code)
			//	3. the socket is closed
			//	4. the Session is added to te free vector
			//
			//	Scenarious 1:
			//		THREAD 2 is between 1 and 2
			//		THREAD 1 executes all the steps
			//
			//		In this case this deleteSocket fails and the THREAD 2
			//		must not continue to close the socket and must not add the
			//		session to the free vector
			//
			//	Scenarious 2:
			//		THREAD 2 is between 3 and 4
			//
			//	In this scenario, the THREAD 1 fails when the readRequest
			//	is executed and is always followed by the warning
			//	CM_SESSION_REQUESTNOTREADBECAUSEOLDSOCKET because the
			//	THREAD 1 will go to read but it found the Session
			//	already closed
			_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
				(const char *) errDeleteSocket, __FILE__, __LINE__);

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_DELETESOCKETFAILED,
				1, _ciConnectionInformation. _ulConnectionIdentifier);
			// Error err = SocketErrors (__FILE__, __LINE__,
			// 	SCK_SOCKETSPOOL_DELETESOCKET_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
				(const char *) err, __FILE__, __LINE__);

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (_csClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if ((errSocket = _csClientSocket. init ()) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED,
			2, "<not applicable>", -1);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// 2013-05-21. Previously, next settings (to 0) were after
	// _mtSession. lock. They were moved here because in some cases
	// the above check on bDeleteFromSocketsPool return to the calling
	// method. That means variable setting to 0, and return error to the calling
	// method. Because of the error returned, the calling method does not
	// add the session in the free vector and, because of the variables
	// set to 0, the session does not never enter in the expired sessions and
	// this session was "lost"
	_ciConnectionInformation. _tUTCStartConnectionTime			= 0;
	_ciConnectionInformation. _tUTCLastRead						= 0;

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_SESSION_CONNECTIONCLOSED,
			4, _ciConnectionInformation. _ulConnectionIdentifier,
			_pClientIPAddress, _lClientPort,
			(unsigned long) time (NULL));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5, (const char *) msg,
			__FILE__, __LINE__);
	}

	if (_mtSession. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error Session:: isOlderThan (
	time_t *ptNowInSeconds,
	unsigned long *pulMaxTTLSession,
	Boolean_p pbIsOld, Boolean_p pbIsFreeSession)

{

	time_t					tMaxTTLSession;
	time_t					tConnectionDuration;


	tMaxTTLSession			= *pulMaxTTLSession;

	if (_mtSession. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_ciConnectionInformation. _tUTCStartConnectionTime != 0)
	{
		*pbIsFreeSession				= false;

		tConnectionDuration				= *ptNowInSeconds -
			_ciConnectionInformation. _tUTCLastRead;

		if (tConnectionDuration >= tMaxTTLSession)
		{
			*pbIsOld			= true;

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_FINDOLDCONNECTION,
				2, _ciConnectionInformation. _ulConnectionIdentifier,
				(unsigned long) tConnectionDuration);
			_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
				(const char *) err, __FILE__, __LINE__);
		}
		else
		{
			*pbIsOld			= false;
		}
	}
	else
	{
		*pbIsFreeSession				= true;

		*pbIsOld			= false;
	}

	if (_mtSession. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error Session:: readRequest (
	time_t *ptUTCStartConnectionTime,
	Buffer_p pbRequestHeader,
	Buffer_p pbRequestBody,
	Buffer_p pbURL,
	Buffer_p pbRequestUserAgent,
	Buffer_p pbRequestCookie)

{

	SocketImpl_p					psiClientSocketImpl;
	WebUtility:: HttpMethod_t		hmHttpMethod;
	Error_t							errRead;


	if (_mtSession. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (*ptUTCStartConnectionTime !=
		_ciConnectionInformation. _tUTCStartConnectionTime)
	{
		// this if cover both the cases:
		//	- connection is not actived (_tUTCStartConnectionTime will be 0)
		//	- connection is actived but is not the old one's
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_SESSION_REQUESTNOTREADBECAUSEOLDSOCKET,
			3, _ciConnectionInformation. _ulConnectionIdentifier,
			(unsigned long) (*ptUTCStartConnectionTime),
			(unsigned long) 
				(_ciConnectionInformation. _tUTCStartConnectionTime));
		_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_csClientSocket. getSocketImpl (
		&psiClientSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	#ifdef WIN32
		// In windows I saw it's better to check if there is something to read
		// before start to read
		if ((errRead = WebUtility:: readHttpHeaderAndBody (
			psiClientSocketImpl,
			_ulTimeoutToWaitRequestInSeconds,
			_ulAdditionalMicrosecondsToWait,
			pbRequestHeader,
			pbRequestBody,
			&hmHttpMethod,
			pbURL,
			(Buffer_p) NULL,
			pbRequestUserAgent,
			pbRequestCookie,
			(Buffer_p) NULL,
			(Buffer_p) NULL		// Location
		)) != errNoError)
	#else
		if ((errRead = WebUtility:: readHttpHeaderAndBody (
			psiClientSocketImpl,
			0, // timeout already initialized before
			0,
			pbRequestHeader,
			pbRequestBody,
			&hmHttpMethod,
			pbURL,
			(Buffer_p) NULL,
			pbRequestUserAgent,
			pbRequestCookie,
			(Buffer_p) NULL,
			(Buffer_p) NULL		// Location
		)) != errNoError)
	#endif
	{
		if ((unsigned long) errRead !=
			WEBTOOLS_HTTPGETTHREAD_BODYTIMEOUTEXPIRED)
		{
			if ((unsigned long) errRead != SCK_READ_EOFREACHED)
			{
				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 	(const char *) errRead, __FILE__, __LINE__);

				if ((unsigned long) errRead ==
					WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED)
				{
					Error_t					errAddSocket;


					// that should be a streamer problem, we have this error
					// when the streamer is like "blocked for some seconds
					// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					// 	CM_SESSION_CONNECTIONFAILED,
					// 	2, _ciConnectionInformation. _ulConnectionIdentifier,
					// 	(const char *) errRead);
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) err, __FILE__, __LINE__);

					{
						Message msg = ConnectionsManagerMessages (
							__FILE__, __LINE__,
							CM_SESSION_ADDSOCKET,
							2,
							_ciConnectionInformation. _ulConnectionIdentifier,
							0);
							// (long) (psiClientSocketImpl -> _iFd));
						_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
							(const char *) msg, __FILE__, __LINE__);
					}

					if ((errAddSocket = _pspSocketsPool -> addSocket (
						SocketsPool:: SOCKETSTATUS_READ |
						SocketsPool:: SOCKETSTATUS_EXCEPTION,
						CM_CMSOCKETPOOL_CMSOCKET,
						&_csClientSocket,
						&_ciConnectionInformation)) != errNoError)
					{
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) errAddSocket, __FILE__, __LINE__);

						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_ADDSOCKETFAILED,
							1,
							_ciConnectionInformation. _ulConnectionIdentifier);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if (_mtSession. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						return err;
					}
				}
				else
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_CONNECTIONFAILED,
						2, _ciConnectionInformation. _ulConnectionIdentifier,
						(const char *) errRead);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
				/*
				{
					Error err = WebToolsErrors (__FILE__, __LINE__,
						WEBTOOLS_WEBUTILITY_READHTTPHEADERANDBODY_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
				*/
			}

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return errRead;
		}
		else
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errRead, __FILE__, __LINE__);

			// 2013-07-25: Since in the calling CMProcessor, when this error
			// 	is returned, the session is not placed in _pvFreeSessions,
			// 	in this case we have to add this socket to the pool otherwise
			// 	it will be lost (neither in the _pvFreeSessions and neither
			// 	in socketpool)
			{
				Error_t					errAddSocket;


				// that should be a streamer problem, we have this error
				// when the streamer is like "blocked for some seconds
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				// 	CM_SESSION_CONNECTIONFAILED,
				// 	2, _ciConnectionInformation. _ulConnectionIdentifier,
				// 	(const char *) errRead);
				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 	(const char *) err, __FILE__, __LINE__);

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_SESSION_ADDSOCKET,
						2,
						_ciConnectionInformation. _ulConnectionIdentifier,
						0);
						// (long) (psiClientSocketImpl -> _iFd));
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
						(const char *) msg, __FILE__, __LINE__);
				}

				if ((errAddSocket = _pspSocketsPool -> addSocket (
					SocketsPool:: SOCKETSTATUS_READ |
					SocketsPool:: SOCKETSTATUS_EXCEPTION,
					CM_CMSOCKETPOOL_CMSOCKET,
					&_csClientSocket,
					&_ciConnectionInformation)) != errNoError)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errAddSocket, __FILE__, __LINE__);

					Error err = ConnectionsManagerErrors (
						__FILE__, __LINE__,
						CM_SESSION_ADDSOCKETFAILED,
						1,
						_ciConnectionInformation. _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_mtSession. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}
		}
	}

	_ciConnectionInformation. _tUTCLastRead		= time (NULL);

	if (_mtSession. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error Session:: writeResponse (
	time_t *ptUTCStartConnectionTime,
	Buffer_p pbResponseHeader,
	Buffer_p pbResponseBody,
	Boolean_t bIsResponseFinished)

{

	SocketImpl_p			psiClientSocketImpl;
	Error_t					errAddSocket;
	Error_t					errWrite;


	if (_mtSession. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (*ptUTCStartConnectionTime !=
		_ciConnectionInformation. _tUTCStartConnectionTime)
	{
		// this if cover both the cases:
		//	- connection is not actived (_tUTCStartConnectionTime will be 0)
		//	- connection is actived but is not the old one's
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_SESSION_RESPONSENOTSENTBECAUSEOLDSOCKET,
			4, _ciConnectionInformation. _ulConnectionIdentifier,
			(unsigned long) *ptUTCStartConnectionTime,
			(unsigned long)
				(_ciConnectionInformation. _tUTCStartConnectionTime));
		_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_csClientSocket. getSocketImpl (
		&psiClientSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtSession. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (pbResponseHeader != (Buffer_p) NULL &&
		(unsigned long) (*pbResponseHeader) > 0)
	{
		if ((errWrite = psiClientSocketImpl -> writeString (
			(const char *) (*pbResponseHeader), true, 2, 0)) !=
			errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errWrite, __FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (pbResponseBody != (Buffer_p) NULL &&
		(unsigned long) (*pbResponseBody) > 0)
	{
		if ((errWrite = psiClientSocketImpl -> writeString (
			(const char *) (*pbResponseBody), true, 2, 0)) !=
			errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errWrite, __FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	{
		if (pbResponseHeader != (Buffer_p) NULL)
			pbResponseHeader -> substitute (CM_SESSION_HTTPNEWLINE, " "); 

		if (pbResponseBody != (Buffer_p) NULL)
			pbResponseBody -> substitute (CM_SESSION_NEWLINE, " "); 

		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_SESSION_WROTERESPONSE,
			8,
			_ciConnectionInformation. _ulConnectionIdentifier,
			_pClientIPAddress, _lClientPort,
			bIsResponseFinished,
			(pbResponseHeader != (Buffer_p) NULL) ?
				((unsigned long) (*pbResponseHeader)) : 0,
			(pbResponseHeader != (Buffer_p) NULL) ?
				((const char *) (*pbResponseHeader)) : "<no header>",
			(pbResponseBody != (Buffer_p) NULL) ?
				((unsigned long) (*pbResponseBody)) : 0,
			(pbResponseBody != (Buffer_p) NULL) ?
				((const char *) (*pbResponseBody)) : "");
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6, (const char *) msg,
			__FILE__, __LINE__);
	}

	if (bIsResponseFinished)
	{
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_SESSION_ADDSOCKET,
				2, _ciConnectionInformation. _ulConnectionIdentifier,
				0);
				// (long) (psiClientSocketImpl -> _iFd));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);
		}

		// add socked is made only if writeResponseToStreamer success 
		if ((errAddSocket = _pspSocketsPool -> addSocket (
			SocketsPool:: SOCKETSTATUS_READ |
			SocketsPool:: SOCKETSTATUS_EXCEPTION,
			CM_CMSOCKETPOOL_CMSOCKET,
			&_csClientSocket,
			&_ciConnectionInformation)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errAddSocket, __FILE__, __LINE__);

			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SESSION_ADDSOCKETFAILED,
				1, _ciConnectionInformation. _ulConnectionIdentifier);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtSession. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (_mtSession. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


unsigned long Session:: getConnectionIdentifier (void)

{

	return _ciConnectionInformation. _ulConnectionIdentifier;
}

