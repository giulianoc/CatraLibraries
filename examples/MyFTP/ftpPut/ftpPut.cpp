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

/*
struct NetBuf {
	char *cput,*cget;
	int handle;			// socket
	int cavail,cleft;
	char *buf;			// buffer
	int dir;			// CONTROL (0), READ (1), WRITE (2)
	netbuf *ctrl;
	int cmode;			// FTPLIB_PASSIVE 1 default, FTPLIB_PORT 2
	struct timeval idletime;
	FtpCallback idlecb;
	void *idlearg;
	int xfered;
	int cbbytes;
	int xfered1;
	char response[256];
};
*/
#include "MyFTP.h"
#include "FileIO.h"
#include <iostream>
#include <stdlib.h>
#ifdef WIN32
	#include <time.h>
#endif

int main (int argc, char **argv)

{

	char						*pFilePathName;
	char						*pFTPIPAddress;
	char						*pFTPPort;
	char						*pFTPUser;
	char						*pFTPPwd;
	const char					*pRemoteContentName;
	time_t						tFtpPutStartTime;
	Tracer_t					tTracer;



	if (argc != 6)
	{
		std:: cout << "Usage: " << argv [0] << " <FilePathName> <FTPIPAddress> <FTPPort> <FTPUser> <FTPPwd>" << std:: endl;

		return 1;
	}

	pFilePathName			= argv [1];
	pFTPIPAddress			= argv [2];
	pFTPPort				= argv [3];
	pFTPUser				= argv [4];
	pFTPPwd					= argv [5];


	if (tTracer. init (
		"ftpDemo",						// pName
		-1,								// lCacheSizeOfTraceFile K-byte
		"./logs",						// pTraceDirectory
		"traceFile",					// pTraceFileName
		5000,							// lMaxTraceFileSize K-byte
		150000,							// lTraceFilePeriodInSecs
		false,							// bCompressedTraceFile
		false,							// _bClosedFileToBeCopied
		(const char *) NULL,			// _pClosedFilesRepository
		100,							// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		true,							// bTraceOnTTY
		0,								// lTraceLevel
		7,								// lSecondsBetweenTwoCheckTraceToManage
		3000,							// lMaxTracesNumber
		-1,								// lListenPort
		10000,							// lTracesNumberAllocatedOnOverflow
		1000) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{

		std:: cerr << "Tracer. init failed" << std:: endl;

		return 1;
	}

	if (tTracer. start () != errNoError)
	{
		std:: cerr << "Tracer. start failed" << std:: endl;

		tTracer. finish (true);

		return 1;
	}

	#ifdef WIN32
		if ((pRemoteContentName = strrchr (pFilePathName, '\\')) ==
			(char *) NULL)
		{
			std:: cerr << "Content name not found" << std:: endl;

			return 1;
		}
	#else
		if ((pRemoteContentName = strrchr (pFilePathName, '/')) ==
			(char *) NULL)
		{
			std:: cerr << "Content name not found" << std:: endl;

			return 1;
		}
	#endif
	pRemoteContentName++;

	MyFTP_t					mfMyFTP;
	Error_t					errMyFTP;


	if ((errMyFTP = mfMyFTP. init (pFTPIPAddress, atol (pFTPPort),
		pFTPUser, pFTPPwd, MyFTP:: FTP_MODE_IMAGE, &tTracer,
		(const char *) NULL, 10, 0, 10, 0)) != errNoError)
	{
		std:: cerr << (const char *) errMyFTP << std:: endl;

		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "putting... " << pFilePathName  << std:: endl;

	if ((errMyFTP = mfMyFTP. put (pFilePathName, pRemoteContentName)) !=
		errNoError)
	{
		std:: cerr << (const char *) errMyFTP << std:: endl;

		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if ((errMyFTP = mfMyFTP. finish ()) != errNoError)
		{
			std:: cerr << (const char *) errMyFTP << std:: endl;

			Error err = MyFTPErrors (__FILE__, __LINE__,
				FTP_MYFTP_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	/*
	std:: cout << "putting... " << pFilePathName  << std:: endl;

	if ((errMyFTP = mfMyFTP. put (pFilePathName, pRemoteContentName)) !=
		errNoError)
	{
		std:: cerr << (const char *) errMyFTP << std:: endl;

		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if ((errMyFTP = mfMyFTP. finish ()) != errNoError)
		{
			std:: cerr << (const char *) errMyFTP << std:: endl;

			Error err = MyFTPErrors (__FILE__, __LINE__,
				FTP_MYFTP_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}
	*/

	if ((errMyFTP = mfMyFTP. finish ()) != errNoError)
	{
		std:: cerr << (const char *) errMyFTP << std:: endl;

		Error err = MyFTPErrors (__FILE__, __LINE__,
			FTP_MYFTP_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (tTracer. cancel () != errNoError)
	{
		std:: cout << "Tracer. cancel failed" << std:: endl;

		return 1;
	}

	if (tTracer. finish (true) != errNoError)
	{
		std:: cout << "Tracer. finish failed" << std:: endl;

		return 1;
	}


	/*
	netbuf						*pnConnection;
	char						pFTPHostSite [512];


	if (!strcmp (pFTPPort, ""))
		sprintf (pFTPHostSite, "%s", pFTPIPAddress);
	else
		sprintf (pFTPHostSite, "%s:%s",
			pFTPIPAddress, pFTPPort);

	FtpInit ();

	std:: cerr << "Connecting..." << std:: endl;

	if (FtpConnect (pFTPHostSite, &pnConnection) != 1)
	{
		std:: cerr << "connect failed" << std:: endl;

		return 1;
	}

	std:: cerr << "log in..." << std:: endl;

	if (FtpLogin (pFTPUser, pFTPPwd,
		pnConnection) != 1)
	{
		std:: cerr << "FtpLogin failed" << std:: endl;

		FtpQuit (pnConnection);

		return 1;
	}

	if (FtpOptions (FTPLIB_CALLBACK, (long) NULL, pnConnection) != 1)
	{
		std:: cerr << "FtpOptions failed" << std:: endl;

		FtpQuit (pnConnection);

		return 1;
	}

	tFtpPutStartTime			= time (NULL);

	std:: cerr << "FtpPut..." << std:: endl;

	if (FtpPut (pFilePathName, pRemoteContentName + 1,
		FTPLIB_IMAGE, pnConnection) != 1)
	{
		std:: cerr << "FtpPut failed" << std:: endl;

		FtpQuit (pnConnection);

		return 1;
	}

	std:: cerr << "FtpPut elapsed time: " << time (NULL) - tFtpPutStartTime
		<< std:: endl;

	std:: cerr << "FtpQuit..." << std:: endl;

	FtpQuit (pnConnection);
	*/


	return 0;
}

