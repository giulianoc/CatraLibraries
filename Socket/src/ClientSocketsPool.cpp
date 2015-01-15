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

// #include "DateTime.h"
#include <assert.h>
#ifdef WIN32
	#include <winsock2.h>
#endif
#include "ClientSocketsPool.h"
#include "FileIO.h"


ClientSocketsPool:: ClientSocketsPool (void)

{

	_cssClientSocketsPoolStatus			= CSP_BUILDED;

// char aaa [512];
// sprintf (aaa, "%ld ClientSocketsPool:: ClientSocketsPool", (long) getpid());
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);

}


ClientSocketsPool:: ~ClientSocketsPool (void)

{

// char aaa [512];
// sprintf (aaa, "%ld ClientSocketsPool:: ~ClientSocketsPool", (long) getpid());
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);

	if (_cssClientSocketsPoolStatus == CSP_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKETSPOOL_FINISH_FAILED);
		}
	}
}


ClientSocketsPool:: ClientSocketsPool (const ClientSocketsPool &)

{

	assert (1==0);
}


ClientSocketsPool &ClientSocketsPool:: operator = (const ClientSocketsPool &)

{

	assert (1==0);

	return *this;
}


Error ClientSocketsPool:: init (
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
	Boolean_t bReuseAddr,
	const char *pLocalAddress,
	long lRemotePort,
	unsigned long ulClientSocketsToBeAllocatedPerType,
	long lTTLInSeconds)

{

	if (_cssClientSocketsPoolStatus != CSP_BUILDED ||
		(pLocalAddress != (const char *) NULL &&
		strlen (pLocalAddress) >= SCK_MAXIPADDRESSLENGTH))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED,
			1, (long) _cssClientSocketsPoolStatus);

		return err;
	}

	#if defined(__CYGWIN__)
		if (_mtClientSocketsInfo. init (
			PMutex:: MUTEX_RECURSIVE) != errNoError)
	#else							// POSIX.1-1996 standard (HPUX 11)
		if (_mtClientSocketsInfo. init (
			PMutex:: MUTEX_RECURSIVE) != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		return err;
	}

	if ((_phHasher = new BufferHasher_t) == (BufferHasher_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		if (_mtClientSocketsInfo. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		return err;
	}

	if ((_pcComparer = new BufferCmp_t) == (BufferCmp_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		delete _phHasher;
		_phHasher		= (BufferHasher_p) NULL;

		if (_mtClientSocketsInfo. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		return err;
	}

	if ((_phmClientSocketsInfo = new ClientSocketsInfoHashMap_t (
		20, *_phHasher, *_pcComparer)) ==
		(ClientSocketsInfoHashMap_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		delete _pcComparer;
		_pcComparer		= (BufferCmp_p) NULL;

		delete _phHasher;
		_phHasher		= (BufferHasher_p) NULL;

		if (_mtClientSocketsInfo. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		return err;
	}

	_ulReceivingTimeoutInSeconds			= ulReceivingTimeoutInSeconds;
	_ulReceivingAdditionalTimeoutInMicroSeconds		=
		ulReceivingAdditionalTimeoutInMicroSeconds;
	_ulSendingTimeoutInSeconds				= ulSendingTimeoutInSeconds;
	_ulSendingAdditionalTimeoutInMicroSeconds		=
		ulSendingAdditionalTimeoutInMicroSeconds;
	_bReuseAddr								= bReuseAddr;
	if (pLocalAddress == (const char *) NULL)
		strcpy (_pLocalAddress, "");
	else
		strcpy (_pLocalAddress, pLocalAddress);
	_lRemotePort							= lRemotePort;

	_ulClientSocketsToBeAllocatedPerType		=
		ulClientSocketsToBeAllocatedPerType;
	_lTTLInSeconds								= lTTLInSeconds;

	_cssClientSocketsPoolStatus					= CSP_INITIALIZED;


	return errNoError;
}


Error ClientSocketsPool:: finish (void)

{

	if (_cssClientSocketsPoolStatus != CSP_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED,
			1, (long) _cssClientSocketsPoolStatus);

		return err;
	}

	{
		ClientSocketsInfoHashMap_t:: iterator	it;
		Buffer_p								pbRemoteIPAddress;
		ClientSocketsInfo_p						pcstClientSocketsInfo;
		long									lClientSocketIndex;


		if (_mtClientSocketsInfo. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			// return err;
		}

		for (it = _phmClientSocketsInfo -> begin ();
			it != _phmClientSocketsInfo -> end ();
			++it)
		{
			pbRemoteIPAddress				= it -> first;

			pcstClientSocketsInfo			= it -> second;

			if (pbRemoteIPAddress -> finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			for (lClientSocketIndex = 0;
				lClientSocketIndex < _ulClientSocketsToBeAllocatedPerType;
				lClientSocketIndex++)
			{
				if (((pcstClientSocketsInfo -> _pcsClientSockets) [
					lClientSocketIndex]). getClientSocketStatus () ==
					ClientSocket:: SCK_CLIENTSOCKET_INITIALIZED)
				{
					if ((pcstClientSocketsInfo -> _pcsClientSockets) [
						lClientSocketIndex]. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_INIT_FAILED);
					}
				}
			}

			delete [] (pcstClientSocketsInfo -> _pcsClientSockets);

			(pcstClientSocketsInfo -> _vFreeClientSockets). clear ();

			delete pcstClientSocketsInfo;
			pcstClientSocketsInfo		= (ClientSocketsInfo_p) NULL;
		}

		_phmClientSocketsInfo -> clear ();

		delete _phmClientSocketsInfo;

		if (_mtClientSocketsInfo. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		delete _pcComparer;
		delete _phHasher;

		if (_mtClientSocketsInfo. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	}

	_cssClientSocketsPoolStatus		= CSP_BUILDED;


	return errNoError;
}


Error ClientSocketsPool:: getFreeClientSocket (
	Buffer_p pbRemoteIPAddress,
	ClientSocket_p *pcsClientSocket)

{

	ClientSocketsInfoHashMap_t:: iterator		it;
	ClientSocketsInfo_p							pcstClientSocketsInfo;


	if (_mtClientSocketsInfo. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

// char aaa [512];
// sprintf (aaa, "%ld Look for : %s (%ld)", (long) getpid(), (const char *) (*pbRemoteIPAddress),
	// (long) (_phmClientSocketsInfo -> size()));
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);
	it		= _phmClientSocketsInfo -> find (pbRemoteIPAddress);

	if (it == _phmClientSocketsInfo -> end ())
	{
		long					lClientSocketIndex;
		int						iDidInsert;


		if ((pcstClientSocketsInfo = new ClientSocketsInfo_t) ==
			(ClientSocketsInfo_p) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if ((pcstClientSocketsInfo -> _bRemoteIPAddress). init (
			(const char *) (*pbRemoteIPAddress)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			delete pcstClientSocketsInfo;
			pcstClientSocketsInfo			= (ClientSocketsInfo_p) NULL;

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (((pcstClientSocketsInfo -> _pcsClientSockets) =
			new ClientSocket_t [_ulClientSocketsToBeAllocatedPerType]) ==
			(ClientSocket_p) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			if ((pcstClientSocketsInfo -> _bRemoteIPAddress). finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			delete pcstClientSocketsInfo;
			pcstClientSocketsInfo			= (ClientSocketsInfo_p) NULL;

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		(pcstClientSocketsInfo -> _vFreeClientSockets). clear ();
		(pcstClientSocketsInfo -> _vFreeClientSockets). reserve (
			_ulClientSocketsToBeAllocatedPerType);

		for (lClientSocketIndex = 0;
			lClientSocketIndex < _ulClientSocketsToBeAllocatedPerType;
			lClientSocketIndex++)
		{
			/*
			if ((errClientSocketInit =
				(pcstClientSocketsInfo -> _pcsClientSockets) [
				lClientSocketIndex]. init (
				SocketImpl:: STREAM,
				_ulReceivingTimeoutInSeconds,
				_ulReceivingAdditionalTimeoutInMicroSeconds,
				_ulSendingTimeoutInSeconds,
				_ulSendingAdditionalTimeoutInMicroSeconds,
				_bReuseAddr,
				_pLocalAddress,
				(const char *) (pcstClientSocketsInfo -> _bRemoteIPAddress),
				_lRemotePort)) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_INIT_FAILED);

				while (--lClientSocketIndex >= 0)
				{
					if ((pcstClientSocketsInfo -> _pcsClientSockets) [
						lClientSocketIndex]. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_INIT_FAILED);
					}
				}

				(pcstClientSocketsInfo -> _vFreeClientSockets). clear ();

				delete [] (pcstClientSocketsInfo -> _pcsClientSockets);
				pcstClientSocketsInfo -> _pcsClientSockets		=
					(ClientSocket_p) NULL;

				if ((pcstClientSocketsInfo -> _bRemoteIPAddress). finish () !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				delete pcstClientSocketsInfo;
				pcstClientSocketsInfo			= (ClientSocketsInfo_p) NULL;

				if (_mtClientSocketsInfo. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
			*/

			if ((pcstClientSocketsInfo -> _pcsClientSockets) [
				lClientSocketIndex]. setIdentifier (lClientSocketIndex) !=
				errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_SETIDENTIFIER_FAILED);

				(pcstClientSocketsInfo -> _vFreeClientSockets). clear ();

				delete [] (pcstClientSocketsInfo -> _pcsClientSockets);
				pcstClientSocketsInfo -> _pcsClientSockets		=
					(ClientSocket_p) NULL;

				if ((pcstClientSocketsInfo -> _bRemoteIPAddress). finish () !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				delete pcstClientSocketsInfo;
				pcstClientSocketsInfo			= (ClientSocketsInfo_p) NULL;

				if (_mtClientSocketsInfo. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}

// {
// char aaa [512];
// sprintf (aaa, "%ld CLIENT[%ld]: %lu (%ld), status: %ld",
	// (long) getpid(),
	// lClientSocketIndex,
	// ((pcstClientSocketsInfo -> _pcsClientSockets) [lClientSocketIndex]).getIdentifier(),
	// (pcstClientSocketsInfo -> _vFreeClientSockets). size(),
	// (long) (((pcstClientSocketsInfo -> _pcsClientSockets) [lClientSocketIndex]). getClientSocketStatus ()));
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);
// }
			(pcstClientSocketsInfo -> _vFreeClientSockets). insert (
				(pcstClientSocketsInfo -> _vFreeClientSockets). end (),
				&((pcstClientSocketsInfo -> _pcsClientSockets) [
					lClientSocketIndex])
				);
		}

		_phmClientSocketsInfo -> InsertWithoutDuplication (
			&(pcstClientSocketsInfo -> _bRemoteIPAddress),
			pcstClientSocketsInfo, &iDidInsert);
// sprintf (aaa, "%ld ADDED RemoteIPAddress: %s (%ld)",
	// (long) getpid(),
	// (const char *) (pcstClientSocketsInfo -> _bRemoteIPAddress),
	// (long) (_phmClientSocketsInfo -> size()));
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);

		if (iDidInsert == 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKETSPOOL_CLIENTSOCKETTYPEALREADYPRESENT,
				1,
				(const char *) (pcstClientSocketsInfo -> _bRemoteIPAddress));

			/*
			for (lClientSocketIndex = 0;
				lClientSocketIndex < _ulClientSocketsToBeAllocatedPerType;
				lClientSocketIndex++)
			{
				if ((pcstClientSocketsInfo -> _pcsClientSockets) [
					lClientSocketIndex]. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_INIT_FAILED);
				}
			}
			*/

			(pcstClientSocketsInfo -> _vFreeClientSockets). clear ();

			delete [] (pcstClientSocketsInfo -> _pcsClientSockets);
			pcstClientSocketsInfo -> _pcsClientSockets		=
				(ClientSocket_p) NULL;

			if ((pcstClientSocketsInfo -> _bRemoteIPAddress). finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			delete pcstClientSocketsInfo;
			pcstClientSocketsInfo			= (ClientSocketsInfo_p) NULL;

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}
	else
	{
		pcstClientSocketsInfo			= it -> second;
	}

	if ((pcstClientSocketsInfo -> _vFreeClientSockets). size () == 0)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKETSPOOL_NOCLIENTSOCKETAVAILABLE,
			1, (const char *) (pcstClientSocketsInfo -> _bRemoteIPAddress));

		if (_mtClientSocketsInfo. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	*pcsClientSocket			=
		*((pcstClientSocketsInfo -> _vFreeClientSockets). begin ());

	(pcstClientSocketsInfo -> _vFreeClientSockets). erase (
		(pcstClientSocketsInfo -> _vFreeClientSockets). begin ());

	if ((*pcsClientSocket) -> getClientSocketStatus () ==
		ClientSocket:: SCK_CLIENTSOCKET_BUILDED)
	{
		Error_t					errClientSocketInit;


		if ((errClientSocketInit =
			(*pcsClientSocket) -> init (
			SocketImpl:: STREAM,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingAdditionalTimeoutInMicroSeconds,
			_ulSendingTimeoutInSeconds,
			_ulSendingAdditionalTimeoutInMicroSeconds,
			_ulReceivingTimeoutInSeconds,
			_bReuseAddr,
			_pLocalAddress,
			(const char *) (pcstClientSocketsInfo -> _bRemoteIPAddress),
			_lRemotePort)) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED);

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
// sprintf (aaa, "%ld GETCLIENT->INIT, id: %lu (%ld), status: %ld", (long) getpid(), (*pcsClientSocket) -> getIdentifier(), (pcstClientSocketsInfo -> _vFreeClientSockets). size(), (long) ((*pcsClientSocket) -> getClientSocketStatus ()));
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);
	}
	else
	{
		SocketImpl_p				pClientSocketImpl;
		Boolean_t					bIsReadyForWriting;


// sprintf (aaa, "%ld GETCLIENT->NOINIT, id: %lu (%ld)", (long) getpid(), (*pcsClientSocket)->getIdentifier(), (long) (pcstClientSocketsInfo -> _vFreeClientSockets). size());
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);

		if (_lTTLInSeconds != -1 &&
			time (NULL) - (*pcsClientSocket) ->
			getConnectUTCStartTimeInSeconds () >= _lTTLInSeconds)
		{
			if ((*pcsClientSocket) -> reinit () != errNoError)
			{
				Error_t err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_REINIT_FAILED);

				if (_mtClientSocketsInfo. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}

		if ((*pcsClientSocket) -> getSocketImpl (&pClientSocketImpl) !=
			errNoError)
		{
			Error_t err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (pClientSocketImpl -> isReadyForWriting (&bIsReadyForWriting,
			1, 0) != errNoError)
		{
			Error_t err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_ISREADYFORWRITING_FAILED);

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (!bIsReadyForWriting)
		{
			if ((*pcsClientSocket) -> reinit () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_REINIT_FAILED);

				if (_mtClientSocketsInfo. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}
	}

	if (_mtClientSocketsInfo. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error ClientSocketsPool:: releaseClientSocket (
	Buffer_p pbRemoteIPAddress,
	ClientSocket_p pcsClientSocket,
	Boolean_t bSocketToBeClosed)

{

	ClientSocketsInfoHashMap_t:: iterator		it;
	ClientSocketsInfo_p							pcstClientSocketsInfo;


	if (_mtClientSocketsInfo. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

// char aaa [256];
// sprintf (aaa, "%ld RELEASECLIENT, id: %lu status: %ld", (long) getpid(), pcsClientSocket->getIdentifier(), (long) (pcsClientSocket -> getClientSocketStatus ()));
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);

	it		= _phmClientSocketsInfo -> find (pbRemoteIPAddress);

	if (it == _phmClientSocketsInfo -> end ())
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKETSPOOL_CLIENTSOCKETTYPENOTFOUND,
			1, (const char *) (*pbRemoteIPAddress));

		if (_mtClientSocketsInfo. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (bSocketToBeClosed)
	{
		if (pcsClientSocket -> finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);

			if (_mtClientSocketsInfo. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}

	pcstClientSocketsInfo			= it -> second;

	(pcstClientSocketsInfo -> _vFreeClientSockets). insert (
		(pcstClientSocketsInfo -> _vFreeClientSockets). begin (),
		pcsClientSocket);

	if (_mtClientSocketsInfo. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error ClientSocketsPool:: getFreeClientSocket (
	const char *pRemoteIPAddress,
	ClientSocket_p *pcsClientSocket)

{
	Buffer_t				bRemoteIPAddress;


	if (bRemoteIPAddress. init (pRemoteIPAddress) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (getFreeClientSocket (&bRemoteIPAddress, pcsClientSocket) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKETSPOOL_GETFREECLIENTSOCKET_FAILED);

		if (bRemoteIPAddress. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bRemoteIPAddress. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error ClientSocketsPool:: releaseClientSocket (
	const char *pRemoteIPAddress,
	ClientSocket_p pcsClientSocket,
	Boolean_t bSocketToBeClosed)

{
	Buffer_t				bRemoteIPAddress;


	if (bRemoteIPAddress. init (pRemoteIPAddress) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (releaseClientSocket (&bRemoteIPAddress, pcsClientSocket) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKETSPOOL_RELEASECLIENTSOCKET_FAILED);

		if (bRemoteIPAddress. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bRemoteIPAddress. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


