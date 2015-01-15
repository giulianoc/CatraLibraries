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


//	If it is used the select function and you want to increase the number
//	of file descriptors, for Windows, is sufficient
//	define again FD_SETSIZE inside this file.
//	For linux system it is necessary to use the poll function
#define FD_SETSIZE			1024 * 4

#ifdef WIN32
#elif __QTCOMPILER__
	#define HAVE_POLL
#else
	#include <CatraLibrariesConfig.h>
#endif
#if defined(WIN32)
	#include <io.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2spi.h>
	#include <ws2tcpip.h>
	#include <Iphlpapi.h>
#else
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <net/if.h>
	#include <netdb.h>
#endif
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#ifdef TELNET_DEBUG
	#include <iostream>
#endif

#ifdef HAVE_POLL
	#include <sys/poll.h>
#endif

#include "SocketImpl.h"
#include "DateTime.h"

#ifdef WIN32
#elif __QTCOMPILER__
#else
	extern int errno;
#endif


SocketImpl:: SocketImpl (void)

{

	#ifdef WIN32
		WSADATA			wsaData;

		if (WSAStartup (MAKEWORD (2, 2), &wsaData))
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_WSASTARTUP_FAILED);
		}
	#endif

	_iFd						= -1;
	_bIsConnectionRealized		= false;
	_lIAC_State					= SCK_IAC_RESET;
	_lRemotePort				= -1;
	_lLocalPort					= -1;
	strcpy (_pRemoteAddress, "");
	strcpy (_pLocalAddress, "");

	_ulReceivingTimeoutInSeconds							= 0;
	_ulReceivingAdditionalTimeoutInMicroSeconds				= 0;
	_ulSendingTimeoutInSeconds								= 0;
	_ulSendingAdditionalTimeoutInMicroSeconds				= 0;

}


SocketImpl:: ~SocketImpl (void)

{

	if (_iFd != -1)
	{
		if (close () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_CLOSE_FAILED);
		}
	}

	#ifdef WIN32
		if (WSACleanup ())
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_WSACLEANUP_FAILED);
		}
	#endif
}


SocketImpl:: SocketImpl (const SocketImpl &t)

{

	assert (1 == 0);

}


Error SocketImpl:: create (SocketType_t stSocketType,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
	Boolean_t bReuseAddr)

{

	#ifdef WIN32
		BOOL				iReuseAddrFlag;
	#else
		int					iReuseAddrFlag;
	#endif


	if (stSocketType == STREAM)
	{
		if ((_iFd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CREATE_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}
	}
	else if (stSocketType == DGRAM)
	{
		if ((_iFd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CREATE_FAILED, 1, iErrno);

			return err;
		}
	}
	else
		;

	_stSocketType					= stSocketType;

	if (bReuseAddr)
	{
		#ifdef WIN32
			iReuseAddrFlag				= TRUE;

			if (setsockopt (_iFd, SOL_SOCKET, SO_REUSEADDR,
				(const char *) &iReuseAddrFlag, sizeof (BOOL)) != 0)
		#else
			iReuseAddrFlag				= 1;

			if (setsockopt (_iFd, SOL_SOCKET, SO_REUSEADDR,
				&iReuseAddrFlag, sizeof (int)) == -1)
		#endif
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SETSOCKOPT_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			#ifdef WIN32
				if (::closesocket (_iFd) != 0)
			#else
				if (::close (_iFd) == -1)
			#endif
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			_iFd				= -1;


			return err;
		}
	}
	else
	{
		#ifdef WIN32
			iReuseAddrFlag				= FALSE;

			if (setsockopt (_iFd, SOL_SOCKET, SO_REUSEADDR,
				(const char *) &iReuseAddrFlag, sizeof (BOOL)) != 0)
		#else
			iReuseAddrFlag				= 0;

			if (setsockopt (_iFd, SOL_SOCKET, SO_REUSEADDR,
				&iReuseAddrFlag, sizeof (int)) == -1)
		#endif
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SETSOCKOPT_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			#ifdef WIN32
				if (::closesocket (_iFd) != 0)
			#else
				if (::close (_iFd) == -1)
			#endif
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			_iFd				= -1;

			return err;
		}
	}

	if (ulReceivingTimeoutInSeconds != 0 ||
		ulReceivingAdditionalTimeoutInMicroSeconds != 0)
	{
		if (setReceivingTimeout (ulReceivingTimeoutInSeconds,
			ulReceivingAdditionalTimeoutInMicroSeconds) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETRECEIVINGTIMEOUT_FAILED);

			#ifdef WIN32
				if (::closesocket (_iFd) != 0)
			#else
				if (::close (_iFd) == -1)
			#endif
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			_iFd				= -1;


			return err;
		}
	}

	if (ulSendingTimeoutInSeconds != 0 ||
		ulSendingAdditionalTimeoutInMicroSeconds != 0)
	{
		if (setSendingTimeout (ulSendingTimeoutInSeconds,
			ulSendingAdditionalTimeoutInMicroSeconds) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETSENDINGTIMEOUT_FAILED);

			#ifdef WIN32
				if (::closesocket (_iFd) != 0)
			#else
				if (::close (_iFd) == -1)
			#endif
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			_iFd				= -1;

			return err;
		}
	}


	return errNoError;
}


Error SocketImpl:: connect (const char *pRemoteAddress, long lRemotePort,
	unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS)

{

	struct sockaddr_in			sckServerAddr;
	int							iConnectReturn;


	if (pRemoteAddress == (const char *) NULL ||
		lRemotePort < 1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (strlen (pRemoteAddress) > SCK_MAXIPADDRESSLENGTH - 1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_REMOTEADDRESS_TOOLONG);

		return err;
	}
	else
		strcpy (_pRemoteAddress, pRemoteAddress);

	_lRemotePort				= lRemotePort;

	memset ((char *) &sckServerAddr, 0,
		(unsigned short) sizeof (sckServerAddr));

	sckServerAddr. sin_family				= AF_INET;
	sckServerAddr. sin_addr. s_addr			= inet_addr (_pRemoteAddress);
	sckServerAddr. sin_port					= htons (
		(unsigned short) _lRemotePort);


	do
	{
		iConnectReturn = ::connect (_iFd,
			(const struct sockaddr *) &sckServerAddr,
			sizeof (sckServerAddr));
	}
	#ifdef WIN32
		while ((iConnectReturn < 0) && (WSAGetLastError () == EINTR));
	#else
		while ((iConnectReturn < 0) && (errno == EINTR));
	#endif

	if (iConnectReturn == -1)
	{
		#ifdef WIN32
		 	if (errno == WSAEINPROGRESS)
		#else
		 	if (errno == EINPROGRESS)
		#endif
		{
			/*
			 * description of the EINPROGRESS errno from 'man connec'
			 *
			EINPROGRESS
			The  socket  is  non-blocking  and the connection cannot be com-
			pleted immediately.  It is possible to select(2) or poll(2)  for
			completion by selecting the socket for writing.  After select(2)
			indicates writability, use getsockopt(2) to  read  the  SO_ERROR
			option  at  level SOL_SOCKET to determine whether connect() com-
			pleted  successfully  (SO_ERROR  is  zero)   or   unsuccessfully
			(SO_ERROR  is one of the usual error codes listed here, explain-
			ing the reason for the failure).

			The description asks to call select/poll and in general the next
			API called after this connect method is something like writeString
			calling isReadyForWriting (poll/select).
			So let's just ignore this error.

			2011/09/27: I just verified, this error is returned also
			when the machine is switched off. In this case it is correct
			to return an error.

			2013/01/14: My considerations:
				- 1. EINPROGRESS could be returned in case of stress (a lot of
					concurrently requests. In this case, this is not really
					an error and the next API could be called because
					it will work.
				- 2. EINPROGRESS is returned also when the IP Address
					is wrong or the machine is down. In this case this
					is really an error and it could cause a crash
					if the next API are called (it happened). So it is important
					to treat it as an error.
				To manage both the above scenarios (specially the second one)
				and allow the procedure calling this method (connect) to avoid
				to manage the EINPROGRESS error, it is called
				the isReadyForWriting API, as suggested above by 'man connect'.
				In this way it seems that, providing an enough
				timeout parameter to isReadyForWriting,
				the crash does not happen any more. Instead, we will have
				an error raised by the next API (i.e.: SocketImpl->writeString)

				If the calling procedure needs to know if the machine is down
				(i.e. LoadBalancer), this error is important because will tell
				the machine is down. So in this case the timeout parameters of
				this call has to be set to 0.
			*/
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS != 0)
			{
				Boolean_t		bIsReadyForWriting;
				Error_t			errIsReady;


				if ((errIsReady = isReadyForWriting (&bIsReadyForWriting,
					ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS, 0)) !=
					errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					// 	SCK_SOCKETIMPL_ISREADYFORWRITING_FAILED);

					return errIsReady;
				}

				if (!bIsReadyForWriting)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CONNECT_EINPROGRESSFAILED, 3, iErrno,
						_pRemoteAddress, _lRemotePort);
					err. setUserData (&iErrno, sizeof (int));

					return err;
				}
			}
			else
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CONNECT_EINPROGRESSFAILED, 3, iErrno,
					_pRemoteAddress, _lRemotePort);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}
		}
		else
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CONNECT_FAILED, 3, iErrno,
				_pRemoteAddress, _lRemotePort);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}
	}

	_bIsConnectionRealized			= true;


	return errNoError;
}


Error SocketImpl:: connect (struct sockaddr_in *psckServerAddr,
	unsigned long ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS)

{

	int							iConnectReturn;


	if (psckServerAddr == (sockaddr_in *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	{
		char			*pRemoteAddress;


		pRemoteAddress = inet_ntoa (psckServerAddr -> sin_addr);

		if (pRemoteAddress == (char *) NULL)
		{
			strcpy (_pRemoteAddress, "");
		}
		else
		{
			if (strlen (pRemoteAddress) > SCK_MAXIPADDRESSLENGTH)
				strcpy (_pRemoteAddress, "");
			else
				strcpy (_pRemoteAddress, pRemoteAddress);
		}
	}
	_lRemotePort			= ntohs (psckServerAddr -> sin_port);

	do
	{
		iConnectReturn = ::connect (_iFd,
			(const struct sockaddr *) psckServerAddr,
			sizeof (*psckServerAddr));
	}
	#ifdef WIN32
		while ((iConnectReturn < 0) && (WSAGetLastError () == EINTR));
	#else
		while ((iConnectReturn < 0) && (errno == EINTR));
	#endif

	if (iConnectReturn == -1)
	{
		#ifdef WIN32
		 	if (errno == WSAEINPROGRESS)
		#else
		 	if (errno == EINPROGRESS)
		#endif
		{
			/*
			 * 	... see comments in the other SocketImpl::connect method
			*/
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS != 0)
			{
				Boolean_t		bIsReadyForWriting;
				Error_t			errIsReady;


				if ((errIsReady = isReadyForWriting (&bIsReadyForWriting,
					ulConnectTimeoutInSecondsOnlyInCaseOfEINPROGRESS, 0)) !=
					errNoError)
				{
					// Error err = SocketErrors (__FILE__, __LINE__,
					// 	SCK_SOCKETIMPL_ISREADYFORWRITING_FAILED);

					return errIsReady;
				}

				if (!bIsReadyForWriting)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CONNECT_EINPROGRESSFAILED, 3, iErrno,
						_pRemoteAddress, _lRemotePort);
					err. setUserData (&iErrno, sizeof (int));

					return err;
				}
			}
			else
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CONNECT_EINPROGRESSFAILED, 3, iErrno,
					_pRemoteAddress, _lRemotePort);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}
		}
		else
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CONNECT_FAILED, 3, iErrno,
				_pRemoteAddress, _lRemotePort);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}
	}

	_bIsConnectionRealized			= true;


	return errNoError;
}


Error SocketImpl:: bind (const char *pLocalAddress, long lLocalPort)

{

	struct sockaddr_in			sckServerAddr;


	if (lLocalPort < 0 ||
		(pLocalAddress != (const char *) NULL &&
		strlen (pLocalAddress) > SCK_MAXIPADDRESSLENGTH - 1))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (pLocalAddress == (const char *) NULL)
		strcpy (_pLocalAddress, "");
	else
		strcpy (_pLocalAddress, pLocalAddress);
	_lLocalPort					= lLocalPort;

	memset ((char *) &sckServerAddr, 0,
		(unsigned short) sizeof (sckServerAddr));

	sckServerAddr. sin_family				= AF_INET;
	if (pLocalAddress == (const char *) NULL ||
		!strcmp (pLocalAddress, ""))
		sckServerAddr. sin_addr. s_addr			= htonl (INADDR_ANY);
	else
		sckServerAddr. sin_addr. s_addr			= inet_addr (pLocalAddress);
	sckServerAddr. sin_port					= htons (
		(unsigned short) _lLocalPort);

	//	su HP11 che possiede i threads nativi, l'errore dovrebbe
	//	essere il ritorno della funzione ::bind anzicche'
	//	errno. Cio' perche' altrimenti, usando la variabile globale
	//	errno, sarebbe un problema verificare l'errore su due chiamate
	//	concorrenti di ::bind.
	//	Il comando man bind su HP11 pero' dice che l'errore deve essere
	//	controllato con errno. Sara' una dimenticanza di HP11 non aver
	//	fatto il porting di ::bind dalla vers. 10 alla 11???
	if (::bind (_iFd, (struct sockaddr *) &sckServerAddr,
		sizeof (sckServerAddr)) == -1)
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_BIND_FAILED, 3, iErrno,
			pLocalAddress == (const char *) NULL ? "<NULL>" : pLocalAddress,
			_lLocalPort);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


Error SocketImpl:: listen (long lClientsQueueLength)

{

	if (lClientsQueueLength < 1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (::listen (_iFd, (int) lClientsQueueLength) == -1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_LISTEN_FAILED);

		return err;
	}


	return errNoError;
}


Error SocketImpl:: acceptConnection (SocketImpl_p pSocketImpl)

{

	struct sockaddr_in			sckClientAddr;


	if (pSocketImpl == (SocketImpl_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	#ifdef AF_CCITT		// for example HPUX11
		// int					sockaddrSize;
		socklen_t			sockaddrSize;


		sockaddrSize	= sizeof (sckClientAddr);

		if ((pSocketImpl -> _iFd = ::accept (_iFd,
			(struct sockaddr *) &sckClientAddr, &sockaddrSize)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACCEPT_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}
	#else	// _XOPEN_SOURCE_EXTENDED only (UNIX 98)
		socklen_t					sockaddrSize;


		sockaddrSize	= sizeof (sckClientAddr);

		if ((pSocketImpl -> _iFd = ::accept (_iFd,
			(struct sockaddr *) &sckClientAddr, &sockaddrSize)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACCEPT_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}
	#endif

	{
		char			*pRemoteAddress;


		pRemoteAddress = inet_ntoa (sckClientAddr. sin_addr);

		if (pRemoteAddress == (char *) NULL)
		{
			strcpy (pSocketImpl -> _pRemoteAddress, "");
		}
		else
		{
			if (strlen (pRemoteAddress) > SCK_MAXIPADDRESSLENGTH)
				strcpy (pSocketImpl -> _pRemoteAddress, "");
			else
				strcpy (pSocketImpl -> _pRemoteAddress, pRemoteAddress);
		}
	}
	pSocketImpl -> _lRemotePort				= ntohs (sckClientAddr. sin_port);

	pSocketImpl -> _stSocketType			= STREAM;
 
	pSocketImpl -> _bIsConnectionRealized	= true;


	return errNoError;
}


Error SocketImpl:: close (void)

{

	#ifdef WIN32
		if (::closesocket (_iFd) != 0)
	#else
		if (::close (_iFd) == -1)
	#endif
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLOSE_FAILED);

		return err;
	}

	_iFd				= -1;


	return errNoError;
}


Error SocketImpl:: setBlocking (Boolean_t bBlocking)

{

	#ifdef WIN32
		unsigned long			ulInputBuffer;


		if (bBlocking)
			ulInputBuffer			= 0;
		else
			ulInputBuffer			= 1;

		if (ioctlsocket (_iFd, FIONBIO, &ulInputBuffer))
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_IOCTLSOCKET_FAILED);

			return err;
		}
	#else
		int				iFlags;


		if ((iFlags = ::fcntl (_iFd, F_GETFL)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_FCNTL_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}

		if (!bBlocking)
		{
			if (::fcntl (_iFd, F_SETFL, iFlags | O_NONBLOCK) == -1)
			{
				int				iErrno;


				#ifdef WIN32
					iErrno			= WSAGetLastError ();
				#else
					iErrno			= errno;
				#endif

				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_FCNTL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}
		}
		else
		{
			if (::fcntl (_iFd, F_SETFL, iFlags & (~O_NONBLOCK)) == -1)
			{
				int				iErrno;


				#ifdef WIN32
					iErrno			= WSAGetLastError ();
				#else
					iErrno			= errno;
				#endif

				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_FCNTL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}
		}
	#endif


	return errNoError;
}


#ifdef WIN32
#else
Error SocketImpl:: getBlocking (Boolean_p pbBlocking)

{

	int				iFcntlReturn;


	if (pbBlocking == (Boolean_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if ((iFcntlReturn = ::fcntl (_iFd, F_GETFL)) == -1)
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_FCNTL_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}

	if (iFcntlReturn & O_NONBLOCK)
		*pbBlocking				= false;
	else
		*pbBlocking				= true;


	return errNoError;
}
#endif


Error SocketImpl:: setReceivingTimeout (
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds)

{

	#ifdef WIN32
		int						iTimeout;
	#else
		struct timeval			tvTimeval;
	#endif


	#ifdef WIN32
		iTimeout			= (ulReceivingTimeoutInSeconds * 1000) +
			(ulReceivingAdditionalTimeoutInMicroSeconds / 1000);
	#else
		tvTimeval. tv_sec	= ulReceivingTimeoutInSeconds;
		tvTimeval. tv_usec	= ulReceivingAdditionalTimeoutInMicroSeconds;
	#endif

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_RCVTIMEO,
			(const char *) (&iTimeout), sizeof (int)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_RCVTIMEO,
			&tvTimeval, sizeof (struct timeval)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}

	_ulReceivingTimeoutInSeconds					=
		ulReceivingTimeoutInSeconds;
	_ulReceivingAdditionalTimeoutInMicroSeconds		=
		ulReceivingAdditionalTimeoutInMicroSeconds;


	return errNoError;
}


Error SocketImpl:: setSendingTimeout (
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds)

{
	#ifdef WIN32
		int						iTimeout;
	#else
		struct timeval			tvTimeval;
	#endif


	#ifdef WIN32
		iTimeout				= (ulSendingTimeoutInSeconds * 1000) +
			(ulSendingAdditionalTimeoutInMicroSeconds / 1000);
	#else
		tvTimeval. tv_sec		= ulSendingTimeoutInSeconds;
		tvTimeval. tv_usec		= ulSendingAdditionalTimeoutInMicroSeconds;
	#endif

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_SNDTIMEO,
			(const char *) (&iTimeout), sizeof (int)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_SNDTIMEO,
			&tvTimeval, sizeof (struct timeval)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}

	_ulSendingTimeoutInSeconds					=
		ulSendingTimeoutInSeconds;
	_ulSendingAdditionalTimeoutInMicroSeconds	=
		ulSendingAdditionalTimeoutInMicroSeconds;


	return errNoError;
}


Error SocketImpl:: setMaxSendBuffer (unsigned long ulMaxSendBuffer)

{

	int					iMaxSendBuffer;


	iMaxSendBuffer				= ulMaxSendBuffer;

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_SNDBUF,
			(char *) (&iMaxSendBuffer), sizeof (int)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_SNDBUF,
			(char *) &iMaxSendBuffer, sizeof (int)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


Error SocketImpl:: setMaxReceiveBuffer (unsigned long ulMaxReceiveBuffer)

{

	int					iMaxReceiveBuffer;


	iMaxReceiveBuffer				= ulMaxReceiveBuffer;

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_RCVBUF,
			(char *) (&iMaxReceiveBuffer), sizeof (int)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_RCVBUF,
			(char *) &iMaxReceiveBuffer, sizeof (int)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


Error SocketImpl:: setLinger (
	Boolean_t bLingerOnOff,
	unsigned long ulManySecondsToLingerFor)

{

	struct linger			lLinger;


	lLinger. l_onoff			= bLingerOnOff ? 1 : 0;
	lLinger. l_linger			= (int) ulManySecondsToLingerFor;

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_LINGER,
			(char *) (&lLinger), sizeof (lLinger)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_LINGER,
			(char *) &lLinger, sizeof (lLinger)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


Error SocketImpl:: setKeepAlive (Boolean_t bKeepAlive)

{

	#ifdef WIN32
		BOOL				iKeepAlive;
	#else
		int					iKeepAlive;
	#endif


	#ifdef WIN32
		if (bKeepAlive)
			iKeepAlive			= TRUE;
		else
			iKeepAlive			= FALSE;
	#else
		if (bKeepAlive)
			iKeepAlive			= 1;
		else
			iKeepAlive			= 0;
	#endif

	#ifdef WIN32
		if (setsockopt (_iFd, SOL_SOCKET, SO_KEEPALIVE,
			(char *) &iKeepAlive, sizeof (BOOL)) != 0)
	#else
		if (setsockopt (_iFd, SOL_SOCKET, SO_KEEPALIVE,
			(char *) &iKeepAlive, sizeof (int)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


Error SocketImpl:: setNoDelay (Boolean_t bNoDelay)

{

	#ifdef WIN32
		BOOL				iNoDelay;
	#else
		int					iNoDelay;
	#endif


	#ifdef WIN32
		if (bNoDelay)
			iNoDelay			= TRUE;
		else
			iNoDelay			= FALSE;
	#else
		if (bNoDelay)
			iNoDelay			= 1;
		else
			iNoDelay			= 0;
	#endif

	#ifdef WIN32
		if (setsockopt (_iFd, IPPROTO_TCP, TCP_NODELAY, (char *) &iNoDelay,
			sizeof (BOOL)) != 0)
	#else
		if (setsockopt (_iFd, IPPROTO_TCP, TCP_NODELAY, (char *) &iNoDelay,
			sizeof (int)) == -1)
	#endif
	{
		int				iErrno;


		#ifdef WIN32
			iErrno			= WSAGetLastError ();
		#else
			iErrno			= errno;
		#endif

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SETSOCKOPT_FAILED, 1, iErrno);
		err. setUserData (&iErrno, sizeof (int));

		return err;
	}


	return errNoError;
}


#ifdef HAVE_POLL
	Error SocketImpl:: isReadyForReading (Boolean_p pbIsReadyForReading,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		int						iMilliSeconds;
		int						iPollReturn;
		struct pollfd			pfDescriptor;
		#ifdef WIN32
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		#endif
		Boolean_t				bSystemCallFinished;


		if (pbIsReadyForReading == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		iMilliSeconds			= (ulSecondsToWait * 1000) +
			(ulAdditionalMicrosecondsToWait / 1000);

		pfDescriptor. fd			= _iFd;

		pfDescriptor. events		= 0;
		pfDescriptor. events		|= POLLIN;

		pfDescriptor. revents		= 0;

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iPollReturn			= ::poll (&pfDescriptor,
				1, iMilliSeconds);

			if (iPollReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= iMilliSeconds)
					bSystemCallFinished			= true;
				else
					iMilliSeconds				= iMilliSeconds -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs);
			}
			else
				bSystemCallFinished				= true;
		}

		if (iPollReturn == 0)
		{
			*pbIsReadyForReading				= false;
		}
		else if (iPollReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_POLL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsReadyForReading				= false;
		}
		else
		{
			*pbIsReadyForReading				= true;
		}


		return errNoError;
	}


	Error SocketImpl:: isReadyForWriting (Boolean_p pbIsReadyForWriting,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		int						iMilliSeconds;
		int						iPollReturn;
		struct pollfd			pfDescriptor;
		#ifdef WIN32
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		#endif
		Boolean_t				bSystemCallFinished;


		if (pbIsReadyForWriting == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		iMilliSeconds			= (ulSecondsToWait * 1000) +
			(ulAdditionalMicrosecondsToWait / 1000);

		pfDescriptor. fd			= _iFd;

		pfDescriptor. events		= 0;
		pfDescriptor. events		|= POLLOUT;

		pfDescriptor. revents		= 0;

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iPollReturn			= ::poll (&pfDescriptor,
				1, iMilliSeconds);

			if (iPollReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= iMilliSeconds)
					bSystemCallFinished			= true;
				else
					iMilliSeconds				= iMilliSeconds -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs);
			}
			else
				bSystemCallFinished				= true;
		}

		if (iPollReturn == 0)
		{
			*pbIsReadyForWriting				= false;
		}
		else if (iPollReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_POLL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsReadyForWriting				= false;
		}
		else
		{
			// FD_ISSET (_iFd, &fdWriteFd) will be true
			*pbIsReadyForWriting				= true;
		}


		return errNoError;
	}


	Error SocketImpl:: isThereException (Boolean_p pbIsThereException,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		int						iMilliSeconds;
		int						iPollReturn;
		struct pollfd			pfDescriptor;
		#ifdef WIN32
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		#endif
		Boolean_t				bSystemCallFinished;


		if (pbIsThereException == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		iMilliSeconds			= (ulSecondsToWait * 1000) +
			(ulAdditionalMicrosecondsToWait / 1000);

		pfDescriptor. fd			= _iFd;

		pfDescriptor. events		= 0;
		pfDescriptor. events		|= POLLPRI;

		pfDescriptor. revents		= 0;

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iPollReturn			= ::poll (&pfDescriptor,
				1, iMilliSeconds);

			if (iPollReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= iMilliSeconds)
					bSystemCallFinished			= true;
				else
					iMilliSeconds				= iMilliSeconds -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs);
			}
			else
				bSystemCallFinished				= true;
		}

		if (iPollReturn == 0)
		{
			*pbIsThereException				= false;
		}
		else if (iPollReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_POLL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsThereException				= false;
		}
		else
		{
			*pbIsThereException				= true;
		}


		return errNoError;
	}
#else
	Error SocketImpl:: isReadyForReading (Boolean_p pbIsReadyForReading,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		struct timeval			tvTimeval;
		fd_set					fdReadFd;
		int						iSelectReturn;
		/*
		#ifdef WIN32
			__int64				ullMilliSecondsToWait;
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
		*/
			unsigned long long	ullMilliSecondsToWait;
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		// #endif
		Boolean_t				bSystemCallFinished;


		if (pbIsReadyForReading == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		#ifdef WIN32
		#else
			if (_iFd >= FD_SETSIZE)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_FD_SETSIZETOBEINCREMENTED,
					2, (long) _iFd, (long) FD_SETSIZE);

				return err;
			}
		#endif

		tvTimeval. tv_sec  = ulSecondsToWait;
		tvTimeval. tv_usec = ulAdditionalMicrosecondsToWait;
		ullMilliSecondsToWait		=
			(ulSecondsToWait * 1000) + (ulAdditionalMicrosecondsToWait / 1000);

		FD_ZERO (&fdReadFd);
		FD_SET (_iFd, &fdReadFd);

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iSelectReturn			= ::select (_iFd + 1, &fdReadFd,
				(fd_set *) NULL, (fd_set *) NULL, &tvTimeval);

			if (iSelectReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= ullMilliSecondsToWait)
					bSystemCallFinished			= true;
				else
				{
					tvTimeval. tv_sec  = (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) / 1000;
					tvTimeval. tv_usec = ((ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) -
						tvTimeval. tv_sec * 1000) * 1000;
					ullMilliSecondsToWait		= (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs));
				}
			}
			else
				bSystemCallFinished				= true;
		}

		if (iSelectReturn == 0)
		{
			*pbIsReadyForReading				= false;
		}
		else if (iSelectReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SELECT_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsReadyForReading				= false;
		}
		else
		{
			// FD_ISSET (_iFd, &fdReadFd) will be true
			*pbIsReadyForReading				= true;
		}


		return errNoError;
	}


	Error SocketImpl:: isReadyForWriting (Boolean_p pbIsReadyForWriting,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		struct timeval			tvTimeval;
		fd_set					fdWriteFd;
		int						iSelectReturn;
		/*
		#ifdef WIN32
			__int64				ullMilliSecondsToWait;
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
		*/
			unsigned long long	ullMilliSecondsToWait;
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		// #endif
		Boolean_t				bSystemCallFinished;


		if (pbIsReadyForWriting == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		#ifdef WIN32
		#else
			if (_iFd >= FD_SETSIZE)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_FD_SETSIZETOBEINCREMENTED,
					2, (long) _iFd, (long) FD_SETSIZE);

				return err;
			}
		#endif

		tvTimeval. tv_sec  = ulSecondsToWait;
		tvTimeval. tv_usec = ulAdditionalMicrosecondsToWait;
		ullMilliSecondsToWait		=
			(ulSecondsToWait * 1000) + (ulAdditionalMicrosecondsToWait / 1000);

		FD_ZERO (&fdWriteFd);
		FD_SET (_iFd, &fdWriteFd);

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iSelectReturn			= ::select (_iFd + 1, (fd_set *) NULL,
				&fdWriteFd, (fd_set *) NULL, &tvTimeval);

			if (iSelectReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= ullMilliSecondsToWait)
					bSystemCallFinished			= true;
				else
				{
					tvTimeval. tv_sec  = (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) / 1000;
					tvTimeval. tv_usec = ((ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) -
						tvTimeval. tv_sec * 1000) * 1000;
					ullMilliSecondsToWait		= (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs));
				}
			}
			else
				bSystemCallFinished				= true;
		}

		if (iSelectReturn == 0)
		{
			*pbIsReadyForWriting				= false;
		}
		else if (iSelectReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SELECT_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsReadyForWriting				= false;
		}
		else
		{
			// FD_ISSET (_iFd, &fdWriteFd) will be true
			*pbIsReadyForWriting				= true;
		}


		return errNoError;
	}


	Error SocketImpl:: isThereException (Boolean_p pbIsThereException,
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMicrosecondsToWait)

	{

		struct timeval			tvTimeval;
		fd_set					fdExceptFd;
		int						iSelectReturn;
		/*
		#ifdef WIN32
			__int64				ullMilliSecondsToWait;
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
		*/
			unsigned long long	ullMilliSecondsToWait;
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		// #endif
		Boolean_t				bSystemCallFinished;


		if (pbIsThereException == (Boolean_p) NULL ||
			ulAdditionalMicrosecondsToWait >= 1000000)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			return err;
		}

		#ifdef WIN32
		#else
			if (_iFd >= FD_SETSIZE)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_FD_SETSIZETOBEINCREMENTED,
					2, (long) _iFd, (long) FD_SETSIZE);

				return err;
			}
		#endif

		tvTimeval. tv_sec  = ulSecondsToWait;
		tvTimeval. tv_usec = ulAdditionalMicrosecondsToWait;
		ullMilliSecondsToWait		=
			(ulSecondsToWait * 1000) + (ulAdditionalMicrosecondsToWait / 1000);

		FD_ZERO (&fdExceptFd);
		FD_SET (_iFd, &fdExceptFd);

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iSelectReturn			= ::select (_iFd + 1, (fd_set *) NULL,
				(fd_set *) NULL, &fdExceptFd, &tvTimeval);

			if (iSelectReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= ullMilliSecondsToWait)
					bSystemCallFinished			= true;
				else
				{
					tvTimeval. tv_sec  = (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) / 1000;
					tvTimeval. tv_usec = ((ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) -
						tvTimeval. tv_sec * 1000) * 1000;
					ullMilliSecondsToWait		= (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs));
				}
			}
			else
				bSystemCallFinished				= true;
		}

		if (iSelectReturn == 0)
		{
			*pbIsThereException				= false;
		}
		else if (iSelectReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SELECT_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				return err;
			}

			*pbIsThereException				= false;
		}
		else
		{
			// FD_ISSET (_iFd, &fdExceptFd) will be true
			*pbIsThereException				= true;
		}


		return errNoError;
	}
#endif


Error SocketImpl:: waitForBuffer (const char *pBufferToWait,
	long lTimeoutInSeconds, long lAdditionalTimeoutInMicroSeconds)

{

	long					lBufferToWaitLength;
	char					*pBufferRead;
	long					lBufferReadIndex;
	#ifdef WIN32
		DWORD					tvStartTime;
		DWORD					tvCurrentTime;
		DWORD					tvElapsedTime;
		DWORD					tvRemainingTime;
	#else
		timeval					tvStartTime;
		timeval					tvCurrentTime;
		timeval					tvElapsedTime;
		timeval					tvRemainingTime;
		struct timezone			tzTimeZone;
	#endif
	Boolean_t				bIsReadyForReading;
	Boolean_t				bIsBufferRead;
	Error					errIsReady;
	unsigned long			ulLocalTimeoutInSeconds;
	unsigned long			ulLocalAdditionalTimeoutInMicroSeconds;


	if (pBufferToWait == (const char *) NULL ||
		(lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lAdditionalTimeoutInMicroSeconds < 0 &&
		lAdditionalTimeoutInMicroSeconds != -1) ||
		lAdditionalTimeoutInMicroSeconds >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	lBufferToWaitLength				= strlen (pBufferToWait);

	if ((pBufferRead = new char [lBufferToWaitLength + 1]) ==
		(char *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		return err;
	}

	memset (pBufferRead, '\0', lBufferToWaitLength + 1);

	if (lTimeoutInSeconds == -1)
	{
		ulLocalTimeoutInSeconds					= SCK_MAXTIMEOUTINSECONDS;
		ulLocalAdditionalTimeoutInMicroSeconds	= 0;
	}
	else
	{
		ulLocalTimeoutInSeconds					= lTimeoutInSeconds;
		ulLocalAdditionalTimeoutInMicroSeconds	=
			lAdditionalTimeoutInMicroSeconds;
	}

	lBufferReadIndex		= 0;

	bIsBufferRead			= false;

	#ifdef WIN32
		tvStartTime			= GetTickCount ();
		tvCurrentTime		= GetTickCount ();
	#else
		if (gettimeofday (&tvStartTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			delete [] pBufferRead;

			return err;
		}

		tvCurrentTime			= tvStartTime;

		/*
		if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			delete [] pBufferRead;

			return err;
		}
		*/
	#endif

	while (!bIsBufferRead)
	{
		#ifdef WIN32
			unsigned long				ulSeconds;
			unsigned long				ulAdditionalMicroSeconds;

			// current - start
			if (tvStartTime <= tvCurrentTime)
			{
				tvElapsedTime			= tvCurrentTime - tvStartTime;
			}
			else
			{
				// ???
				tvElapsedTime			=
					tvCurrentTime - 1000 - tvStartTime;
			}

			if (tvElapsedTime * 1000 > ulLocalTimeoutInSeconds * 1000000 +
				ulLocalAdditionalTimeoutInMicroSeconds)
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			tvRemainingTime			= ulLocalTimeoutInSeconds * 1000 +
				(ulLocalAdditionalTimeoutInMicroSeconds / 1000) - tvElapsedTime;

			ulSeconds					= tvRemainingTime / 1000;
			ulAdditionalMicroSeconds	=
				(tvRemainingTime - (ulSeconds * 1000)) * 1000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				ulSeconds, ulAdditionalMicroSeconds)) != errNoError)
			{
				/*
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
				*/

				delete [] pBufferRead;
			
				return errIsReady;
			}
		#else
			// current - start
			if (tvStartTime. tv_usec <= tvCurrentTime. tv_usec)
			{
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - tvStartTime. tv_sec;
			}
			else
			{
				// ???
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - 1 - tvStartTime. tv_sec;
			}

			tvElapsedTime. tv_usec			=
				(tvCurrentTime. tv_usec + tvStartTime. tv_usec) % 1000000;

			if (tvElapsedTime. tv_sec > ulLocalTimeoutInSeconds ||
				(tvElapsedTime. tv_sec == ulLocalTimeoutInSeconds &&
				tvElapsedTime. tv_usec >
				ulLocalAdditionalTimeoutInMicroSeconds))
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			if (tvElapsedTime. tv_usec <=
				ulLocalAdditionalTimeoutInMicroSeconds)
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - tvElapsedTime. tv_sec;
			else
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - 1 - tvElapsedTime. tv_sec;
			tvRemainingTime. tv_usec			=
				(ulLocalAdditionalTimeoutInMicroSeconds +
				tvElapsedTime. tv_usec) % 1000000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				tvRemainingTime. tv_sec, tvRemainingTime. tv_usec)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				delete [] pBufferRead;
			
				return errIsReady;
			}
		#endif

		if (bIsReadyForReading)
		{
			unsigned long		ulBufferLength;

			ulBufferLength				= 1;

			if (read (pBufferRead + lBufferReadIndex, &ulBufferLength,
				false, 0, 0) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);

				delete [] pBufferRead;
			
				return err;
			}

			if (strstr (pBufferRead, pBufferToWait) != (char *) NULL)
			{
				bIsBufferRead			= true;

				continue;
			}

			if (lBufferReadIndex >= lBufferToWaitLength - 1)
			{
				memcpy (pBufferRead, pBufferRead + 1, lBufferToWaitLength - 1);
			}
			else
			{
				lBufferReadIndex++;
			}
		}

		#ifdef WIN32
			tvCurrentTime		= GetTickCount ();
		#else
			if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_GETTIMEOFDAY_FAILED);

				delete [] pBufferRead;

				return err;
			}
		#endif
	}

	delete [] pBufferRead;

	if (!bIsBufferRead)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_TIMEOUTEXPIRED);

		return err;
	}


	return errNoError;
}


Error SocketImpl:: vacuum (long lTimeoutInSeconds,
	long lAdditionalTimeoutInMicroSeconds)

{

	char					pBufferRead [1 + 1];
	#ifdef WIN32
		DWORD					tvStartTime;
		DWORD					tvCurrentTime;
		DWORD					tvElapsedTime;
		DWORD					tvRemainingTime;
	#else
		timeval					tvStartTime;
		timeval					tvCurrentTime;
		timeval					tvElapsedTime;
		timeval					tvRemainingTime;
		struct timezone			tzTimeZone;
	#endif
	Boolean_t				bIsReadyForReading;
	Error					errIsReady;
	unsigned long			ulLocalTimeoutInSeconds;
	unsigned long			ulLocalAdditionalTimeoutInMicroSeconds;


	if ((lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lAdditionalTimeoutInMicroSeconds < 0 &&
		lAdditionalTimeoutInMicroSeconds != -1) ||
		lAdditionalTimeoutInMicroSeconds >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (lTimeoutInSeconds == -1)
	{
		ulLocalTimeoutInSeconds					= SCK_MAXTIMEOUTINSECONDS;
		ulLocalAdditionalTimeoutInMicroSeconds	= 0;
	}
	else
	{
		ulLocalTimeoutInSeconds					= lTimeoutInSeconds;
		ulLocalAdditionalTimeoutInMicroSeconds	=
			lAdditionalTimeoutInMicroSeconds;
	}

	#ifdef WIN32
		tvStartTime			= GetTickCount ();
		tvCurrentTime		= GetTickCount ();
	#else
		if (gettimeofday (&tvStartTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			return err;
		}

		tvCurrentTime			= tvStartTime;

		/*
		if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			return err;
		}
		*/
	#endif

	while (1)
	{
		#ifdef WIN32
			unsigned long				ulSeconds;
			unsigned long				ulAdditionalMicroSeconds;

			// current - start
			if (tvStartTime <= tvCurrentTime)
			{
				tvElapsedTime			= tvCurrentTime - tvStartTime;
			}
			else
			{
				// ???
				tvElapsedTime			=
					tvCurrentTime - 1000 - tvStartTime;
			}

			if (tvElapsedTime * 1000 > ulLocalTimeoutInSeconds * 1000000 +
				ulLocalAdditionalTimeoutInMicroSeconds)
				break;


			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			tvRemainingTime			= ulLocalTimeoutInSeconds * 1000 +
				(ulLocalAdditionalTimeoutInMicroSeconds / 1000) -
				tvElapsedTime;

			ulSeconds					= tvRemainingTime / 1000;
			ulAdditionalMicroSeconds	=
				(tvRemainingTime - (ulSeconds * 1000)) * 1000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				ulSeconds, ulAdditionalMicroSeconds)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				return errIsReady;
			}
		#else
			// current - start
			if (tvStartTime. tv_usec <= tvCurrentTime. tv_usec)
			{
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - tvStartTime. tv_sec;
			}
			else
			{
				// ???
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - 1 - tvStartTime. tv_sec;
			}

			tvElapsedTime. tv_usec			=
				(tvCurrentTime. tv_usec + tvStartTime. tv_usec) % 1000000;

			if (tvElapsedTime. tv_sec > ulLocalTimeoutInSeconds ||
				(tvElapsedTime. tv_sec == ulLocalTimeoutInSeconds &&
				tvElapsedTime. tv_usec >
				ulLocalAdditionalTimeoutInMicroSeconds))
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			if (tvElapsedTime. tv_usec <=
				ulLocalAdditionalTimeoutInMicroSeconds)
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - tvElapsedTime. tv_sec;
			else
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - 1 - tvElapsedTime. tv_sec;
			tvRemainingTime. tv_usec			=
				(ulLocalAdditionalTimeoutInMicroSeconds +
				tvElapsedTime. tv_usec) % 1000000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				tvRemainingTime. tv_sec, tvRemainingTime. tv_usec)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
			
				return errIsReady;
			}
		#endif

		if (bIsReadyForReading)
		{
			unsigned long			ulBufferLength;

			ulBufferLength				= 1;

			if (read (pBufferRead, &ulBufferLength, false, 0, 0) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);

				return err;
			}

			#ifdef TELNET_DEBUG
				cout << "'" << (unsigned char) pBufferRead [0] << "'";
			#endif
		}

		#ifdef WIN32
			tvCurrentTime		= GetTickCount ();
		#else
			if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_GETTIMEOFDAY_FAILED);

				return err;
			}
		#endif
	}

	#ifdef TELNET_DEBUG
		cout << endl;
	#endif


	return errNoError;
}


Error SocketImpl:: vacuumByTelnet (long lTimeoutInSeconds,
	long lAdditionalTimeoutInMicroSeconds)

{

	char					pBufferRead [1 + 1];
	#ifdef WIN32
		DWORD					tvStartTime;
		DWORD					tvCurrentTime;
		DWORD					tvElapsedTime;
		DWORD					tvRemainingTime;
	#else
		timeval					tvStartTime;
		timeval					tvCurrentTime;
		timeval					tvElapsedTime;
		timeval					tvRemainingTime;
		struct timezone			tzTimeZone;
	#endif
	Boolean_t				bIsReadyForReading;
	Error					errIsReady;
	Error					errTelnetDecoder;
	unsigned long			ulLocalTimeoutInSeconds;
	unsigned long			ulLocalAdditionalTimeoutInMicroSeconds;


	if ((lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lAdditionalTimeoutInMicroSeconds < 0 &&
		lAdditionalTimeoutInMicroSeconds != -1) ||
		lAdditionalTimeoutInMicroSeconds >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (lTimeoutInSeconds == -1)
	{
		ulLocalTimeoutInSeconds					= SCK_MAXTIMEOUTINSECONDS;
		ulLocalAdditionalTimeoutInMicroSeconds	= 0;
	}
	else
	{
		ulLocalTimeoutInSeconds					= lTimeoutInSeconds;
		ulLocalAdditionalTimeoutInMicroSeconds	=
			lAdditionalTimeoutInMicroSeconds;
	}

	#ifdef WIN32
		tvStartTime			= GetTickCount ();
		tvCurrentTime		= GetTickCount ();
	#else
		if (gettimeofday (&tvStartTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			return err;
		}

		tvCurrentTime				= tvStartTime;

		/*
		if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			return err;
		}
		*/
	#endif

	while (1)
	{
		#ifdef WIN32
			unsigned long				ulSeconds;
			unsigned long				ulAdditionalMicroSeconds;

			// current - start
			if (tvStartTime <= tvCurrentTime)
			{
				tvElapsedTime			= tvCurrentTime - tvStartTime;
			}
			else
			{
				// ???
				tvElapsedTime			=
					tvCurrentTime - 1000 - tvStartTime;
			}

			if (tvElapsedTime * 1000 > ulLocalTimeoutInSeconds * 1000000 +
				ulLocalAdditionalTimeoutInMicroSeconds)
				break;


			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			tvRemainingTime			= ulLocalTimeoutInSeconds * 1000 +
				(ulLocalAdditionalTimeoutInMicroSeconds / 1000) - tvElapsedTime;

			ulSeconds					= tvRemainingTime / 1000;
			ulAdditionalMicroSeconds	=
				(tvRemainingTime - (ulSeconds * 1000)) * 1000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				ulSeconds, ulAdditionalMicroSeconds)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				return errIsReady;
			}
		#else
			// current - start
			if (tvStartTime. tv_usec <= tvCurrentTime. tv_usec)
			{
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - tvStartTime. tv_sec;
			}
			else
			{
				// ???
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - 1 - tvStartTime. tv_sec;
			}

			tvElapsedTime. tv_usec			=
				(tvCurrentTime. tv_usec + tvStartTime. tv_usec) % 1000000;

			if (tvElapsedTime. tv_sec > ulLocalTimeoutInSeconds ||
				(tvElapsedTime. tv_sec == ulLocalTimeoutInSeconds &&
				tvElapsedTime. tv_usec >
				ulLocalAdditionalTimeoutInMicroSeconds))
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			if (tvElapsedTime. tv_usec <=
				ulLocalAdditionalTimeoutInMicroSeconds)
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - tvElapsedTime. tv_sec;
			else
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - 1 - tvElapsedTime. tv_sec;
			tvRemainingTime. tv_usec			=
				(ulLocalAdditionalTimeoutInMicroSeconds +
				tvElapsedTime. tv_usec) % 1000000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				tvRemainingTime. tv_sec, tvRemainingTime. tv_usec)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
			
				return errIsReady;
			}
		#endif

		if (bIsReadyForReading)
		{
			unsigned long			ulBufferLength;

			ulBufferLength				= 1;

			if (read (pBufferRead, &ulBufferLength, false, 0, 0) != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_READ_FAILED);

				return err;
			}

			if ((errTelnetDecoder = telnetDecoder ((unsigned char *) pBufferRead,
				1)) != errNoError)
			{
				//	Error err = SocketErrors (__FILE__, __LINE__,
				//		SCK_SOCKETIMPL_TELNETDECODER_FAILED);

				return errTelnetDecoder;
			}

			#ifdef TELNET_DEBUG
				cout << "'" << (unsigned char) pBufferRead [0] << "'";
			#endif
		}

		#ifdef WIN32
			tvCurrentTime		= GetTickCount ();
		#else
			if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_GETTIMEOFDAY_FAILED);

				return err;
			}
		#endif
	}

	#ifdef TELNET_DEBUG
		cout << endl;
	#endif


	return errNoError;
}


Error SocketImpl:: read (void *pvBuffer, unsigned long *pulBufferLength,
	Boolean_t bReadingCheckToBePerformed,
	unsigned long ulSecondsToWait, unsigned long ulAdditionalMicrosecondsToWait,
	Boolean_t bOneShotRead, Boolean_t bRemoveDataFromSocket)

{

	long						lRemainBufferLengthToRead;
	long						lReadByte;
	long						lAllBytesRead;
	struct sockaddr_in			sckServerAddr;
	#ifdef WIN32
		int							lServerAddrSize;
	#elif defined hpux || defined __hpux
		int							lServerAddrSize;
	#else
		socklen_t					lServerAddrSize;
	#endif
	int							iFlags;


	if (pvBuffer == (void *) NULL ||
		*pulBufferLength == 0 ||
		ulAdditionalMicrosecondsToWait >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (bReadingCheckToBePerformed)
	{
		Error_t				errIsReady;
		Boolean_t			bIsReadyForReading;

		if ((errIsReady = isReadyForReading (&bIsReadyForReading,
			ulSecondsToWait, ulAdditionalMicrosecondsToWait)) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			return errIsReady;
		}

		if (!bIsReadyForReading)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NOTHINGTOREAD,
				3, (long) _iFd, ulSecondsToWait,
				ulAdditionalMicrosecondsToWait);

			return err;
		}
	}

	memset (pvBuffer, 0, *pulBufferLength);

	if (bRemoveDataFromSocket)
		iFlags			= 0;
	else
		iFlags			= MSG_PEEK;

	lAllBytesRead						= 0;
	lRemainBufferLengthToRead			= *pulBufferLength;

	while (lRemainBufferLengthToRead > 0)
	{
		lServerAddrSize			= sizeof (sckServerAddr);

		do
		{
			#ifdef WIN32
				lReadByte = ::recvfrom (_iFd,
					(((char *) pvBuffer) + *pulBufferLength -
					lRemainBufferLengthToRead),
					(size_t) lRemainBufferLengthToRead, iFlags,
					(struct sockaddr *) &sckServerAddr, &lServerAddrSize);
			#elif defined hpux || defined __hpux
				lReadByte = ::recvfrom (_iFd,
					(void *) (((char *) pvBuffer) + *pulBufferLength -
					lRemainBufferLengthToRead),
					(int) lRemainBufferLengthToRead, iFlags,
					(void *) &sckServerAddr, &lServerAddrSize);
			#else
				lReadByte = ::recvfrom (_iFd,
					(void *) (((char *) pvBuffer) + *pulBufferLength -
					lRemainBufferLengthToRead),
					(size_t) lRemainBufferLengthToRead, iFlags,
					(struct sockaddr *) &sckServerAddr, &lServerAddrSize);
			#endif
		}
		#ifdef WIN32
			while ((lReadByte < 0) && (WSAGetLastError () == EINTR));
		#else
			while ((lReadByte < 0) && (errno == EINTR));
		#endif

		if (lReadByte == -1)
		{
			int				iErrno;
			Error			err;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			#ifdef WIN32
				if (iErrno == WSAECONNRESET)
			#else
				if (iErrno == ECONNRESET)
			#endif
			{
				err = SocketErrors (__FILE__, __LINE__,
					SCK_READ_EOFREACHED,
					3, _pRemoteAddress, _lRemotePort, lAllBytesRead);
			}
			#ifdef WIN32
			#else
				else if (iErrno == EAGAIN)
				{
					// we have this error when the socket is set as BLOCKING
					// and the default timeout expires
					// Inside the error 0 is passed as argument because we don't
					// have available the timeout information and,
					// for performance reason, we don't want to retrieve them
					err = SocketErrors (__FILE__, __LINE__,
						SCK_NOTHINGTOREAD,
						3, (long) _iFd, 0, 0);
				}
			#endif
			else
			{
				err = SocketErrors (__FILE__, __LINE__,
					SCK_READ_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));
			}

			return err;
		}
		else if (lReadByte == 0)
		{
			// zero indicates end of file
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_READ_EOFREACHED,
				3, _pRemoteAddress, _lRemotePort, lAllBytesRead);

			return err;
		}
		else
		{
			lRemainBufferLengthToRead			-= lReadByte;
			lAllBytesRead						+= lReadByte;
		}

		if (bOneShotRead)
			break;
	}

	*pulBufferLength				= lAllBytesRead;


	return errNoError;
}


/*
Error SocketImpl:: readLine (char *pBuffer, long lBufferLength,
	long *plCharsRead, long lTimeoutInSeconds,
	long lTimeoutInMicroSeconds)

{

	long						lBufferIndex;
	long						lReadReturn;
	Boolean_t					bLineIsFinish;
	char						c;
	#ifdef WIN32
		DWORD					tvStartTime;
		DWORD					tvCurrentTime;
		DWORD					tvElapsedTime;
		DWORD					tvRemainingTime;
		int						lServerAddrSize;
	#elif defined hpux || defined __hpux
		timeval					tvStartTime;
		timeval					tvCurrentTime;
		timeval					tvElapsedTime;
		timeval					tvRemainingTime;
		struct timezone			tzTimeZone;
		int						lServerAddrSize;
	#else
		timeval					tvStartTime;
		timeval					tvCurrentTime;
		timeval					tvElapsedTime;
		timeval					tvRemainingTime;
		struct timezone			tzTimeZone;
		socklen_t				lServerAddrSize;
	#endif
	Boolean_t					bIsReadyForReading;
	struct sockaddr_in			sckServerAddr;
	Error					errIsReady;
	unsigned long			ulLocalTimeoutInSeconds;
	unsigned long			ulLocalTimeoutInMicroSeconds;


	if (pBuffer == (char *) NULL ||
		lBufferLength < 2 ||
		plCharsRead == (long *) NULL ||
		(lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lTimeoutInMicroSeconds < 0 && lTimeoutInMicroSeconds != -1) ||
		lTimeoutInMicroSeconds >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	memset (pBuffer, 0, (unsigned short) lBufferLength);

	if (lTimeoutInSeconds == -1)
	{
		ulLocalTimeoutInSeconds					= SCK_MAXTIMEOUTINSECONDS;
		ulLocalTimeoutInMicroSeconds			= 0;
	}
	else
	{
		ulLocalTimeoutInSeconds					= lTimeoutInSeconds;
		ulLocalTimeoutInMicroSeconds			= lTimeoutInMicroSeconds;
	}

	lBufferIndex		= 0;

	bLineIsFinish		= false;

	#ifdef WIN32
		tvStartTime			= GetTickCount ();
		tvCurrentTime		= GetTickCount ();
	#else
		if (gettimeofday (&tvStartTime, &tzTimeZone) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETTIMEOFDAY_FAILED);

			return err;
		}

		tvCurrentTime			= tvStartTime;

		// if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
		// {
		//	Error err = SocketErrors (__FILE__, __LINE__,
		//		SCK_GETTIMEOFDAY_FAILED);

		//	return err;
		//}
	#endif

	while (!bLineIsFinish)
	{
		#ifdef WIN32
			unsigned long				ulSeconds;
			unsigned long				ulMicroSeconds;

			// current - start
			if (tvStartTime <= tvCurrentTime)
			{
				tvElapsedTime			= tvCurrentTime - tvStartTime;
			}
			else
			{
				// ???
				tvElapsedTime			=
					tvCurrentTime - 1000 - tvStartTime;
			}

			if (tvElapsedTime * 1000 > ulLocalTimeoutInSeconds * 1000000 +
				ulLocalTimeoutInMicroSeconds)
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			tvRemainingTime			= ulLocalTimeoutInSeconds * 1000 +
				(ulLocalTimeoutInMicroSeconds / 1000) - tvElapsedTime;

			ulSeconds		= tvRemainingTime / 1000;
			ulMicroSeconds	= (tvRemainingTime - (ulSeconds * 1000)) * 1000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				ulSeconds, ulMicroSeconds)) != errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

				return errIsReady;
			}
		#else
			// current - start
			if (tvStartTime. tv_usec <= tvCurrentTime. tv_usec)
			{
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - tvStartTime. tv_sec;
			}
			else
			{
				// ???
				tvElapsedTime. tv_sec			=
					tvCurrentTime. tv_sec - 1 - tvStartTime. tv_sec;
			}

			tvElapsedTime. tv_usec			=
				(tvCurrentTime. tv_usec + tvStartTime. tv_usec) % 1000000;

			if (tvElapsedTime. tv_sec > ulLocalTimeoutInSeconds ||
				(tvElapsedTime. tv_sec == ulLocalTimeoutInSeconds &&
				tvElapsedTime. tv_usec > ulLocalTimeoutInMicroSeconds))
				break;

			//	(ulLocalTimeoutInSeconds, ulLocalTimeoutInMicroSeconds) -
			//		tvElapsedTime
			if (tvElapsedTime. tv_usec <= ulLocalTimeoutInMicroSeconds)
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - tvElapsedTime. tv_sec;
			else
				tvRemainingTime. tv_sec			=
					ulLocalTimeoutInSeconds - 1 - tvElapsedTime. tv_sec;
			tvRemainingTime. tv_usec			=
				(ulLocalTimeoutInMicroSeconds + tvElapsedTime. tv_usec) %
				1000000;

			if ((errIsReady = isReadyForReading (&bIsReadyForReading,
				tvRemainingTime. tv_sec, tvRemainingTime. tv_usec)) !=
				errNoError)
			{
				// Error err = SocketErrors (__FILE__, __LINE__,
				//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
			
				return errIsReady;
			}
		#endif

		if (bIsReadyForReading)
		{
			lServerAddrSize			= sizeof (sckServerAddr);

			#ifdef WIN32
				lReadReturn = ::recvfrom (_iFd, (&c), 1, 0,
					(struct sockaddr *) &sckServerAddr, &lServerAddrSize);
			#else
				lReadReturn = ::recvfrom (_iFd, (void *) (&c), (size_t) 1, 0,
					(struct sockaddr *) &sckServerAddr, &lServerAddrSize);
			#endif

			switch (lReadReturn)
			{
				case 1:
					//	CR or 0x0D or 13 (in dec.)
					//	LF or '\n' or 0x0A or 10 (in dec.)
					if (c == SCK_TN_LF)
					{
						// found the pair (CR, LF)
						if (lBufferIndex > 0 &&
							pBuffer [lBufferIndex - 1] == SCK_TN_CR)
						{
							lBufferIndex--;
							pBuffer [lBufferIndex]		= '\0';
						}

						bLineIsFinish = true;
						continue;
					}
					else
					{
						//	questo controllo oltre a proteggere da eventuali
						//	scritture fuori buffer garantisce lo '\0' a
						//	fine linea
						if (lBufferIndex >= lBufferLength - 1)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_BUFFER_SMALL);

							return err;
						}

						pBuffer [lBufferIndex]		= c;
						lBufferIndex++;
					}

					break;
				case 0:
					{
						// zero indicates end of file
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_READ_EOFREACHED);

						return err;
					}
				default:
					{
						int				iErrno;


						#ifdef WIN32
							iErrno			= WSAGetLastError ();
						#else
							iErrno			= errno;
						#endif

						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_READ_FAILED, 1, iErrno);
						err. setUserData (&iErrno, sizeof (int));

						return err;
					}
			}
		}

		#ifdef WIN32
			tvCurrentTime		= GetTickCount ();
		#else
			if (gettimeofday (&tvCurrentTime, &tzTimeZone) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_GETTIMEOFDAY_FAILED);

				return err;
			}
		#endif
	}

	*plCharsRead				= lBufferIndex;

	if (!bLineIsFinish)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_TIMEOUTEXPIRED);

		return err;
	}


	return errNoError;
}
*/


Error SocketImpl:: readLine (char *pBuffer,
	unsigned long ulBufferLength,
	unsigned long *pulCharsRead, unsigned long ulTimeoutInSeconds,
	unsigned long ulAdditionalTimeoutInMicroSeconds)

{

	Error_t					errRead;
	unsigned long			ulBytesToRead;
	char					*pEndLine;
	Boolean_t				bReadingCheckToBePerformed;


	if (ulTimeoutInSeconds == 0 &&
		ulAdditionalTimeoutInMicroSeconds == 0)
		bReadingCheckToBePerformed			= false;
	else
		bReadingCheckToBePerformed			= true;

	ulBytesToRead			= ulBufferLength - 1;

	// peek the first junk
	if ((errRead = read (
		pBuffer, &ulBytesToRead, bReadingCheckToBePerformed,
		ulTimeoutInSeconds, ulAdditionalTimeoutInMicroSeconds,
		true, false)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_READ_FAILED);

		return errRead;
	}

	// we could have "\r\n" or "\n"
	if ((pEndLine = strstr (pBuffer, "\r\n")) !=
		(char *) NULL)
	{
		*pulCharsRead				= pEndLine - pBuffer;
		ulBytesToRead				= (*pulCharsRead + 2);
	}
	else
	{
		if ((pEndLine = strchr (pBuffer, '\n')) !=
			(char *) NULL)
		{
			*pulCharsRead			= pEndLine - pBuffer;
			ulBytesToRead			= (*pulCharsRead + 1);
		}
		else
		{
			*pulCharsRead			= ulBytesToRead;
			// ulBytesToRead			= *pulCharsRead;
		}
	}

	// read the line
	if ((errRead = read (
		pBuffer, &ulBytesToRead, false,
		0, 0, true, true)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_READ_FAILED);

		return errRead;
	}

	pBuffer [*pulCharsRead]		= '\0';


	return errNoError;
}


/*
Error SocketImpl:: readLines (char *pBuffer, long lBufferLength,
	long *plCharsRead, long lTimeoutInSeconds,
	long lTimeoutInMicroSeconds, const char *pNewLine)

{

	char						*pLine;
	Boolean_t					bIsReadyToRead;
	Error						errReadLine;
	Error						errIsReadyForRead;
	long						lLineLength;


	if (pBuffer == (char *) NULL ||
		lBufferLength < 2 ||
		plCharsRead == (long *) NULL ||
		(lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lTimeoutInMicroSeconds < 0 && lTimeoutInMicroSeconds != -1) ||
		lTimeoutInMicroSeconds >= 1000000 ||
		pNewLine == (char *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	memset (pBuffer, 0, (unsigned short) lBufferLength);

	if ((errIsReadyForRead = isReadyForReading (&bIsReadyToRead,
		lTimeoutInSeconds, lTimeoutInMicroSeconds)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

		return errIsReadyForRead;
	}

	if (!bIsReadyToRead)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NOTHINGTOREAD,
			3, _iFd, lTimeoutInSeconds, lTimeoutInMicroSeconds);

		return err;
	}

	strcpy (pBuffer, "");

	*plCharsRead		= 0;

	if ((pLine = new char [lBufferLength]) == (char *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		return err;
	}

	while (bIsReadyToRead)
	{
		if ((errReadLine = readLine (pLine, lBufferLength - 1,
			&lLineLength)) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_READLINE_FAILED);

			delete [] pLine;

			return errReadLine;
		}

		if (*plCharsRead + lLineLength >= lBufferLength)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_BUFFER_SMALL);

			delete [] pLine;

			return errReadLine;
		}

		(*plCharsRead)			+= lLineLength;

		strcat (pBuffer, pLine);

		strcat (pBuffer, pNewLine);

		if ((errIsReadyForRead = isReadyForReading (&bIsReadyToRead,
			1, 0)) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			delete [] pLine;

			return errIsReadyForRead;
		}
	}

	delete [] pLine;


	return errNoError;
}
*/


Error SocketImpl:: readLineByTelnet (char *pBuffer,
	long lBufferLength, long lTimeoutInSeconds,
	long lAdditionalTimeoutInMicroSeconds)

{
	Error			errReadLine;
	Error			errTelnetDecoder;
	unsigned long	ulCharsRead;


	if (pBuffer == (char *) NULL ||
		lBufferLength < 1 ||
		(lTimeoutInSeconds < 0 && lTimeoutInSeconds != -1) ||
		(lAdditionalTimeoutInMicroSeconds < 0 &&
		lAdditionalTimeoutInMicroSeconds != -1) ||
		lAdditionalTimeoutInMicroSeconds >= 1000000)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if ((errReadLine = readLine (pBuffer, lBufferLength,
		&ulCharsRead, lTimeoutInSeconds, lAdditionalTimeoutInMicroSeconds)) !=
		errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		// 	SCK_SOCKETIMPL_READLINE_FAILED);

		return errReadLine;
	}

	if ((errTelnetDecoder = telnetDecoder ((unsigned char *) pBuffer,
		ulCharsRead)) != errNoError)
	{
		//	Error err = SocketErrors (__FILE__, __LINE__,
		//		SCK_SOCKETIMPL_TELNETDECODER_FAILED);

		return errTelnetDecoder;
	}


	return errNoError;
}


/*
Error SocketImpl:: readLineByTelnet (char *pBuffer,
	unsigned long *pulBufferLength, Boolean_t bOneShotRead)

{

	Error			errRead;
	Error			errTelnetDecoder;


	if (pBuffer == (char *) NULL ||
		*pulBufferLength == 0)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	memset (pBuffer, 0, (unsigned short) (*pulBufferLength));

	// Potrebbero essere utilizzati i parametri della read
	// bReadingCheckToBePerformed, ulSecondsToWait and ulMicrosecondsToWait
	if ((errRead = readLine (pBuffer, pulBufferLength, false, 0, 0,
		bOneShotRead)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_READLINE_FAILED);

		return errRead;
	}

	if ((errTelnetDecoder = telnetDecoder ((unsigned char *) pBuffer,
		*pulBufferLength)) != errNoError)
	{
		//	Error err = SocketErrors (__FILE__, __LINE__,
		//		SCK_SOCKETIMPL_TELNETDECODER_FAILED);

		return errTelnetDecoder;
	}


	return errNoError;
}
*/


Error SocketImpl:: readByTelnet (char *pBuffer,
	unsigned long *pulBufferLength, Boolean_t bOneShotRead)

{

	Error			errRead;
	Error			errTelnetDecoder;


	if (pBuffer == (char *) NULL ||
		*pulBufferLength == 0)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	memset (pBuffer, 0, (unsigned short) (*pulBufferLength));

	// Potrebbero essere utilizzati i parametri della read
	// bReadingCheckToBePerformed, ulSecondsToWait and ulMicrosecondsToWait
	if ((errRead = read (pBuffer, pulBufferLength, false, 0, 0,
		bOneShotRead)) != errNoError)
	{
		// Error err = SocketErrors (__FILE__, __LINE__,
		//	SCK_SOCKETIMPL_READLINE_FAILED);

		return errRead;
	}

	if ((errTelnetDecoder = telnetDecoder ((unsigned char *) pBuffer,
		*pulBufferLength)) != errNoError)
	{
		//	Error err = SocketErrors (__FILE__, __LINE__,
		//		SCK_SOCKETIMPL_TELNETDECODER_FAILED);

		return errTelnetDecoder;
	}


	return errNoError;
}


Error SocketImpl:: telnetDecoder (
	unsigned char *pucBuffer, long lBufferToProcessLength)

{

	long				lBufferToProcessIndex;
	unsigned char		cCharToProcess;
	long				lBufferProcessedIndex;


	if (pucBuffer == (unsigned char *) NULL ||
		lBufferToProcessLength < 0)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	lBufferProcessedIndex					= 0;

	for (lBufferToProcessIndex = 0;
		lBufferToProcessIndex < lBufferToProcessLength; lBufferToProcessIndex++)
	{
		cCharToProcess			= pucBuffer [lBufferToProcessIndex];

		#ifdef TELNET_DEBUG
			switch (cCharToProcess)
			{
				case SCK_TN_IAC:
					cout << "'TN_IAC'";

					break;
				case SCK_TN_DO:
					cout << "'TN_DO'";

					break;
				case SCK_TN_DONT:
					cout << "'TN_DONT'";

					break;
				case SCK_TN_WILL:
					cout << "'TN_WILL'";

					break;
				case SCK_TN_WONT:
					cout << "'TN_WONT'";

					break;
				case SCK_TN_SE:
					cout << "'TN_SE'";

					break;
				case SCK_TN_TERMTYPE:
					cout << "'TN_TERMTYPE'";

					break;
				case SCK_TN_SEND:
					cout << "'TN_SEND'";

					break;
				case SCK_TN_IS:
					cout << "'TN_IS'";

					break;
				default:
					cout << "'" << (unsigned char) cCharToProcess << "'";

					break;
			}
		#endif

		if (_lIAC_State)
		{
			//	Telnet IAC State Machine */
			switch (_lIAC_State)
			{
				case SCK_IAC_OPCODE:
					{
						switch (cCharToProcess)
						{
							case SCK_TN_WILL:
								_lIAC_State			= SCK_IAC_WILL;

								break;
							case SCK_TN_DO:
								_lIAC_State			= SCK_IAC_DO;

								break;
							case SCK_TN_SB:
								_lIAC_State			= SCK_IAC_SB;

								break;
							case  SCK_TN_IAC:
								//	0xFF quoted via IAC
								pucBuffer [lBufferProcessedIndex++]		= 0xFF;
								_lIAC_State			= SCK_IAC_RESET;

								break;
							default:
								_lIAC_State			= SCK_IAC_RESET;

								break;
						}
					}

					break;
				case SCK_IAC_SB:
					//	If cCharToProcess is TN_TERMTYPE then we continue,
					//	otherwise we ignore this...
					if (cCharToProcess == SCK_TN_TERMTYPE)
						_lIAC_State				= SCK_IAC_TERMTYPE;
					else
						_lIAC_State				= SCK_IAC_RESET;

					break;
				case SCK_IAC_TERMTYPE:
					//	If cCharToProcess is TN_SEND then we continue,
					//	otherwise we ignore this sequence...
					if (cCharToProcess == SCK_TN_SEND)
						_lIAC_State				= SCK_IAC_SEND;
					else
						_lIAC_State				= SCK_IAC_RESET;

					break;
				case SCK_IAC_SEND:
					//	If cCharToProcess is TN_IAC  then we continue,
					//	otherwise we ignore this sequence...
					if (cCharToProcess == SCK_TN_IAC)
						_lIAC_State				= SCK_IAC_IAC;
					else
						_lIAC_State				= SCK_IAC_RESET;

					break;
				case SCK_IAC_IAC:
					_lIAC_State					= SCK_IAC_RESET;

					//	If cCharToProcess is TN_SE  then we send our terminal
					//	type otherwise, we abort this sequence
					if (cCharToProcess == SCK_TN_SE)
					{
						//	Send Terminal Type here...
						//	By adding it to the queue
						char			pResponce [
							SCK_TN_MAXRESPONSELENGTH];


						pResponce [0]			= SCK_TN_IAC;
						pResponce [1]			= SCK_TN_SB;
						pResponce [2]			= SCK_TN_TERMTYPE;
						pResponce [3]			= SCK_TN_IS;
						pResponce [4]			= 'd';
						pResponce [5]			= 'u';
						pResponce [6]			= 'm';
						pResponce [7]			= 'b';
						pResponce [8]			= SCK_TN_IAC;
						pResponce [9]			= SCK_TN_SE;

						if (write (pResponce, 10, true, 0, 1000) != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITESTRING_FAILED);
							
							return err;
						}

						#ifdef TELNET_DEBUG
							cout << endl << "Responce: '";
							cout << "TN_IAC''TN_SB''TN_TERMTYPE''TN_IS''d''u''m''b''TN_IAC''TN_SE'" << endl;
						#endif
					}

					break;
				case SCK_IAC_WILL:
					{
						//	Always Respond with 'DONT'
						//	End of sequence.
						char			pResponce [
							SCK_TN_MAXRESPONSELENGTH];


						_lIAC_State				= SCK_IAC_RESET;

						pResponce [0]			= SCK_TN_IAC;
						pResponce [1]			= SCK_TN_DONT;
						pResponce [2]			= cCharToProcess;

						if (write (pResponce, 3, true, 0, 1000) != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITESTRING_FAILED);
						
							return err;
						}

						#ifdef TELNET_DEBUG
							cout << endl << "Responce: '";
							cout << "'TN_IAC''TN_DONT'" << "'"
							<< (unsigned char) cCharToProcess << "'" << endl;
						#endif
					}

					break;
				case SCK_IAC_DO:
					_lIAC_State					= SCK_IAC_RESET;

					if (cCharToProcess == SCK_TN_TERMTYPE)
					{
						//	Send IAC WILL TERMTYPE here...
						//	By adding it to the queue
						char			pResponce [
							SCK_TN_MAXRESPONSELENGTH];


						pResponce [0]			= SCK_TN_IAC;
						pResponce [1]			= SCK_TN_WILL;
						pResponce [2]			= SCK_TN_TERMTYPE;

						if (write (pResponce, 3, true, 0, 1000) != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITESTRING_FAILED);
						
							return err;
						}

						#ifdef TELNET_DEBUG
							cout << endl << "Responce: '";
							cout << "'TN_IAC''TN_WILL''TN_TERMTYPE'" << endl;
						#endif
					}
					else
					{
						/* Send WONT <cCharToProcess> here...     */
						/* By adding it to the queue */
						char			pResponce [
							SCK_TN_MAXRESPONSELENGTH];


						pResponce [0]			= SCK_TN_IAC;
						pResponce [1]			= SCK_TN_WONT;
						pResponce [2]			= cCharToProcess;

						if (write (pResponce, 3, true, 0, 1000) != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITESTRING_FAILED);
						
							return err;
						}

						#ifdef TELNET_DEBUG
							cout << endl << "Responce: '";
							cout << "'TN_IAC''TN_WONT'" << "'"
								<< (unsigned char) cCharToProcess << "'"
								<< endl;
						#endif
					}

					break;
				default:
					_lIAC_State					= SCK_IAC_RESET;

					break;
			}
		}
		else
		{
			//	If we get the IAC introducer (255) we set up to expect an
			//	operation code next...
			if (cCharToProcess == SCK_TN_IAC)
			{
				_lIAC_State			= SCK_IAC_OPCODE;
			}
			else 
			{
				//	If not in IAC Mode, or not an IAC escape sequence,
				//	we just copy the data to the buffer...
				pucBuffer [lBufferProcessedIndex++]		=
					cCharToProcess;
			}
		}
	}

	#ifdef TELNET_DEBUG
		cout << endl;
	#endif

	pucBuffer [lBufferProcessedIndex]		= '\0';


	return errNoError;
}


Error SocketImpl:: readLineFromJava (char *pBuffer, long lBufferLength)

{

	long			lBufferIndex;
	long			lReadReturn;
	Boolean_t		bLineIsFinish;
	char			pJavaChar [2];


	// DA RIVEDERE

	if (pBuffer == (char *) NULL ||
		lBufferLength < 1)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	memset (pBuffer, 0, (unsigned short) lBufferLength);

	lBufferIndex		= 0;

	bLineIsFinish		= false;

	//	Se lReadReturn e' 2, in pJavaChar ci sara'
	//		'\0''\0' op. '\0'<char> op <char>'\0'
	// Se lReadReturn 1, in pJavaChar ci sara' '\n''dirty'
	while (!bLineIsFinish)
	{
		// use the read function of this class
		lReadReturn = ::read (_iFd, (void *) pJavaChar, (size_t) 2);

		switch (lReadReturn)
		{
			case 2:
				if (pJavaChar [0] == '\0' && pJavaChar [1] == '\0')
					;
				else if (pJavaChar [0] == '\0')	// '\0'<char>
				{
					if (pJavaChar [1] == '\n')
					{
						bLineIsFinish = true;
						continue;
					}
					else
					{
						pBuffer [lBufferIndex] = pJavaChar [1];
						lBufferIndex++;
					}
				}
				else if (pJavaChar [1] == '\0')	// <char>'\0'
				{
					if (pJavaChar [0] == '\n')
					{
						bLineIsFinish = true;
						continue;
					}
					else
					{
						pBuffer [lBufferIndex] = pJavaChar [0];
						lBufferIndex++;
					}
				}
				else	// <char><char>
				{
					int				iErrno;


					#ifdef WIN32
						iErrno			= WSAGetLastError ();
					#else
						iErrno			= errno;
					#endif

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_READ_FAILED, 1, iErrno);
					err. setUserData (&iErrno, sizeof (int));

					return err;
				}

				break;
			case 1:	
				if (pJavaChar [0] == '\n')
				{
					bLineIsFinish = true;

					continue;
				}

				break;
			case 0:
				{
					// zero indicates end of file
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_READ_EOFREACHED,
						2, _pRemoteAddress, _lRemotePort);

					return err;
				}
			default:
				{
					int				iErrno;


					#ifdef WIN32
						iErrno			= WSAGetLastError ();
					#else
						iErrno			= errno;
					#endif

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_READ_FAILED, 1, iErrno);
					err. setUserData (&iErrno, sizeof (int));

					return err;
				}
		}

		if (lBufferIndex == lBufferLength)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_BUFFER_SMALL);

			return err;
		}
	}


	return errNoError;
}


Error SocketImpl:: write (void *pvBuffer, long lBufferLength,
	Boolean_t bWritingCheckToBePerformed,
	unsigned long ulSecondsToWait, unsigned long ulAdditionalMicrosecondsToWait,
	const char *pRemoteAddress, long lRemotePort)

{
	long						lBytesNotWritten;
	long						lBytesWritten;
	long						lWriteReturn;
	struct sockaddr_in			sckServerAddr;


	if (pvBuffer == (void *) NULL ||
		lBufferLength < 1 ||
		ulAdditionalMicrosecondsToWait >= 1000000 ||
		(_bIsConnectionRealized == false &&
		 (pRemoteAddress == (const char *) NULL ||
		 lRemotePort == -1)))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (bWritingCheckToBePerformed)
	{
		Error_t				errIsReady;
		Boolean_t			bIsReadyForWriting;

		if ((errIsReady = isReadyForWriting (&bIsReadyForWriting,
			ulSecondsToWait, ulAdditionalMicrosecondsToWait)) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			return errIsReady;
		}

		if (!bIsReadyForWriting)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NOTREADYFORWRITING,
				3, (long) _iFd, ulSecondsToWait,
				ulAdditionalMicrosecondsToWait);

			return err;
		}
	}

	memset ((char *) &sckServerAddr, 0,
		(unsigned short) sizeof (sckServerAddr));
	sckServerAddr. sin_family				= AF_INET;
	sckServerAddr. sin_addr. s_addr			= inet_addr (
		_bIsConnectionRealized ? _pRemoteAddress : pRemoteAddress);
	sckServerAddr. sin_port					= htons (
		_bIsConnectionRealized ? (unsigned short) _lRemotePort :
		(unsigned short) lRemotePort);

	lBytesWritten		= 0;
	lBytesNotWritten	= lBufferLength;
	while (lBytesNotWritten > 0)
	{
		#ifdef WIN32
			lWriteReturn = ::sendto (_iFd,
				(((char *) pvBuffer) + lBytesWritten),
				lBytesNotWritten, 0,
				(const struct sockaddr *) &sckServerAddr,
				sizeof (sckServerAddr));
		#else
			lWriteReturn = ::sendto (_iFd,
				(const void *) (((char *) pvBuffer) + lBytesWritten),
				(size_t) lBytesNotWritten, 0,
				(const struct sockaddr *) &sckServerAddr,
				sizeof (sckServerAddr));
		#endif

		if (lWriteReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SEND_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}

		lBytesNotWritten	-= lWriteReturn;
		lBytesWritten		+= lWriteReturn;
	}


	return errNoError;
}


Error SocketImpl:: writeString (const char *pString,
	Boolean_t bWritingCheckToBePerformed,
	unsigned long ulSecondsToWait, unsigned long ulAdditionalMicrosecondsToWait,
	const char *pRemoteAddress, long lRemotePort)

{

	long						lBytesNotWritten;
	long						lBytesWritten;
	long						lWriteReturn;
	struct sockaddr_in			sckServerAddr;


	if (pString == (char *) NULL ||
		ulAdditionalMicrosecondsToWait >= 1000000 ||
		(_bIsConnectionRealized == false &&
		 (pRemoteAddress == (const char *) NULL ||
		 lRemotePort == -1)))
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	if (bWritingCheckToBePerformed)
	{
		Error_t				errIsReady;
		Boolean_t			bIsReadyForWriting;

		if ((errIsReady = isReadyForWriting (&bIsReadyForWriting,
			ulSecondsToWait, ulAdditionalMicrosecondsToWait)) != errNoError)
		{
			// Error err = SocketErrors (__FILE__, __LINE__,
			//	SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);

			return errIsReady;
		}

		if (!bIsReadyForWriting)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NOTREADYFORWRITING,
				3, (long) _iFd, ulSecondsToWait,
				ulAdditionalMicrosecondsToWait);

			return err;
		}
	}

	memset ((char *) &sckServerAddr, 0,
		(unsigned short) sizeof (sckServerAddr));
	sckServerAddr. sin_family				= AF_INET;
	sckServerAddr. sin_addr. s_addr			= inet_addr (
		_bIsConnectionRealized ? _pRemoteAddress : pRemoteAddress);
	sckServerAddr. sin_port					= htons (
		_bIsConnectionRealized ? (unsigned short) _lRemotePort :
		(unsigned short) lRemotePort);

	lBytesWritten		= 0;
	lBytesNotWritten	= strlen (pString);
	while (lBytesNotWritten > 0)
	{
		#ifdef WIN32
			lWriteReturn = ::sendto (_iFd,
				(const char *) (pString + lBytesWritten),
				lBytesNotWritten, 0,
				(const struct sockaddr *) &sckServerAddr,
				sizeof (sckServerAddr));
		#elif __APPLE__
			lWriteReturn = ::sendto (_iFd,
				(const void *) (pString + lBytesWritten),
				(size_t) lBytesNotWritten,
				_stSocketType == STREAM ? MSG_OOB : 0,
				(const struct sockaddr *) &sckServerAddr,
				sizeof (sckServerAddr));
		#else
			lWriteReturn = ::sendto (_iFd,
				(const void *) (pString + lBytesWritten),
				(size_t) lBytesNotWritten,
				_stSocketType == STREAM ? MSG_NOSIGNAL : 0,
				(const struct sockaddr *) &sckServerAddr,
				sizeof (sckServerAddr));
		#endif

		if (lWriteReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_WRITE_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}

		lBytesNotWritten	-= lWriteReturn;
		lBytesWritten		+= lWriteReturn;
	}


	return errNoError;
}


Error SocketImpl:: getRemoteAddress (char *pRemoteAddress,
	unsigned long ulBufferLength)

{

	if (pRemoteAddress == (char *) NULL ||
		strlen (_pRemoteAddress) >= ulBufferLength)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	strcpy (pRemoteAddress, _pRemoteAddress);


	return errNoError;
}


Error SocketImpl:: getRemotePort (long *plRemotePort)

{

	if (plRemotePort == (long *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	*plRemotePort			= _lRemotePort;


	return errNoError;
}


Error SocketImpl:: getLocalAddress (char *pLocalAddress,
	unsigned long ulBufferLength)

{

	if (pLocalAddress == (char *) NULL ||
		strlen (_pLocalAddress) >= ulBufferLength)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	strcpy (pLocalAddress, _pLocalAddress);


	return errNoError;
}


Error SocketImpl:: getLocalPort (long *plLocalPort)

{

	if (plLocalPort == (long *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	*plLocalPort			= _lLocalPort;


	return errNoError;
}


Error SocketImpl:: getReceivingTimeouts (
	unsigned long *pulReceivingTimeoutInSeconds,
	unsigned long *pulReceivingAdditionalTimeoutInMicroSeconds)

{

	if (pulReceivingTimeoutInSeconds == (unsigned long *) NULL ||
		pulReceivingAdditionalTimeoutInMicroSeconds == (unsigned long *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	*pulReceivingTimeoutInSeconds					=
		_ulReceivingTimeoutInSeconds;
	*pulReceivingAdditionalTimeoutInMicroSeconds	=
		_ulReceivingAdditionalTimeoutInMicroSeconds;


	return errNoError;
}


Error SocketImpl:: getSendingTimeouts (
	unsigned long *pulSendingTimeoutInSeconds,
	unsigned long *pulSendingAdditionalTimeoutInMicroSeconds)

{

	if (pulSendingTimeoutInSeconds == (unsigned long *) NULL ||
		pulSendingAdditionalTimeoutInMicroSeconds == (unsigned long *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	*pulSendingTimeoutInSeconds					=
		_ulSendingTimeoutInSeconds;
	*pulSendingAdditionalTimeoutInMicroSeconds	=
		_ulSendingAdditionalTimeoutInMicroSeconds;


	return errNoError;
}


Error SocketImpl:: getIPAddressesList (
	std:: vector<IPAddress_t> *pvIPAddresses)

{

	#ifdef WIN32
		int							iTemporaryFd;
		char						pInputBuffer [SCK_MAXADDRESSBUFFERSIZE];
		char						pOutputBuffer [SCK_MAXADDRESSBUFFERSIZE];
		unsigned long				ulReturnedBytes;
		LPSOCKET_ADDRESS_LIST		palAddressList;
		unsigned long				ulIPAddressesNumber;
		unsigned long				ulIPAddressIndex;
		struct sockaddr_in			*psckServerAddr;
		struct hostent				*phDNSName;
		IPAddress_t					iaIPAddress;


		if ((iTemporaryFd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			int				iErrno;


			iErrno			= WSAGetLastError ();

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CREATE_FAILED, 1, iErrno);

			return err;
		}

		if (::WSAIoctl (iTemporaryFd, SIO_GET_INTERFACE_LIST,
			pInputBuffer, SCK_MAXADDRESSBUFFERSIZE,
			pOutputBuffer, SCK_MAXADDRESSBUFFERSIZE,
			&ulReturnedBytes,
			NULL, NULL) != 0)
		{
			int				iErrno;


			iErrno			= WSAGetLastError ();

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_WSAIOCTL_FAILED, 1, iErrno);

			if (::closesocket (iTemporaryFd) != 0)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			return err;
		}

		palAddressList		= (LPSOCKET_ADDRESS_LIST) &(pOutputBuffer [0]);
	    
		ulIPAddressesNumber	= palAddressList -> iAddressCount;

		pvIPAddresses -> clear ();

		for (ulIPAddressIndex = 0; ulIPAddressIndex < ulIPAddressesNumber;
			ulIPAddressIndex++)
		{
			psckServerAddr		= (struct sockaddr_in *)
				(&(((palAddressList -> Address) [
				ulIPAddressIndex]). lpSockaddr));

			if (psckServerAddr -> sin_family != AF_INET)
				continue;

			strcpy (iaIPAddress. pIPAddress, ::inet_ntoa (
				psckServerAddr -> sin_addr));

			if (!strcmp (iaIPAddress. pIPAddress, "127.0.0.1") ||
				!strcmp (iaIPAddress. pIPAddress, "255.255.255.255") ||
				!strcmp (iaIPAddress. pIPAddress, "255.255.255.128"))
				continue;

			phDNSName		= ::gethostbyaddr (
				(char *) (&(psckServerAddr -> sin_addr)),
				sizeof (psckServerAddr -> sin_addr), AF_INET);

			if (phDNSName != (struct hostent *) NULL)
			{
				strcpy (iaIPAddress. pHostName, phDNSName -> h_name);
			}
			else
			{
				strcpy (iaIPAddress. pHostName, "");
			}

			pvIPAddresses -> insert (pvIPAddresses -> end (), iaIPAddress);
		}

		if (::closesocket (iTemporaryFd) != 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLOSE_FAILED);

			return err;
		}
	#else
		int							iTemporaryFd;
		struct ifconf				ifc;
		struct ifreq				*ifr;
		char						pBuffer [SCK_MAXADDRESSBUFFERSIZE];
		char						*pBufferPointer;
		struct sockaddr_in			*psckServerAddr;
		struct hostent				*phDNSName;
		IPAddress_t					iaIPAddress;


		if ((iTemporaryFd = socket (AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CREATE_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}

		::memset (&ifc, 0, sizeof (ifc));
		ifc. ifc_len			= SCK_MAXADDRESSBUFFERSIZE;
		ifc. ifc_buf			= pBuffer;

		if (::ioctl (iTemporaryFd, SIOCGIFCONF, (char*) (&ifc)) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_IOCTL_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			if (::close (iTemporaryFd) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			return err;
		}

		if (::close (iTemporaryFd) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLOSE_FAILED);

			return err;
		}

		pvIPAddresses -> clear ();

		for (pBufferPointer = pBuffer;
			pBufferPointer < (pBuffer + ifc. ifc_len); )
		{
			ifr			= (struct ifreq *) pBufferPointer;

			{
				pBufferPointer		+= sizeof (ifr -> ifr_name);

				switch (ifr -> ifr_addr. sa_family)
				{
					case AF_INET:
						pBufferPointer		+= sizeof (struct sockaddr_in);

						break;
					default:
						pBufferPointer		+= sizeof (struct sockaddr);
				}
			}
        
		    // Some platforms have lo as the first interface, so we have code to
		    // work around this problem below
		    //if (::strncmp(ifr->ifr_name, "lo", 2) == 0)
		    //  Assert(sNumIPAddrs > 0); // If the loopback interface
			//		is interface 0, we've got problems

			//Only count interfaces in the AF_INET family.
			if (ifr -> ifr_addr. sa_family != AF_INET)
				continue;

			psckServerAddr		= (struct sockaddr_in *) (&(ifr -> ifr_addr));

			strcpy (iaIPAddress. pIPAddress, ::inet_ntoa (
				psckServerAddr -> sin_addr));

			if (!strcmp (iaIPAddress. pIPAddress, "127.0.0.1"))
				continue;

			phDNSName		= ::gethostbyaddr (
				(char *) (&(psckServerAddr -> sin_addr)),
				sizeof (psckServerAddr -> sin_addr), AF_INET);

			if (phDNSName != (struct hostent *) NULL)
			{
				strcpy (iaIPAddress. pHostName, phDNSName -> h_name);
			}
			else
			{
				strcpy (iaIPAddress. pHostName, "");
			}

			pvIPAddresses -> insert (pvIPAddresses -> end (), iaIPAddress);
		}
	#endif


	return errNoError;
}


Error SocketImpl:: getMACAddress (
	const char *pNetworkIdentifier,
	unsigned char pucMACAddress [6])

{

	memset (pucMACAddress, 0, 6);

	#ifdef O	// old WIN32
		NCB				Ncb;

		struct ASTAT
		{
			ADAPTER_STATUS		adapt;
			NAME_BUFFER			NameBuff [30];
		} Adapter;

		int nAdapterNum				= 0;

		// Reset the LAN adapter so that we can begin querying it 

		memset (&Ncb, 0, sizeof (Ncb));
		Ncb. ncb_command		= NCBRESET;
		Ncb. ncb_lana_num		= (UCHAR) (atol (pNetworkIdentifier));

		// Netbios reset
		if (Netbios (&Ncb) != NRC_GOODRET)
		{
			long			lError;

			lError			= Ncb. ncb_retcode;

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NETBIOS_FAILED, 1, lError);
			err. setUserData (&lError, sizeof (long));

			return err;
		}

		// Prepare to get the adapter status block 
		memset (&Ncb, 0, sizeof (Ncb));
		Ncb.ncb_command			= NCBASTAT;
		Ncb.ncb_lana_num		= nAdapterNum;
		strcpy ((char *) Ncb. ncb_callname, "*");

		memset (&Adapter, 0, sizeof (Adapter));
		Ncb. ncb_buffer			= (unsigned char *) &Adapter;
		Ncb. ncb_length			= sizeof (Adapter);

		// Get the adapter's info and, if this works, return it in standard,
		// colon-delimited form.
		if (Netbios (&Ncb) != NRC_GOODRET)
		{
			long			lError;

			lError			= Ncb. ncb_retcode;

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NETBIOS_FAILED, 1, lError);
			err. setUserData (&lError, sizeof (long));

			return err;
		}

		pucMACAddress [0]		= Adapter. adapt. adapter_address [0];
		pucMACAddress [1]		= Adapter. adapt. adapter_address [1];
		pucMACAddress [2]		= Adapter. adapt. adapter_address [2];
		pucMACAddress [3]		= Adapter. adapt. adapter_address [3];
		pucMACAddress [4]		= Adapter. adapt. adapter_address [4];
		pucMACAddress [5]		= Adapter. adapt. adapter_address [5];
	#elif WIN32
		IP_ADAPTER_INFO			AdapterInfo [16]; // Allocate information for up to 16 NICs
		DWORD					dwBufLen = sizeof (AdapterInfo);

		DWORD					dwStatus = GetAdaptersInfo (
			AdapterInfo,	// [out] buffer to receive data
			&dwBufLen);		// [in] size of receive data buffer

		if (dwStatus != ERROR_SUCCESS)
		{
			long			lError;

			lError			= dwStatus;

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_GETADAPTERSINFO_FAILED, 1, lError);
			err. setUserData (&lError, sizeof (long));

			return err;
		}

		PIP_ADAPTER_INFO	pAdapterInfo = AdapterInfo;

		while (pAdapterInfo)
		{
			if (!(pAdapterInfo -> Address [0] == 0 &&
				pAdapterInfo -> Address[1] == 0 &&
				pAdapterInfo -> Address[2] == 0 &&
				pAdapterInfo -> Address[3] == 0 &&
				pAdapterInfo -> Address[4] == 0 &&
				pAdapterInfo -> Address[5] == 0))
			{
				pucMACAddress [0]		= pAdapterInfo -> Address [0];
				pucMACAddress [1]		= pAdapterInfo -> Address [1];
				pucMACAddress [2]		= pAdapterInfo -> Address [2];
				pucMACAddress [3]		= pAdapterInfo -> Address [3];
				pucMACAddress [4]		= pAdapterInfo -> Address [4];
				pucMACAddress [5]		= pAdapterInfo -> Address [5];

				// std:: cout << pAdapterInfo -> AdapterName << std:: endl;
				// std:: cout << pAdapterInfo -> Index << std:: endl;
				// std:: cout << pAdapterInfo -> Description << std:: endl;
				// std:: cout << pAdapterInfo -> Type << std:: endl;
				// std:: cout << std:: endl;

				break;	// mi fermo al primo NIC
			}

			pAdapterInfo = pAdapterInfo->Next;
		}
	/* implementation for Linux */
	#elif __linux__
		struct ifreq		irIfreq;
		struct ifreq		*pirIfreq;
		struct ifconf		icIfconf;
		char				pBuffer [1024];
		int					iTemporaryFd;
		int					iIndex;
		Boolean_t			bMACAddressFound;


		if ((iTemporaryFd = ::socket (AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			int				iErrno;


			iErrno			= errno;

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CREATE_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			return err;
		}

		icIfconf. ifc_len		= sizeof (pBuffer);
		icIfconf. ifc_buf		= pBuffer;

		if (::ioctl (iTemporaryFd, SIOCGIFCONF, &icIfconf) == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_IOCTL_FAILED, 1, iErrno);
			err. setUserData (&iErrno, sizeof (int));

			if (::close (iTemporaryFd) == -1)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLOSE_FAILED);
			}

			return err;
		}

		pirIfreq		= icIfconf. ifc_req;

		bMACAddressFound	= false;

    	for (iIndex = (icIfconf. ifc_len / sizeof (struct ifreq));
			--iIndex >= 0; pirIfreq++)
		{
			if (strcmp (pirIfreq -> ifr_name, pNetworkIdentifier))
				continue;

			strcpy (irIfreq. ifr_name, pirIfreq -> ifr_name);

			if (::ioctl (iTemporaryFd, SIOCGIFFLAGS, &irIfreq) == 0)
			{
				if (!(irIfreq. ifr_flags & IFF_LOOPBACK))
				{
					if (::ioctl (iTemporaryFd, SIOCGIFHWADDR, &irIfreq) ==
						0)
					{
						bMACAddressFound	= true;

                    	break;
					}
				}
			}
		}

		if (::close (iTemporaryFd) == -1)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLOSE_FAILED);

			return err;
		}

		if (!bMACAddressFound)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_MACADDRESSNOTFOUND);

			return err;
		}

		memcpy (pucMACAddress, irIfreq. ifr_hwaddr. sa_data, 6);


	// alternativa all'implementazione linux copiata ma non verificata
	/*
	#elif LINUX
		int fd;
		char buf[6]="";
		int size=0, nLine = 256;
		char line[256], ifname[64];
		const char     *scan_line_2_2 =
			"%lu %lu %lu %lu %*lu %*lu %*lu %*lu %lu %lu %lu %lu %*lu %lu";
		const char     *scan_line_2_0 =
			"%lu %lu %*lu %*lu %*lu %lu %lu %*lu %*lu %lu";
		const char     *scan_line_to_use;
		FILE *devin;
		struct ifreq	ifrq;
		fd = socket(AF_INET, SOCK_DGRAM, 0);

		// at least linux v1.3.53 says EMFILE without reason... 
		if (!(devin = fopen("/proc/net/dev", "r"))) {
			close(fd);
			printf("cannot open /proc/net/dev - continuing...\n");
			size = -1;
			return false; // exit (1);
		}

		// read the second line (a header) and determine the fields we
		// should read from.  This should be done in a better way by
		// actually looking for the field names we want.  But thats too
		// much work for today.  -- Wes 
		MyFgets(line, nLine, devin);
		MyFgets(line, nLine, devin);
		if (strstr(line, "compressed")) {
			scan_line_to_use = scan_line_2_2;
	//	    printf("using linux 2.2 kernel /proc/net/dev\n");
		} else {
			scan_line_to_use = scan_line_2_0;
	//	    printf("using linux 2.0 kernel /proc/net/dev\n"); 
		}

		pAddr_Info->Addr_Num = 0;
		while (MyFgets(line, nLine, devin)) 
		{
			char           *stats, *ifstart = line;

			while (*ifstart == ' ')
				ifstart++;

			if ((stats = strchr(ifstart, ':')) == NULL) 
			{
				printf("/proc/net/dev data format error, line ==|%s|", line);
				continue;
			}

			if ((scan_line_to_use == scan_line_2_2) && ((stats - line) < 6))
			{
				printf("/proc/net/dev data format error, line ==|%s|", line);
		 		continue;
			}

      		*stats++ = 0;
			strcpy(ifname, ifstart);

			// bae
			if(ifname[0] != 'l' && ifname[0] != 's')
			{
				memset(&ifrq,0,sizeof(ifreq));
				//printf("ifname = %s\n",ifname);
				strcpy(ifrq.ifr_name, ifname);
				if (ioctl(fd, SIOCGIFHWADDR, &ifrq) < 0)
	{
       				memset(buf, (0), 6);
			printf("SIOCGIFHWADDR error \n");
	}
    			else 
	{
					memcpy(buf, ifrq.ifr_hwaddr.sa_data, 6);
	}
	// printf("name:%s - %02X:%02X:%02X:%02X:%02X:%02X\n",
	//	ifname, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

				memcpy((char *)pAddr_Info->Addr_Info[pAddr_Info->Addr_Num], buf, sizeof(buf));

				// printf("1'=(%02x)\n", pAddr_Info->Addr_Info[pAddr_Info->Addr_Num][1]);
				// printf("2'=(%02x)\n", pAddr_Info->Addr_Info[pAddr_Info->Addr_Num][2]);
				// printf("3'=(%02x)\n", pAddr_Info->Addr_Info[pAddr_Info->Addr_Num][3]);
				// printf("4'=(%02x)\n", pAddr_Info->Addr_Info[pAddr_Info->Addr_Num][4]);
				// printf("5'=(%02x)\n", pAddr_Info->Addr_Info[pAddr_Info->Addr_Num][5]);
				
				pAddr_Info->Addr_Num++;
				size = 6;
			}
		}
	*/

	/* implementation for HP-UX */
	#elif HPUX

		#define LAN_DEV0 "/dev/lan0"

		int		fd;
		struct fis	iocnt_block;
		int		i;
		char	net_buf[sizeof(LAN_DEV0)+1];
		char	*p;

		(void)sprintf(net_buf, "%s", LAN_DEV0);
		p = net_buf + strlen(net_buf) - 1;

		// Get 802.3 address from card by opening
		// the driver and interrogating it.
		for (i = 0; i < 10; i++, (*p)++)
		{
			if ((fd = open (net_buf, O_RDONLY)) != -1)
			{
				iocnt_block.reqtype = LOCAL_ADDRESS;
				::ioctl (fd, NETSTAT, &iocnt_block);
				close (fd);

				if (iocnt_block.vtype == 6)
					break;
			}
		}

		if (fd == -1 || iocnt_block.vtype != 6)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_MACADDRESSNOTFOUND);

			return err;
		}

		bcopy( &iocnt_block.value.s[0], addr, 6);

		return 0;

	// implementation for AIX
	#elif AIX

		int size;
		struct kinfo_ndd *nddp;

		size = getkerninfo(KINFO_NDD, 0, 0, 0);
		if (size <= 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_MACADDRESSNOTFOUND);

			return err;
		}
		nddp = (struct kinfo_ndd *)malloc(size);

		if (!nddp)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			return err;
		}

		if (getkerninfo(KINFO_NDD, nddp, &size, 0) < 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_MACADDRESSNOTFOUND);

			free(nddp);

			return err;
		}

		bcopy(nddp->ndd_addr, addr, 6);
		free(nddp);

		return 0;
	/* copiato ma non verificato ancora
	#elif SOLARIS
 		int size = 7;
		char buf[7]="";
		int             i, idx = 1;
		int             ifsd;
		char	*ifbuf;
		int     ifbufsize = 0; 
		struct ifconf   ifconf;
		struct ifreq   *ifrp;

		ifbufsize = 10240;
		if ( (ifbuf = (char*) malloc(ifbufsize) ) == NULL )
		{
			size = -1;
			return false;
		}

		if ((ifsd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			size = -1;
			return false; // exit (1);
		}

		ifconf.ifc_buf = ifbuf;
		ifconf.ifc_len = ifbufsize;
		while (ioctl(ifsd, SIOCGIFCONF, &ifconf) == -1) 
		{
			ifbufsize += 10240;
			free(ifbuf);
			ifbuf = (char*)malloc(ifbufsize);
			if (!ifbuf) 
			{
				// size = -1;
				// goto Return ;
				close(ifsd);
				return false;
			}
			ifconf.ifc_buf = ifbuf;
			ifconf.ifc_len = ifbufsize;
		}
	    

		pAddr_Info->Addr_Num = 0;
		for (i = 0, ifrp = ifconf.ifc_req;
			(char *) ifrp < ((char *) ifconf.ifc_buf + ifconf.ifc_len)
			; i++, ifrp++, idx++) 
		{
			//printf("getEthernetID ...... ifr_name = %s\n",
			//           ifrp->ifr_name);
			// An attempt to determine the physical address of the interface.
			// There should be a more elegant solution using DLPI, but
			// "the margin is too small to put it here ..."
			if (ifrp->ifr_name[0] == 'l' )
		 		continue;
			// if (ioctl(ifsd, SIOCGIFADDR, ifrp) < 0) {
			// 	*size = -1;
			// 	goto Return;
			// }
			// *size = 6;
			// memcpy(buf, ifrp->ifr_enaddr,6);
		
			if (!getPhyAddr(ifrp->ifr_name,buf))
			{
				// size = -1;
				// goto Return;
				close(ifsd);
				return false;
			}

			// bae
			int ibreak = 0;
			for(int k=0; k< pAddr_Info->Addr_Num; k++)
			{
				if( (pAddr_Info->Addr_Info[k][1] == (unsigned char)buf[1]) &&
					(pAddr_Info->Addr_Info[k][2] == (unsigned char)buf[2]) &&
					(pAddr_Info->Addr_Info[k][3] == (unsigned char)buf[3]) &&
					(pAddr_Info->Addr_Info[k][4] == (unsigned char)buf[4]) &&
					(pAddr_Info->Addr_Info[k][5] == (unsigned char)buf[5]) )
				{
					ibreak = 1;
					break;
				}
				
			}
	 		if(ibreak == 1)		
				continue;
			
			memcpy((char *)pAddr_Info->Addr_Info[pAddr_Info->Addr_Num], buf, sizeof(buf));
			pAddr_Info->Addr_Num++;
			
			size = 6;
		}

	Return:
		close(ifsd);
	*/

	#endif


	return errNoError;
}

