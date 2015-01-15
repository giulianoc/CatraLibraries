
#include "TracerService.h"
#include "stdlib.h"

#include "FileIO.h"
#include "DateTime.h"
#include <sys/types.h>
#include <sys/stat.h>

#define VERSION			"1.0"


int main (int iArgc, char *pArgv [])

{

	Error_t								errService;
	TracerService_t						tsTracerService;
	unsigned long						ulReservedArgumentsNumberFound;
	const char							*pConfigurationPathName;


	if (iArgc > 2 ||
		(iArgc == 2 && !strcmp (pArgv [1], "--h")) ||
		(iArgc == 2 && !strcmp (pArgv [1], "-h")) ||
		(iArgc == 2 && !strcmp (pArgv [1], "-v")) ||
		(iArgc == 2 && !strcmp (pArgv [1], "--help")) ||
		(iArgc == 2 && !strcmp (pArgv [1], "-help")))
	{
		if (!strcmp (pArgv [1], "-v"))
		{
			std:: cout << pArgv [0] << " Version: " << VERSION
				<< std:: endl;
		}
		else
		{
			std:: cerr << "Usage: " << pArgv [0]
				<< " [ -i | -u | -h | -start | -stop | -nodaemon ]"
				<< std:: endl;
		}

		return 1;
	}

	#ifdef WIN32
		// With this API, when you call Sleep (1), it will truly sleep for
		// just 1 millisecond rather than the default 10 milliseconds! In
		// fact, by default, the maximum precision in most of the Windows OS
		// is 10ms. Note that calling timeBeginPeriod() also affects the
		// granularity of some other timing calls, such as
		// WaitForSingleObject()
		if (timeBeginPeriod (1) != TIMERR_NOERROR)
		{
			std:: cerr << "timeBeginPeriod failed" << std:: endl;
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
			FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"timeBeginPeriod failed");

			return 1;
		}
	#endif

	#ifdef WIN32
	#else
		FileIO:: changePermission (tsTracerService. _pServiceDebugFile,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	#endif

	#ifdef WIN32
	#else
		if (iArgc == 1)
		{
			// no parameter
			if ((errService = Service:: launchUnixDaemon (
				TS_TRACERSERVICE_PIDFILEPATHNAME)) != errNoError)
			{
				std:: cerr << (const char *) errService << std:: endl;

				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_LAUNCHUNIXDAEMON_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				return 1;
			}
		}
		/*
		else
		{
			char				pBuffer [1024];

			sprintf (pBuffer, "Parameters. iArgc: %ld, pArgv [1]: %s",
				(long) iArgc, pArgv [1]);
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
			FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				pBuffer);
		}
		*/
	#endif

	if ((pConfigurationPathName =
		getenv (TS_TRACERSERVICE_CONFPATHNAMEENVIRONMENTVARIABLE)) ==
		(char *) NULL)
	{
		Error err = TracerServerErrors (__FILE__, __LINE__,
			TS_TRACERSERVICE_ENVIRONMENTVARIABLE_NOTDEFINED,
			1, TS_TRACERSERVICE_CONFPATHNAMEENVIRONMENTVARIABLE);
		std:: cerr << (const char *) err << std:: endl;
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
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) errService);

		#ifdef WIN32
			if (timeEndPeriod (1) != TIMERR_NOERROR)
			{
				std:: cerr << "timeEndPeriod failed" << std:: endl;
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
				FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"timeEndPeriod failed");
			}
		#endif

		return 1;
	}

	if ((errService = tsTracerService. init (
		TS_TRACERSERVICE_SERVICENAME,
		VERSION,
		pConfigurationPathName,
		TS_TRACERSERVICE_PIDFILEPATHNAME)) != errNoError)
	{
		std:: cerr << (const char *) errService << std:: endl;
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
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) errService);

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_INIT_FAILED);
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) err);

		#ifdef WIN32
			if (timeEndPeriod (1) != TIMERR_NOERROR)
			{
				std:: cerr << "timeEndPeriod failed" << std:: endl;
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
				FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"timeEndPeriod failed");
			}
		#endif

		return 1;
	}

	#ifdef WIN32
	#else
		FileIO:: changePermission (tsTracerService. _pServiceDebugFile,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	#endif

	if ((errService = tsTracerService. parseArguments (
		iArgc, pArgv, &ulReservedArgumentsNumberFound)) != errNoError)
	{
		std:: cerr << (const char *) errService << std:: endl;
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
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) errService);

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_PARSEARGUMENTS_FAILED);
		std:: cerr << (const char *) err << std:: endl;
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) err);

		if (tsTracerService. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		#ifdef WIN32
			if (timeEndPeriod (1) != TIMERR_NOERROR)
			{
				std:: cerr << "timeEndPeriod failed" << std:: endl;
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
				FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"timeEndPeriod failed");
			}
		#endif

		return 1;
	}

	if (ulReservedArgumentsNumberFound == 0)
	{
		if ((errService = tsTracerService. start (iArgc, pArgv)) !=
			errNoError)
		{
			std:: cerr << (const char *) errService << std:: endl;
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
			FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				(const char *) errService);

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SERVICE_START_FAILED);
			std:: cerr << (const char *) err << std:: endl;
			FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				(const char *) err);

			if (tsTracerService. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SERVICE_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			#ifdef WIN32
				if (timeEndPeriod (1) != TIMERR_NOERROR)
				{
					std:: cerr << "timeEndPeriod failed" << std:: endl;
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
					FileIO:: appendBuffer (
						tsTracerService. _pServiceDebugFile,
						true, 2, pCurrentDateTime,
						"timeEndPeriod failed");
				}
			#endif

			return 1;
		}
	}

	if (tsTracerService. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;
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
		FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
			true, 2, pCurrentDateTime,
			(const char *) err);

		#ifdef WIN32
			if (timeEndPeriod (1) != TIMERR_NOERROR)
			{
				std:: cerr << "timeEndPeriod failed" << std:: endl;
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
				FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
					true, 2, pCurrentDateTime,
					"timeEndPeriod failed");
			}
		#endif

		return 1;
	}

	#ifdef WIN32
		if (timeEndPeriod (1) != TIMERR_NOERROR)
		{
			std:: cerr << "timeEndPeriod failed" << std:: endl;
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
			FileIO:: appendBuffer (tsTracerService. _pServiceDebugFile,
				true, 2, pCurrentDateTime,
				"timeEndPeriod failed");

			return 1;
		}
	#endif

	return 0;
}

