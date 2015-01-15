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


#ifdef WIN32
	#include <winsock2.h>
#else
	#include <unistd.h>
//	#include <stream.h>
#endif
#include "Tracer.h"
#include "FileIO.h"
#include "DateTime.h"
#include "ServerSocket.h"
#include "ClientSocket.h"
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _USEGZIPLIB
	#include <zlib.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#ifdef __QTCOMPILER__
    #include <QDebug>
#endif

#ifdef WIN32
	Tracer:: Tracer (void): WinThread ()
#else
	Tracer:: Tracer (void): PosixThread ()
#endif

{

	_tsTracerStatus			= TRACER_BUILDED;
}


Tracer:: ~Tracer (void)

{

	if (_tsTracerStatus == TRACER_INITIALIZED)
	{
		if (getThreadState() == THREADLIB_STARTED ||
			getThreadState() == THREADLIB_STARTED_AND_JOINED)
		{
			if (cancel () != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_CANCEL_FAILED);
			}
		}

		if (finish (true) != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FINISH_FAILED);
		}
	}
}


Error Tracer:: init (
	const char *pName,
	long lCacheSizeOfTraceFile,
	const char *pTraceDirectory,
	const char *pTraceFileName,
	#ifdef WIN32
		__int64 ullMaxTraceFileSize,
	#else
		unsigned long long ullMaxTraceFileSize,
	#endif
	long lTraceFilePeriodInSecs,
	Boolean_t bCompressedTraceFile,
	Boolean_t bClosedFileToBeCopied,
	const char *pClosedFilesRepository,
	long lTraceFilesNumberToMaintain,
	Boolean_t bTraceOnFile,
	Boolean_t bTraceOnTTY,
	long lTraceLevel,
	long lSecondsBetweenTwoCheckTraceToManage,
	long lMaxTracesNumber,
	long lListenPort,
	long lTracesNumberAllocatedOnOverflow,
	long lSizeOfEachBlockToGzip)

{

	char	pDefaultTraceLevelLabel [TRACER_MAXDEFAULTTRACELEVELSNUMBER][
		128 + 1] = {
		"DBG1",
		"DBG2",
		"DBG3",
		"DBG4",
		"DBG5",
		"DBG6",
		"INFO",
		"MESG",
		"WRNG",
		"ERRR",
		"FTAL"
	} ;
	long		lTraceLevelIndex;


	if (lTraceFilesNumberToMaintain > 9999 ||
		(bClosedFileToBeCopied &&
		pClosedFilesRepository == (const char *) NULL))
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_ACTIVATION_WRONG);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	if (_tsTracerStatus != TRACER_BUILDED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_lTracesNumber								= 0;
	_lTracesNumberAllocatedOnOverflow			=
		lTracesNumberAllocatedOnOverflow;
	{
		long				lTraceIndex;


		_lAllocatedTracesNumber					=
			_lTracesNumberAllocatedOnOverflow;

		if ((_ptiTraces = new TraceInfo_t [_lAllocatedTracesNumber]) ==
			(TraceInfo_p) NULL)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_NEW_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		for (lTraceIndex = 0; lTraceIndex < _lAllocatedTracesNumber;
			lTraceIndex++)
		{
			if (((_ptiTraces [lTraceIndex]). _bTraceMessage).
				init ("", 0, 10) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if (((_ptiTraces [lTraceIndex]). _bFileName).
				init ("", 0, 10) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
	}

	_ptiTracesToManage							= (TraceInfo_p) NULL;
	_lTracesNumberToManage						= 0;
	_lAllocatedTracesNumberToManage				= 0;
	_lTracesNumberToManageAllocatedOnOverflow	=
		lTracesNumberAllocatedOnOverflow;
	{
		long				lTraceIndex;


		_lAllocatedTracesNumberToManage					=
			_lTracesNumberToManageAllocatedOnOverflow;

		if ((_ptiTracesToManage = new TraceInfo_t [
			_lAllocatedTracesNumberToManage]) == (TraceInfo_p) NULL)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_NEW_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		for (lTraceIndex = 0; lTraceIndex < _lAllocatedTracesNumberToManage;
			lTraceIndex++)
		{
			if (((_ptiTracesToManage [lTraceIndex]). _bTraceMessage).
				init ("", 0, 10) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				delete [] _ptiTracesToManage;
				_ptiTracesToManage					= (TraceInfo_p) NULL;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if (((_ptiTracesToManage [lTraceIndex]). _bFileName).
				init ("", 0, 10) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				delete [] _ptiTracesToManage;
				_ptiTracesToManage					= (TraceInfo_p) NULL;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
	}

	_lSecondsBetweenTwoCheckTraceToManage	=
		lSecondsBetweenTwoCheckTraceToManage;

	_lMaxTracesNumber						= lMaxTracesNumber;
	_lLostTracesNumber						= 0;

	_lListenPort							= lListenPort;

	_lSizeOfEachBlockToGzip					= lSizeOfEachBlockToGzip;

	if ((_pName = new char [strlen (pName) + 1]) == (char *) NULL)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_NEW_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}
	strcpy (_pName, pName);
	_lNameLength			= strlen (_pName);

	if (lCacheSizeOfTraceFile == -1)
	{
		_bTraceFileCacheActive			= false;
		_lCacheSizeOfTraceFile			= TRACER_DEFAULTCACHESIZEOFTRACEFILE;
	}
	else
	{
		_bTraceFileCacheActive			= true;
		_lCacheSizeOfTraceFile			= lCacheSizeOfTraceFile;
	}

	if ((_pTraceFileCache = new char [(_lCacheSizeOfTraceFile * 1024) + 1]) ==
		(char *) NULL)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_NEW_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}
	_pTraceFileCache [0]			= '\0';

	_lCurrentCacheBusyInBytes		= 0;

	if (_bTraceDirectory. init (pTraceDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	#ifdef WIN32
		if (pTraceDirectory [strlen (pTraceDirectory) - 1] != '\\')
		{
			if (_bTraceDirectory. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_bTraceDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				delete [] _pTraceFileCache;
				_pTraceFileCache					= (char *) NULL;

				delete [] _pName;
				_pName								= (char *) NULL;

				delete [] _ptiTracesToManage;
				_ptiTracesToManage					= (TraceInfo_p) NULL;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
	#else
		if (pTraceDirectory [strlen (pTraceDirectory) - 1] != '/')
		{
			if (_bTraceDirectory. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_bTraceDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				delete [] _pTraceFileCache;
				_pTraceFileCache					= (char *) NULL;

				delete [] _pName;
				_pName								= (char *) NULL;

				delete [] _ptiTracesToManage;
				_ptiTracesToManage					= (TraceInfo_p) NULL;

				delete [] _ptiTraces;
				_ptiTraces						= (TraceInfo_p) NULL;

				#ifdef WIN32
					if (WinThread:: finish () != errNoError)
				#else
					if (PosixThread:: finish () != errNoError)
				#endif
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
	#endif

	{
		char				pWorkingDirectory [TRACER_MAXPATHFILENAMELENGTH];


		if (FileIO:: getWorkingDirectory (pWorkingDirectory,
			TRACER_MAXPATHFILENAMELENGTH) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_GETWORKINGDIRECTORY_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (_bTraceDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			delete [] _pTraceFileCache;
			_pTraceFileCache					= (char *) NULL;

			delete [] _pName;
			_pName								= (char *) NULL;

			delete [] _ptiTracesToManage;
			_ptiTracesToManage					= (TraceInfo_p) NULL;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (_bTraceDirectory. substitute (
			"@CURRENTDIR@", pWorkingDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (_bTraceDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			delete [] _pTraceFileCache;
			_pTraceFileCache					= (char *) NULL;

			delete [] _pName;
			_pName								= (char *) NULL;

			delete [] _ptiTracesToManage;
			_ptiTracesToManage					= (TraceInfo_p) NULL;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
	}

	{
		Error_t						errCreateDir;


		#ifdef WIN32
			if ((errCreateDir = FileIO:: createDirectory (
				_bTraceDirectory. str(),
				0, true, true)) != errNoError)
		#else
			if ((errCreateDir = FileIO:: createDirectory (
				_bTraceDirectory. str(),
				S_IRUSR | S_IWUSR | S_IXUSR |
				S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, true, true)) !=
				errNoError)
		#endif
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
				1, _bTraceDirectory.str());
			std:: cerr << (const char *) err << std:: endl;

			if (_bTraceDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			delete [] _pTraceFileCache;
			_pTraceFileCache					= (char *) NULL;

			delete [] _pName;
			_pName								= (char *) NULL;

			delete [] _ptiTracesToManage;
			_ptiTracesToManage					= (TraceInfo_p) NULL;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return errCreateDir;
		}
	}

	if (_bTraceFileName. init (pTraceFileName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (strstr ((const char *) _bTraceFileName, "@YYYY@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@MM@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@DD@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@HI24@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@MI@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@SS@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@MSS@") != (char *) NULL ||
		strstr ((const char *) _bTraceFileName, "@CURRENTDIR@") != (char *) NULL
	)
		_bAreSubstitutionToDo		= true;
	else
		_bAreSubstitutionToDo		= false;

	if (_bLastTraceFilePathName. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bTraceFilePathInfoName. init ((const char *) _bTraceDirectory) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bTraceFilePathInfoName. append (_pName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bLinkToTheLastTraceFileName. init (
		(const char *) _bTraceDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bLinkToTheLastTraceFileName. append (_pName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bLinkToTheLastTraceFileName. append (".trace") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	_bClosedFileToBeCopied			= bClosedFileToBeCopied;
	if (_bClosedFileToBeCopied)
	{
		if ((_pClosedFilesRepository =
			new char [strlen (pClosedFilesRepository) + 1]) == (char *) NULL)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_NEW_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (_bLinkToTheLastTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceFilePathInfoName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bLastTraceFilePathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			delete [] _pTraceFileCache;
			_pTraceFileCache					= (char *) NULL;

			delete [] _pName;
			_pName								= (char *) NULL;

			delete [] _ptiTracesToManage;
			_ptiTracesToManage					= (TraceInfo_p) NULL;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
		strcpy (_pClosedFilesRepository, pClosedFilesRepository);
	}
	else
	{
		_pClosedFilesRepository		= (char *) NULL;
	}

	if (_bLockPathName. init ((const char *) _bTraceDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bClosedFileToBeCopied)
		{
			delete [] _pClosedFilesRepository;
			_pClosedFilesRepository					= (char *) NULL;
		}

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bLockPathName. append (_pName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bLockPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bClosedFileToBeCopied)
		{
			delete [] _pClosedFilesRepository;
			_pClosedFilesRepository					= (char *) NULL;
		}

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_bLockPathName. append (".lck") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_bLockPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bClosedFileToBeCopied)
		{
			delete [] _pClosedFilesRepository;
			_pClosedFilesRepository					= (char *) NULL;
		}

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache					= (char *) NULL;

		delete [] _pName;
		_pName								= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	{
		for (lTraceLevelIndex = 0; lTraceLevelIndex < TRACER_MAXTRACELEVES;
			lTraceLevelIndex++)
		{
			if (lTraceLevelIndex < TRACER_MAXDEFAULTTRACELEVELSNUMBER)
			{
				if ((_pTraceLevelLabel [lTraceLevelIndex] = new char [
					strlen (pDefaultTraceLevelLabel [lTraceLevelIndex]) +
					1]) == (char *) NULL)
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_NEW_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					while (lTraceLevelIndex-- == -1)
					{
						delete [] _pTraceLevelLabel [lTraceLevelIndex];
						_pTraceLevelLabel [lTraceLevelIndex]		=
							(char *) NULL;
					}

					if (_bLockPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_bClosedFileToBeCopied)
					{
						delete [] _pClosedFilesRepository;
						_pClosedFilesRepository					= (char *) NULL;
					}

					if (_bLinkToTheLastTraceFileName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_bTraceFilePathInfoName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_bLastTraceFilePathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_bTraceFileName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_bTraceDirectory. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					delete [] _pTraceFileCache;
					_pTraceFileCache					= (char *) NULL;

					delete [] _pName;
					_pName								= (char *) NULL;

					delete [] _ptiTracesToManage;
					_ptiTracesToManage					= (TraceInfo_p) NULL;

					delete [] _ptiTraces;
					_ptiTraces						= (TraceInfo_p) NULL;

					#ifdef WIN32
						if (WinThread:: finish () != errNoError)
					#else
						if (PosixThread:: finish () != errNoError)
					#endif
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PTHREAD_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}
				strcpy (_pTraceLevelLabel [lTraceLevelIndex],
					pDefaultTraceLevelLabel [lTraceLevelIndex]);
			}
			else
			{
				_pTraceLevelLabel [lTraceLevelIndex]		= (char *) NULL;
			}
		}
	}

	_ullMaxTraceFileSize						= ullMaxTraceFileSize;
	_lTraceFilePeriodInSecs						= lTraceFilePeriodInSecs;
	//	update of _tStartTraceFile each time
	//	the file number is updated inside _pBaseTraceFileName
	//	(just because this means another file)
	_tStartTraceFile							= time (NULL);

	_bCompressedTraceFile						= bCompressedTraceFile;

	_lTraceFilesNumberToMaintain				= lTraceFilesNumberToMaintain;


	{
		Error_t				errGet;
		long				lCurrentTraceFileNumber;
		Boolean_t			bHasFileNumberRound;


		if ((errGet = getCurrentTraceFileNumber (&lCurrentTraceFileNumber,
			&bHasFileNumberRound)) != errNoError)
		{
			std:: cerr << (const char *) errGet << std:: endl;

			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_GETCURRENTTRACEFILENUMBER_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			for (lTraceLevelIndex = 0;
				lTraceLevelIndex < TRACER_MAXTRACELEVES;
				lTraceLevelIndex++)
			{
				if (_pTraceLevelLabel [lTraceLevelIndex] !=
					(char *) NULL)
				{
					delete [] _pTraceLevelLabel [lTraceLevelIndex];
					_pTraceLevelLabel [lTraceLevelIndex]		=
						(char *) NULL;
				}
			}

			if (_bLockPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bClosedFileToBeCopied)
			{
				delete [] _pClosedFilesRepository;
				_pClosedFilesRepository					= (char *) NULL;
			}

			if (_bLinkToTheLastTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceFilePathInfoName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bLastTraceFilePathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_bTraceDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			delete [] _pTraceFileCache;
			_pTraceFileCache							= (char *) NULL;

			delete [] _pName;
			_pName										= (char *) NULL;

			delete [] _ptiTracesToManage;
			_ptiTracesToManage					= (TraceInfo_p) NULL;

			delete [] _ptiTraces;
			_ptiTraces						= (TraceInfo_p) NULL;

			#ifdef WIN32
				if (WinThread:: finish () != errNoError)
			#else
				if (PosixThread:: finish () != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return errGet;
		}

		{
			{
				// remove del file se e' raggiunto il numero max
				//	di files da mantenere
				if (lCurrentTraceFileNumber >= _lTraceFilesNumberToMaintain)
				{
					Error_t			errDelete;


					if (removeOldFile (
						lCurrentTraceFileNumber -
						_lTraceFilesNumberToMaintain + 1) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_REMOVEOLDFILE_FAILED,
							1, lCurrentTraceFileNumber -
							_lTraceFilesNumberToMaintain + 1);
						// std:: cerr << (const char *) err << std:: endl;

						/*
						Questo errore puo' verificarsi nel caso in cui
						il programma viene interrotto prima che il Tracer
						faccia il flush dei messaggi. In questo caso infatti
						quando il programma riparte, il Tracer utilizza il
						numero corrente (di cui in questo caso particolare
						non esiste il file) + 1. Ci si trova cosi' che nel
						file system viene saltato un file e cio' potrebbe
						portare ad un errore di unlink.
						Ci sono altri casi particolari che possono condurre
						a questo errore, per questo motivo la gestione
						di questo errore viene commentata
						*/
					}
				}
				else
				{
					// check if the file number arrived at the end (9999)
					// and restarted again
					if (bHasFileNumberRound)
					{
						Error_t			errDelete;


						if (removeOldFile (
							lCurrentTraceFileNumber + 9999 -
							_lTraceFilesNumberToMaintain + 1) != errNoError)
						{
							Error err = TracerErrors (__FILE__, __LINE__,
								TRACER_TRACER_REMOVEOLDFILE_FAILED,
								1, lCurrentTraceFileNumber + 9999 -
								_lTraceFilesNumberToMaintain + 1);
							// std:: cerr << (const char *) err
							//	<< std:: endl;

							/*
							Questo errore puo' verificarsi nel caso in cui
							il programma viene interrotto prima che il
							Tracer faccia il flush dei messaggi.
							In questo caso infatti quando il programma
							riparte, il Tracer utilizza il numero corrente
							(di cui in questo caso particolare
							non esiste il file) + 1. Ci si trova cosi'
							che nel file system viene saltato un file e
							cio' potrebbe portare ad un errore di unlink.
							Ci sono altri casi particolari che possono
							condurre a questo errore, per questo motivo
							la gestione di questo errore viene commentata
							*/
						}
					}
				}
			}
		}
	}

	// the next two fields are initialized when the trace file is opened
	_lCurrentTraceFileNumber		= -1;
	_bHasFileNumberRound			= false;

	_iFileDescriptor							= -1;
	_ullCurrentTraceFileSize					= 0;

	_bTraceOnFile								= bTraceOnFile;

	_bTraceOnTTY								= bTraceOnTTY;

	_lTraceLevel								= lTraceLevel;

	_lFunctionsStackDeep						= 0;

	if (_mtMutexForTraces. init (PMutex:: MUTEX_RECURSIVE) !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		for (lTraceLevelIndex = 0; lTraceLevelIndex < TRACER_MAXTRACELEVES;
			lTraceLevelIndex++)
		{
			if (_pTraceLevelLabel [lTraceLevelIndex] != (char *) NULL)
			{
				delete [] _pTraceLevelLabel [lTraceLevelIndex];
				_pTraceLevelLabel [lTraceLevelIndex]		=
					(char *) NULL;
			}
		}

		if (_bLockPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bClosedFileToBeCopied)
		{
			delete [] _pClosedFilesRepository;
			_pClosedFilesRepository					= (char *) NULL;
		}

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache								= (char *) NULL;

		delete [] _pName;
		_pName											= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_mtMutexForTracerVariable. init (PMutex:: MUTEX_RECURSIVE) !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_mtMutexForTraces. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		for (lTraceLevelIndex = 0; lTraceLevelIndex < TRACER_MAXTRACELEVES;
			lTraceLevelIndex++)
		{
			if (_pTraceLevelLabel [lTraceLevelIndex] != (char *) NULL)
			{
				delete [] _pTraceLevelLabel [lTraceLevelIndex];
				_pTraceLevelLabel [lTraceLevelIndex]		=
					(char *) NULL;
			}
		}

		if (_bLockPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bClosedFileToBeCopied)
		{
			delete [] _pClosedFilesRepository;
			_pClosedFilesRepository					= (char *) NULL;
		}

		if (_bLinkToTheLastTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFilePathInfoName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bLastTraceFilePathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (_bTraceDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		delete [] _pTraceFileCache;
		_pTraceFileCache								= (char *) NULL;

		delete [] _pName;
		_pName											= (char *) NULL;

		delete [] _ptiTracesToManage;
		_ptiTracesToManage					= (TraceInfo_p) NULL;

		delete [] _ptiTraces;
		_ptiTraces						= (TraceInfo_p) NULL;

		#ifdef WIN32
			if (WinThread:: finish () != errNoError)
		#else
			if (PosixThread:: finish () != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	_tsTracerStatus			= TRACER_INITIALIZED;


	return errNoError;
}


Error Tracer:: finish (Boolean_t bFlushOfTraces)

{

	long		lTraceLevelIndex;
	Error_t		errMutexFinish;


	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (bFlushOfTraces)
	{
		if (flushOfTraces () != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FLUSHOFTRACES_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}
	}

	if (_iFileDescriptor != -1)
	{
		if (FileIO:: close (_iFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		_iFileDescriptor		= -1;

		if (traceFileClosed ((const char *) _bLastTraceFilePathName) !=
			errNoError)
		{
			/*
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_TRACEFILECLOSED_FAILED);
			std:: cerr << (const char *) err << std:: endl;
			*/
		}

		#ifdef _USEGZIPLIB
			if (_bCompressedTraceFile)
			{
				if (gzipAndDeleteCurrentTraceFile () != errNoError)
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_GZIPANDDELETECURRENTTRACEFILE_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}
		#endif

		if (_bClosedFileToBeCopied)
		{
			Error_t					errFileIO;
			Buffer_t				bClosedFilesRepository;


			if (bClosedFilesRepository. init () != errNoError)
			{
				Error_t err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (fillClosedFilesRepository (&bClosedFilesRepository) !=
				errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_FILLCLOSEDFILESREPOSITORY_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}
			else
			{
				if (checkFileSystemSize (
					(const char *) bClosedFilesRepository) != errNoError)
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_CHECKFILESYSTEMSIZE_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
				else
				{
					if ((errFileIO = FileIO:: copyFile (
						(const char *) _bLastTraceFilePathName,
						(const char *) bClosedFilesRepository)) != errNoError)
					{
						std:: cerr << (const char *) errFileIO << std:: endl;

						Error_t err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_COPYFILE_FAILED,
							2, (const char *) _bLastTraceFilePathName,
							(const char *) bClosedFilesRepository);
						std:: cerr << (const char *) err << std:: endl;
					}
				}
			}

			if (bClosedFilesRepository. finish () != errNoError)
			{
				Error_t err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}
		}
	}

	if ((errMutexFinish = _mtMutexForTracerVariable. finish ()) !=
		errNoError)
	{
		/*
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
		*/
		std:: cerr << (const char *) errMutexFinish << std:: endl;
	}

	if ((errMutexFinish = _mtMutexForTraces. finish ()) !=
		errNoError)
	{
		/*
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
		*/
		std:: cerr << (const char *) errMutexFinish << std:: endl;
	}

	{
		for (lTraceLevelIndex = 0; lTraceLevelIndex < TRACER_MAXTRACELEVES;
			lTraceLevelIndex++)
		{
			if (_pTraceLevelLabel [lTraceLevelIndex] != (char *) NULL)
			{
				delete [] _pTraceLevelLabel [lTraceLevelIndex];
				_pTraceLevelLabel [lTraceLevelIndex]		= (char *) NULL;
			}
		}
	}

	if (_bLockPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (_bClosedFileToBeCopied)
	{
		delete [] _pClosedFilesRepository;
		_pClosedFilesRepository					= (char *) NULL;
	}

	if (_bLinkToTheLastTraceFileName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (_bTraceFilePathInfoName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (_bLastTraceFilePathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (_bTraceFileName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (_bTraceDirectory. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	delete [] _pTraceFileCache;
	_pTraceFileCache						= (char *) NULL;

	delete [] _pName;
	_pName									= (char *) NULL;

	delete [] _ptiTracesToManage;
	_ptiTracesToManage					= (TraceInfo_p) NULL;

	delete [] _ptiTraces;
	_ptiTraces								= (TraceInfo_p) NULL;

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		// std:: cerr << (const char *) err << std:: endl;
	}

	_tsTracerStatus			= TRACER_BUILDED;


	return errNoError;
}


Error Tracer:: run (void)

{

	Boolean_t					bLocalTracerShutdown;
	time_t						tTracesProcessingStartTime;
	time_t						tTracesProcessingEndTime;
	ServerSocket_t				ssServerSocket;
	ClientSocket_t				csClientSocket;
	SocketImpl_p				pSocketImpl;
	Error						errAcceptConnection;
	char						pConnectionBuffer [TRACER_MAXCONNECTIONBUFFER];
	Error						errPerformTrace;
	unsigned long				ulCharsRead;
	Error						errSocketInit;
	Error						errSocketFinish;


	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	bLocalTracerShutdown			= false;

	if (setTracerShutdown (bLocalTracerShutdown) != errNoError)
	{
		_erThreadReturn = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_SETTRACERSHUTDOWN_FAILED);
		std:: cerr << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	if (_lListenPort != -1)
	{
		if ((errSocketInit = ssServerSocket. init ((const char *) NULL,
			_lListenPort, true, SocketImpl:: STREAM, 10, 0, 10)) != errNoError)
		{
			std:: cerr << (const char *) errSocketInit << std:: endl;
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_INIT_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		if (ssServerSocket. getSocketImpl (&pSocketImpl) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		if (pSocketImpl -> setBlocking (false) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETBLOCKING_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}
	}

	tTracesProcessingStartTime			= time (NULL);

	while (!bLocalTracerShutdown)
	{
		tTracesProcessingEndTime				= time (NULL);


		if (tTracesProcessingEndTime - tTracesProcessingStartTime <
			_lSecondsBetweenTwoCheckTraceToManage)
		{
			long			lSleepTime;

			lSleepTime			= _lSecondsBetweenTwoCheckTraceToManage -
				((long) (tTracesProcessingEndTime -
					tTracesProcessingStartTime));

//			cout << "Sleep (" << lSleepTime << ")" << std:: endl;

			#ifdef WIN32
				if (WinThread:: getSleep (lSleepTime, 0) != errNoError)
			#else
				if (PosixThread:: getSleep (lSleepTime, 0) != errNoError)
			#endif
			{
				_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_GETSLEEP_FAILED);
				std:: cerr << (const char *) _erThreadReturn << std:: endl;

				if (_lListenPort != -1)
				{
					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}
				}

				return _erThreadReturn;
			}
		}

		tTracesProcessingStartTime			= time (NULL);

		if (_lListenPort != -1)
		{
			if (csClientSocket. init () != errNoError)
			{
				_erThreadReturn = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_INIT_FAILED,
					2, "<not applicable>", -1);
				std:: cerr << (const char *) _erThreadReturn << std:: endl;

				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return _erThreadReturn;
			}

			if ((errAcceptConnection = ssServerSocket. acceptConnection (
				&csClientSocket)) != errNoError)
			{
				Boolean_t			bIsThereError;


				bIsThereError				= false;

				if ((long) errAcceptConnection == SCK_ACCEPT_FAILED)
				{
					int					iErrno;
					unsigned long		ulUserDataBytes;

					errAcceptConnection. getUserData (&iErrno,
						&ulUserDataBytes);
					#ifdef WIN32
						if (iErrno == WSAEWOULDBLOCK)
					#else
						if (iErrno == EAGAIN)
					#endif
					{
						// Nonblocking I/O is enabled and no
						// connections are present to be accepted.
					}
					else
					{
						bIsThereError				= true;
					}
				}
				else
				{
					bIsThereError				= true;
				}

				if (bIsThereError)
				{
					std:: cerr << (const char *) errAcceptConnection
						<< std:: endl;
					_erThreadReturn = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_ACCEPTCONNECTION_FAILED);
					std:: cerr << (const char *) _erThreadReturn << std:: endl;

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return _erThreadReturn;
				}
			}

			if (errAcceptConnection == errNoError)
			{
				if (csClientSocket. getSocketImpl (&pSocketImpl) != errNoError)
				{
					_erThreadReturn = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKET_GETSOCKETIMPL_FAILED);
					std:: cerr << (const char *) _erThreadReturn << std:: endl;

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return _erThreadReturn;
				}

				if (pSocketImpl -> readLine (pConnectionBuffer,
					TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
					2, 0) != errNoError)
				{
					_erThreadReturn = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETIMPL_READLINE_FAILED);
					std:: cerr << (const char *) _erThreadReturn << std:: endl;

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return _erThreadReturn;
				}

				if (!strcmp (pConnectionBuffer, TRACER_FLUSHCOMMAND))
				{
					if (flushOfTraces () != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_FLUSHOFTRACES_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETMAXTRACEFILESIZECOMMAND))
				{
					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (setMaxTraceFileSize (atol (pConnectionBuffer)) !=
						errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETMAXTRACEFILESIZE_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETCOMPRESSEDTRACEFILECOMMAND))
				{
					Boolean_t				bCompressedTraceFile;

					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (atol (pConnectionBuffer) == 0)
						bCompressedTraceFile		= false;
					else
						bCompressedTraceFile		= true;

					if (setCompressedTraceFile (bCompressedTraceFile) !=
						errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETCOMPRESSEDTRACEFILE_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETTRACEFILESNUMBERTOMAINTAINCOMMAND))
				{
					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (setTraceFilesNumberToMaintain (
						atol (pConnectionBuffer)) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETTRACEFILESNUMBERTOMAINTAIN_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETTRACEONFILECOMMAND))
				{
					Boolean_t				bTraceOnFile;

					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (atol (pConnectionBuffer) == 0)
						bTraceOnFile			= false;
					else
						bTraceOnFile			= true;

					if (setTraceOnFile (bTraceOnFile) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETTRACEONFILE_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETTRACEONTTYCOMMAND))
				{
					Boolean_t				bTraceOnTTY;

					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (atol (pConnectionBuffer) == 0)
						bTraceOnTTY				= false;
					else
						bTraceOnTTY				= true;

					if (setTraceOnTTY (bTraceOnTTY) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETTRACEONTTY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else if (!strcmp (pConnectionBuffer,
					TRACER_SETTRACELEVELCOMMAND))
				{
					if (pSocketImpl -> readLine (pConnectionBuffer,
						TRACER_MAXCONNECTIONBUFFER - 1, &ulCharsRead,
						2, 0) != errNoError)
					{
						_erThreadReturn = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETIMPL_READLINE_FAILED);
						std:: cerr << (const char *) _erThreadReturn
							<< std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return _erThreadReturn;
					}

					if (setTraceLevel (atol (pConnectionBuffer)) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_SETTRACELEVEL_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (csClientSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_CLIENTSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (ssServerSocket. finish () != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SERVERSOCKET_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						// return _erThreadReturn;
					}
				}
				else
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_COMMANDWRONG, 1, pConnectionBuffer);
					std:: cerr << (const char *) err << std:: endl;

					if (csClientSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_CLIENTSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (ssServerSocket. finish () != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SERVERSOCKET_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					// return _erThreadReturn;
				}
			}

			if (csClientSocket. finish () != errNoError)
			{
				_erThreadReturn = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_FINISH_FAILED);
				std:: cerr << (const char *) _erThreadReturn << std:: endl;

				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return _erThreadReturn;
			}
		}

		if (populateTraceFileCache () != errNoError)
		{
			_erThreadReturn = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_POPULATETRACEFILECACHE_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			if (_lListenPort != -1)
			{
				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}

			return _erThreadReturn;
		}

		if (getTracerShutdown (&bLocalTracerShutdown) != errNoError)
		{
			_erThreadReturn = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_GETTRACERSHUTDOWN_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			if (_lListenPort != -1)
			{
				if (ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}

			return _erThreadReturn;
		}
	}

	if (_lListenPort != -1)
	{
		if ((errSocketFinish = ssServerSocket. finish ()) != errNoError)
		{
			std:: cerr << (const char *) errSocketFinish << std:: endl;
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			std:: cerr << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}
	}


	return _erThreadReturn;
}


Error Tracer:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t	stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif


	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (setTracerShutdown (true) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_SETTRACERSHUTDOWN_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (getThreadState (&stThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	tUTCNow					= time (NULL);

	while (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		if (time (NULL) - tUTCNow >= 2 * _lSecondsBetweenTwoCheckTraceToManage)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (0, 1000) != errNoError)
		#else
			if (PosixThread:: getSleep (0, 1000) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		if (getThreadState (&stThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
	}

	if (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		#ifdef WIN32
			// no cancel for Windows threads
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				return err;
			}
		#endif
	}


	return errNoError;
}


Error Tracer:: traceFunctionBegin (long lTraceLevel, const char *pFunctionName,
	const char *pFileName, long lFileLine, unsigned long ulThreadId)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (addTrace (lTraceLevel, pFunctionName, pFileName, lFileLine,
		ulThreadId, +1) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_ADDTRACE_FAILED,
			1, pFunctionName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: traceFunctionEnd (long lTraceLevel, const char *pFunctionName,
	const char *pFileName, long lFileLine, unsigned long ulThreadId)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (addTrace (lTraceLevel, pFunctionName, pFileName, lFileLine,
		ulThreadId, -1) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_ADDTRACE_FAILED,
			1, pFunctionName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	
	return errNoError;
}


Error Tracer:: trace (long lTraceLevel, const char *pTraceMessage,
	const char *pFileName, long lFileLine, unsigned long ulThreadId)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (addTrace (lTraceLevel, pTraceMessage, pFileName, lFileLine,
		ulThreadId, 0) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_ADDTRACE_FAILED,
			1, pTraceMessage);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: addTrace (long lTraceLevel, const char *pTraceMessage,
	const char *pFileName, long lFileLine, unsigned long ulThreadId,
	long lUpdateStackDeep)

{

	Boolean_t				bLocalTraceOnFile;
	Boolean_t				bLocalTraceOnTTY;
	long					lLocalTraceLevel;
	long					lTraceIndex;
	Error_t					errSetBuffer;


	if (getTraceOnFile (&bLocalTraceOnFile) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_GETTRACEONFILE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (getTraceOnTTY (&bLocalTraceOnTTY) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_GETTRACEONTTY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (getTraceLevel (&lLocalTraceLevel) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_GETTRACELEVEL_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((!bLocalTraceOnFile && !bLocalTraceOnTTY) ||
		lTraceLevel < lLocalTraceLevel)
	{
		;
	}
	else
	{
		if (_mtMutexForTraces. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
//		(cout << "Locked" << std:: endl). flush ();

		if (_lAllocatedTracesNumber <= _lTracesNumber)
		{
			TraceInfo_p				ptiLocalTraces;


			if ((ptiLocalTraces = new TraceInfo_t [_lAllocatedTracesNumber +
				_lTracesNumberAllocatedOnOverflow]) == (TraceInfo_p) NULL)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_NEW_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTraces. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			for (lTraceIndex = 0; lTraceIndex < _lAllocatedTracesNumber;
				lTraceIndex++)
			{
				ptiLocalTraces [lTraceIndex]			=
					_ptiTraces [lTraceIndex];
			}

			for (; lTraceIndex < _lAllocatedTracesNumber +
				_lTracesNumberAllocatedOnOverflow; lTraceIndex++)
			{
				if (((ptiLocalTraces [lTraceIndex]). _bTraceMessage).
					init ("", 0, 10) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					delete [] ptiLocalTraces;
					ptiLocalTraces						= (TraceInfo_p) NULL;

					if (_mtMutexForTraces. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				if (((ptiLocalTraces [lTraceIndex]). _bFileName).
					init ("", 0, 10) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					delete [] ptiLocalTraces;
					ptiLocalTraces						= (TraceInfo_p) NULL;

					if (_mtMutexForTraces. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}
			}

			if (_lAllocatedTracesNumber)
				delete [] _ptiTraces;

			_ptiTraces									= ptiLocalTraces;

			_lAllocatedTracesNumber						+=
				_lTracesNumberAllocatedOnOverflow;

//			cout << "Allocation for _ptiTraces. _lAllocatedTracesNumber: "
//				<< _lAllocatedTracesNumber << std:: endl;
		}

		if (_lTracesNumber < _lMaxTracesNumber)
		{
			if (DateTime:: get_tm_LocalTime (
				&((_ptiTraces [_lTracesNumber]). _tmDateTime),
				&((_ptiTraces [_lTracesNumber]). _ulMilliSecs)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTraces. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			(_ptiTraces [_lTracesNumber]). _lTraceLevel		= lTraceLevel;

			if ((errSetBuffer =
				((_ptiTraces [_lTracesNumber]). _bTraceMessage). setBuffer (
				pTraceMessage)) != errNoError)
			{
				std:: cerr << (const char *) errSetBuffer << std:: endl;

				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTraces. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if (((_ptiTraces [_lTracesNumber]). _bFileName). setBuffer (
				pFileName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTraces. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			(_ptiTraces [_lTracesNumber]). _lFileLine			= lFileLine;

			(_ptiTraces [_lTracesNumber]). _ulThreadId			= ulThreadId;

			(_ptiTraces [_lTracesNumber]). _lUpdateStackDeep	=
				lUpdateStackDeep;

			_lTracesNumber++;
		}
		else		// _lTracesNumber >= _lMaxTracesNumber
			_lLostTracesNumber++;

//		(cout << "unLocked" << std:: endl). flush ();
		if (_mtMutexForTraces. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		// This check must be after the previous unLock
		if (!_bTraceFileCacheActive)
		{
			if (flushOfTraces () != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_FLUSHOFTRACES_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				return err;
			}
		}

	}


	return errNoError;
}


Error Tracer:: flushOfTraces (void)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	// the populateTraceFileCache has its mutexes management
	if (populateTraceFileCache () != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_POPULATETRACEFILECACHE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	// the flushTraceFileCache has its mutexes management
	if (flushTraceFileCache () != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: populateTraceFileCache (void)

{

	Error							errMutexLock;


	// this method exchange the pools and populate the cache
	// in memory after the building the trace messages.
	// The nested mutex (_mtMutexForTraces) is used just to exchange the pools.
	// After this activity, the application clients can continue to trace.
	// The _mtMutexForTracerVariable mutex is used because I cannot exchange
	// again the pools if I didn't finish to populate the cache
	// with the SECONDARY POOL.
	// For this reason we must use both the mutex here.

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errMutexLock = _mtMutexForTraces. lock ()) !=
		errNoError)
	{
		std:: cerr << (const char *) errMutexLock << std:: endl;

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_mtMutexForTracerVariable. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return errMutexLock;
	}
//	(cout << "Locked" << std:: endl). flush ();

	if (_lLostTracesNumber > 0)
	{
		Buffer_t			bTracesMissing;


		if (bTracesMissing. init (_lLostTracesNumber) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (_mtMutexForTraces. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTracerVariable. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (bTracesMissing. append (" TRACES MISSING") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bTracesMissing. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTraces. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTracerVariable. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		#ifdef WIN32
			if (addTrace (0, (const char *) bTracesMissing,
				__FILE__, __LINE__, WinThread:: getCurrentThreadIdentifier (),
				0) != errNoError)
		#else
			if (addTrace (0, (const char *) bTracesMissing,
				__FILE__, __LINE__, PosixThread:: getCurrentThreadIdentifier (),
				0) != errNoError)
		#endif
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_ADDTRACE_FAILED,
				1, (const char *) bTracesMissing);
			std:: cerr << (const char *) err << std:: endl;

			if (bTracesMissing. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTraces. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTracerVariable. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (bTracesMissing. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (_mtMutexForTraces. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_mtMutexForTracerVariable. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		_lLostTracesNumber			= 0;
	}

	// scambio i puntatori ai due buffer di traces
	// ed inizializzazione a 0 di _lTracesNumber
//	cout << "Begin change pointer" << std:: endl;
	{
		TraceInfo_p				ptiLocalTraces;
		long					lLocalTracesNumber;
		long					lLocalAllocatedTracesNumber;
		long					lLocalTracesNumberAllocatedOnOverflow;


		ptiLocalTraces							= _ptiTraces;
		_ptiTraces								= _ptiTracesToManage;
		_ptiTracesToManage						= ptiLocalTraces;

		lLocalTracesNumber						= _lTracesNumber;
		_lTracesNumber							= _lTracesNumberToManage;
		_lTracesNumberToManage					= lLocalTracesNumber;

		_lTracesNumber							= 0;

		lLocalAllocatedTracesNumber				= _lAllocatedTracesNumber;
		_lAllocatedTracesNumber					=
			_lAllocatedTracesNumberToManage;
		_lAllocatedTracesNumberToManage			=
			lLocalAllocatedTracesNumber;

		lLocalTracesNumberAllocatedOnOverflow	=
			_lTracesNumberAllocatedOnOverflow;
		_lTracesNumberAllocatedOnOverflow		=
			_lTracesNumberToManageAllocatedOnOverflow;
		_lTracesNumberToManageAllocatedOnOverflow	=
			lLocalTracesNumberAllocatedOnOverflow;
	}
//	cout << "End change pointer" << std:: endl;

//	(cout << "unLocked" << std:: endl). flush ();
	if (_mtMutexForTraces. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (_mtMutexForTracerVariable. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (_lTracesNumberToManage > 0)
	{
		long							lTraceIndex;
		Error							errPerformTrace;

		//	cout << "begin performTrace" << std:: endl;
		for (lTraceIndex = 0; lTraceIndex < _lTracesNumberToManage;
			lTraceIndex++)
		{
			if ((errPerformTrace = performTrace (
				&((_ptiTracesToManage [lTraceIndex]). _tmDateTime),
				&((_ptiTracesToManage [lTraceIndex]). _ulMilliSecs),
				(_ptiTracesToManage [lTraceIndex]). _lTraceLevel,
				(const char *) ((_ptiTracesToManage [lTraceIndex]).
				_bTraceMessage),
				(const char *) ((_ptiTracesToManage [lTraceIndex]). _bFileName),
				(_ptiTracesToManage [lTraceIndex]). _lFileLine,
				(_ptiTracesToManage [lTraceIndex]). _ulThreadId,
				(_ptiTracesToManage [lTraceIndex]). _lUpdateStackDeep)) !=
				errNoError)
			{
				if ((long) errPerformTrace == TRACER_TRACETOOLONG)
				{
				}
				else
				{
					std:: cerr << (const char *) errPerformTrace << std:: endl;

					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_PERFORMTRACE_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}
			}
		}
		//	cout << "end performTrace" << std:: endl;
	}
	else
	{
		// no trace to manage
		// call flushTraceFileCache in case the _lTraceFilePeriodInSecs
		//	expired in order to switch the trace file
		if (time (NULL) - _tStartTraceFile > _lTraceFilePeriodInSecs)
		{
			if (flushTraceFileCache () != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
	}

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getTracerShutdown (Boolean_p pbTracerShutdown)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*pbTracerShutdown		= _bTracerShutdown;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setTracerShutdown (Boolean_t bTracerShutdown)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_bTracerShutdown			= bTracerShutdown;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getTraceOnFile (Boolean_p pbTraceOnFile)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*pbTraceOnFile			= _bTraceOnFile;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setTraceOnFile (Boolean_t bTraceOnFile)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_bTraceOnFile						= bTraceOnFile;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getTraceOnTTY (Boolean_p pbTraceOnTTY)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*pbTraceOnTTY			= _bTraceOnTTY;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setTraceOnTTY (Boolean_t bTraceOnTTY)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_bTraceOnTTY					= bTraceOnTTY;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getName (char *pName)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	strcpy (pName, _pName);

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


/*
Error Tracer:: getBaseTraceFileName (char *pBaseTraceFileName)

{

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	strcpy (pBaseTraceFileName, _pBaseTraceFileName);

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}
*/


#ifdef WIN32
	Error Tracer:: getMaxTraceFileSize (
		__int64 *pullMaxTraceFileSize)
#else
	Error Tracer:: getMaxTraceFileSize (
		unsigned long long *pullMaxTraceFileSize)
#endif

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*pullMaxTraceFileSize				= _ullMaxTraceFileSize;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error Tracer:: setMaxTraceFileSize (__int64 ullMaxTraceFileSize)
#else
	Error Tracer:: setMaxTraceFileSize (unsigned long long ullMaxTraceFileSize)
#endif

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_ullMaxTraceFileSize				= ullMaxTraceFileSize;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setCompressedTraceFile (Boolean_t bCompressedTraceFile)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_bCompressedTraceFile				= bCompressedTraceFile;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getTraceFilesNumberToMaintain (
	long *plTraceFilesNumberToMaintain)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*plTraceFilesNumberToMaintain		= _lTraceFilesNumberToMaintain;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setTraceFilesNumberToMaintain (long lTraceFilesNumberToMaintain)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_lTraceFilesNumberToMaintain		= lTraceFilesNumberToMaintain;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getTraceLevel (long *plTraceLevel)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	*plTraceLevel						= _lTraceLevel;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: setTraceLevel (long lTraceLevel)

{

	if (_tsTracerStatus != TRACER_INITIALIZED)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_OPERATION_NOTALLOWED, 1, (long) _tsTracerStatus);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_lTraceLevel						= lTraceLevel;

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: resetTraceLevels (void)

{

	return errNoError;
}


Error Tracer:: addTraceLevel (const char *pTraceLevelLabel)

{

	return errNoError;
}


Error Tracer:: performTrace (
	tm *ptmDateTime, unsigned long *pulMilliSecs, long lTraceLevel,
	const char *pTraceMessage, const char *pFileName, long lFileLine,
	unsigned long ulThreadId, long lUpdateStackDeep)

{

	char		pCurrentDate [TRACER_MAXDATELENGTH];
	char		pCurrentTime [TRACER_MAXTIMELENGTH];
	long		lFileNameLength;
	char		pFileLine [32 + 1];
	long		lFileLineLength;
	char		pThreadId [32 + 1];
	long		lThreadIdLength;
	long		lTraceMessageLength;
	long		lCompleteTraceMessageLength;
	long		lTraceLevelLabelLength;


//	trace format:
//	[date time][traceLevel][fileName fileLine][sessionName threadId][traceMsg]

	sprintf (pCurrentDate, "%04lu-%02lu-%02lu",
		(unsigned long) (ptmDateTime -> tm_year + 1900),
		(unsigned long) (ptmDateTime -> tm_mon + 1),
		(unsigned long) (ptmDateTime -> tm_mday));

	sprintf (pCurrentTime, "%02lu:%02lu:%02lu.%03lu",
		(unsigned long) (ptmDateTime -> tm_hour),
		(unsigned long) (ptmDateTime -> tm_min),
		(unsigned long) (ptmDateTime -> tm_sec),
		*pulMilliSecs);

	if (lUpdateStackDeep < 0)
		_lFunctionsStackDeep		-= 1;

	if (getCompleteTraceLength (lTraceLevel, pTraceMessage,
		pFileName, lFileLine, ulThreadId, &lTraceLevelLabelLength,
		&lFileNameLength, pFileLine, &lFileLineLength,
		pThreadId, &lThreadIdLength, &lTraceMessageLength,
		&lCompleteTraceMessageLength) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_GETCOMPLETETRACELENGTH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	// a causa dello '\n' finale
	lCompleteTraceMessageLength			+= 1;

	if (_lCacheSizeOfTraceFile * 1024 < lCompleteTraceMessageLength)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACETOOLONG,
			2,
			(unsigned long) lCompleteTraceMessageLength,
			(unsigned long) (_lCacheSizeOfTraceFile * 1024));
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_lCurrentCacheBusyInBytes + lCompleteTraceMessageLength >=
		_lCacheSizeOfTraceFile * 1024)
	{
		// since there is no enough space on the cache to add the message
		// it is necessary to flush the cache
		if (flushTraceFileCache () != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
	}

	if (fillCompleteTraceMessage (_pTraceFileCache + _lCurrentCacheBusyInBytes,
		pCurrentDate, pCurrentTime, lTraceLevel, lTraceLevelLabelLength,
		pFileName, lFileNameLength,
		pFileLine, lFileLineLength,
		pThreadId, lThreadIdLength,
		pTraceMessage, lTraceMessageLength) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_FILLCOMPLETETRACEMESSAGE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	_pTraceFileCache [_lCurrentCacheBusyInBytes +
		lCompleteTraceMessageLength - 1]	= '\n';
	_pTraceFileCache [_lCurrentCacheBusyInBytes +
		lCompleteTraceMessageLength]		= '\0';

	_lCurrentCacheBusyInBytes				+= lCompleteTraceMessageLength;

	if (lUpdateStackDeep > 0)
		_lFunctionsStackDeep		+= 1;

	if (time (NULL) - _tStartTraceFile > _lTraceFilePeriodInSecs)
	{
		if (flushTraceFileCache () != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
	}

	if (_bTraceOnTTY)
	{
        #ifdef __QTCOMPILER__
            qDebug() << _pTraceFileCache + (_lCurrentCacheBusyInBytes - lCompleteTraceMessageLength);
        #else
            std:: cout << _pTraceFileCache + (_lCurrentCacheBusyInBytes - lCompleteTraceMessageLength);
            std:: cout. flush ();
        #endif
/*
		long lIndex;
		for (lIndex = 0; lIndex < lCompleteTraceMessageLength; lIndex++)
			printf ("%c", _pTraceFileCache [_lCurrentCacheBusyInBytes -
				lCompleteTraceMessageLength + lIndex]);
*/
	}


	return errNoError;
}


Error Tracer:: getCompleteTraceLength (long lTraceLevel,
	const char *pTraceMessage, const char *pFileName, long lFileLine,
	unsigned long ulThreadId, long *plTraceLevelLabelLength,
	long *plFileNameLength, char *pFileLine, long *plFileLineLength,
	char *pThreadId, long *plThreadIdLength, long *plTraceMessageLength,
	long *plCompleteTraceMessageLength)

{

//	trace format:
//	[date time][traceLevel][fileName fileLine][sessionName threadId][traceMsg]

	(*plCompleteTraceMessageLength)		= _lFunctionsStackDeep;

	(*plCompleteTraceMessageLength)		+=
		((1 + (TRACER_MAXDATELENGTH - 1) + 1 +
		(TRACER_MAXTIMELENGTH - 1) + 1 + 1));

//	traceLevel][fileName fileLine][sessionName threadId][traceMsg]
	if (_pTraceLevelLabel [lTraceLevel] != (char *) NULL)
	{
		(*plTraceLevelLabelLength)			= strlen (
			_pTraceLevelLabel [lTraceLevel]);
		(*plCompleteTraceMessageLength)		+= (*plTraceLevelLabelLength);
	}
	else
		(*plTraceLevelLabelLength)			= 0;

//	][fileName fileLine][sessionName threadId][traceMsg]
	(*plCompleteTraceMessageLength)		+= (1 + 1);

//	fileName fileLine][sessionName threadId][traceMsg]
	(*plFileNameLength)					= strlen (pFileName);
	(*plCompleteTraceMessageLength)		+= (*plFileNameLength);

//	 fileLine][sessionName threadId][traceMsg]
	(*plCompleteTraceMessageLength)		+= 1;

//	fileLine][sessionName threadId][traceMsg]
	sprintf (pFileLine, "%ld", lFileLine);

	(*plFileLineLength)					= strlen (pFileLine);
	(*plCompleteTraceMessageLength)		+= (*plFileLineLength);

//	][sessionName threadId][traceMsg]
	(*plCompleteTraceMessageLength)		+= (1 + 1);

//	sessionName threadId][traceMsg]
	(*plCompleteTraceMessageLength)		+= (_lNameLength + 1);

//	threadId][traceMsg]
	sprintf (pThreadId, "%lu", ulThreadId);

	(*plThreadIdLength)					= strlen (pThreadId);
	(*plCompleteTraceMessageLength)		+= (*plThreadIdLength);

//	][traceMsg]
	(*plCompleteTraceMessageLength)		+= (1 + 1);

//	traceMsg]
	(*plTraceMessageLength)				= strlen (pTraceMessage);
	(*plCompleteTraceMessageLength)		+= (*plTraceMessageLength);

//	]
	(*plCompleteTraceMessageLength)		+= 1;


	return errNoError;
}


Error Tracer:: fillCompleteTraceMessage (char *pCompleteTraceMessageToFill,
	char *pCurrentDate, char *pCurrentTime,
	long lTraceLevel, long lTraceLevelLabelLength,
	const char *pFileName, long lFileNameLength,
	const char *pFileLine, long lFileLineLength,
	const char *pThreadId, long lThreadIdLength,
	const char *pTraceMessage, long lTraceMessageLength)

{

	long			lIndex;
	long			lCurrentCompleteTrace;


	lCurrentCompleteTrace		= 0;

//	format trace
//	[date time][traceLevel][fileName fileLine][sessionName threadId][traceMsg]

	for (lIndex = 0; lIndex < _lFunctionsStackDeep; lIndex++)
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]	= '\t';

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '[';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pCurrentDate, 10);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + 10;
	/*
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [0];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [1];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [2];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [3];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [4];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [5];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [6];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [7];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [8];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentDate [9];
	*/
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '-';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pCurrentTime, 12);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + 12;

	/*
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [0];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [1];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [2];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [3];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [4];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [5];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [6];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [7];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [8];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [9];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [10];
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
		pCurrentTime [11];
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ']';
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '[';

	if (_pTraceLevelLabel [lTraceLevel] != (char *) NULL)
	{
		for (lIndex = 0; lIndex < lTraceLevelLabelLength; lIndex++)
		{
			pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
				_pTraceLevelLabel [lTraceLevel][lIndex];
		}
	}

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ']';
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '[';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pFileName, lFileNameLength);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + lFileNameLength;

	/*
	for (lIndex = 0; lIndex < lFileNameLength; lIndex++)
	{
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
			pFileName [lIndex];
	}
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ' ';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pFileLine, lFileLineLength);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + lFileLineLength;

	/*
	for (lIndex = 0; lIndex < lFileLineLength; lIndex++)
	{
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
			pFileLine [lIndex];
	}
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ']';
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '[';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		_pName, _lNameLength);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + _lNameLength;

	/*
	for (lIndex = 0; lIndex < _lNameLength; lIndex++)
	{
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
			_pName [lIndex];
	}
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ' ';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pThreadId, lThreadIdLength);
	lCurrentCompleteTrace			= lCurrentCompleteTrace + lThreadIdLength;

	/*
	for (lIndex = 0; lIndex < lThreadIdLength; lIndex++)
	{
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
			pThreadId [lIndex];
	}
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ']';
	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= '[';

	memcpy (pCompleteTraceMessageToFill + lCurrentCompleteTrace,
		pTraceMessage, lTraceMessageLength);
	lCurrentCompleteTrace		= lCurrentCompleteTrace + lTraceMessageLength;

	/*
	for (lIndex = 0; lIndex < lTraceMessageLength; lIndex++)
	{
		pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		=
			pTraceMessage [lIndex];
	}
	*/

	pCompleteTraceMessageToFill [lCurrentCompleteTrace++]		= ']';
	pCompleteTraceMessageToFill [lCurrentCompleteTrace]			= '\0';


	return errNoError;
}


Error Tracer:: flushTraceFileCache (void)

{

	#ifdef WIN32
		__int64					llBytesWritten;
	#else
		long long				llBytesWritten;
	#endif
	Error				errDelete;
	Error				errCreateLink;
	Error				errWriteChars;


	if (_mtMutexForTracerVariable. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_bTraceOnFile)
	{
//		cout << "Start flush on file" << std:: endl;

		Error errCheckFileSystem;

		if ((errCheckFileSystem = checkFileSystemSize (
			(const char *) _bTraceDirectory)) != errNoError)
		{
			std:: cerr << (const char *) errCheckFileSystem << std:: endl;

			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_CHECKFILESYSTEMSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;

			if (setTraceOnFile (false) != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_SETTRACEONFILE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			/*
			// It seems this error management creates problems,
			// so the management of it is done below in this procedure
			// Look for 'errCheckFileSystem'

			// if _bLastTraceFilePathName is initialized with a non .gz file
			if ((unsigned long) _bLastTraceFilePathName > 3 &&
				strcmp (((const char *) _bLastTraceFilePathName) +
					(unsigned long) _bLastTraceFilePathName - 3, ".gz"))
			{
				FileIO:: appendBuffer ((const char *) _bLastTraceFilePathName,
					(const char *) errCheckFileSystem, true);
			}
			else
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_TRACEFILEPATHNAMENOTAVAILABLE);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (_lCurrentTraceFileNumber == -1)
			{
				if (getCurrentTraceFileNumber (&_lCurrentTraceFileNumber,
					&_bHasFileNumberRound) != errNoError)
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_GETCURRENTTRACEFILENUMBER_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}

			_lCurrentCacheBusyInBytes			= 0;
			_pTraceFileCache [0]				= '\0';

			if (_mtMutexForTracerVariable. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			// It seems that returning the error will create problems
			return err;
			*/
		}

		if (_iFileDescriptor == -1)
		{
			if (getNextTraceFileNumber (&_lCurrentTraceFileNumber,
				&_bHasFileNumberRound) != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_GETNEXTTRACEFILENUMBER_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if (getNewTraceFilePathName (_lCurrentTraceFileNumber,
				&_bLastTraceFilePathName) != errNoError)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_GETNEWTRACEPATHFILENAME_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			#ifdef WIN32
				if (FileIO:: open ((const char *) _bLastTraceFilePathName,
					O_WRONLY | O_TRUNC | O_CREAT,
					_S_IREAD | _S_IWRITE, &_iFileDescriptor) !=
					errNoError)
			#else
				if (FileIO:: open ((const char *) _bLastTraceFilePathName,
					O_WRONLY | O_TRUNC | O_CREAT,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
					&_iFileDescriptor) != errNoError)
			#endif
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_OPEN_FAILED, 1,
					(const char *) _bLastTraceFilePathName);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			//	update of _tStartTraceFile each time
			//	the file number is updated inside _pBaseTraceFileName
			//	(just because this means another file)
			_tStartTraceFile			= time (NULL);

			#ifdef WIN32
			#else
				if ((errCreateLink = FileIO:: createLink (
					(const char *) _bLastTraceFilePathName,
					(const char *) _bLinkToTheLastTraceFileName,
					true)) != errNoError)
				{
					std:: cerr << (const char *) errCreateLink << std:: endl;

					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CREATELINK_FAILED,
						2, (const char *) _bLastTraceFilePathName,
						(const char *) _bLinkToTheLastTraceFileName);
					std:: cerr << (const char *) err << std:: endl;

					/*
					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
					*/
				}
			#endif
		}

		if (_lCurrentCacheBusyInBytes != 0)
		{
			if (errCheckFileSystem == errNoError)
			{
				if ((errWriteChars = FileIO:: writeChars (_iFileDescriptor,
					_pTraceFileCache, _lCurrentCacheBusyInBytes,
					&llBytesWritten)) != errNoError)
				{
					std:: cerr << (const char *) errWriteChars << std:: endl;

					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);
					std:: cerr << (const char *) err << std:: endl;
		
					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				_ullCurrentTraceFileSize		=
					_ullCurrentTraceFileSize + llBytesWritten;
			}

			_lCurrentCacheBusyInBytes			= 0;
			_pTraceFileCache [0]		= '\0';
		}

		if (_ullCurrentTraceFileSize >= _ullMaxTraceFileSize * 1024 ||
			time (NULL) - _tStartTraceFile > _lTraceFilePeriodInSecs ||
			errCheckFileSystem != errNoError)
		{
			if (FileIO:: close (_iFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			_iFileDescriptor		= -1;

			_ullCurrentTraceFileSize	= 0;

			if (traceFileClosed ((const char *) _bLastTraceFilePathName) !=
				errNoError)
			{
				/*
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_TRACER_TRACEFILECLOSED_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				if (_mtMutexForTracerVariable. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
				*/
			}

			#ifdef _USEGZIPLIB
				if (_bCompressedTraceFile)
				{
					if (gzipAndDeleteCurrentTraceFile () != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_GZIPANDDELETECURRENTTRACEFILE_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (_mtMutexForTracerVariable. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return err;
					}
				}
			#endif

			if (_bClosedFileToBeCopied)
			{
				Error_t					errFileIO;
				Buffer_t				bClosedFilesRepository;


				if (bClosedFilesRepository. init () != errNoError)
				{
					Error_t err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				if (fillClosedFilesRepository (&bClosedFilesRepository) !=
					errNoError)
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_FILLCLOSEDFILESREPOSITORY_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					if (bClosedFilesRepository. finish () != errNoError)
					{
						Error_t err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				if ((errFileIO = FileIO:: copyFile (
					(const char *) _bLastTraceFilePathName,
					(const char *) bClosedFilesRepository)) != errNoError)
				{
					std:: cerr << (const char *) errFileIO << std:: endl;

					Error_t err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_COPYFILE_FAILED,
						2, (const char *) _bLastTraceFilePathName,
						(const char *) bClosedFilesRepository);
					std:: cerr << (const char *) err << std:: endl;

					if (bClosedFilesRepository. finish () != errNoError)
					{
						Error_t err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				if (bClosedFilesRepository. finish () != errNoError)
				{
					Error_t err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;

					if (_mtMutexForTracerVariable. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}
			}

			if (_lTraceFilesNumberToMaintain != -1)
			{
				// remove del file se e' raggiunto il numero max
				//	di files da mantenere
				if (_lCurrentTraceFileNumber >= _lTraceFilesNumberToMaintain)
				{
					if (removeOldFile (
						_lCurrentTraceFileNumber -
						_lTraceFilesNumberToMaintain + 1) != errNoError)
					{
						Error err = TracerErrors (__FILE__, __LINE__,
							TRACER_TRACER_REMOVEOLDFILE_FAILED,
							1, _lCurrentTraceFileNumber -
							_lTraceFilesNumberToMaintain + 1);
						// std:: cerr << (const char *) err << std:: endl;

						/*
						Questo errore puo' verificarsi nel caso in cui
						il programma viene interrotto prima che il Tracer
						faccia il flush dei messaggi. In questo caso infatti
						quando il programma riparte, il Tracer utilizza il
						numero corrente (di cui in questo caso particolare
						non esiste il file) + 1. Ci si trova cosi' che nel
						file system viene saltato un file e cio' potrebbe
						portare ad un errore di unlink.
						Ci sono altri casi particolari che possono condurre
						a questo errore, per questo motivo la gestione
						di questo errore viene commentata
						*/
					}
				}
				else
				{
					if (_bHasFileNumberRound)
					{
						if (removeOldFile (
							_lCurrentTraceFileNumber + 9999 -
							_lTraceFilesNumberToMaintain + 1) != errNoError)
						{
							Error err = TracerErrors (__FILE__, __LINE__,
								TRACER_TRACER_REMOVEOLDFILE_FAILED,
								1, _lCurrentTraceFileNumber + 9999 -
								_lTraceFilesNumberToMaintain + 1);
							// std:: cerr << (const char *) err << std:: endl;

							/*
							Questo errore puo' verificarsi nel caso in cui
							il programma viene interrotto prima che il Tracer
							faccia il flush dei messaggi. In questo caso infatti
							quando il programma riparte, il Tracer utilizza il
							numero corrente (di cui in questo caso particolare
							non esiste il file) + 1. Ci si trova cosi' che nel
							file system viene saltato un file e cio' potrebbe
							portare ad un errore di unlink.
							Ci sono altri casi particolari che possono condurre
							a questo errore, per questo motivo la gestione
							di questo errore viene commentata
							*/
						}
					}
				}
			}

			if (errCheckFileSystem != errNoError)
			{
				// if _bLastTraceFilePathName is initialized with a non .gz file
				if ((unsigned long) _bLastTraceFilePathName > 3 &&
					strcmp (((const char *) _bLastTraceFilePathName) +
						(unsigned long) _bLastTraceFilePathName - 3, ".gz"))
				{
					FileIO:: appendBuffer (
						(const char *) _bLastTraceFilePathName,
						(const char *) errCheckFileSystem, true);
				}
				else
				{
					Error err = TracerErrors (__FILE__, __LINE__,
						TRACER_TRACER_TRACEFILEPATHNAMENOTAVAILABLE);
					std:: cerr << (const char *) err << std:: endl;
				}
			}
		}
	}
	else
	{
		_lCurrentCacheBusyInBytes			= 0;
		_pTraceFileCache [0]				= '\0';
	}

	if (_mtMutexForTracerVariable. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


#ifdef _USEGZIPLIB
	Error Tracer:: gzipAndDeleteCurrentTraceFile (void)

	{

		Buffer_t			bUnCompressedTraceFileName;
		#ifdef WIN32
			__int64			llUncompressedBytesRead;
		#else
			long long		llUncompressedBytesRead;
		#endif
		int					iSrcFileDescriptor;
		unsigned char		*pUncompressedBuffer;
		Error				errDelete;
		gzFile				gzfGZipFile;


		if (bUnCompressedTraceFileName. init (
			(const char *) _bLastTraceFilePathName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			std:: cerr << (const char *) err << std:: endl;

       		return err;
		}

		if (_bLastTraceFilePathName. append (".gz") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (checkFileSystemSize ((const char *) _bTraceDirectory) != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_CHECKFILESYSTEMSIZE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

   		if (FileIO:: open ((const char *) bUnCompressedTraceFileName,
			O_RDONLY, &iSrcFileDescriptor) != errNoError)
   		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED,
				1, (const char *) bUnCompressedTraceFileName);
			std:: cerr << (const char *) err << std:: endl;

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

       		return err;
   		}

		if ((pUncompressedBuffer = new unsigned char [
			_lSizeOfEachBlockToGzip * 1024]) == (unsigned char *) NULL)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_NEW_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: close (iSrcFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if ((gzfGZipFile = gzopen ((const char *) _bLastTraceFilePathName,
			"wb+")) == (gzFile) NULL)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_GZOPEN_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			delete [] pUncompressedBuffer;

			if (FileIO:: close (iSrcFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		do
		{
			#ifdef WIN32
				if (FileIO:: readChars (iSrcFileDescriptor,
					(char *) pUncompressedBuffer,
					(__int64) (_lSizeOfEachBlockToGzip * 1024),
					&llUncompressedBytesRead) != errNoError)
			#else
				if (FileIO:: readChars (iSrcFileDescriptor,
					(char *) pUncompressedBuffer,
					(unsigned long long) (_lSizeOfEachBlockToGzip * 1024),
					&llUncompressedBytesRead) != errNoError)
			#endif
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_READCHARS_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				gzclose (gzfGZipFile);

				delete [] pUncompressedBuffer;

				if (FileIO:: close (iSrcFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				if (bUnCompressedTraceFileName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if (llUncompressedBytesRead == 0)
				continue;

			if (gzwrite (gzfGZipFile, pUncompressedBuffer,
				llUncompressedBytesRead) == 0)
			{
				Error err = TracerErrors (__FILE__, __LINE__,
					TRACER_GZWRITE_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				gzclose (gzfGZipFile);

				delete [] pUncompressedBuffer;

				if (FileIO:: close (iSrcFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				if (bUnCompressedTraceFileName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}
		}
		#ifdef WIN32
			while (llUncompressedBytesRead ==
				(__int64) (_lSizeOfEachBlockToGzip * 1024));
		#else
			while (llUncompressedBytesRead ==
				(long long) (_lSizeOfEachBlockToGzip * 1024));
		#endif

		// The gzclose function closes iDestFileDescriptor too
		gzclose (gzfGZipFile);

		delete [] pUncompressedBuffer;

		if (FileIO:: close (iSrcFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if ((errDelete = FileIO:: remove (
			(const char *) bUnCompressedTraceFileName)) != errNoError)
		{
			std:: cerr << (const char *) errDelete << std:: endl;
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			/*
			Questo errore puo' verificarsi nel caso in cui
			il programma viene interrotto prima che il Tracer
			faccia il flush dei messaggi. In questo caso infatti
			quando il programma riparte, il Tracer utilizza il
			numero corrente (di cui in questo caso particolare
			non esiste il file) + 1. Ci si trova cosi' che nel
			file system viene saltato un file e cio' potrebbe
			portare ad un errore di unlink.
			Ci sono altri casi particolari che possono condurre
			a questo errore, per questo motivo la gestione
			di questo errore viene commentata

			if (bUnCompressedTraceFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
			*/
		}

		if (bUnCompressedTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}


		return errNoError;
	}
#endif


Error Tracer:: fillClosedFilesRepository (Buffer_p pbClosedFilesRepository)

{

	if (pbClosedFilesRepository -> setBuffer (_pClosedFilesRepository) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		std:: cerr << (const char *) err << std:: endl;

       	return err;
	}


	return errNoError;
}


Error Tracer:: traceFileClosed (const char *pTraceFilePathName)

{

	/*
	Buffer_t			bClosedTraceFileName;
	char				pCurrentTraceFileNumber [12 + 1];	// _????_.trace


	if (bClosedTraceFileName. init (_pBaseTraceFileName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

       	return err;
	}

	sprintf (pCurrentTraceFileNumber, "_%04ld_.trace",
		lCurrentTraceFileNumber);

	if (bClosedTraceFileName. append (pCurrentTraceFileNumber) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bClosedTraceFileName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

       	return err;
	}

	// ...

	if (bClosedTraceFileName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	*/


	return errNoError;
}


Error Tracer:: getCurrentTraceFileNumber (
	long *plCurrentTraceFileNumber,
	Boolean_p pbHasFileNumberRound)

{

	int					iTraceFileNumberFileDescriptor;
	int					ifdLockFile;
	#ifdef WIN32
		__int64			llBytesWritten;
		__int64			llBytesRead;
	#else
		long long		llBytesWritten;
		long long		llBytesRead;
	#endif
	Error_t				errLockFile;


	if ((errLockFile = FileIO:: lockFile ((const char *) _bLockPathName, 10,
		&ifdLockFile)) != errNoError)
	{
		std:: cerr << (const char *) errLockFile << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_LOCKFILE_FAILED, 1, (const char *) _bLockPathName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (FileIO:: open ((const char *) _bTraceFilePathInfoName,
		O_RDONLY, &iTraceFileNumberFileDescriptor) != errNoError)
	{
		*plCurrentTraceFileNumber		= 0;
		*pbHasFileNumberRound			= false;

		#ifdef WIN32
			if (FileIO:: open ((const char *) _bTraceFilePathInfoName,
				O_WRONLY | O_TRUNC | O_CREAT,
				_S_IREAD | _S_IWRITE, &iTraceFileNumberFileDescriptor) !=
				errNoError)
		#else
			if (FileIO:: open ((const char *) _bTraceFilePathInfoName,
				O_WRONLY | O_TRUNC | O_CREAT,
				S_IRUSR | S_IWUSR |
				S_IRGRP | S_IWGRP |
				S_IROTH | S_IWOTH,
				&iTraceFileNumberFileDescriptor) != errNoError)
		#endif
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED, 1,
				(const char *) _bTraceFilePathInfoName);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (FileIO:: writeChars (iTraceFileNumberFileDescriptor,
			(char *) (plCurrentTraceFileNumber),
			(unsigned int) (sizeof (long)), &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (FileIO:: writeChars (iTraceFileNumberFileDescriptor,
			(char *) (pbHasFileNumberRound),
			(unsigned int) (sizeof (Boolean_t)), &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
	}
	else
	{
		if (FileIO:: readChars (iTraceFileNumberFileDescriptor,
			(char *) plCurrentTraceFileNumber,
			(unsigned int) (sizeof (long)), &llBytesRead) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READCHARS_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (FileIO:: readChars (iTraceFileNumberFileDescriptor,
			(char *) (pbHasFileNumberRound),
			(unsigned int) (sizeof (Boolean_t)), &llBytesRead) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READCHARS_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: unLockFile ((const char *) _bLockPathName,
				ifdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1,
					(const char *) _bLockPathName);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
	}

	if (FileIO:: unLockFile ((const char *) _bLockPathName,
		ifdLockFile) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_LOCKFILE_FAILED, 1,
			(const char *) _bLockPathName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: getNextTraceFileNumber (
	long *plCurrentTraceFileNumber,
	Boolean_p pbHasFileNumberRound)

{
	int					iTraceFileNumberFileDescriptor;
	int					ifdLockFile;
	#ifdef WIN32
		__int64			llBytesWritten;
		__int64			llBytesRead;
		__int64			llCurrentPosition;
	#else
		long long		llBytesWritten;
		long long		llBytesRead;
		long long		llCurrentPosition;
	#endif


	if (FileIO:: lockFile ((const char *) _bLockPathName, 10,
		&ifdLockFile) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_LOCKFILE_FAILED, 1, (const char *) _bLockPathName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	// the file must already exist because
	//	it is already called getCurrentTraceFileNumber
	if (FileIO:: open ((const char *) _bTraceFilePathInfoName,
		O_RDWR, &iTraceFileNumberFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED,
			1, (const char *) _bTraceFilePathInfoName);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: readChars (iTraceFileNumberFileDescriptor,
		(char *) plCurrentTraceFileNumber,
		(unsigned int) (sizeof (long)), &llBytesRead) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READCHARS_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: readChars (iTraceFileNumberFileDescriptor,
		(char *) pbHasFileNumberRound,
		(unsigned int) (sizeof (Boolean_t)), &llBytesRead) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READCHARS_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: seek (iTraceFileNumberFileDescriptor,
		0, SEEK_SET, &llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_SEEK_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (*plCurrentTraceFileNumber >= 9999)
	{
		*plCurrentTraceFileNumber			= 1;
		*pbHasFileNumberRound				= true;
	}
	else
		*plCurrentTraceFileNumber			+= 1;

	if (FileIO:: writeChars (iTraceFileNumberFileDescriptor,
		(char *) plCurrentTraceFileNumber,
		(unsigned int) (sizeof (long)), &llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: writeChars (iTraceFileNumberFileDescriptor,
		(char *) pbHasFileNumberRound,
		(unsigned int) (sizeof (Boolean_t)), &llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: close (iTraceFileNumberFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (FileIO:: unLockFile ((const char *) _bLockPathName,
			ifdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1,
				(const char *) _bLockPathName);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (FileIO:: unLockFile ((const char *) _bLockPathName,
		ifdLockFile) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_LOCKFILE_FAILED, 1,
			(const char *) _bLockPathName);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: checkFileSystemSize (const char *pDirectoryPathName)

{

	unsigned long long		ullUsedInKB;
	unsigned long long		ullAvailableInKB;
	long					lPercentUsed;
	Error_t					errFileIO;
	Buffer_t				bDirectoryPathName;


	if (bDirectoryPathName. init (pDirectoryPathName) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errFileIO = FileIO:: getFileSystemInfo (
		(const char *) bDirectoryPathName,
		&ullUsedInKB, &ullAvailableInKB, &lPercentUsed)) !=
		errNoError)
	{
		std:: cerr << (const char *) errFileIO << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_GETFILESYSTEMINFO_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (bDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	// to be safe we will check that at least we have space
	// to save 5 times the max size of one trace file
	// if (ullAvailableInKB <= _ullMaxTraceFileSize * 3)
	if (ullAvailableInKB <= 100000)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_NOSPACEAVAILABLE,
			2, (const char *) _bTraceDirectory, ullAvailableInKB);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


Error Tracer:: removeOldFile (long lTraceFileNumber)

{

	// 6 -> .trace, 3 -> .gz
	char						pTailOfTheFile [
		TRACER_MAXTRACEFILENUMBERLENGTH + 6 + 1];
	unsigned long				ulTailOfTheFileLength;
	char						pTailOfTheFileGZIP [
		TRACER_MAXTRACEFILENUMBERLENGTH + 6 + 3 + 1];
	unsigned long				ulTailOfTheFileGZIPLength;
	Boolean_t					bFoundFileToBeRemoved;

	FileIO:: Directory_t		dDirectory;
	Error_t						errDir;
	Buffer_t					bDirectoryEntry;
	FileIO:: DirectoryEntryType_t		detDirectoryEntryType;
	Error_t						errDelete;


	sprintf (pTailOfTheFile, "_%04ld_.trace", lTraceFileNumber);
	ulTailOfTheFileLength			= strlen (pTailOfTheFile);

	sprintf (pTailOfTheFileGZIP, "_%04ld_.trace.gz", lTraceFileNumber);
	ulTailOfTheFileGZIPLength		= strlen (pTailOfTheFileGZIP);

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errDir = FileIO:: openDirectory ((const char *) _bTraceDirectory,
		&dDirectory)) != errNoError)
	{
		std:: cerr << (const char *) errDir << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	bFoundFileToBeRemoved		= false;

	while ((errDir = FileIO:: readDirectory (&dDirectory,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		if (detDirectoryEntryType == FileIO:: TOOLS_FILEIO_REGULARFILE)
		{
			if (
				(
				(unsigned long) bDirectoryEntry >=
				ulTailOfTheFileLength &&
				!strcmp (
					(const char *) bDirectoryEntry +
						(unsigned long) bDirectoryEntry -
						ulTailOfTheFileLength,
						pTailOfTheFile)
				) ||
				(
				(unsigned long) bDirectoryEntry >=
				ulTailOfTheFileGZIPLength &&
				!strcmp (
					(const char *) bDirectoryEntry +
						(unsigned long) bDirectoryEntry -
						ulTailOfTheFileGZIPLength,
						pTailOfTheFileGZIP)
				)
				)
			{
				bFoundFileToBeRemoved		= true;


				break;
			}
		}
	}

	if (errDir != errNoError &&
		(long) errDir != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
	{

		std:: cerr << (const char *) errDir << std:: endl;

		if (FileIO:: closeDirectory (&dDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return errDir;
	}

	if (bFoundFileToBeRemoved)
	{
		if (bDirectoryEntry. insertAt (0,
			(const char *) _bTraceDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INSERTAT_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: closeDirectory (&dDirectory) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if ((errDelete = FileIO:: remove (
			(const char *) bDirectoryEntry)) != errNoError)
		{
			std:: cerr << (const char *) errDelete
				<< std:: endl;

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (FileIO:: closeDirectory (&dDirectory) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
	}

	if (FileIO:: closeDirectory (&dDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	if (bDirectoryEntry. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}



	return errNoError;
}


Error Tracer:: getNewTraceFilePathName (
	long lTraceFileNumber, Buffer_p pbNewTraceFileName)

{

	if (pbNewTraceFileName -> setBuffer (
		_bTraceFileName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (_bAreSubstitutionToDo)
	{
		tm					tmDateTime;
		unsigned long		ulMilliSeconds;
		char				pTime [TRACER_MAXTIMELENGTH];


		if (DateTime:: get_tm_LocalTime (
			&tmDateTime, &ulMilliSeconds) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%04lu",
			(unsigned long) (tmDateTime. tm_year + 1900));

		if (pbNewTraceFileName -> substitute (
			"@YYYY@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%02lu",
			(unsigned long) (tmDateTime. tm_mon + 1));

		if (pbNewTraceFileName -> substitute (
			"@MM@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%02lu",
			(unsigned long) (tmDateTime. tm_mday));

		if (pbNewTraceFileName -> substitute (
			"@DD@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%02lu",
			(unsigned long) (tmDateTime. tm_hour));

		if (pbNewTraceFileName -> substitute (
			"@HI24@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%02lu",
			(unsigned long) (tmDateTime. tm_min));

		if (pbNewTraceFileName -> substitute (
			"@MI@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%02lu",
			(unsigned long) (tmDateTime. tm_sec));

		if (pbNewTraceFileName -> substitute (
			"@SS@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		sprintf (pTime, "%03lu", ulMilliSeconds);

		if (pbNewTraceFileName -> substitute (
			"@MSS@", pTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SUBSTITUTE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
	}

	// append lTraceFileNumber and ".trace"
	{
		char				pTraceFileNumber [TRACER_MAXTRACEFILENUMBERLENGTH];


		sprintf (pTraceFileNumber, "_%04ld_", lTraceFileNumber);

		if (pbNewTraceFileName -> append (pTraceFileNumber) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		if (pbNewTraceFileName -> append (".trace") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}
	}

	if (pbNewTraceFileName -> insertAt (
		0, (const char *) _bTraceDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}

