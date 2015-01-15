
#include "CheckSocketsPoolTimes.h"
#include "ConnectionsManagerMessages.h"
#include "DateTime.h"
#include <assert.h>


CheckSocketsPoolTimes:: CheckSocketsPoolTimes (void): Times ()

{

}


CheckSocketsPoolTimes:: ~CheckSocketsPoolTimes (void)

{

}


CheckSocketsPoolTimes:: CheckSocketsPoolTimes (const CheckSocketsPoolTimes &t)

{

	assert (1==0);

	// to do

	*this = t;
}


Error CheckSocketsPoolTimes:: init (
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
		CM_CHECKSOCKETSPOOLTIMES_CLASSNAME) != errNoError)
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


Error CheckSocketsPoolTimes:: finish (void)

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


Error CheckSocketsPoolTimes:: handleTimeOut (void)

{

	Event_p							pevEvent;
	unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs;
	Error_t						errGetFreeEvent;
	#ifdef CHECKSOCKETSPOOL_STATISTICS
		static unsigned long long
			ullLocalExpirationLocalDateTimeInMilliSecs0 = 0;
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
	#endif


	// SE NECESSARIO, PER MAGGIORI PERFORMANCE, INVECE DI INSERIRE UN EVENTO
	// CON LA DATA CORRENTE, SI POTREBBE INSERIRE L'EVENTO CON LA DATA DI
	// SCADENZA DEL TIMES (_pCurrentExpirationDateTime) come fatto in
	// CheckServerSocketTimes.
	// INFATTI, IN CONDIZIONI DI STRESS, LA DATA CORRENTE NON E' PRECISA E
	// SICURAMENTE E SICCESSIVA ALLA SCADENZA REALE
	// (_pCurrentExpirationDateTime)

	if (DateTime:: nowLocalInMilliSecs (
		&ullLocalExpirationLocalDateTimeInMilliSecs) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#ifdef CHECKSOCKETSPOOL_STATISTICS
	{
		if (ullLocalExpirationLocalDateTimeInMilliSecs0 == 0)
			ullLocalExpirationLocalDateTimeInMilliSecs0		=
				ullLocalExpirationLocalDateTimeInMilliSecs;
		else
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, "Difference with previous handleTimeout: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs -
					ullLocalExpirationLocalDateTimeInMilliSecs0));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);

			ullLocalExpirationLocalDateTimeInMilliSecs0		=
				ullLocalExpirationLocalDateTimeInMilliSecs;
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

	if ((errGetFreeEvent = _pesEventsSet -> getFreeEvent (
		CMEventsSet:: CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
		&pevEvent)) != errNoError)
	{
		if ((unsigned long) errGetFreeEvent ==
			EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CHECKSOCKETSPOOLTIMES_REACHEDMAXIMUMEVENTSTOALLOCATE);
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
		CM_CHECKSOCKETSPOOLTIMES_SOURCE,
		(const char *) _bMainProcessor,
		CM_EVENT_CHECKSOCKETSPOOL,
		"CM_EVENT_CHECKSOCKETSPOOL",
		ullLocalExpirationLocalDateTimeInMilliSecs) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_pesEventsSet -> releaseEvent (
			CMEventsSet:: CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
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
			CMEventsSet:: CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
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

	#ifdef CHECKSOCKETSPOOL_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_PROCESSOR_STATISTICS,
				2, "CheckSocketsPool handleTimeout elapsed: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
				(const char *) msg, __FILE__, __LINE__);
		}
	}
	#endif


	return errNoError;
}

