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


#include "SocketErrors.h"


ErrMsgBase:: ErrMsgsInfo SocketErrorsStr = {

	// SocketImpl
	{ SCK_SOCKETIMPL_CREATE_FAILED,
		"The create method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_CONNECT_FAILED,
		"The connect method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_BIND_FAILED,
		"The bind method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_LISTEN_FAILED,
		"The listen method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETBLOCKING_FAILED,
		"The setBlocking method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETBLOCKING_FAILED,
		"The getBlocking method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_ISREADYFORREADING_FAILED,
		"The isReadyForReading method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_ISREADYFORWRITING_FAILED,
		"The isReadyForWriting method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_ISTHEREEXCEPTION_FAILED,
		"The isThereException method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_ACCEPTCONNECTION_FAILED,
		"The acceptConnection method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_CLOSE_FAILED,
		"The close method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_VACUUM_FAILED,
		"The vacuum method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_VACUUMBYTELNET_FAILED,
		"The vacuumByTelnet method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_WAITFORBUFFER_FAILED,
		"The waitForBuffer method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_READ_FAILED,
		"The read method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_READLINE_FAILED,
		"The readLine method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_READLINEBYTELNET_FAILED,
		"The readLineByTelnet method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_READBYTELNET_FAILED,
		"The readByTelnet method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_TELNETDECODER_FAILED,
		"The telnetDecoder method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_WRITESTRING_FAILED,
		"The writeString method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_WRITE_FAILED,
		"The write method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETREMOTEADDRESS_FAILED,
		"The getRemoteAddress method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETREMOTEPORT_FAILED,
		"The getRemotePort method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETLOCALADDRESS_FAILED,
		"The getLocalAddress method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETLOCALPORT_FAILED,
		"The getLocalPort method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETLOCALPORT_FAILED,
		"The getLocalPort method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETIPADDRESSESLIST_FAILED,
		"The getIPAddressesList method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETRECEIVINGTIMEOUT_FAILED,
		"The setReceivingTimeout method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETSENDINGTIMEOUT_FAILED,
		"The setSendingTimeout method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETMAXSENDBUFFER_FAILED,
		"The setMaxSendBuffer method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED,
		"The setMaxReceiveBuffer method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETKEEPALIVE_FAILED,
		"The setKeepAlive method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_SETNODELAY_FAILED,
		"The setNoDelay method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_GETMACADDRESS_FAILED,
		"The getMACAddress method of SocketImpl class failed" },
	{ SCK_SOCKETIMPL_TIMEOUTEXPIRED,
		"Timeout expired" },
	{ SCK_SOCKETIMPL_FD_SETSIZETOBEINCREMENTED,
	"The FD_SETSIZE C++ compiler variable must be incremented. Max file descriptor: %ld, FD_SETSIZE: %ld" },
	{ SCK_SOCKETIMPL_MACADDRESSNOTFOUND,
		"MAC address was not found" },

	// Socket
	{ SCK_SOCKET_INIT_FAILED,
		"The init method of Socket class failed" },
	{ SCK_SOCKET_FINISH_FAILED,
		"The finish method of Socket class failed" },
	{ SCK_SOCKET_GETSOCKETIMPL_FAILED,
		"The getSocketImpl method of Socket class failed" },

	// ClientSocket
	{ SCK_CLIENTSOCKET_INIT_FAILED,
		"The init method of ClientSocket class failed. Remote IP Address: %s, Port: %ld" },
	{ SCK_CLIENTSOCKET_FINISH_FAILED,
		"The finish method of ClientSocket class failed" },
	{ SCK_CLIENTSOCKET_REINIT_FAILED,
		"The reinit method of ClientSocket class failed. Remote IP Address: %s, Port: %ld" },
	{ SCK_CLIENTSOCKET_SETIDENTIFIER_FAILED,
		"The setIdentifier method of ClientSocket class failed" },
	{ SCK_CLIENTSOCKET_GETIDENTIFIER_FAILED,
		"The getIdentifier method of ClientSocket class failed" },

	// TelnetClient
	{ SCK_TELNETCLIENT_INIT_FAILED,
		"The init method of TelnetClient class failed" },
	{ SCK_TELNETCLIENT_FINISH_FAILED,
		"The finish method of TelnetClient class failed" },

	// ServerSocket
	{ SCK_SERVERSOCKET_INIT_FAILED,
		"The init method of ServerSocket class failed" },
	{ SCK_SERVERSOCKET_FINISH_FAILED,
		"The finish method of ServerSocket class failed" },
	{ SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED,
		"The acceptConnection method of ServerSocket class failed" },

	// SocketsPool
	{ SCK_SOCKETSPOOL_INIT_FAILED,
		"The init method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_FINISH_FAILED,
		"The finish method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_ADDSOCKET_FAILED,
		"The addSocket method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_DELETESOCKET_FAILED,
		"The deleteSocket method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED,
		"The updateSocketStatus method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_CHECKSOCKETSSTATUS_FAILED,
		"The checkSocketsStatus method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_FINDSERVERSOCKET_FAILED,
		"The findServerSocket method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_SETISSHUTDOWN_FAILED,
		"The setIsShutdown method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_GETISSHUTDOWN_FAILED,
		"The getIsShutdown method of SocketsPool class failed" },
	{ SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED,
		"Socket status not changed" },
	{ SCK_SOCKETSPOOL_SOCKETALREADYADDED,
		"Socket already added. File Descriptor: %ld" },
	{ SCK_SOCKETSPOOL_SOCKETNOTFOUND,
		"Socket not found. File descriptor: %ld" },
	{ SCK_SOCKETSPOOL_KEYDUPLICATED,
		"The key (%ld) has already been inserted" },
	{ SCK_SOCKETSPOOL_REACHEDMAXSOCKETSNUMBER,
		"Reached the MaxSocketsNumber (%lu)" },
	{ SCK_SOCKETSPOOL_FREESOCKETSVECTORNOTCONSISTENT,
		"Free sockets vector not consistent" },
	{ SCK_SOCKETSPOOL_WRONGSOCKETTYPE,
		"Wrong socket type (%ld)" },
	{ SCK_SOCKETSPOOL_POOLEMPTY,
		"Sockets pool empty" },

	// ClientSocketsPool
	{ SCK_CLIENTSOCKETSPOOL_INIT_FAILED,
		"The init method of ClientSocketsPool class failed" },
	{ SCK_CLIENTSOCKETSPOOL_FINISH_FAILED,
		"The finish method of ClientSocketsPool class failed" },
	{ SCK_CLIENTSOCKETSPOOL_GETFREECLIENTSOCKET_FAILED,
		"The getFreeClientSocket method of ClientSocketsPool class failed" },
	{ SCK_CLIENTSOCKETSPOOL_RELEASECLIENTSOCKET_FAILED,
		"The releaseClientSocket method of ClientSocketsPool class failed" },
	{ SCK_CLIENTSOCKETSPOOL_CLIENTSOCKETTYPEALREADYPRESENT,
		"Client Socket Type (%s) already present" },
	{ SCK_CLIENTSOCKETSPOOL_NOCLIENTSOCKETAVAILABLE,
		"No Client Socket available for the '%s' Client Socket Type" },
	{ SCK_CLIENTSOCKETSPOOL_CLIENTSOCKETTYPENOTFOUND,
		"Client Socket Type (%s) was not found" },

	// common
	{ SCK_CREATE_FAILED,
		"The socket function failed (errno: %ld)" },
	{ SCK_CONNECT_FAILED,
		"The connect function failed (errno: %ld, Remote IP address: %s, Remote Port: %ld)" },
	{ SCK_CONNECT_EINPROGRESSFAILED,
		"The connect function failed because EINPROGRESS (errno: %ld, Remote IP address: %s, Remote Port: %ld)" },
	{ SCK_REMOTEADDRESS_TOOLONG,
		"The remote address is too long" },
	{ SCK_SETSOCKOPT_FAILED,
		"The setsockopt function failed (errno: %ld)" },
	{ SCK_BIND_FAILED,
		"The bind function failed (errno: %ld, IP address: %s, port: %ld)" },
	{ SCK_LISTEN_FAILED,
		"The listen function failed" },
	{ SCK_ACCEPT_FAILED,
		"The accept function failed (errno: %ld)" },
	{ SCK_CLOSE_FAILED,
		"The close function failed" },
	{ SCK_WRITE_FAILED,
		"The write function failed (errno: %ld)" },
	{ SCK_SEND_FAILED,
		"The send function failed (errno: %ld)" },
	{ SCK_READ_FAILED,
		"The read function failed (errno: %ld)" },
	{ SCK_READ_EOFREACHED,
		"Reached the end of file. Remote IP Address: %s, Remote Port: %ld, Bytes read up to now: %ld" },
	{ SCK_SELECT_FAILED,
		"The select function failed (errno: %ld)" },
	{ SCK_POLL_FAILED,
		"The poll function failed (errno: %ld)" },
	{ SCK_FCNTL_FAILED,
		"The fcntl function failed (errno: %ld)" },
	{ SCK_IOCTLSOCKET_FAILED,
		"The ioctlsocket function failed" },
	{ SCK_GETTIMEOFDAY_FAILED,
		"The gettimeofday function failed" },
	{ SCK_FCNTL_RETURNUNKNOWN,
		"The return of the fcntl function is unknown" },
	{ SCK_BUFFER_SMALL,
		"The buffer is small" },
	{ SCK_NOTHINGTOREAD,
		"Nothing to read. File descriptor: %ld, timeout used in seconds: %lu, in micro-seconds: %lu" },
	{ SCK_NOTREADYFORWRITING,
		"Not ready for writing. File descriptor: %ld, timeout used in seconds: %lu, in micro-seconds: %lu" },
	{ SCK_WSASTARTUP_FAILED,		// windows
		"The WSAStartup function failed" },
	{ SCK_WSACLEANUP_FAILED,		// windows
		"The WSACleanup function failed" },
	{ SCK_WSAIOCTL_FAILED,		// windows
		"The WSAIoctl function failed (errno: %ld)" },
	{ SCK_NETBIOS_FAILED,		// windows
		"The Netbios function failed (errno: %ld)" },
	{ SCK_GETADAPTERSINFO_FAILED,	// windows
		"The GetAdaptersInfo function failed (errno: %ld)" },
	{ SCK_IOCTL_FAILED,			// linux
		"The ioctl function failed (errno: %ld)" },
	{ SCK_ACTIVATION_WRONG,
		"Activation wrong" },
	{ SCK_NEW_FAILED,
		"new failed" },
	{ SCK_OPERATION_NOTALLOWED,
		"Operation not allowed (Current object status: %ld)" }

	// Insert here other errors...

} ;

