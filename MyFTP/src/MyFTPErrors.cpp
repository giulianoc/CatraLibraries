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


#include "MyFTPErrors.h"


ErrMsgBase:: ErrMsgsInfo MyFTPErrorsStr = {

	// MyFTP
	FTP_MYFTP_INIT_FAILED,
		"The init method of MyFTP class failed",
	FTP_MYFTP_FINISH_FAILED,
		"The finish method of MyFTP class failed",
	FTP_MYFTP_PROGRESS_FAILED,
		"The progress method of MyFTP class failed",
	FTP_MYFTP_GETDATAFTPCLIENTSOCKET_FAILED,
		"The getDataFTPClientSocket method of MyFTP class failed",
	FTP_MYFTP_READFTPRESPONSE_FAILED,
		"The readFTPResponse method of MyFTP class failed",
	FTP_MYFTP_WRITEFTPCOMMAND_FAILED,
		"The writeFTPCommand method of MyFTP class failed",
	FTP_MYFTP_UNEXPECTEDFTPRESPONSE,
		"Unexpected FTP response. Received FTP Response: %s, Expected FTP Response: %s",
	FTP_MYFTP_FTPCOMMANDTOOLONG,
		"%s FTP command too long",

	// common
	FTP_NEW_FAILED,
		"new failed",
	FTP_NOTIMPLEMENTEDYET,
		"feature not implemented yet",
	FTP_ACTIVATION_WRONG,
		"The activation of the method is wrong",
	FTP_OPERATION_NOTALLOWED,
		"Operation not allowed"

	// Insert here other errors...

} ;

