
#include "CheckServerSocketTimes.h"
#include "ConnectionEvent.h"
#include "ConnectionsManagerMessages.h"
#include "DateTime.h"
#include <assert.h>


CheckServerSocketTimes:: CheckServerSocketTimes (void): Times ()

{

}


CheckServerSocketTimes:: ~CheckServerSocketTimes (void)

{

}


CheckServerSocketTimes:: CheckServerSocketTimes (
	const CheckServerSocketTimes &t)

{

	assert (1==0);

	// to do

	*this = t;
}


Error CheckServerSocketTimes:: init (
	unsigned long ulPeriodInMilliSecs,
	CMEventsSet_p pesEventsSet,
	Tracer_p ptTracer)

{

	_pesEventsSet				= pesEventsSet;
	_ptSystemTracer				= ptTracer;

	if (_bMainProcessor. init ("CMProcessor") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (Times:: init (ulPeriodInMilliSecs,
		CM_CHECKSERVERSOCKETTIMES_CLASSNAME) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bMainProcessor. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}


	return errNoError;
}


Error CheckServerSocketTimes:: finish (void)

{

	if (Times:: finish () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_bMainProcessor. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error CheckServerSocketTimes:: handleTimeOut (void)

{

	Event_p							pevEvent;
	Error_t						errGetFreeEvent;
	#ifdef CHECKSERVERSOCKET_STATISTICS
		unsigned long long		ullLocalDateTimeInMilliSecs;
		static unsigned long long
			ullLocalDateTimeInMilliSecs0 = 0;
		unsigned long long		ullLocalDateTimeInMilliSecs1;
	#endif
	unsigned long				ulCurrentExpirationYear;
	unsigned long				ulCurrentExpirationMon;
	unsigned long				ulCurrentExpirationDay;
	unsigned long				ulCurrentExpirationHour;
	unsigned long				ulCurrentExpirationMin;
	unsigned long				ulCurrentExpirationSec;
	unsigned long				ulCurrentExpirationMilliSeconds;
	unsigned long long		ullCurrentExpirationDateTimeInSecs;
	unsigned long long		ullCurrentExpirationDateTimeInMilliSecs;


	#ifdef CHECKSERVERSOCKET_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (ullLocalDateTimeInMilliSecs0 == 0)
			ullLocalDateTimeInMilliSecs0		=
				ullLocalDateTimeInMilliSecs;
		else
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, "Difference with previous handleTimeout: ", (long)
				(ullLocalDateTimeInMilliSecs -
					ullLocalDateTimeInMilliSecs0));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);

			ullLocalDateTimeInMilliSecs0		=
				ullLocalDateTimeInMilliSecs;
		}
	}
	#endif

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errNoError;
	}

	if (_schTimesStatus != SCHTIMES_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errNoError;
	}

	if (sscanf (_pCurrentExpirationDateTime,
		"%4lu-%2lu-%2lu %2lu:%2lu:%2lu:%4lu",
		&ulCurrentExpirationYear,
		&ulCurrentExpirationMon,
		&ulCurrentExpirationDay,
		&ulCurrentExpirationHour,
		&ulCurrentExpirationMin,
		&ulCurrentExpirationSec,
		&ulCurrentExpirationMilliSeconds) != 7)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SSCANF_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (DateTime:: convertFromLocalDateTimeToLocalInSecs (
		ulCurrentExpirationYear, ulCurrentExpirationMon,
		ulCurrentExpirationDay, ulCurrentExpirationHour,
		ulCurrentExpirationMin, ulCurrentExpirationSec,
		_bCurrentDaylightSavingTime,
		&ullCurrentExpirationDateTimeInSecs) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMLOCALDATETIMETOLOCALINSECS_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	ullCurrentExpirationDateTimeInMilliSecs			=
		(ullCurrentExpirationDateTimeInSecs * 1000) +
		ulCurrentExpirationMilliSeconds;

	if ((errGetFreeEvent = _pesEventsSet -> getFreeEvent (
		CMEventsSet:: CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
		&pevEvent)) != errNoError)
	{
		if ((unsigned long) errGetFreeEvent ==
			EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CHECKSERVERSOCKETTIMES_REACHEDMAXIMUMEVENTSTOALLOCATE);
			_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
				(const char *) err, __FILE__, __LINE__);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return errNoError;
		}
		else
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_GETFREEEVENT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	// pevStreamerConnection			= (StreamerConnectionEvent_p) pevEvent;

	if (pevEvent -> init (
		CM_CHECKSERVERSOCKETTIMES_SOURCE,
		(const char *) _bMainProcessor,
		CM_EVENT_CONNECTIONTOACCEPT,
		"CM_EVENT_CONNECTIONTOACCEPT",
		ullCurrentExpirationDateTimeInMilliSecs) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_pesEventsSet -> releaseEvent (
			CMEventsSet:: CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
			pevEvent) != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_RELEASEEVENT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errNoError;
	}

	if (_pesEventsSet -> addEvent (
		pevEvent) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_ADDEVENT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (pevEvent -> finish () != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENT_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (_pesEventsSet -> releaseEvent (
			CMEventsSet:: CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
			pevEvent) != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_RELEASEEVENT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errNoError;
	}

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#ifdef CHECKSERVERSOCKET_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, "CheckServerSocket handleTimeout elapsed: ", (long)
				(ullLocalDateTimeInMilliSecs -
					ullLocalDateTimeInMilliSecs1));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);
		}
	}
	#endif


	return errNoError;
}


