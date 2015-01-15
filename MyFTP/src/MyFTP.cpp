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


#if defined(WIN32)
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif
#include <stdio.h>
#include <assert.h>
#include "MyFTP.h"
#include "FileIO.h"
#include "FileReader.h"
#ifdef WIN32
	#include "WinThread.h"
#else
	#include "PosixThread.h"
#endif



MyFTP:: MyFTP (void)

{

}


MyFTP:: ~MyFTP (void)

{

}


MyFTP:: MyFTP (const MyFTP &)

{

	assert (1==0);

	// to do

}


Error MyFTP:: init (
	const char *pFTPIPAddress, unsigned long ulFTPPort,
	const char *pFTPUser, const char *pFTPPwd,
	FTPMode_t fFTPMode,
	Tracer_p ptSystemTracer,
	const char *pLocalIPAddress,
	unsigned long ulReceivingTimeoutInSeconds,
	unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
	unsigned long ulSendingTimeoutInSeconds,
	unsigned long ulSendingAdditionalTimeoutInMicroSeconds)

{

	Error_t				errSocketInit;
	Error_t				errRead;


	if (pFTPIPAddress == (const char *) NULL ||
		strlen (pFTPIPAddress) >= SCK_MAXHOSTNAMELENGTH ||
		pFTPUser == (const char *) NULL ||
		strlen (pFTPUser) >= FTP_MYFTP_MAXFTPUSERLENGTH ||
		pFTPPwd == (const char *) NULL ||
		strlen (pFTPPwd) >= FTP_MYFTP_MAXFTPPWDLENGTH ||
		ptSystemTracer == (Tracer_p) NULL)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_ACTIVATION_WRONG);

		return err;
	}

	strcpy (_pFTPIPAddress, pFTPIPAddress);
	_ulFTPPort					= ulFTPPort;
	strcpy (_pFTPUser, pFTPUser);
	strcpy (_pFTPPwd, pFTPPwd);
	if (pLocalIPAddress == (const char *) NULL)
		strcpy (_pLocalIPAddress, "");
	else
		strcpy (_pLocalIPAddress, pLocalIPAddress);
	_ulReceivingTimeoutInSeconds					=
		ulReceivingTimeoutInSeconds;
	_ulReceivingAdditionalTimeoutInMicroSeconds		=
		ulReceivingAdditionalTimeoutInMicroSeconds;
	_ulSendingTimeoutInSeconds						=
		ulSendingTimeoutInSeconds;
	_ulSendingAdditionalTimeoutInMicroSeconds		=
		ulSendingAdditionalTimeoutInMicroSeconds;
	_fFTPMode										= fFTPMode;
	_ptSystemTracer									= ptSystemTracer;

	_cmConnectionMode								=
		FTP_CONNECTIONMODE_PASSIVE;

	if ((errSocketInit = _csClientSocket. init (SocketImpl:: STREAM,
		_ulReceivingTimeoutInSeconds,
		_ulReceivingAdditionalTimeoutInMicroSeconds,
		_ulSendingTimeoutInSeconds,
		_ulSendingAdditionalTimeoutInMicroSeconds,
		_ulReceivingTimeoutInSeconds,
		true,
		!strcmp (_pLocalIPAddress, "") ? (char *) NULL : _pLocalIPAddress,
		_pFTPIPAddress, _ulFTPPort)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errSocketInit, __FILE__, __LINE__);

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_INIT_FAILED,
			2, _pFTPIPAddress, (long) _ulFTPPort);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errSocketInit;
	}

	if (_csClientSocket. getSocketImpl (&_pSocketImpl) != errNoError)
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

		return err;
	}

	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "2", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "2...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (strlen ("USER ") + strlen (pFTPUser) + 2 + 1 >=
		FTP_MYFTP_MAXFTPCOMMANDLENGTH)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FTPCOMMANDTOOLONG,
			1, "USER");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	sprintf (_pLastFTPCommand, "USER %s\r\n", pFTPUser);

	if (writeFTPCommand (_pLastFTPCommand) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// Response should be: 331 Please specify the password
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "3", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "3...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (strlen ("PASS ") + strlen (pFTPPwd) + 2 + 1 >=
		FTP_MYFTP_MAXFTPCOMMANDLENGTH)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FTPCOMMANDTOOLONG,
			1, "PASS");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	sprintf (_pLastFTPCommand, "PASS %s\r\n", pFTPPwd);

	if (writeFTPCommand (_pLastFTPCommand) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// Response should be: 230 Login successful
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "2", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "2...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	sprintf (_pLastFTPCommand, "TYPE %c\r\n",
		_fFTPMode == FTP_MODE_ASCII ? 'A' : 'I');

	if (writeFTPCommand (_pLastFTPCommand) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// Response should be: 200 Switching to Binary mode
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "2", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "2...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_csClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}


	return errNoError;
}


Error MyFTP:: finish (void)

{

	Error_t						errFinish;


	if ((errFinish = _csClientSocket. finish ()) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errFinish, __FILE__, __LINE__);

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error MyFTP:: put (const char *pLocalPathName, const char *pRemotePathName)

{

	ClientSocket_t			csDataFTPClientSocket;
	SocketImpl_p			pSocketImplForPut;
	Error_t					errRead;


	if (getDataFTPClientSocket (&csDataFTPClientSocket) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_GETDATAFTPCLIENTSOCKET_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (csDataFTPClientSocket. getSocketImpl (&pSocketImplForPut) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (strlen ("STOR ") + strlen (pRemotePathName) + 2 + 1 >=
		FTP_MYFTP_MAXFTPCOMMANDLENGTH)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FTPCOMMANDTOOLONG,
			1, "STOR");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	sprintf (_pLastFTPCommand, "STOR %s\r\n",
		pRemotePathName);

	if (writeFTPCommand (_pLastFTPCommand) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	// Response should be: 150 Ok to send data
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "1", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "1...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	{
		FileReader					frFileReader;
		unsigned long long			ullBytesRead;
		unsigned long long			ullBytesTransferred;
		Error_t						errRead;
		Error_t						errWrite;
		char						cLastBufferChar = ' ';	// just != '\r'


		if (frFileReader. init (pLocalPathName, 1024) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_INIT_FAILED,
				1, pLocalPathName);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (csDataFTPClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		ullBytesTransferred			= 0;

		while ((errRead = frFileReader. readBytes (_pucBuffer,
			FTP_MYFTP_MAXBUFFERLENGTH, false, &ullBytesRead)) == errNoError)
		{
			if (_fFTPMode == FTP_MODE_IMAGE)
			{
				// std:: cout << "IN: " << ullBytesRead << ", "
				// 	<< _ulSendingTimeoutInSeconds << ", "
				// 	<< _ulSendingAdditionalTimeoutInMicroSeconds << std:: endl;
				if ((errWrite = pSocketImplForPut -> write (_pucBuffer,
					(long) ullBytesRead, true, _ulSendingTimeoutInSeconds,
					_ulSendingAdditionalTimeoutInMicroSeconds)) != errNoError)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errWrite, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_WRITE_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (frFileReader. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEREADER_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (csDataFTPClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return errWrite;
				}
			}
			else	// if (fFTPMode == FTP_MODE_ASCII)
			{
				// if ASCHI, only in case of \n, change it in \r\n
				char				*pBeginPointer;
				char				*pEndPointer;


				pBeginPointer			= (char *) _pucBuffer;
				pEndPointer				= pBeginPointer;

				while ((pEndPointer = strchr (pEndPointer, '\n')) !=
					(char *) NULL)
				{
					if (pEndPointer == (char *) _pucBuffer &&
						cLastBufferChar != '\r')
					{
						if ((errWrite = pSocketImplForPut -> write (
							(char *) "\r\n", 2,
							true, _ulSendingTimeoutInSeconds,
							_ulSendingAdditionalTimeoutInMicroSeconds)) !=
							errNoError)
						{
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) errWrite, __FILE__, __LINE__);

							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITE_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);

							if (frFileReader. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEREADER_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							if (csDataFTPClientSocket. finish () !=
								errNoError)
							{
								Error err = SocketErrors (
									__FILE__, __LINE__,
									SCK_CLIENTSOCKET_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							return errWrite;
						}

						pEndPointer++;
						pBeginPointer		= pEndPointer;
					}
					else
					{
						if (*(pEndPointer - 1) != '\r')
						{
							if ((errWrite = pSocketImplForPut -> write (
								pBeginPointer, pEndPointer - pBeginPointer,
								true, _ulSendingTimeoutInSeconds,
								_ulSendingAdditionalTimeoutInMicroSeconds)) !=
								errNoError)
							{
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errWrite,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_WRITE_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								if (frFileReader. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_FILEREADER_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								if (csDataFTPClientSocket. finish () !=
									errNoError)
								{
									Error err = SocketErrors (
										__FILE__, __LINE__,
										SCK_CLIENTSOCKET_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								return errWrite;
							}

							if ((errWrite = pSocketImplForPut -> write (
								(char *) "\r\n", 2,
								true, _ulSendingTimeoutInSeconds,
								_ulSendingAdditionalTimeoutInMicroSeconds)) !=
								errNoError)
							{
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errWrite,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_WRITE_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								if (frFileReader. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_FILEREADER_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								if (csDataFTPClientSocket. finish () !=
									errNoError)
								{
									Error err = SocketErrors (
										__FILE__, __LINE__,
										SCK_CLIENTSOCKET_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								return errWrite;
							}

							pEndPointer++;
							pBeginPointer		= pEndPointer;
						}
						else
						{
							pEndPointer++;
						}
					}
				}

				if ((errWrite = pSocketImplForPut -> write (pBeginPointer,
					(((char *) _pucBuffer) + ullBytesRead) - pBeginPointer,
					true, _ulSendingTimeoutInSeconds,
					_ulSendingAdditionalTimeoutInMicroSeconds)) != errNoError)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errWrite, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_WRITE_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (frFileReader. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEREADER_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (csDataFTPClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return errWrite;
				}

				cLastBufferChar		= _pucBuffer [ullBytesRead - 1];
			}

			ullBytesTransferred		+= ullBytesRead;

			if (progress (ullBytesTransferred,
				(unsigned long long) frFileReader) != errNoError)
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_PROGRESS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (frFileReader. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEREADER_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (csDataFTPClientSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}

		if ((long) errRead != TOOLS_FILEREADER_REACHEDENDOFFILE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_READLINE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (frFileReader. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (csDataFTPClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return errRead;
		}

		if (frFileReader. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (csDataFTPClientSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (csDataFTPClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	// Response should be: 226 File receive OK
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "2", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "1...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error MyFTP:: put (Buffer_p pbContentToBeUpload, const char *pRemotePathName)

{

	ClientSocket_t			csDataFTPClientSocket;
	SocketImpl_p			pSocketImplForPut;
	Error_t					errRead;


	if (getDataFTPClientSocket (&csDataFTPClientSocket) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_GETDATAFTPCLIENTSOCKET_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (csDataFTPClientSocket. getSocketImpl (&pSocketImplForPut) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (strlen ("STOR ") + strlen (pRemotePathName) + 2 + 1 >=
		FTP_MYFTP_MAXFTPCOMMANDLENGTH)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FTPCOMMANDTOOLONG,
			1, "STOR");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	sprintf (_pLastFTPCommand, "STOR %s\r\n",
		pRemotePathName);

	if (writeFTPCommand (_pLastFTPCommand) != errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	// Response should be: 150 Ok to send data
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "1", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "1...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (csDataFTPClientSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return err;
		}

		return err;
	}

	{
		unsigned long long		ullBytesToWrite;
		unsigned long long		ullBytesTransferred;
		Error_t						errWrite;
		char						cLastBufferChar = ' ';	// just != '\r'


		ullBytesTransferred			= 0;

		while (strlen (((const char *) (*pbContentToBeUpload)) +
			ullBytesTransferred) > 0)
		{
			if (_fFTPMode == FTP_MODE_IMAGE)
			{
				if (FTP_MYFTP_MAXBUFFERLENGTH <=
					strlen (((const char *) (*pbContentToBeUpload)) +
					ullBytesTransferred))
					ullBytesToWrite		= FTP_MYFTP_MAXBUFFERLENGTH;
				else
					ullBytesToWrite		=
						strlen (((const char *) (*pbContentToBeUpload)) +
							ullBytesTransferred);

				if ((errWrite = pSocketImplForPut -> write (
					(void *) (((const char *) (*pbContentToBeUpload)) +
						ullBytesTransferred),
					(long) ullBytesToWrite, true, _ulSendingTimeoutInSeconds,
					_ulSendingAdditionalTimeoutInMicroSeconds)) != errNoError)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errWrite, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_WRITE_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (csDataFTPClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return errWrite;
				}
			}
			else	// if (fFTPMode == FTP_MODE_ASCII)
			{
				// if ASCHI, only in case of \n, change it in \r\n
				char				*pBeginPointer;
				char				*pEndPointer;


				if (FTP_MYFTP_MAXBUFFERLENGTH <=
					strlen (((const char *) (*pbContentToBeUpload)) +
					ullBytesTransferred))
					ullBytesToWrite		= FTP_MYFTP_MAXBUFFERLENGTH;
				else
					ullBytesToWrite		=
						strlen (((const char *) (*pbContentToBeUpload)) +
							ullBytesTransferred);

				memcpy (_pucBuffer, ((const char *) (*pbContentToBeUpload)) +
					ullBytesTransferred, (size_t) ullBytesToWrite);

				pBeginPointer			= (char *) _pucBuffer;
				pEndPointer				= pBeginPointer;

				while ((pEndPointer = strchr (pEndPointer, '\n')) !=
					(char *) NULL)
				{
					if (pEndPointer == (char *) _pucBuffer &&
						cLastBufferChar != '\r')
					{
						if ((errWrite = pSocketImplForPut -> write (
							(char *) "\r\n", 2,
							true, _ulSendingTimeoutInSeconds,
							_ulSendingAdditionalTimeoutInMicroSeconds)) !=
							errNoError)
						{
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) errWrite, __FILE__, __LINE__);

							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_WRITE_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);

							if (csDataFTPClientSocket. finish () !=
								errNoError)
							{
								Error err = SocketErrors (
									__FILE__, __LINE__,
									SCK_CLIENTSOCKET_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							return errWrite;
						}

						pEndPointer++;
						pBeginPointer		= pEndPointer;
					}
					else
					{
						if (*(pEndPointer - 1) != '\r')
						{
							if ((errWrite = pSocketImplForPut -> write (
								pBeginPointer, pEndPointer - pBeginPointer,
								true, _ulSendingTimeoutInSeconds,
								_ulSendingAdditionalTimeoutInMicroSeconds)) !=
								errNoError)
							{
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errWrite,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_WRITE_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								if (csDataFTPClientSocket. finish () !=
									errNoError)
								{
									Error err = SocketErrors (
										__FILE__, __LINE__,
										SCK_CLIENTSOCKET_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								return errWrite;
							}

							if ((errWrite = pSocketImplForPut -> write (
								(char *) "\r\n", 2,
								true, _ulSendingTimeoutInSeconds,
								_ulSendingAdditionalTimeoutInMicroSeconds)) !=
								errNoError)
							{
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errWrite,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_WRITE_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								if (csDataFTPClientSocket. finish () !=
									errNoError)
								{
									Error err = SocketErrors (
										__FILE__, __LINE__,
										SCK_CLIENTSOCKET_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								return errWrite;
							}

							pEndPointer++;
							pBeginPointer		= pEndPointer;
						}
						else
						{
							pEndPointer++;
						}
					}
				}

				if ((errWrite = pSocketImplForPut -> write (pBeginPointer,
					(((char *) _pucBuffer) + ullBytesToWrite) - pBeginPointer,
					true, _ulSendingTimeoutInSeconds,
					_ulSendingAdditionalTimeoutInMicroSeconds)) != errNoError)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errWrite, __FILE__, __LINE__);

					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_WRITE_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (csDataFTPClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return errWrite;
				}

				cLastBufferChar		= _pucBuffer [ullBytesToWrite - 1];
			}

			ullBytesTransferred		+= ullBytesToWrite;

			if (progress (ullBytesTransferred, ullBytesToWrite) != errNoError)
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_PROGRESS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (csDataFTPClientSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}
	}

	if (csDataFTPClientSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_CLIENTSOCKET_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	// Response should be: 226 File receive OK
	if ((errRead = readFTPResponse (
		_pLastFTPResponse, FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
		errNoError)
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_READFTPRESPONSE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errRead;
	}

	if (strncmp (_pLastFTPResponse, "2", 1))
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
			2, _pLastFTPResponse, "1...");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error MyFTP:: getDataFTPClientSocket (
	ClientSocket_p pcsDataFTPClientSocket)

{

	Error_t						errSocketInit;
	Error_t						errRead;
	union {
		struct sockaddr			sa;
		struct sockaddr_in		in;
	} sin;


	if (_cmConnectionMode == FTP_CONNECTIONMODE_PASSIVE)
	{
		{
			char						*pConnectionInfo;
			unsigned int				puiSaData [6];


			memset (&sin, 0, sizeof (sin));
			sin. in. sin_family		= AF_INET;

			sprintf (_pLastFTPCommand, "PASV\r\n");

			if (writeFTPCommand (_pLastFTPCommand) != errNoError)
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_WRITEFTPCOMMAND_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			// Response should be:
			// 		227 Entering Passive Mode (213,92,10,167,183,52)
			if ((errRead = readFTPResponse (_pLastFTPResponse,
				FTP_MYFTP_MAXFTPRESPONSELENGTH)) !=
				errNoError)
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_READFTPRESPONSE_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return errRead;
			}

			if (strncmp (_pLastFTPResponse, "2", 1))
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
					2, _pLastFTPResponse, "2...");
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			if ((pConnectionInfo = strchr (_pLastFTPResponse, '(')) ==
				(char *) NULL)
			{
				Error err = MyFTPErrors (__FILE__, __LINE__,
					FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
					2, _pLastFTPResponse, "(...");
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			pConnectionInfo++;

			sscanf (pConnectionInfo, "%u,%u,%u,%u,%u,%u",
				&puiSaData [2], &puiSaData [3], &puiSaData [4],
				&puiSaData [5], &puiSaData [0], &puiSaData [1]);
			sin. sa. sa_data [2]		= puiSaData [2];
			sin. sa. sa_data [3]		= puiSaData [3];
			sin. sa. sa_data [4]		= puiSaData [4];
			sin. sa. sa_data [5]		= puiSaData [5];
			sin. sa. sa_data [0]		= puiSaData [0];
			sin. sa. sa_data [1]		= puiSaData [1];
		}

		if ((errSocketInit = pcsDataFTPClientSocket -> init (
			SocketImpl:: STREAM,
			_ulReceivingTimeoutInSeconds,
			_ulReceivingAdditionalTimeoutInMicroSeconds,
			_ulSendingTimeoutInSeconds,
			_ulSendingAdditionalTimeoutInMicroSeconds,
			_ulReceivingTimeoutInSeconds,
			true,
			!strcmp (_pLocalIPAddress, "") ? (char *) NULL :
			_pLocalIPAddress, &(sin. in))) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errSocketInit, __FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_CLIENTSOCKET_INIT_FAILED,
				2, ::inet_ntoa ((sin. in). sin_addr),
				(long) ntohs ((sin. in). sin_port));
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return errSocketInit;
		}

		// set linger a false, 0 (NB: prima del connect del socket)

	}
	else
	{
		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_NOTIMPLEMENTEDYET);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
		/*
	 	* In this case the client (this is) will listen on a port
	 	* and the server should connect to it.
	 	* It should not be much used unless the FTP server is an old one

		if (getsockname(nControl->handle, &sin.sa, &l) < 0)
		{
			perror("getsockname");
			return 0;
		}

		sData = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
		if (sData == -1)
		{
			perror("socket");
			return -1;
		}
		if (setsockopt(sData,SOL_SOCKET,SO_REUSEADDR,
			SETSOCKOPT_OPTVAL_TYPE &on,sizeof(on)) == -1)
		{
			perror("setsockopt");
			net_close(sData);
			return -1;
		}
		if (setsockopt(sData,SOL_SOCKET,SO_LINGER,
			SETSOCKOPT_OPTVAL_TYPE &lng,sizeof(lng)) == -1)
		{
			perror("setsockopt");
			net_close(sData);
			return -1;
		}

		sin.in.sin_port = 0;
		if (bind(sData, &sin.sa, sizeof(sin)) == -1)
		{
			perror("bind");
			net_close(sData);
			return 0;
		}
		if (listen(sData, 1) < 0)
		{
			perror("listen");
			net_close(sData);
			return 0;
		}
		if (getsockname(sData, &sin.sa, &l) < 0)
			return 0;
		sprintf(buf, "PORT %d,%d,%d,%d,%d,%d",
			(unsigned char) sin.sa.sa_data[2],
			(unsigned char) sin.sa.sa_data[3],
			(unsigned char) sin.sa.sa_data[4],
			(unsigned char) sin.sa.sa_data[5],
			(unsigned char) sin.sa.sa_data[0],
			(unsigned char) sin.sa.sa_data[1]);
		if (!FtpSendCmd(buf,'2',nControl))
		{
			net_close(sData);
			return 0;
		}
	*/
	}


	return errNoError;
}


Error MyFTP:: progress (unsigned long long ullBytesTransferred,
	unsigned long long ullFileSizeInBytes)

{

	return errNoError;
}


Error MyFTP:: readFTPResponse (
	char *pFTPResponse, unsigned long ulBufferLength)

{
	unsigned long		ulCharsRead;
	Error_t				errRead;


	if ((errRead = _pSocketImpl -> readLine (pFTPResponse,
		FTP_MYFTP_MAXFTPRESPONSELENGTH,
		&ulCharsRead, _ulReceivingTimeoutInSeconds,
		_ulReceivingAdditionalTimeoutInMicroSeconds)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errRead, __FILE__, __LINE__);

		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_READLINE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errRead;
	}

	// std:: cout << "FTPResponse: " << _pLastFTPResponse << std:: endl;

	/*
	 * nella lib ftplib c'era anche:
    if (nControl->response[3] == '-')
    {
    strncpy(match,nControl->response,3);
    match[3] = ' ';
    match[4] = '\0';
    do
    {
        if (readline(nControl->response,256,nControl) == -1)
        {
        perror("Control socket read failed");
        return 0;
        }
        if (ftplib_debug > 1)
        fprintf(stderr,"%s",nControl->response);
    }
    while (strncmp(nControl->response,match,4));
    }

	In pratica, se il quarto carattere della risposta era '-'
	continuava a leggere finchè non trovava di nuovo
	gli stessi primi 3 charatteri
	 */


	return errNoError;
}


Error MyFTP:: writeFTPCommand (char *pFTPCommand)

{

	// std:: cout << "writeFTPCommand: " << pFTPCommand << std:: endl;

	if (_pSocketImpl -> writeString (pFTPCommand,
		true, _ulSendingTimeoutInSeconds,
		_ulSendingAdditionalTimeoutInMicroSeconds) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_WRITESTRING_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}

