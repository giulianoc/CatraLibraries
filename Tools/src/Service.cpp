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

#include "Service.h"
#include "ProcessUtility.h"
#include "FileIO.h"
#include "DateTime.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#else
	#include <unistd.h>
#endif

#ifdef WIN32
	Service *Service:: _pThis		= NULL;
#endif


Service:: Service (void)

{
	#ifdef WIN32
		_pThis				= this;
	#endif

	// Set the initial default service debug path name
	#ifdef WIN32
		strcpy (_pServiceDebugFile, "C:\\ServiceDebug.log");
	#else
		strcpy (_pServiceDebugFile, "/tmp/ServiceDebug.log");
	#endif

}


Service:: ~Service (void)

{

}


#ifdef WIN32
#else
	Error Service:: launchUnixDaemon (
		const char *pPIDFilePathName)

	{

		/* Our process ID and Session ID */
		pid_t				pid, sid;


		/* Fork off the parent process */
		pid			= fork();
		if (pid < 0)
		{
			exit (EXIT_FAILURE);
		}

		/* If we got a good PID, then
			we can exit the parent process. */
		if (pid > 0)
		{
			exit (EXIT_SUCCESS);
		}

		/*
			In order to write to any files (including logs) created
			by the daemon, the file mode mask (umask) must be changed
			to ensure that they can be written to or read from properly.
			umask default value: 0x022
		*/
		umask (0x002);

		/*
			From here, the child process must get a unique SID from the kernel
			in order to operate. Otherwise, the child process becomes
			an orphan in the system.
		*/
		sid			= setsid ();
		if (sid < 0)
		{
			/* Log the failure */

			exit (EXIT_FAILURE);
		}

		/*
			The current working directory should be changed to some place
			that is guaranteed to always be there.
		*/
		if ((chdir("/")) < 0)
		{
			/* Log the failure */

			exit (EXIT_FAILURE);
		}

		/* Close out the standard file descriptors */
		/*
			Since a daemon cannot use the terminal, it is better to close
			the standard file descriptors (STDIN, STDOUT, STDERR) that
			are redundant and potential security hazard.
		*/
		close (STDIN_FILENO);
		close (STDOUT_FILENO);
		// close (STDERR_FILENO);

		{
			Buffer_t				bPIDFile;
			long					lProcessIdentifier;
			Error_t					errWriteBuffer;


			if (ProcessUtility:: getCurrentProcessIdentifier (
				&lProcessIdentifier) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_PROCESSUTILITY_GETCURRENTPROCESSIDENTIFIER_FAILED);

				return err;
			}

			if (bPIDFile. init (lProcessIdentifier) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);

				return err;
			}

			if ((errWriteBuffer = bPIDFile. writeBufferOnFile (
				pPIDFilePathName)) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED);

				if (bPIDFile. finish () != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_BUFFER_FINISH_FAILED);
				}

				// return err;
				return errWriteBuffer;
			}

			if (bPIDFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);

				return err;
			}
		}


		return errNoError;
	}
#endif


#ifdef WIN32
	Error Service:: init (
		const char *pServiceName, const char *pServiceDescription,
		const char *pServiceUserName, const char *pServicePassword,
		DWORD dwStartType)
#else
	Error Service:: init (
		const char *pServiceName, const char *pServiceDescription)
#endif

{

	#ifdef WIN32
		if (strlen ("C:\\Service_") + strlen (pServiceName) +
			strlen (".log") >= TOOLS_SERVICE_MAXSERVICEDEBUGLOG)
		{
			strcpy (_pServiceDebugFile, "C:\\ServiceDebug.log");
		}
		else
		{
			sprintf (_pServiceDebugFile, "%s%s.log",
				"C:\\Service_",
				pServiceName,
				".log");
		}
	#else
		if (strlen ("/tmp/Service_") + strlen (pServiceName) +
			strlen (".log") >= TOOLS_SERVICE_MAXSERVICEDEBUGLOG)
		{
			strcpy (_pServiceDebugFile, "/tmp/ServiceDebug.log");
		}
		else
		{
			sprintf (_pServiceDebugFile, "%s%s.log",
				"/tmp/Service_",
				pServiceName);
		}
	#endif

	if (_bServiceName. init (pServiceName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime, (const char *) err);
		#endif

		return err;
	}

	if (_bServiceDescription. init (pServiceDescription) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime, (const char *) err);
		#endif

		if (_bServiceName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif
		}

		return err;
	}

	#ifdef WIN32
		if (_bServiceUserName. init (
			pServiceUserName == (const char *) NULL ? "" : pServiceUserName) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (_bServiceDescription. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		if (_bServicePassword. init (
			pServicePassword == (const char *) NULL ? "" : pServicePassword) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (_bServiceUserName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceDescription. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		_dwStartType			= dwStartType;

		if ((_pscmServiceControlManager = OpenSCManager (NULL,
			SERVICES_ACTIVE_DATABASE, SC_MANAGER_ALL_ACCESS)) == NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_OPENSCMANAGER_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (_bServicePassword. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceUserName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceDescription. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			if (_bServiceName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		_bServiceRunning			= false;
		_bServicePaused				= false;
	#endif


	return errNoError;
}


Error Service:: finish (void)

{

	#ifdef WIN32
    	if (_pscmServiceControlManager != NULL)
    	{
        	if (::CloseServiceHandle (_pscmServiceControlManager) == 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_CLOSESERVICEHANDLE_FAILED,
					1, (long) GetLastError ());
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}
    	}

		if (_bServicePassword. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (_bServiceUserName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}
	#endif

	if (_bServiceDescription. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime, (const char *) err);
		#endif

		return err;
	}

	if (_bServiceName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime, (const char *) err);
		#endif

		return err;
	}


	return errNoError;
}


Error Service:: isInstalled (Boolean_p pbIsInstalled)

{

	#ifdef WIN32
		SC_HANDLE			pscServiceControl;


		if ((pscServiceControl = ::OpenService (
			_pscmServiceControlManager, (const char *) _bServiceName,
			SERVICE_QUERY_CONFIG)) == NULL)
		{
			long			lErrorCode;

			lErrorCode		= GetLastError ();

			if (lErrorCode != ERROR_SERVICE_DOES_NOT_EXIST)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_OPENSERVICE_FAILED,
					1, lErrorCode);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}
		}

		if (pscServiceControl)
		{
			*pbIsInstalled			= true;

        	if (::CloseServiceHandle (pscServiceControl) == 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_CLOSESERVICEHANDLE_FAILED,
					1, (long) GetLastError ());
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}
		}
		else
		{
			*pbIsInstalled			= false;
		}
	#else
		Buffer_t			bInitTabFile;
		const char			*pCurrentRunLevelBegin;
		const char			*pCurrentRunLevelEnd;
		char				pCurrentRunLevelDirectory [
			TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH];


		if (bInitTabFile. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (bInitTabFile. readBufferFromFile ("/etc/inittab") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (bInitTabFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		if ((pCurrentRunLevelBegin = strstr (
			(const char *) bInitTabFile, "\nid:")) == (char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_CURRENTRUNLEVELNOTFOUND);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (bInitTabFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		pCurrentRunLevelBegin			+= 4;

		if ((pCurrentRunLevelEnd = strchr (pCurrentRunLevelBegin, ':')) ==
			(char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_CURRENTRUNLEVELNOTFOUND);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (bInitTabFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		if (strlen ("/etc/rc.d/rc") +
			(pCurrentRunLevelEnd - pCurrentRunLevelBegin) +
			strlen (".d/") >= TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_RUNLEVELDIRECTORYLENGTHTOOSMALL);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		strcpy (pCurrentRunLevelDirectory, "/etc/rc.d/rc");

		strncat (pCurrentRunLevelDirectory, pCurrentRunLevelBegin,
			pCurrentRunLevelEnd - pCurrentRunLevelBegin);

		strcat (pCurrentRunLevelDirectory, ".d/");

		if (bInitTabFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		// loop of the services installed for the specified runlevel
		{
			FileIO:: Directory_t		dDirectory;
			Error_t						errReadDirectory;
			Buffer_t					bDirectoryEntry;
			FileIO:: DirectoryEntryType_t
				detDirectoryEntryType;
			char						pRealPathName [
				TOOLS_SERVICE_MAXPATHNAME];
			const char					*pLocalServiceName;


			if (bDirectoryEntry. init () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}

			if (FileIO:: openDirectory (pCurrentRunLevelDirectory,
				&dDirectory) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_OPENDIRECTORY_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bDirectoryEntry. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime, (const char *) err);
					#endif

					return err;
				}

				return err;
			}

			*pbIsInstalled			= false;

			while ((errReadDirectory = FileIO:: readDirectory (&dDirectory,
				&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
			{
				if (detDirectoryEntryType == FileIO:: TOOLS_FILEIO_LINKFILE)
				{
					// link name format: ['S'|'K']<number><service name>
					//	'S' if must be started when the machine is starting
					//	'K' if must be not started when the machine is starting
					//	<number> indicates the order of starting of the services

					if (bDirectoryEntry. insertAt (0,
						pCurrentRunLevelDirectory) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						if (FileIO:: closeDirectory (&dDirectory) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							#ifdef SERVICEDEBUG
								tm					tmDateTime;
								unsigned long		ulMilliSecs;
								char				pCurrentDateTime [24 + 1];
								DateTime:: get_tm_LocalTime (&tmDateTime,
									&ulMilliSecs);
								sprintf (pCurrentDateTime,
									"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
									(unsigned long) (tmDateTime. tm_year + 1900),
									(unsigned long) (tmDateTime. tm_mon + 1),
									(unsigned long) (tmDateTime. tm_mday),
									(unsigned long) (tmDateTime. tm_hour),
									(unsigned long) (tmDateTime. tm_min),
									(unsigned long) (tmDateTime. tm_sec),
									ulMilliSecs);
								FileIO:: appendBuffer (_pServiceDebugFile,
									true, 2, pCurrentDateTime,
									(const char *) err);
							#endif
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							#ifdef SERVICEDEBUG
								tm					tmDateTime;
								unsigned long		ulMilliSecs;
								char				pCurrentDateTime [24 + 1];
								DateTime:: get_tm_LocalTime (&tmDateTime,
									&ulMilliSecs);
								sprintf (pCurrentDateTime,
									"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
									(unsigned long) (tmDateTime. tm_year + 1900),
									(unsigned long) (tmDateTime. tm_mon + 1),
									(unsigned long) (tmDateTime. tm_mday),
									(unsigned long) (tmDateTime. tm_hour),
									(unsigned long) (tmDateTime. tm_min),
									(unsigned long) (tmDateTime. tm_sec),
									ulMilliSecs);
								FileIO:: appendBuffer (_pServiceDebugFile,
									true, 2, pCurrentDateTime,
									(const char *) err);
							#endif
						}

						return err;
					}

					if (FileIO:: readLink ((const char *) bDirectoryEntry,
						pRealPathName, TOOLS_SERVICE_MAXPATHNAME) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_READLINK_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						if (FileIO:: closeDirectory (&dDirectory) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							#ifdef SERVICEDEBUG
								tm					tmDateTime;
								unsigned long		ulMilliSecs;
								char				pCurrentDateTime [24 + 1];
								DateTime:: get_tm_LocalTime (&tmDateTime,
									&ulMilliSecs);
								sprintf (pCurrentDateTime,
									"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
									(unsigned long) (tmDateTime. tm_year + 1900),
									(unsigned long) (tmDateTime. tm_mon + 1),
									(unsigned long) (tmDateTime. tm_mday),
									(unsigned long) (tmDateTime. tm_hour),
									(unsigned long) (tmDateTime. tm_min),
									(unsigned long) (tmDateTime. tm_sec),
									ulMilliSecs);
								FileIO:: appendBuffer (_pServiceDebugFile,
									true, 2, pCurrentDateTime,
									(const char *) err);
							#endif
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							#ifdef SERVICEDEBUG
								tm					tmDateTime;
								unsigned long		ulMilliSecs;
								char				pCurrentDateTime [24 + 1];
								DateTime:: get_tm_LocalTime (&tmDateTime,
									&ulMilliSecs);
								sprintf (pCurrentDateTime,
									"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
									(unsigned long) (tmDateTime. tm_year + 1900),
									(unsigned long) (tmDateTime. tm_mon + 1),
									(unsigned long) (tmDateTime. tm_mday),
									(unsigned long) (tmDateTime. tm_hour),
									(unsigned long) (tmDateTime. tm_min),
									(unsigned long) (tmDateTime. tm_sec),
									ulMilliSecs);
								FileIO:: appendBuffer (_pServiceDebugFile,
									true, 2, pCurrentDateTime,
									(const char *) err);
							#endif
						}

						return err;
					}

					if ((pLocalServiceName = strrchr (pRealPathName, '/')) ==
						(char *) NULL)
						pLocalServiceName			= pRealPathName;
					else
						pLocalServiceName			+= 1;

					if (!strcmp (pLocalServiceName,
						(const char *) _bServiceName))
					{
						*pbIsInstalled			= true;

						break;
					}
				}
			}

			// errReadDirectory could be errNoError in case it found the service
			//	and exit from the previous while through the break command
			if (errReadDirectory != errNoError &&
				(long) errReadDirectory != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
			{
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) errReadDirectory);
				#endif

				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_READDIRECTORY_FAILED);
				#ifdef SERVICEDEBUG
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (FileIO:: closeDirectory (&dDirectory) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				if (bDirectoryEntry. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return errReadDirectory;
			}

			if (FileIO:: closeDirectory (&dDirectory) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bDirectoryEntry. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}
		}
	#endif


	return errNoError;
}


Error Service:: install (int iArgc, char **pArgv)

{

	#ifdef WIN32
		char				pModuleFileName [
			TOOLS_SERVICE_MAXMODULEFILENAMELENGTH];
		Buffer_t			bModuleFileName;
		SC_HANDLE			pscServiceControl;
		unsigned long		ulArgumentIndex;


		if (GetModuleFileName (GetModuleHandle (NULL),
			pModuleFileName, TOOLS_SERVICE_MAXMODULEFILENAMELENGTH) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETMODULEFILENAME_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (bModuleFileName. init ("\"") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (bModuleFileName. append (pModuleFileName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (bModuleFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		for (ulArgumentIndex = 1; ulArgumentIndex < (unsigned long) iArgc;
			ulArgumentIndex++)
		{
			if (!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_INSTALL) ||
				!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_UNINSTALL) ||
				!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_START) ||
				!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_STOP))
				continue;

			if (bModuleFileName. append (" ") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bModuleFileName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bModuleFileName. append (pArgv [ulArgumentIndex]) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bModuleFileName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}
		}

		if (bModuleFileName. append ("\"") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			if (bModuleFileName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif
			}

			return err;
		}

		/*
			std:: cout << "CreateService. Name: "
				<< (const char *) _bServiceName ", Description: "
				<< (const char *) _bServiceDescription << ", User name: "
				(const char *) _bServiceUserName << ", Password: "
				(const char *) _bServicePassword << std:: endl;
		*/

		if ((pscServiceControl = CreateService (_pscmServiceControlManager,
			(const char *) _bServiceName, (const char *) _bServiceDescription,
			SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        	_dwStartType,
        	SERVICE_ERROR_NORMAL, (const char *) bModuleFileName,
        	NULL, NULL, NULL,
			!strcmp ((const char *) _bServiceUserName, "") ?
				(const char *) NULL : (const char *) _bServiceUserName,
			!strcmp ((const char *) _bServicePassword, "") ?
				(const char *) NULL : (const char *) _bServicePassword)) ==
				NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_CREATESERVICE_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

    	if (::CloseServiceHandle (pscServiceControl) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_CLOSESERVICEHANDLE_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (onInstall () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONINSTALL_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}
	#else
		Buffer_t				bServiceScriptFile;
		char					pServiceNamePathName [
			TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH];
		Buffer_t				bInstallServiceExecuteCommand;
		int						iExecuteCommandStatus;
		Boolean_t				bServiceScriptFileExist;
		Error_t					errWriteBuffer;


		if (strlen ("/etc/rc.d/init.d/") +
			(unsigned long) _bServiceName >=
			TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_RUNLEVELDIRECTORYLENGTHTOOSMALL);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		sprintf (pServiceNamePathName, "/etc/rc.d/init.d/%s",
			(const char *) _bServiceName);

		/*
		if (FileIO:: exist (pServiceNamePathName, &bServiceScriptFileExist) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_EXIST_FAILED);
			#ifdef SERVICEDEBUG
				time_t		tTime		= time (NULL);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime, (const char *) err);
			#endif

			return err;
		}

		if (!bServiceScriptFileExist)
		*/
		{
			/*
			Each  service which should be manageable
			by chkconfig needs two or more
			commented lines added to its init.d script.
			The first line  tells  chk-
			config  what  runlevels the service should be started
			in by default, as
			well as the start and stop priority levels.
			If the service should  not,
			by default, be started in any runlevels, a -
			should be used in place of
			the runlevels list.  The second line contains 
			a  description  for  the
			service,  and may be extended across multiple lines
			with backslash continuation.
			*/
			if (bServiceScriptFile. init (
				"#!/bin/sh\n"
				"#\n"
				"# chkconfig: - 91 35\n"
				"# description: ") !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				return err;
			}

			if (bServiceScriptFile. append (
				(const char *) _bServiceDescription) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"#\n"
				"# Source function library." "\n"
				"if [ -f /etc/init.d/functions ] ; then" "\n"
				"\t. /etc/init.d/functions" "\n"
				"elif [ -f /etc/rc.d/init.d/functions ] ; then" "\n"
				"\t. /etc/rc.d/init.d/functions" "\n"
				"else" "\n"
				"\texit 0" "\n"
				"fi" "\n"
				"\n"
				"start() {\n"
				) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (appendStartScriptCommand (&bServiceScriptFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_APPENDSTARTSCRIPTCOMMAND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"}\n") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime, (const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"stop() {\n"
				) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (appendStopScriptCommand (&bServiceScriptFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_APPENDSTOPSCRIPTCOMMAND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"}\n") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"restart() {\n"
				"\tstop\n"
				"\tstart\n"
				"}\n") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"rhstatus() {\n"
				) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (appendStatusScriptCommand (&bServiceScriptFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_APPENDSTATUSSCRIPTCOMMAND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"}\n") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"case \"$1\" in\n"
				"\tstart)\n"
				"\t\tstart\n"
				"\t\t;;\n"
				"\tstop)\n"
				"\t\tstop\n"
				"\t\t;;\n"
				"\trestart)\n"
				"\t\trestart\n"
				"\t\t;;\n"
				"\tstatus)\n"
				"\t\trhstatus\n"
				"\t\t;;\n"
				"\t*)\n"
				"\t\techo $\"Usage: $0 {start|stop|restart|status}\"\n"
				"\t\texit 1\n"
				"esac\n"
				) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. append (
				"\n"
				"exit $?\n"
				"\n"
				) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if ((errWriteBuffer = bServiceScriptFile. writeBufferOnFile (
				pServiceNamePathName)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) errWriteBuffer);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (FileIO:: changePermission (
				pServiceNamePathName,
				S_IRUSR | S_IWUSR | S_IXUSR |
				S_IRGRP | S_IXGRP |
				S_IROTH | S_IXOTH) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CHANGEPERMISSION_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				if (bServiceScriptFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif
				}

				return err;
			}

			if (bServiceScriptFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}
		}

		if (bInstallServiceExecuteCommand. init (
			"/usr/lib/lsb/install_initd --add ") !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (bInstallServiceExecuteCommand. append (
			(const char *) _bServiceName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				false, 2, pCurrentDateTime,
				"Execute command: ");
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				(const char *) bInstallServiceExecuteCommand);
		#endif

		if (ProcessUtility:: execute (
			(const char *) bInstallServiceExecuteCommand,
			&iExecuteCommandStatus) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_PROCESSUTILITY_EXECUTE_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		if (iExecuteCommandStatus != 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_INSTALLSERVICEEXECUTECOMMANDFAILED,
				2, (const char *) bInstallServiceExecuteCommand,
				(long) iExecuteCommandStatus);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		if (bInstallServiceExecuteCommand. finish () !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (onInstall () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONINSTALL_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}
	#endif


	return errNoError;
}


#ifdef WIN32
#else
	Error Service:: appendStartScriptCommand (
		Buffer_p pbServiceScriptFile)

	{

		if (pbServiceScriptFile -> append (
			"\techo \"Started\" >> "
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}
		if (pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\treturn 0"
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		return errNoError;
	}

	Error Service:: appendStopScriptCommand (
		Buffer_p pbServiceScriptFile)

	{

		if (pbServiceScriptFile -> append (
			"\techo \"Stopped\" >> "
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\treturn 0"
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}


		return errNoError;
	}

	Error Service:: appendStatusScriptCommand (
		Buffer_p pbServiceScriptFile)

	{

		if (pbServiceScriptFile -> append (
			"\techo \"Status\" >> "
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (pbServiceScriptFile -> append (
			"\n"
			"echo \"Service is running?"
			"\n"
			"\n"
			"\treturn 0"
			) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}


		return errNoError;
	}
#endif


Error Service:: unInstall (void)

{

	#ifdef WIN32
		SC_HANDLE			pscServiceControl;


		if ((pscServiceControl = ::OpenService (
			_pscmServiceControlManager, (const char *) _bServiceName,
			SERVICE_ALL_ACCESS)) == NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_OPENSERVICE_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (::DeleteService (pscServiceControl) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DELETESERVICE_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (::CloseServiceHandle (pscServiceControl) == 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_CLOSESERVICEHANDLE_FAILED,
					1, (long) GetLastError ());
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

    	if (::CloseServiceHandle (pscServiceControl) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_CLOSESERVICEHANDLE_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (onUnInstall () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONUNINSTALL_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}
	#else
		Buffer_t				bUnInstallServiceExecuteCommand;
		int						iExecuteCommandStatus;


		if (bUnInstallServiceExecuteCommand. init (
			"/usr/lib/lsb/remove_initd --del ") !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		if (bUnInstallServiceExecuteCommand. append (
			(const char *) _bServiceName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bUnInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				false, 2, pCurrentDateTime,
				"Execute command: ");
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				(const char *) bUnInstallServiceExecuteCommand);
		#endif

		if (ProcessUtility:: execute (
			(const char *) bUnInstallServiceExecuteCommand,
			&iExecuteCommandStatus) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_PROCESSUTILITY_EXECUTE_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bUnInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		if (iExecuteCommandStatus != 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_REMOVESERVICEEXECUTECOMMANDFAILED,
				1, (long) iExecuteCommandStatus);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (bUnInstallServiceExecuteCommand. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

			return err;
		}

		if (bUnInstallServiceExecuteCommand. finish () !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		{
			char					pServiceNamePathName [
				TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH];

			sprintf (pServiceNamePathName, "/etc/rc.d/init.d/%s",
				(const char *) _bServiceName);

			if (FileIO:: remove (pServiceNamePathName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_REMOVE_FAILED,
					1, pServiceNamePathName);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}
		}

		if (onUnInstall () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONUNINSTALL_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}
	#endif


	return errNoError;
}


Error Service:: parseArguments (int iArgc, char **pArgv,
	unsigned long *pulReservedArgumentsNumberFound)

{

	unsigned long				ulArgumentIndex;


	*pulReservedArgumentsNumberFound			= 0;

	for (ulArgumentIndex = 1; ulArgumentIndex < (unsigned long) iArgc;
		ulArgumentIndex++)
	{
		if (!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_INSTALL))
		{
			Boolean_t				bIsInstalled;


			(*pulReservedArgumentsNumberFound)			+= 1;

			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service installing...");
			#endif

			if (isInstalled (&bIsInstalled) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_ISINSTALLED_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			if (bIsInstalled)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_SERVICEALREADYINSTALLED,
					1, (const char *) _bServiceDescription);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			if (install (iArgc, pArgv) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_INSTALL_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			#ifdef SERVICEDEBUG
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service installed");
			#endif


		}
		else if (!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_UNINSTALL))
		{
			Boolean_t				bIsInstalled;


			(*pulReservedArgumentsNumberFound)			+= 1;

			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service un-installing...");
			#endif

			if (isInstalled (&bIsInstalled) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_ISINSTALLED_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			if (!bIsInstalled)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_SERVICENOTINSTALLED,
					1, (const char *) _bServiceDescription);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			if (unInstall () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_UNINSTALL_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return err;
			}

			#ifdef SERVICEDEBUG
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service un-installed");
			#endif

		}
		else if (!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_START))
		{
			// start the service using the API of the OS

			(*pulReservedArgumentsNumberFound)			+= 1;

			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					false, 2, pCurrentDateTime,
					"Service ");
				FileIO:: appendBuffer (_pServiceDebugFile,
					false, 2, pCurrentDateTime,
					(const char *) _bServiceName);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					" starting...");
			#endif

			#ifdef WIN32
				SC_HANDLE				pscServiceControl;


				if ((pscServiceControl = ::OpenService (
					_pscmServiceControlManager, (const char *) _bServiceName,
					SERVICE_ALL_ACCESS)) == NULL)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_OPENSERVICE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}

				if (::StartService (pscServiceControl, 0, NULL) == 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STARTSERVICE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (::CloseServiceHandle (pscServiceControl) == 0)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_CLOSESERVICEHANDLE_FAILED,
							1, (long) GetLastError ());
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (::CloseServiceHandle (pscServiceControl) == 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_CLOSESERVICEHANDLE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}
			#else
				Buffer_t				bStartServiceExecuteCommand;
				int						iExecuteCommandStatus;


				if (bStartServiceExecuteCommand. init (
					"/sbin/service ") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}

				if (bStartServiceExecuteCommand. append (
					(const char *) _bServiceName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStartServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (bStartServiceExecuteCommand. append (
					" start") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStartServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				#ifdef SERVICEDEBUG
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						false, 2, pCurrentDateTime,
						"Execute command: ");
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) bStartServiceExecuteCommand);
				#endif

				if (ProcessUtility:: execute (
					(const char *) bStartServiceExecuteCommand,
					&iExecuteCommandStatus) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_PROCESSUTILITY_EXECUTE_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStartServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (iExecuteCommandStatus != 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_SERVICE_STARTSERVICEEXECUTECOMMANDFAILED,
						1, (long) iExecuteCommandStatus);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStartServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (bStartServiceExecuteCommand. finish () !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}
			#endif

			#ifdef SERVICEDEBUG
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service started");
			#endif
		}
		else if (!strcmp (pArgv [ulArgumentIndex], TOOLS_SERVICE_TO_STOP))
		{
			// stop the service using the API of the OS

			(*pulReservedArgumentsNumberFound)			+= 1;

			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					false, 2, pCurrentDateTime,
					"Service ");
				FileIO:: appendBuffer (_pServiceDebugFile,
					false, 2, pCurrentDateTime,
					(const char *) _bServiceName);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					" stopping...");
			#endif

			#ifdef WIN32
				SC_HANDLE				pscServiceControl;
				SERVICE_STATUS			ssServiceStatus; 


				if ((pscServiceControl = ::OpenService (
					_pscmServiceControlManager, (const char *) _bServiceName,
					SERVICE_ALL_ACCESS)) == NULL)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_OPENSERVICE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}

				if (::ControlService (pscServiceControl, SERVICE_CONTROL_STOP,
					&ssServiceStatus) == 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_CONTROLSERVICE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (::CloseServiceHandle (pscServiceControl) == 0)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_CLOSESERVICEHANDLE_FAILED,
							1, (long) GetLastError ());
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (ssServiceStatus. dwCurrentState != SERVICE_STOPPED)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_SERVICE_STOPFAILED,
						1, (long) ssServiceStatus. dwCurrentState);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (::CloseServiceHandle (pscServiceControl) == 0)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_CLOSESERVICEHANDLE_FAILED,
							1, (long) GetLastError ());
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}
				else
				{
				}

				if (::CloseServiceHandle (pscServiceControl) == 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_CLOSESERVICEHANDLE_FAILED,
						1, (long) GetLastError ());
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}
			#else
				Buffer_t				bStopServiceExecuteCommand;
				int						iExecuteCommandStatus;


				if (bStopServiceExecuteCommand. init (
					"/sbin/service ") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}

				if (bStopServiceExecuteCommand. append (
					(const char *) _bServiceName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStopServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (bStopServiceExecuteCommand. append (
					" stop") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStopServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				#ifdef SERVICEDEBUG
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer (_pServiceDebugFile,
						false, 2, pCurrentDateTime,
						"Execute command: ");
					FileIO:: appendBuffer (_pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) bStopServiceExecuteCommand);
				#endif
				if (ProcessUtility:: execute (
					(const char *) bStopServiceExecuteCommand,
					&iExecuteCommandStatus) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_PROCESSUTILITY_EXECUTE_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStopServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (iExecuteCommandStatus != 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_SERVICE_STOPSERVICEEXECUTECOMMANDFAILED,
						1, (long) iExecuteCommandStatus);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					if (bStopServiceExecuteCommand. finish () !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer (_pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif
					}

					return err;
				}

				if (bStopServiceExecuteCommand. finish () !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer (_pServiceDebugFile,
							true, 2, pCurrentDateTime,
							(const char *) err);
					#endif

					return err;
				}
			#endif

			#ifdef SERVICEDEBUG
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"Service stopped");
			#endif
		}
	}


    return errNoError;
}


Error Service:: start (
	int iArgc, char **pArgv)

{


	#ifdef WIN32
		SERVICE_TABLE_ENTRY		dispatchTable [] =
		{
			{ (char *) ((const char *) _bServiceName), Service::ServiceMain},
			{NULL, NULL}
		};

		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"StartServiceCtrlDispatcher is calling...");
		#endif

		if (::StartServiceCtrlDispatcher (dispatchTable) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STARTSERVICECTRLDISPATCHER_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}

		#ifdef SERVICEDEBUG
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"StartServiceCtrlDispatcher called");
		#endif
	#else
		Error_t				errOnInit;
		Error_t				errOnStart;
		Error_t				errOnStop;


		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"onInit is calling...");
		#endif

		if ((errOnInit = onInit ()) != errNoError)
		{
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) errOnInit);
			#endif

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONINIT_FAILED);
			#ifdef SERVICEDEBUG
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

        	return errOnInit;
    	}

		#ifdef SERVICEDEBUG
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"onStart is calling...");
		#endif

    	if ((errOnStart = onStart ()) != errNoError)
		{
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) errOnStart);
			#endif

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONSTART_FAILED);
			#ifdef SERVICEDEBUG
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return errOnStart;
		}

		#ifdef SERVICEDEBUG
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer (_pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"onStop is calling...");
		#endif

    	if ((errOnStop = onStop ()) != errNoError)
		{
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) errOnStop);
			#endif

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONSTOP_FAILED);
			#ifdef SERVICEDEBUG
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return errOnStop;
		}
	#endif


	return errNoError;
}


#ifdef WIN32
	Error Service:: setStatus (DWORD dwControl)

	{

		SERVICE_STATUS			ssServiceStatus; 

		ssServiceStatus. dwServiceType			= SERVICE_WIN32_OWN_PROCESS;
		ssServiceStatus. dwControlsAccepted		=
			SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;
			// | SERVICE_ACCEPT_PAUSE_CONTINUE;
		ssServiceStatus. dwWin32ExitCode		= 0;
		ssServiceStatus. dwServiceSpecificExitCode		= 0;
		ssServiceStatus. dwCheckPoint			= 0;
		ssServiceStatus. dwWaitHint				= 0;

		ssServiceStatus. dwCurrentState			= dwControl;


    	if (!SetServiceStatus (_sshServiceStatusHandle, &ssServiceStatus)) 
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SETSERVICESTATUS_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer (_pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return err;
		}


		return errNoError;
	}
#endif


#ifdef WIN32
	VOID WINAPI Service:: ServiceMain (DWORD dwArgc,
		LPTSTR *lpszArgv)

	{

		Service_p			psService		= _pThis;
		Error_t				errOnInit;


		#ifdef SERVICEDEBUG
			tm					tmDateTime;
			unsigned long		ulMilliSecs;
			char				pCurrentDateTime [24 + 1];
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"RegisterServiceCtrlHandlerEx is calling...");
		#endif

		if ((psService -> _sshServiceStatusHandle =
			RegisterServiceCtrlHandlerEx (
			(const char *) (psService -> _bServiceName),
			handlerServiceEventsEx,
			(LPVOID) ((const char *) (psService -> _bServiceName)))) ==
			NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_REGISTERSERVICECTRLHANDLEREX_FAILED,
				1, (long) GetLastError ());
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return;
		}

		if (psService -> setStatus (SERVICE_START_PENDING) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_SETSTATUS_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return;
		}

		#ifdef SERVICEDEBUG
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"onInit is calling...");
		#endif

		if ((errOnInit = psService -> onInit ()) != errNoError)
		{
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) errOnInit);
			#endif

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONINIT_FAILED);
			#ifdef SERVICEDEBUG
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			if (psService -> setStatus (SERVICE_STOPPED) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_SETSTATUS_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif
			}

        	return; 
    	}

		if (psService -> setStatus (SERVICE_RUNNING) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_SETSTATUS_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return;
		}

		psService -> _bServiceRunning		= true;

		#ifdef SERVICEDEBUG
			DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
			sprintf (pCurrentDateTime,
				"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
				(unsigned long) (tmDateTime. tm_year + 1900),
				(unsigned long) (tmDateTime. tm_mon + 1),
				(unsigned long) (tmDateTime. tm_mday),
				(unsigned long) (tmDateTime. tm_hour),
				(unsigned long) (tmDateTime. tm_min),
				(unsigned long) (tmDateTime. tm_sec),
				ulMilliSecs);
			FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"onStart is calling...");
		#endif

    	if (psService -> onStart () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_ONSTART_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return;
		}

    	// Send current status. 
		if (psService -> setStatus (SERVICE_STOPPED) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_SETSTATUS_FAILED);
			#ifdef SERVICEDEBUG
				tm					tmDateTime;
				unsigned long		ulMilliSecs;
				char				pCurrentDateTime [24 + 1];
				DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
				sprintf (pCurrentDateTime,
					"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
					(unsigned long) (tmDateTime. tm_year + 1900),
					(unsigned long) (tmDateTime. tm_mon + 1),
					(unsigned long) (tmDateTime. tm_mday),
					(unsigned long) (tmDateTime. tm_hour),
					(unsigned long) (tmDateTime. tm_min),
					(unsigned long) (tmDateTime. tm_sec),
					ulMilliSecs);
				FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					(const char *) err);
			#endif

			return;
		}


    	return;
	}
#endif


#ifdef WIN32
	DWORD WINAPI Service:: handlerServiceEventsEx (
		DWORD dwControl,
		DWORD dwEventType,
		LPVOID lpEventData,
		LPVOID lpContext)

	{

		Service_p				psService;
		DWORD					dwStatus;


		psService			= _pThis;
		dwStatus			= SERVICE_RUNNING; 

		switch (dwControl)
		{
			case SERVICE_CONTROL_CONTINUE: 
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_CONTINUE control received"
							<< std:: endl;
					*/

					if (!(psService -> _bServicePaused))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SERVICENOTINPAUSE);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					if (psService -> onContinue () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONCONTINUE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					psService -> _bServicePaused		= false;
				}

				break;
			case SERVICE_CONTROL_INTERROGATE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_INTERROGATE control received"
							<< std:: endl;
					*/


					if (psService -> onInterrogate () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONINTERROGATE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_NETBINDADD:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_NETBINDADD control received"
							<< std:: endl;
					*/


					if (psService -> onNetBindAdd () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONNETBINDADD_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_NETBINDDISABLE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_NETBINDDISABLE control received"
							<< std:: endl;
					*/


					if (psService -> onNetBindDisable () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONNETBINDDISABLE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_NETBINDENABLE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_NETBINDENABLE control received"
							<< std:: endl;
					*/


					if (psService -> onNetBindEnable () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONNETBINDENABLE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_NETBINDREMOVE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_NETBINDREMOVE control received"
							<< std:: endl;
					*/


					if (psService -> onNetBindRemove () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONNETBINDREMOVE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_PARAMCHANGE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_PARAMCHANGE control received"
							<< std:: endl;
					*/


					if (psService -> onParamChange () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONPARAMCHANGE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_PAUSE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_PAUSE control received"
							<< std:: endl;
					*/


					if (psService -> _bServicePaused)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SERVICEALREADYINPAUSE);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					if (psService -> onPause () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONPAUSE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					psService -> _bServicePaused			= true;
					dwStatus								= SERVICE_PAUSED; 
				}

				break;		
			case SERVICE_CONTROL_SHUTDOWN:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_SHUTDOWN control received"
							<< std:: endl;
					*/


					if (psService -> onShutdown () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONSHUTDOWN_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					if (!(psService -> _bServiceRunning))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SERVICEALREADYSTOPPED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					if (psService -> setStatus (SERVICE_STOP_PENDING) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SETSTATUS_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
							true, 2, pCurrentDateTime,
							"onStop is calling...");
					#endif

					if (psService -> onStop () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONSTOP_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					psService -> _bServiceRunning	= false;
				}

				break;
			case SERVICE_CONTROL_STOP:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_STOP control received"
							<< std:: endl;
					*/


					if (!(psService -> _bServiceRunning))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SERVICEALREADYSTOPPED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					if (psService -> setStatus (SERVICE_STOP_PENDING) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_SETSTATUS_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					#ifdef SERVICEDEBUG
						tm					tmDateTime;
						unsigned long		ulMilliSecs;
						char				pCurrentDateTime [24 + 1];
						DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
						sprintf (pCurrentDateTime,
							"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
							(unsigned long) (tmDateTime. tm_year + 1900),
							(unsigned long) (tmDateTime. tm_mon + 1),
							(unsigned long) (tmDateTime. tm_mday),
							(unsigned long) (tmDateTime. tm_hour),
							(unsigned long) (tmDateTime. tm_min),
							(unsigned long) (tmDateTime. tm_sec),
							ulMilliSecs);
						FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
							true, 2, pCurrentDateTime,
							"onStop is calling...");
					#endif

					if (psService -> onStop () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONSTOP_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}

					psService -> _bServiceRunning	= false;
				}

				break;
			case SERVICE_CONTROL_DEVICEEVENT:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_DEVICEEVENT control received"
							<< std:: endl;
					*/


					if (psService -> onDeviceEvent (dwEventType,
						lpEventData) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONDEVICEEVENT_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_HARDWAREPROFILECHANGE:
				{
					/*
						std:: cout
					<< "SERVICE_CONTROL_HARDWAREPROFILECHANGE control received"
							<< std:: endl;
					*/


					if (psService -> onHardwareProfileChange (
						dwEventType) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONHARDWAREPROFILECHANGE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_POWEREVENT:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_POWEREVENT control received"
							<< std:: endl;
					*/


					if (psService -> onPowerEvent (dwEventType,
						lpEventData) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONPOWEREVENT_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			case SERVICE_CONTROL_SESSIONCHANGE:
				{
					/*
						std:: cout
							<< "SERVICE_CONTROL_SESSIONCHANGE control received"
							<< std:: endl;
					*/


					if (psService -> onSessionChange () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_SERVICE_ONSESSIONCHANGE_FAILED);
						#ifdef SERVICEDEBUG
							tm					tmDateTime;
							unsigned long		ulMilliSecs;
							char				pCurrentDateTime [24 + 1];
							DateTime:: get_tm_LocalTime (&tmDateTime,
								&ulMilliSecs);
							sprintf (pCurrentDateTime,
								"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
								(unsigned long) (tmDateTime. tm_year + 1900),
								(unsigned long) (tmDateTime. tm_mon + 1),
								(unsigned long) (tmDateTime. tm_mday),
								(unsigned long) (tmDateTime. tm_hour),
								(unsigned long) (tmDateTime. tm_min),
								(unsigned long) (tmDateTime. tm_sec),
								ulMilliSecs);
							FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
								true, 2, pCurrentDateTime,
								(const char *) err);
						#endif

						return NO_ERROR;
					}
				}

				break;
			default:
				{
					/*
						std:: cout
							<< "UNKNOWN CONTROL control received"
							<< std:: endl;
					*/


					if (dwControl >= TOOLS_SERVICE_CONTROL_USER) 
					{
						if (psService -> onUserControl (dwControl) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_SERVICE_ONSESSIONCHANGE_FAILED);
							#ifdef SERVICEDEBUG
								tm					tmDateTime;
								unsigned long		ulMilliSecs;
								char				pCurrentDateTime [24 + 1];
								DateTime:: get_tm_LocalTime (&tmDateTime,
									&ulMilliSecs);
								sprintf (pCurrentDateTime,
									"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
									(unsigned long) (tmDateTime. tm_year + 1900),
									(unsigned long) (tmDateTime. tm_mon + 1),
									(unsigned long) (tmDateTime. tm_mday),
									(unsigned long) (tmDateTime. tm_hour),
									(unsigned long) (tmDateTime. tm_min),
									(unsigned long) (tmDateTime. tm_sec),
									ulMilliSecs);
								FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
									true, 2, pCurrentDateTime,
									(const char *) err);
							#endif

							return NO_ERROR;
						}
					}
					else 
					{
					}
				}

				break;
		}

		if (psService -> _bServiceRunning)
		{
			if (psService -> setStatus (dwStatus) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_SETSTATUS_FAILED);
				#ifdef SERVICEDEBUG
					tm					tmDateTime;
					unsigned long		ulMilliSecs;
					char				pCurrentDateTime [24 + 1];
					DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
					sprintf (pCurrentDateTime,
						"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
						(unsigned long) (tmDateTime. tm_year + 1900),
						(unsigned long) (tmDateTime. tm_mon + 1),
						(unsigned long) (tmDateTime. tm_mday),
						(unsigned long) (tmDateTime. tm_hour),
						(unsigned long) (tmDateTime. tm_min),
						(unsigned long) (tmDateTime. tm_sec),
						ulMilliSecs);
					FileIO:: appendBuffer ("C:\\ServiceDebug.log", // _pServiceDebugFile,
						true, 2, pCurrentDateTime,
						(const char *) err);
				#endif

				return NO_ERROR;
			}
		}


		return NO_ERROR;
	}
#endif


Error Service:: onStart (void)

{

	_bIsShutdown		= false;

	while (!_bIsShutdown)
	{
		#ifdef WIN32
			Sleep (2000);
		#else
			sleep (2);
		#endif

		tm					tmDateTime;
		unsigned long		ulMilliSecs;
		char				pCurrentDateTime [24 + 1];
		DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs);
		sprintf (pCurrentDateTime,
			"%04lu-%02lu-%02lu-%02lu:%02lu:%02lu.%03lu ",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday),
			(unsigned long) (tmDateTime. tm_hour),
			(unsigned long) (tmDateTime. tm_min),
			(unsigned long) (tmDateTime. tm_sec),
			ulMilliSecs);
		FileIO:: appendBuffer (_pServiceDebugFile,
			true, 2, pCurrentDateTime,
			"I'm running");
	}


	return errNoError;
}


Error Service:: onStop (void)

{
	_bIsShutdown		= true;

	#ifdef WIN32
		Sleep (2000 + 1000);
	#else
		sleep (2 + 1);
	#endif


	return errNoError;
}


Error Service:: onInstall (void)

{

	return errNoError;
}


Error Service:: onUnInstall (void)

{

	return errNoError;
}


Error Service:: onInit (void)

{

	return errNoError;
}


#ifdef WIN32
	Error Service:: onUserControl (DWORD dwControl)

	{

		return errNoError;
	}


	Error Service:: onContinue (void)

	{

		return errNoError;
	}


	Error Service:: onInterrogate (void)

	{

		return errNoError;
	}


	Error Service:: onShutdown (void)

	{

		return errNoError;
	}


	Error Service:: onSessionChange (void)

	{

		return errNoError;
	}


	Error Service:: onPause (void)

	{

		return errNoError;
	}



	Error Service:: onParamChange (void)

	{

		return errNoError;
	}


	Error Service:: onNetBindAdd (void)

	{

		return errNoError;
	}


	Error Service:: onNetBindRemove (void)

	{

		return errNoError;
	}


	Error Service:: onNetBindEnable (void)

	{

		return errNoError;
	}


	Error Service:: onNetBindDisable (void)

	{

		return errNoError;
	}


	Error Service:: onDeviceEvent (DWORD dwEventType, LPVOID pvEventData)

	{

		return errNoError;
	}


	Error Service:: onPowerEvent (DWORD dwEventType, LPVOID pvEventData)

	{

		return errNoError;
	}


	Error Service:: onHardwareProfileChange (DWORD dwEventType)

	{

		return errNoError;
	}
#endif


