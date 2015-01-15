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


#include "ServerSocket.h"
#include <assert.h>



ServerSocket:: ServerSocket (void): Socket ()

{

	_sServerSocketStatus		= SCK_SERVERSOCKET_BUILDED;

}


ServerSocket:: ~ServerSocket (void)

{

}


ServerSocket:: ServerSocket (const ServerSocket &t)

{
	assert (1 == 0);

}


Error ServerSocket:: init (const char *pLocalAddress, long lLocalPort,
	Boolean_t bReuseAddr, SocketImpl:: SocketType_t stSocketType,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	long lClientsQueueLength)

{

	Error					errServerSocketInit;
	Error					errSocketImplListen;
	Error					errSocketImplCreate;
	Error					errSocketImplBind;


	if (_sServerSocketStatus != SCK_SERVERSOCKET_BUILDED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sServerSocketStatus);

		return err;
	}

	if (lLocalPort < 1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (Socket:: init () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_INIT_FAILED);

		return err;
	}

	if ((errSocketImplCreate = _pSocketImpl -> create (
		stSocketType,
		ulReceivingTimeoutInSeconds,
		ulReceivingAdditionalTimeoutInMicroSeconds,
		0,
		0,
		bReuseAddr)) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_CREATE_FAILED);

		return err;
		*/

		if (Socket:: finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_FINISH_FAILED);
		}

		return errSocketImplCreate;
	}

	if ((errSocketImplBind = _pSocketImpl -> bind (pLocalAddress,
		lLocalPort)) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_BIND_FAILED);

		return err;
		*/

		if (_pSocketImpl -> close () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_CLOSE_FAILED);
		}

		if (Socket:: finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_FINISH_FAILED);
		}

		return errSocketImplBind;
	}

	if (stSocketType == SocketImpl:: STREAM)
	{
		if ((errSocketImplListen = _pSocketImpl -> listen (
			lClientsQueueLength)) != errNoError)
		{
			/*
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_LISTEN_FAILED);

			return err;
			*/

			if (_pSocketImpl -> close () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_CLOSE_FAILED);
			}

			if (Socket:: finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKET_FINISH_FAILED);
			}

			return errSocketImplListen;
		}
	}

	_sServerSocketStatus		= SCK_SERVERSOCKET_INITIALIZED;


	return errNoError;
}


Error ServerSocket:: finish (void)

{

	Error						errClose;


	if (_sServerSocketStatus != SCK_SERVERSOCKET_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sServerSocketStatus);

		return err;
	}

	if ((errClose = _pSocketImpl -> close ()) != errNoError)
	{
		/*
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_CLOSE_FAILED);

		return err;
		*/
		return errClose;
	}

	if (Socket:: finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_FINISH_FAILED);

		return err;
	}

	_sServerSocketStatus		= SCK_SERVERSOCKET_BUILDED;


	return errNoError;
}


Error ServerSocket:: acceptConnection (ClientSocket_p pClientSocket)

{

	SocketImpl_p		pClientSocketImpl;
	Error				errAcceptConnection;


	if (_sServerSocketStatus != SCK_SERVERSOCKET_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_OPERATION_NOTALLOWED, 1, _sServerSocketStatus);

		return err;
	}

	if (pClientSocket == (ClientSocket_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (pClientSocket -> getSocketImpl (&pClientSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		return err;
	}

	errAcceptConnection		= _pSocketImpl -> acceptConnection (
		pClientSocketImpl);

	if (errAcceptConnection == errNoError)
	{
		pClientSocket -> _bSocketImplCreated			= true;
	}


	return errAcceptConnection;
}

