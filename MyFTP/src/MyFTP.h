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


#ifndef MyFTP_h
	#define MyFTP_h

	#include "MyFTPErrors.h"
	#include "ClientSocket.h"
	#include "Tracer.h"

	#define FTP_MYFTP_MAXFTPUSERLENGTH		(64 + 1)
	#define FTP_MYFTP_MAXFTPPWDLENGTH		(64 + 1)
	#define FTP_MYFTP_MAXFTPCOMMANDLENGTH	(256 + 1)
	#define FTP_MYFTP_MAXFTPRESPONSELENGTH	(256 + 1)
	#define FTP_MYFTP_MAXBUFFERLENGTH		(8192)

	typedef class MyFTP

	{
		public:
			typedef enum FTPMode {
				FTP_MODE_ASCII,
				FTP_MODE_IMAGE
			} FTPMode_t, *FTPMode_p;

			typedef enum FTPConnectionMode {
				FTP_CONNECTIONMODE_PASSIVE,
				FTP_CONNECTIONMODE_PORT
			} FTPConnectionMode_t, *FTPConnectionMode_p;

		private:
			char					_pFTPIPAddress [
				SCK_MAXHOSTNAMELENGTH];
			unsigned long			_ulFTPPort;
			char					_pFTPUser [
				FTP_MYFTP_MAXFTPUSERLENGTH];
			char					_pFTPPwd [
				FTP_MYFTP_MAXFTPPWDLENGTH];
			char					_pLocalIPAddress [
				SCK_MAXHOSTNAMELENGTH];
			unsigned long			_ulReceivingTimeoutInSeconds;
			unsigned long			_ulReceivingAdditionalTimeoutInMicroSeconds;
			unsigned long			_ulSendingTimeoutInSeconds;
			unsigned long			_ulSendingAdditionalTimeoutInMicroSeconds;
			FTPMode_t				_fFTPMode;

			FTPConnectionMode_t		_cmConnectionMode;

			ClientSocket_t			_csClientSocket;
			SocketImpl_p			_pSocketImpl;

			char					_pLastFTPCommand [
				FTP_MYFTP_MAXFTPCOMMANDLENGTH];
			char					_pLastFTPResponse [
				FTP_MYFTP_MAXFTPRESPONSELENGTH];
			unsigned char			_pucBuffer [FTP_MYFTP_MAXBUFFERLENGTH];

			Error readFTPResponse (
				char *pFTPResponse, unsigned long ulBufferLength);

			Error writeFTPCommand (char *pFTPCommand);

			Error getDataFTPClientSocket (
				ClientSocket_p pcsDataFTPClientSocket);

		protected:
			Tracer_p				_ptSystemTracer;


			MyFTP (const MyFTP &);

			virtual Error progress (unsigned long long ullBytesTransferred,
				unsigned long long ullFileSizeInBytes);

		public:
			/**
				Costruttore
			*/
			MyFTP (void);

			/**
				Distruttore
			*/
			~MyFTP (void);

			Error init (const char *pFTPIPAddress, unsigned long ulFTPPort,
				const char *pFTPUser, const char *pFTPPwd,
				FTPMode_t fFTPMode, Tracer_p ptSystemTracer,
				const char *pLocalIPAddress = (const char *) NULL,
				unsigned long ulReceivingTimeoutInSeconds = 60,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds = 0,
				unsigned long ulSendingTimeoutInSeconds = 60,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds = 0);

			Error finish (void);

			Error put (const char *pLocalPathName,
				const char *pRemotePathName);

			Error put (Buffer_p pbContentToBeUpload,
				const char *pRemotePathName);

	} MyFTP_t, *MyFTP_p;

#endif

