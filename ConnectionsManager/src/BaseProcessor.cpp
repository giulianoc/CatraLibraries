
#include "BaseProcessor.h"
#include "ConnectionsManagerMessages.h"
#include "FileIO.h"
#include "DateTime.h"



#ifdef WIN32
	BaseProcessor:: BaseProcessor (void): WinThread ()
#else
	BaseProcessor:: BaseProcessor (void): PosixThread ()
#endif

{

}


BaseProcessor:: ~BaseProcessor (void)

{

}


Error BaseProcessor:: init (
	unsigned long ulProcessorIdentifier,
	const char *pProcessorName,
	ConfigurationFile_p pcfConfiguration,
	Scheduler_p pscScheduler,
	CMEventsSet_p pesEventsSet,
	unsigned long ulMaxMilliSecondsToProcessAnEvent,
	Tracer_p ptTracer)

{

	if (pProcessorName == (const char *) NULL ||
		pcfConfiguration == (ConfigurationFile_p) NULL ||
		pscScheduler == (Scheduler_p) NULL ||
		pesEventsSet == (CMEventsSet_p) NULL ||
		ptTracer == (Tracer_p) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_ACTIVATION_WRONG);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	_pcfConfiguration				= pcfConfiguration;
	_ulProcessorIdentifier			= ulProcessorIdentifier;
	_pscScheduler					= pscScheduler;
	_pesEventsSet					= pesEventsSet;
	_ulMaxMilliSecondsToProcessAnEvent	= ulMaxMilliSecondsToProcessAnEvent;
	_ptSystemTracer					= ptTracer;

	if (_bProcessorName. init (pProcessorName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_bMainProcessor. init ("SBProcessor") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#if defined(__CYGWIN__)
		if (_mtShutdown. init (PMutex:: MUTEX_RECURSIVE) !=
			errNoError)
	#else							// POSIX.1-1996 standard (HPUX 11)
		if (_mtShutdown. init (PMutex:: MUTEX_RECURSIVE) !=
			errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

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
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtShutdown. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}


	return errNoError;
}


Error BaseProcessor:: finish ()

{

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_mtShutdown. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error BaseProcessor:: run (void)

{

	Boolean_t			bIsShutdown;
	Error_t				errGetAndRemoveEvent;
	Error_t				errProcessEvent;
	Event_p				pevEvent;
	unsigned long		ulEventsCounter;
	#ifdef NO_CONDITION_VARIABLE
		Boolean_t		bEventFound;
	#endif
	unsigned long		ulInternalTimeInMilliSecsToProcessEvent;

	unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs2;
	unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs3;

	#ifdef PROCESSOR_STATISTICS
		static unsigned long long
			ullLocalExpirationLocalDateTimeInMilliSecs0 = 0;
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
		char						pBuffer [1024];
	#endif
	char						pTypeIdentifier [
		CM_PROCESSOR_MAXTYPEIDENTIFIERLENGTH];


	bIsShutdown					= false;
	if (setIsShutdown (bIsShutdown) != errNoError)
	{
		_erThreadReturn = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_SETISSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) _erThreadReturn, __FILE__, __LINE__);

		return _erThreadReturn;
	}

	ulEventsCounter			= 1;
	#ifdef NO_CONDITION_VARIABLE
		bEventFound			= true;
	#endif

/* GIU TEST PER VERIFICARE BUG WINDOWS
unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs2;
*/
	while (!bIsShutdown)
	{
		#ifdef PROCESSOR_STATISTICS
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullLocalExpirationLocalDateTimeInMilliSecs1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (ullLocalExpirationLocalDateTimeInMilliSecs0 == 0)
				ullLocalExpirationLocalDateTimeInMilliSecs0		=
					ullLocalExpirationLocalDateTimeInMilliSecs1;
			else
			{
				Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
					CM_PROCESSOR_STATISTICS,
					2, "Difference with previous event: ", (long)
					(ullLocalExpirationLocalDateTimeInMilliSecs1 -
						ullLocalExpirationLocalDateTimeInMilliSecs0));
				_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
					(const char *) msg, __FILE__, __LINE__);

				ullLocalExpirationLocalDateTimeInMilliSecs0		=
					ullLocalExpirationLocalDateTimeInMilliSecs1;
			}
		}
		#endif

/* GIU TEST PER VERIFICARE BUG WINDOWS
DateTime:: nowLocalInMilliSecs (&ullLocalExpirationLocalDateTimeInMilliSecs1);
*/
		#ifdef NO_CONDITION_VARIABLE
			if (!bEventFound)
			{
				#ifdef WIN32
					if (WinThread:: getSleep (
						CM_PROCESSOR_SLEEPTIMESECSWAITINGEVENT,
						CM_PROCESSOR_SLEEPTIMEMILLISECSWAITINGEVENT * 1000) !=
						errNoError)
				#else
					if (PosixThread:: getSleep (
						CM_PROCESSOR_SLEEPTIMESECSWAITINGEVENT,
						CM_PROCESSOR_SLEEPTIMEMILLISECSWAITINGEVENT * 1000) !=
						errNoError)
				#endif
				{
					_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_GETSLEEP_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) _erThreadReturn,
						__FILE__, __LINE__);

					return _erThreadReturn;
				}
			}

			bEventFound			= false;
		#endif

		if ((errGetAndRemoveEvent =
			_pesEventsSet -> getAndRemoveFirstEvent (
			&_bProcessorName, &pevEvent, true,
			CM_PROCESSOR_SLEEPTIMESECSWAITINGEVENT,
			CM_PROCESSOR_SLEEPTIMEMILLISECSWAITINGEVENT)) !=
			errNoError)
		{
			if ((long) errGetAndRemoveEvent ==
				EVSET_EVENTSSET_DESTINATIONNOTFOUND ||
			(long) errGetAndRemoveEvent == EVSET_EVENTSSET_NOEVENTSFOUND ||
				(long) errGetAndRemoveEvent ==
				EVSET_EVENTSSET_EVENTNOTEXPIREDYET)
			{
				#ifdef PROCESSOR_STATISTICS
				{
					if (DateTime:: nowLocalInMilliSecs (
						&ullLocalExpirationLocalDateTimeInMilliSecs2) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
						CM_PROCESSOR_STATISTICS,
						2,
						"getAndRemoveFirstEvent time (no event): ", (long)
						(ullLocalExpirationLocalDateTimeInMilliSecs2 -
							ullLocalExpirationLocalDateTimeInMilliSecs1));
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
						(const char *) msg, __FILE__, __LINE__);
				}
				#endif

				if ((long) errGetAndRemoveEvent ==
					EVSET_EVENTSSET_DESTINATIONNOTFOUND)
				{
/* GIU
{
char pBuffer [1024];
sprintf (pBuffer, "Processor: %lu, DEST NOT FOUND (%s)",
	_ulProcessorIdentifier, (const char *) _bMainProcessor);
_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
	pBuffer, __FILE__, __LINE__);
}
*/
					// since the destination are already preallocated we
					// shouldn't arrive here
					#ifdef WIN32
						if (WinThread:: getSleep (0, 700) != errNoError)
					#else
						if (PosixThread:: getSleep (0, 100) != errNoError)
					#endif
					{
						_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PTHREAD_GETSLEEP_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) _erThreadReturn,
							__FILE__, __LINE__);

						return _erThreadReturn;
					}
				}
/* GIU TEST PER VERIFICARE BUG WINDOWS
else
{
// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
// 	(const char *) errGetAndRemoveEvent,
// 	__FILE__, __LINE__);
DateTime:: nowLocalInMilliSecs (&ullLocalExpirationLocalDateTimeInMilliSecs2);
char pBuffer [1024];
sprintf (pBuffer, "Processor: %lu, Time for getAndRemove without events: %ld",
	_ulProcessorIdentifier, (long) (ullLocalExpirationLocalDateTimeInMilliSecs2 -
		ullLocalExpirationLocalDateTimeInMilliSecs1));
_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
	pBuffer, __FILE__, __LINE__);
}
*/
				if (getIsShutdown (&bIsShutdown) != errNoError)
				{
					_erThreadReturn = ConnectionsManagerErrors (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_GETISSHUTDOWN_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) _erThreadReturn, __FILE__, __LINE__);

					return _erThreadReturn;
				}

				continue;
			}
			else
			{
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errGetAndRemoveEvent,
					__FILE__, __LINE__);

				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_GETANDREMOVEFIRSTEVENT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) _erThreadReturn, __FILE__, __LINE__);

				return _erThreadReturn;
			}
		}

/* GIU TEST PER VERIFICARE BUG WINDOWS
{
DateTime:: nowLocalInMilliSecs (&ullLocalExpirationLocalDateTimeInMilliSecs2);
char pBuffer [1024];
sprintf (pBuffer, "Processor: %lu, Time for getAndRemove with events: %ld",
	_ulProcessorIdentifier, (long) (ullLocalExpirationLocalDateTimeInMilliSecs2 -
		ullLocalExpirationLocalDateTimeInMilliSecs1));
_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
	pBuffer, __FILE__, __LINE__);
}
*/

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		#ifdef PROCESSOR_STATISTICS
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, "getAndRemoveFirstEvent time (found event): ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif

		#ifdef NO_CONDITION_VARIABLE
			bEventFound			= true;
		#endif

		if (strlen (pevEvent -> getTypeIdentifier ()) >=
			CM_PROCESSOR_MAXTYPEIDENTIFIERLENGTH)
		{
			strncpy (pTypeIdentifier,
				pevEvent -> getTypeIdentifier (),
				CM_PROCESSOR_MAXTYPEIDENTIFIERLENGTH - 4);
			pTypeIdentifier [CM_PROCESSOR_MAXTYPEIDENTIFIERLENGTH - 4]	='\0';
			strcat (pTypeIdentifier, "...");
		}
		else
		{
			strcpy (pTypeIdentifier, pevEvent -> getTypeIdentifier ());
		}

		ulInternalTimeInMilliSecsToProcessEvent		= 0;

		if ((errProcessEvent = processEvent (pevEvent,
			&ulInternalTimeInMilliSecsToProcessEvent)) != errNoError)
		{
			// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			// 	CM_PROCESSOR_PROCESSEVENT_FAILED,
			// 	1, _ulProcessorIdentifier);
			// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			// 	(const char *) err, __FILE__, __LINE__);

			// delete pevEvent;

			// return err;
		}

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs3) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (ullLocalExpirationLocalDateTimeInMilliSecs3 -
			ullLocalExpirationLocalDateTimeInMilliSecs2 >=
			_ulMaxMilliSecondsToProcessAnEvent)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_TOOMANYTIMETOMANAGETHEEVENT,
				3, pTypeIdentifier,
				(unsigned long) (ullLocalExpirationLocalDateTimeInMilliSecs3 -
					ullLocalExpirationLocalDateTimeInMilliSecs2),
					ulInternalTimeInMilliSecsToProcessEvent);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		#ifdef PROCESSOR_STATISTICS
		{
			sprintf (pBuffer, "processEvents time (event: %s): ",
				pTypeIdentifier);

			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, pBuffer, (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs3 -
					ullLocalExpirationLocalDateTimeInMilliSecs2));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif

		if (!(ulEventsCounter % 25))
		{
			if (getIsShutdown (&bIsShutdown) != errNoError)
			{
				_erThreadReturn = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_GETISSHUTDOWN_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) _erThreadReturn, __FILE__, __LINE__);

				return _erThreadReturn;
			}

			if (bIsShutdown)
				break;

			ulEventsCounter			= 1;
		}

		ulEventsCounter++;
	}


	return _erThreadReturn;
}


Error BaseProcessor:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t		stRTPThreadState;
	#else
		PosixThread:: PThreadStatus_t	stRTPThreadState;
	#endif


	if (setIsShutdown (true) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_SETISSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (getThreadState (&stRTPThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	tUTCNow					= time (NULL);

	while (stRTPThreadState == THREADLIB_STARTED ||
		stRTPThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		if (time (NULL) - tUTCNow >= 5)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (getThreadState (&stRTPThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}

	if (stRTPThreadState == THREADLIB_STARTED ||
		stRTPThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		#ifdef WIN32
			// no cancel for windows thread
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		#endif
	}


	return errNoError;
}


Error BaseProcessor:: getIsShutdown (
	Boolean_p pbIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	*pbIsShutdown				= _bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	return errNoError;
}


Error BaseProcessor:: setIsShutdown (
	Boolean_t bIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	_bIsShutdown			= bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	return errNoError;
}


Error BaseProcessor:: startTracer (
	ConfigurationFile_p pcfConfiguration,
	Tracer_p ptTracer, const char *pSectionName)

{

	char								pCacheSizeOfTraceFile [
		CM_MAXLONGLENGTH];
	char								pTraceDirectory [
		CM_MAXTRACEFILELENGTH];
	char								pTraceFileName [
		CM_MAXTRACEFILELENGTH];
	Error_t								errGetItemValue;
	Error_t								errCreateDir;
	char								pMaxTraceFileSize [
		CM_MAXLONGLENGTH];
	char								pTraceFilePeriodInSecs [
		CM_MAXLONGLENGTH];
	char								pCompressedTraceFile [
		CM_MAXBOOLEANLENGTH];
	Boolean_t							bCompressedTraceFile;
	char								pTraceFilesNumberToMaintain [
		CM_MAXLONGLENGTH];
	long								lTraceFilesNumberToMaintain;
	char								pTraceOnTTY [
		CM_MAXBOOLEANLENGTH];
	Boolean_t							bTraceOnTTY;
	char								pTraceLevel [
		CM_MAXLONGLENGTH];
	long								lTraceLevel;
	char								pListenTracePort [
		CM_MAXLONGLENGTH];
	unsigned long						ulListenTracePort;
	long								lSecondsBetweenTwoCheckTraceToManage;
	Error_t								errInit;


	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"CacheSizeOfTraceFile", pCacheSizeOfTraceFile,
		CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "CacheSizeOfTraceFile");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceDirectory", pTraceDirectory,
		CM_MAXTRACEFILELENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceDirectory");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFileName", pTraceFileName,
		CM_MAXTRACEFILELENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFileName");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	#ifdef WIN32
		if ((errCreateDir = FileIO:: createDirectory (pTraceDirectory,
			0, true, true)) != errNoError)
	#else
		if ((errCreateDir = FileIO:: createDirectory (pTraceDirectory,
			S_IRUSR | S_IWUSR | S_IXUSR |
			S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, true, true)) != errNoError)
	#endif
	{
		// std:: cerr << (const char *) errCreateDir << std:: endl;

		return errCreateDir;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"MaxTraceFileSize", pMaxTraceFileSize,
		CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "MaxTraceFileSize");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFilePeriodInSecs", pTraceFilePeriodInSecs,
		CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFilePeriodInSecs");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"CompressedTraceFile", pCompressedTraceFile,
		CM_MAXBOOLEANLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "CompressedTraceFile");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	if (!strcmp (pCompressedTraceFile, "true"))
		bCompressedTraceFile				= true;
	else
		bCompressedTraceFile				= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFilesNumberToMaintain",
		pTraceFilesNumberToMaintain,
		CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFilesNumberToMaintain");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	lTraceFilesNumberToMaintain			= atol (pTraceFilesNumberToMaintain);

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceOnTTY", pTraceOnTTY, CM_MAXBOOLEANLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceOnTTY");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	if (!strcmp (pTraceOnTTY, "true"))
		bTraceOnTTY							= true;
	else
		bTraceOnTTY							= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceLevel", pTraceLevel, CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceLevel");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (!strcmp (pTraceLevel, "LDBG1"))
		lTraceLevel				= 0;
	else if (!strcmp (pTraceLevel, "LDBG2"))
		lTraceLevel				= 1;
	else if (!strcmp (pTraceLevel, "LDBG3"))
		lTraceLevel				= 2;
	else if (!strcmp (pTraceLevel, "LDBG4"))
		lTraceLevel				= 3;
	else if (!strcmp (pTraceLevel, "LDBG5"))
		lTraceLevel				= 4;
	else if (!strcmp (pTraceLevel, "LDBG6"))
		lTraceLevel				= 5;
	else if (!strcmp (pTraceLevel, "LINFO"))
		lTraceLevel				= 6;
	else if (!strcmp (pTraceLevel, "LMESG"))
		lTraceLevel				= 7;
	else if (!strcmp (pTraceLevel, "LWRNG"))
		lTraceLevel				= 8;
	else if (!strcmp (pTraceLevel, "LERRR"))
		lTraceLevel				= 9;
	else if (!strcmp (pTraceLevel, "LFTAL"))
		lTraceLevel				= 10;
	else
		lTraceLevel				= 6;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"ListenTracePort", pListenTracePort, CM_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "ListenTracePort");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	ulListenTracePort		= atol (pListenTracePort);

	lSecondsBetweenTwoCheckTraceToManage		= 2;

	if ((errInit = ptTracer -> init (
		pSectionName,					// pName
		atol (pCacheSizeOfTraceFile),	// lCacheSizeOfTraceFile K-byte
		pTraceDirectory,				// pTraceDirectory
		pTraceFileName,					// pTraceFileName
		atol (pMaxTraceFileSize),		// lMaxTraceFileSize K-byte
		atol (pTraceFilePeriodInSecs),	// lTraceFilePeriodInSecs
		bCompressedTraceFile,			// bCompressedTraceFile
		false,							// bClosedFileToBeCopied
		(const char *) NULL,			// pClosedFilesRepository
		lTraceFilesNumberToMaintain,	// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		bTraceOnTTY,					// bTraceOnTTY
		lTraceLevel,					// lTraceLevel
		lSecondsBetweenTwoCheckTraceToManage,
		3000,							// lMaxTracesNumber
		ulListenTracePort,				// lListenPort
		1000,							// lTracesNumberAllocatedOnOverflow
		1000)) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{
		// Error err = TracerErrors (__FILE__, __LINE__,
		// 	TRACER_TRACER_INIT_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		return errInit;
	}

	/*
	{
		long		lStackSize;
		char		pBuff [1024];

		if (ptTracer -> getStackSize (&lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}
		sprintf (pBuff, "Stack size prima: %ld", lStackSize);
		// std:: cout << pBuff << std:: endl;

		lStackSize		= 20480 * 1024;
		if (ptTracer -> setStackSize (lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		if (ptTracer -> getStackSize (&lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}
		sprintf (pBuff, "Stack size dopo: %ld", lStackSize);
		// std:: cout << pBuff << std:: endl;
	}
	*/

	if (ptTracer -> start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		if (ptTracer -> finish (true) != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FINISH_FAILED);	
			// std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}


	return errNoError;
}


Error BaseProcessor:: stopTracer (Tracer_p ptTracer)

{

	if (ptTracer -> cancel () != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_CANCEL_FAILED);	
		ptTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	if (ptTracer -> finish (true) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_FINISH_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}


