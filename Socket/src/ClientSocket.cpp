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


#include "ClientSocket.h"
#include "Network.h"
#include <assert.h>
#include <time.h>



ClientSocket:: ClientSocket (void): Socket ()

{

	_sClientSocketStatus		= SCK_CLIENTSOCKET_BUILDED;

	_tConnectUTCStartTimeInSeconds		= 0;

	_ulReceivingTimeoutInSeconds		= 0;
	_ulReceivingAdditionalTimeoutInMicroSeconds		= 0;
	_ulSendingTimeoutInSeconds			= 0;
	_ulSendingAdditionalTimeoutInMicroSeconds		= 0;
	_ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS		= 0;
	_bReuseAddr							= false;
	strcpy (_pLocalAddress, "");
	strcpy (_pRemoteAddress, "");
	_lRemotePort						= 0;

}


ClientSocket:: ~ClientSocket (void)

{

	if (_sClientSocketStatus == SCK_CLIENTSOCKET_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}
	}

}


ClientSocket:: ClientSocket (const ClientSocket &t)

{

	assert (1 == 0);

}


ClientSocket:: ClientSocketStatus_t ClientSocket:: getClientSocketStatus (void)

{

	return _sClientSocketStatus;
}


Error ClientSocket:: init (unsigned long ulIdentifier)

{

	if (_sClientSocketStatus != SCK_CLIENTSOCKET_BUILDED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sClientSocketStatus);

		return err;
	}

	if (Socket:: init (ulIdentifier) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_INIT_FAILED);

		return err;
	}

	_tConnectUTCStartTimeInSeconds		= 0;

	_bSocketImplCreated					= false;

	_sClientSocketStatus				= SCK_CLIENTSOCKET_INITIALIZED;


	return errNoError;
}


Error ClientSocket:: init (SocketImpl:: SocketType_t stSocketType,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
	unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS,
	Boolean_t bReuseAddr, const char *pLocalAddress,
	const char *pRemoteServer, long lRemotePort,
	unsigned long ulIdentifier)

{

	Error_t				errCreate;
	Error_t				errConnect;
	Error_t				errBind;


	if (_sClientSocketStatus != SCK_CLIENTSOCKET_BUILDED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sClientSocketStatus);

		return err;
	}

	if (stSocketType == SocketImpl:: STREAM &&
		(pRemoteServer == (const char *) NULL ||
		lRemotePort < 1))
		// || strlen (pRemoteAddress) > SCK_MAXIPADDRESSLENGTH - 1))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	_stSocketType						= stSocketType;
	_ulReceivingTimeoutInSeconds		= ulReceivingTimeoutInSeconds;
	_ulReceivingAdditionalTimeoutInMicroSeconds		=
		ulReceivingAdditionalTimeoutInMicroSeconds;
	_ulSendingTimeoutInSeconds			= ulSendingTimeoutInSeconds;
	_ulSendingAdditionalTimeoutInMicroSeconds		=
		ulSendingAdditionalTimeoutInMicroSeconds;
	_ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS		=
		ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS;
	_bReuseAddr							= bReuseAddr;
	if (pLocalAddress == (const char *) NULL)
		strcpy (_pLocalAddress, "");
	else
		strcpy (_pLocalAddress, pLocalAddress);
	if (pRemoteServer == (const char *) NULL)
		strcpy (_pRemoteAddress, "");
	else
	{
		Error_t				errNetwork;

		if ((errNetwork = Network::getIPAddressFromHostName (pRemoteServer,
			_pRemoteAddress, SCK_MAXHOSTNAMELENGTH)) != errNoError)
		{
            Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NETWORK_GETIPADDRESSFROMHOSTNAME, 
				2, pRemoteServer, errNetwork. str());

			return err;
		}
        // strcpy (_pRemoteAddress, pRemoteAddress);
	}
	_lRemotePort						= lRemotePort;

	if (init (ulIdentifier) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED, 
			2, "<not applicable>", -1);

		return err;
	}

	if ((errCreate = _pSocketImpl -> create (stSocketType,
		ulReceivingTimeoutInSeconds,
		ulReceivingAdditionalTimeoutInMicroSeconds,
		ulSendingTimeoutInSeconds,
		ulSendingAdditionalTimeoutInMicroSeconds,
		bReuseAddr)) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_CREATE_FAILED);

		return err;
		*/

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errCreate;
	}

	_bSocketImplCreated				= true;

	if ((errBind = _pSocketImpl -> bind (pLocalAddress, 0)) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_BIND_FAILED);

		return err;
		*/

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	// setNoDelay important to call (see documentation in SocketImpl.h
	if (_pSocketImpl -> setNoDelay (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETNODELAY_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setKeepAlive (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETKEEPALIVE_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setMaxSendBuffer (15 * 1024) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXSENDBUFFER_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setMaxReceiveBuffer (15 * 1024) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (stSocketType == SocketImpl:: STREAM)
	{
		if ((errConnect = _pSocketImpl -> connect (_pRemoteAddress,
			lRemotePort, ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS)) !=
			errNoError)
		{
			/*
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_CONNECT_FAILED);

			return err;
			*/

			if (finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			return errConnect;
		}
	}

	_tConnectUTCStartTimeInSeconds		= time (NULL);

	_sClientSocketStatus				= SCK_CLIENTSOCKET_INITIALIZED;


	return errNoError;
}


Error ClientSocket:: init (SocketImpl:: SocketType_t stSocketType,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
	unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS,
	Boolean_t bReuseAddr, const char *pLocalAddress,
	struct sockaddr_in *psckServerAddr, unsigned long ulIdentifier)

{

	Error_t				errCreate;
	Error_t				errConnect;
	Error_t				errBind;


	if (_sClientSocketStatus != SCK_CLIENTSOCKET_BUILDED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sClientSocketStatus);

		return err;
	}

	if (stSocketType == SocketImpl:: STREAM &&
		psckServerAddr == (struct sockaddr_in *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (init (ulIdentifier) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED, 
			2, "<not applicable>", -1);

		return err;
	}

	if ((errCreate = _pSocketImpl -> create (stSocketType,
		ulReceivingTimeoutInSeconds,
		ulReceivingAdditionalTimeoutInMicroSeconds,
		ulSendingTimeoutInSeconds,
		ulSendingAdditionalTimeoutInMicroSeconds,
		bReuseAddr)) !=
		errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_CREATE_FAILED);

		return err;
		*/

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errCreate;
	}

	_bSocketImplCreated				= true;

	if ((errBind = _pSocketImpl -> bind (pLocalAddress, 0)) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_BIND_FAILED);

		return err;
		*/

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	// setNoDelay important to call (see documentation in SocketImpl.h
	if (_pSocketImpl -> setNoDelay (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETNODELAY_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setKeepAlive (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETKEEPALIVE_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setMaxSendBuffer (15 * 1024) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXSENDBUFFER_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (_pSocketImpl -> setMaxReceiveBuffer (15 * 1024) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED);

		if (finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
		}

		return errBind;
	}

	if (stSocketType == SocketImpl:: STREAM)
	{
		if ((errConnect = _pSocketImpl -> connect (psckServerAddr,
			ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS)) !=
			errNoError)
		{
			/*
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_CONNECT_FAILED);

			return err;
			*/

			if (finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
			}

			return errConnect;
		}
	}

	_tConnectUTCStartTimeInSeconds		= time (NULL);

	_sClientSocketStatus				= SCK_CLIENTSOCKET_INITIALIZED;


	return errNoError;
}


Error ClientSocket:: finish (void)

{

	if (_sClientSocketStatus != SCK_CLIENTSOCKET_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sClientSocketStatus);

		return err;
	}

	if (_bSocketImplCreated)
	{
		if (_pSocketImpl -> close () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_CLOSE_FAILED);

			return err;
		}
	}

	if (Socket:: finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_FINISH_FAILED);

		return err;
	}

	_tConnectUTCStartTimeInSeconds		= 0;

	_sClientSocketStatus				= SCK_CLIENTSOCKET_BUILDED;


	return errNoError;
}


time_t ClientSocket:: getConnectUTCStartTimeInSeconds (void)

{

	return _tConnectUTCStartTimeInSeconds;
}


Error ClientSocket:: reinit (void)

{

	if (finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);

		return err;
	}

	if (init (_stSocketType,
		_ulReceivingTimeoutInSeconds,
		_ulReceivingAdditionalTimeoutInMicroSeconds,
		_ulSendingTimeoutInSeconds,
		_ulSendingAdditionalTimeoutInMicroSeconds,
		_ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS,
		_bReuseAddr,
		_pLocalAddress,
		_pRemoteAddress,
		_lRemotePort, _ulIdentifier) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED);

		return err;
	}

	return errNoError;
}

