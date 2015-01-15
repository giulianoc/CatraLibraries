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


#include "TelnetClient.h"
#include <assert.h>
#include <time.h>



TelnetClient:: TelnetClient (void): ClientSocket ()

{

}


TelnetClient:: ~TelnetClient (void)

{

}


TelnetClient:: TelnetClient (const TelnetClient &t)

{
	assert (1 == 0);

}


Error TelnetClient:: init (const char *pLocalAddress,
	const char *pRemoteAddress, long lRemotePort,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
	Boolean_t bReuseAddr, const char *pUser,
	const char *pPassword, long lSecondsUsedToNegotiate,
	long lAdditionalMicroSecondsUsedToNegotiate,
	TelnetServerType_t tstTelnetServerType,
	const char *pCommandToBeSentToCloseTheSession)

{

	SocketImpl_p		pLocalSocketImpl;
	Error				errReadLineByTelnet;
	Error				errIsReady;
	char				pBufferToNegotiate [
		SCK_TELNETCLIENT_MAXBUFFERTONEGOTIALE];
	unsigned long		ulBufferToNegotiateLength;
	time_t				tStartTimeToWait;
	Boolean_t			bIsReadyForReading;
	Error_t				errSocket;



	if (pRemoteAddress == (const char *) NULL ||
		lRemotePort < 1 ||
		(lSecondsUsedToNegotiate < 0 && lSecondsUsedToNegotiate != -1) ||
		(lAdditionalMicroSecondsUsedToNegotiate < 0 &&
		lAdditionalMicroSecondsUsedToNegotiate != -1) ||
		(pCommandToBeSentToCloseTheSession != (const char *) NULL &&
			strlen (pCommandToBeSentToCloseTheSession) >=
				SCK_TELNETCLIENT_MAXCLOSECOMMANDLENGTH))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	_lSecondsUsedToNegotiate					= lSecondsUsedToNegotiate;
	_lAdditionalMicroSecondsUsedToNegotiate		=
		lAdditionalMicroSecondsUsedToNegotiate;


	if (pCommandToBeSentToCloseTheSession == (const char *) NULL)
	{
		strcpy (_pCommandToBeSentToCloseTheSession,
			SCK_TELNETCLIENT_CLOSETELNETSESSIONCOMMAND);
	}
	else
	{
		strcpy (_pCommandToBeSentToCloseTheSession,
			pCommandToBeSentToCloseTheSession);
	}

	if ((errSocket = ClientSocket:: init (SocketImpl:: STREAM,
		ulReceivingTimeoutInSeconds,
		ulReceivingAdditionalTimeoutInMicroSeconds,
		ulSendingTimeoutInSeconds,
		ulSendingAdditionalTimeoutInMicroSeconds,
		ulReceivingTimeoutInSeconds,
		bReuseAddr,
		pLocalAddress, pRemoteAddress, lRemotePort)) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED, 
			2, pRemoteAddress, lRemotePort);

		return err;
	}

	if (getSocketImpl (&pLocalSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		return err;
	}

	if (pUser != (char *) NULL)
	{
		tStartTimeToWait			= time (NULL);

		// wait the login
		while (time (NULL) - tStartTimeToWait < _lSecondsUsedToNegotiate +
			(_lAdditionalMicroSecondsUsedToNegotiate / 1000000) + 1)
		{
			if ((errIsReady = pLocalSocketImpl -> isReadyForReading (
				&bIsReadyForReading, _lSecondsUsedToNegotiate,
				_lAdditionalMicroSecondsUsedToNegotiate)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				return errIsReady;
			}

			if (bIsReadyForReading)
			{
				ulBufferToNegotiateLength	= SCK_TELNETCLIENT_MAXBUFFERTONEGOTIALE - 1;

				if ((errReadLineByTelnet = pLocalSocketImpl -> readByTelnet (
					pBufferToNegotiate, &ulBufferToNegotiateLength, true)) !=
					errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					//	SCK_SOCKETIMPL_READLINEBYTELNET_FAILED);

					return errReadLineByTelnet;
				}

				if (strstr (pBufferToNegotiate, "ogin") != (char *) NULL)
					break;
			}
		}

		if (strstr (pBufferToNegotiate, "ogin") == (char *) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_TIMEOUTEXPIRED);

			return err;
		}

		if (pLocalSocketImpl -> writeString (pUser, true, 0, 1000) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);

			return err;
		}

		if (tstTelnetServerType == SCK_WINDOWS_TELNETSERVER)
		{
			if (pLocalSocketImpl -> writeString ("\r\n", true, 0, 1000) !=
				errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_WRITESTRING_FAILED);

				return err;
			}
		}
		else
		{
			if (pLocalSocketImpl -> writeString ("\n", true, 0, 1000) !=
				errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_WRITESTRING_FAILED);

				return err;
			}
		}
	}

	if (pPassword != (char *) NULL)
	{
		tStartTimeToWait			= time (NULL);

		// wait the login
		while (time (NULL) - tStartTimeToWait < _lSecondsUsedToNegotiate +
			(_lAdditionalMicroSecondsUsedToNegotiate / 1000000) + 1)
		{
			if ((errIsReady = pLocalSocketImpl -> isReadyForReading (
				&bIsReadyForReading, _lSecondsUsedToNegotiate,
				_lAdditionalMicroSecondsUsedToNegotiate)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				return errIsReady;
			}

			if (bIsReadyForReading)
			{
				ulBufferToNegotiateLength	= SCK_TELNETCLIENT_MAXBUFFERTONEGOTIALE - 1;

				if ((errReadLineByTelnet = pLocalSocketImpl -> readByTelnet (
					pBufferToNegotiate, &ulBufferToNegotiateLength, true)) !=
					errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					//	SCK_SOCKETIMPL_READLINEBYTELNET_FAILED);

					return errReadLineByTelnet;
				}

				if (strstr (pBufferToNegotiate, "assword") != (char *) NULL)
					break;
			}
		}

		if (strstr (pBufferToNegotiate, "assword") == (char *) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_TIMEOUTEXPIRED);

			return err;
		}

		if (pLocalSocketImpl -> writeString (pPassword, true, 0, 1000) !=
			errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_WRITESTRING_FAILED);

			return err;
		}

		if (tstTelnetServerType == SCK_WINDOWS_TELNETSERVER)
		{
			if (pLocalSocketImpl -> writeString ("\r\n", true, 0, 1000) !=
				errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_WRITESTRING_FAILED);

				return err;
			}
		}
		else
		{
			if (pLocalSocketImpl -> writeString ("\n", true, 0, 1000) !=
				errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_WRITESTRING_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error TelnetClient:: finish (void)

{

	SocketImpl_p			pSocketImpl;
	Error					errVacuum;


	if (getSocketImpl (&pSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		return err;
	}

	if (pSocketImpl -> writeString (
		_pCommandToBeSentToCloseTheSession,
		true, 0, 1000) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);

		return err;
	}

	// we don't wait for 'logout' word (unix world) because for example using the
	// windows telnet server we will have 'Connection to host lost'.
	// It looks that there isn't a standard
	/*
	if ((errVacuum = pSocketImpl -> vacuumByTelnet (
		_lSecondsUsedToNegotiate, _lAdditionalMicroSecondsUsedToNegotiate)) !=
		errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_VACUUMBYTELNET_FAILED);

		return errVacuum;
	}
	*/

	if (ClientSocket:: finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);

		return err;
	}


	return errNoError;
}

