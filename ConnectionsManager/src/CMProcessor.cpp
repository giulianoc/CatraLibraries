
#include "CMProcessor.h"
#include "ConnectionsManagerMessages.h"
#include "CheckServerSocketTimes.h"
#include "CheckUnusedSocketsTimes.h"
#include "DateTime.h"
#include "WebUtility.h"
#include "FileIO.h"
#include "System.h"
#include "StringTokenizer.h"
#include "CheckSocketsPoolTimes.h"
#include "HTTPProxyThread.h"



CMProcessor:: CMProcessor (void): BaseProcessor ()

{

}


CMProcessor:: ~CMProcessor (void)

{

}


#ifdef VECTORFREESESSIONS
	Error CMProcessor:: init (
		unsigned long ulProcessorIdentifier,
		ConfigurationFile_p pcfConfiguration,
		Scheduler_p pscScheduler,
		CMEventsSet_p pesEventsSet,
		Boolean_t bIsHTTPProxyEnabled,
		LoadBalancer_p plbHTTPProxyLoadBalancer,
		unsigned long ulMaxSimultaneousConnectionsToAccept,
		unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
		std:: vector<Session_p> *pvFreeSessions,
		PMutex_p pmtFreeSessions,
		Session_p psSessions,
		unsigned long ulMaxConnections,
		unsigned long ulMaxConnectionTTLInSeconds,
		ServerSocket_p pssServerSocket,
		CMSocketsPool_p pspSocketsPool,
		unsigned long ulMaxMilliSecondsToProcessAnEvent,
		Tracer_p ptTracer)
#else
	Error CMProcessor:: init (
		unsigned long ulProcessorIdentifier,
		ConfigurationFile_p pcfConfiguration,
		Scheduler_p pscScheduler,
		CMEventsSet_p pesEventsSet,
		Boolean_t bIsHTTPProxyEnabled,
		LoadBalancer_p plbHTTPProxyLoadBalancer,
		unsigned long ulMaxSimultaneousConnectionsToAccept,
		unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
		SessionsHashMap_p phmFreeSessions,
		PMutex_p pmtFreeSessions,
		Session_p psSessions,
		unsigned long ulMaxConnections,
		unsigned long ulMaxConnectionTTLInSeconds,
		ServerSocket_p pssServerSocket,
		CMSocketsPool_p pspSocketsPool,
		unsigned long ulMaxMilliSecondsToProcessAnEvent,
		Tracer_p ptTracer)
#endif

{

	Error_t			errGetItemValue;
	char			pConfigurationBuffer [
		CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH];


	if (bIsHTTPProxyEnabled &&
		plbHTTPProxyLoadBalancer == (LoadBalancer_p) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_ACTIVATION_WRONG);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (BaseProcessor:: init (ulProcessorIdentifier,
		"CMProcessor", pcfConfiguration,
		pscScheduler, pesEventsSet, ulMaxMilliSecondsToProcessAnEvent,
		ptTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_INIT_FAILED);
		ptTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	_bIsHTTPProxyEnabled			= bIsHTTPProxyEnabled;
	_plbHTTPProxyLoadBalancer		= plbHTTPProxyLoadBalancer;
	_ulMaxSimultaneousConnectionsToAccept		=
		ulMaxSimultaneousConnectionsToAccept;
	_ulMaxDelayAcceptableInLoopInMilliSecs		=
		ulMaxDelayAcceptableInLoopInMilliSecs;
	_pspSocketsPool					= pspSocketsPool;
	#ifdef VECTORFREESESSIONS
		_pvFreeSessions					= pvFreeSessions;
	#else
		_phmFreeSessions				= phmFreeSessions;
	#endif
	_pmtFreeSessions				= pmtFreeSessions;
	_psSessions						= psSessions;
	_ulMaxConnections				= ulMaxConnections;
	_ulMaxConnectionTTLInSeconds	= ulMaxConnectionTTLInSeconds;

	_pssServerSocket				= pssServerSocket;

	if (_bIsHTTPProxyEnabled)
	{
		if (_pcfConfiguration -> getCfgSectionByName (
			"HTTPProxy", &_csHTTPProxyCfgSection) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (BaseProcessor:: finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if ((errGetItemValue = _pcfConfiguration -> getItemValue ("HTTPProxy",
			"URIHTTPProxyPrefix", _pURIHTTPProxyPrefix,
			CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "HTTPProxy", "URIHTTPProxyPrefix");
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if ((errGetItemValue = _pcfConfiguration -> getItemValue ("HTTPProxy",
			"LocalIPAddress", _pWebServerLocalIPAddress,
			SCK_MAXHOSTNAMELENGTH)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "HTTPProxy", "LocalIPAddress");
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if ((errGetItemValue = _pcfConfiguration -> getItemValue ("HTTPProxy",
			"TimeoutToWaitAnswerInSeconds", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "HTTPProxy", "TimeoutToWaitAnswerInSeconds");
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			return err;
		}
		_ulWebServerTimeoutToWaitAnswerInSeconds        =
			strtoul (pConfigurationBuffer, (char **) NULL, 10);
	}

	if (_bRequestHeader. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bIsHTTPProxyEnabled)
		{
			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (BaseProcessor:: finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);
		}

		return err;
	}

	if (_bRequestBody. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bIsHTTPProxyEnabled)
		{
			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (BaseProcessor:: finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);
		}

		return err;
	}

	if (_bRequestCookie. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bIsHTTPProxyEnabled)
		{
			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (BaseProcessor:: finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);
		}

		return err;
	}

	if (_bRequestUserAgent. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bIsHTTPProxyEnabled)
		{
			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (BaseProcessor:: finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);
		}

		return err;
	}

	if (_bURL. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_bIsHTTPProxyEnabled)
		{
			if (_csHTTPProxyCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (BaseProcessor:: finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);
		}

		return err;
	}


	return errNoError;
}


Error CMProcessor:: finish ()

{

	if (_bIsHTTPProxyEnabled)
	{
		if (_csHTTPProxyCfgSection. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}

	if (BaseProcessor:: finish () != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);
	}


	return errNoError;
}


Error CMProcessor:: processEvent (Event_p pevEvent,
	unsigned long *pulInternalTimeInMilliSecsToProcessEvent)

{

	long				lTypeIdentifier;
	Error_t				errTypeIdentifier;
	Error_t				errHandleConnection;


	if ((errTypeIdentifier = pevEvent -> getTypeIdentifier (
		&lTypeIdentifier)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errTypeIdentifier, __FILE__, __LINE__);

		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_GETTYPEIDENTIFIER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (pevEvent -> finish () != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENT_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		/*
		???????????
		if (_pesEventsSet -> releaseEvent (
			pevRTSPConnection) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_EVENTSSET_RELEASERTSPCONNECTIONEVENT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
		*/

		return err;
	}

	switch (lTypeIdentifier)
	{
		case CM_EVENT_CONNECTIONTOACCEPT:	// 0
			{
				/*
				ConnectionToAcceptEvent_p
					pevConnectionToAcceptEvent;


				pevConnectionToAcceptEvent			=
					(ConnectionToAcceptEvent_p) pevEvent;
				*/

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_EVENTHANDLER,
						2, _ulProcessorIdentifier,
						"CM_EVENT_CONNECTIONTOACCEPT");
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG3,
						(const char *) msg, __FILE__, __LINE__);
				}

				if (handleConnectionToAcceptEvent (pevEvent) !=
					errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_HANDLECONNECTIONTOACCEPTEVENT_FAILED);
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) err, __FILE__, __LINE__);

					if (pevEvent -> finish () != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENT_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet::
							CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (pevEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet::
							CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet::
						CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER,
					pevEvent) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
			}

			break;
		case CM_EVENT_CONNECTIONREADYTOREAD:	// 1
			{
				ConnectionEvent_p		pevConnectionEvent;


				pevConnectionEvent			=
					(ConnectionEvent_p) pevEvent;

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_EVENTHANDLER,
						2, _ulProcessorIdentifier,
						"CM_EVENT_CONNECTIONREADYTOREAD");
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG3,
						(const char *) msg, __FILE__, __LINE__);
				}

				if ((errHandleConnection = handleConnectionReadyToReadEvent (
					pevConnectionEvent,
					pulInternalTimeInMilliSecsToProcessEvent)) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_HANDLECONNECTIONREADYTOREADEVENT_FAILED);
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) err, __FILE__, __LINE__);

					if (pevConnectionEvent -> finish () != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENT_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
						pevConnectionEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return errHandleConnection;
				}

				if (pevConnectionEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
						pevConnectionEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
					pevConnectionEvent) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
			}

			break;
		case CM_EVENT_CHECKSOCKETSPOOL:	// 2
			{
				/*
				ConnectionToAcceptEvent_p
					pevConnectionToAcceptEvent;


				pevConnectionToAcceptEvent			=
					(ConnectionToAcceptEvent_p) pevEvent;
				*/

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_EVENTHANDLER,
						2, _ulProcessorIdentifier,
						"CM_EVENT_CHECKSOCKETSPOOL");
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG3,
						(const char *) msg, __FILE__, __LINE__);
				}

				if (handleCheckSocketsPoolEvent (pevEvent) !=
					errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_HANDLECHECKSOCKETSPOOLEVENT_FAILED);
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) err, __FILE__, __LINE__);

					if (pevEvent -> finish () != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENT_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet::
							CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (pevEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet::
							CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet::
						CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
					pevEvent) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
			}

			break;
		case CM_EVENT_CHECKUNUSEDSOCKETS:	// 3
			{
				/*
				ConnectionToAcceptEvent_p
					pevConnectionToAcceptEvent;


				pevConnectionToAcceptEvent			=
					(ConnectionToAcceptEvent_p) pevEvent;
				*/

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_EVENTHANDLER,
						2, _ulProcessorIdentifier,
						"CM_EVENT_CHECKUNUSEDSOCKETS");
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG3,
						(const char *) msg, __FILE__, __LINE__);
				}

				if (handleCheckUnusedSocketsEvent (pevEvent) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_HANDLECHECKUNUSEDSOCKETSEVENT_FAILED);
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) err, __FILE__, __LINE__);

					if (pevEvent -> finish () != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENT_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet:: CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (pevEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pesEventsSet -> releaseEvent (
						CMEventsSet:: CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER,
						pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_RELEASEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet:: CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER,
					pevEvent) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
			}

			break;
		default:
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_EVENTUNKNOWN,
					1, lTypeIdentifier);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (pevEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				/*
				???????????
				if (_pesEventsSet -> releaseEvent (
					pevRTSPConnection) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_EVENTSSET_RELEASERTSPCONNECTIONEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
				*/

				return err;
			}
	}


	return errNoError;
}


Error CMProcessor:: handleConnectionToAcceptEvent (
	Event_p pevEvent)

{

	Session:: ConnectionInformation_p		pciConnectionInformation;
	unsigned long					ulConnectionsAccepted;
	unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
	unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs2;


	// IL CLIENT SOCKET VIENE TOLTO DAL POOL DAL CMSOCKETSPOOL QUANDO TROVA
	// QUALCOSA DA LEGGERE E VIENE INSERITO DALLA SESSION DOPO AVER
	// RISPOSTO AL CLIENT (writeResponse). NON PUO' ESSERE INSERITO
	// PRIMA DELLA RISPOSTA AL CLIENT NEL SOCKET POOL IN QUANTO
	// SE LO STREAMER MANDASSE EOF, LA SUCCESSIVA RISPOSTA AL CLIENT
	// POTREBBE TROVARE UN SOCKET CHIUSO

	#ifdef PROCESSOR_ACCEPT_STATISTICS
		static unsigned long long
			ullLocalExpirationLocalDateTimeInMilliSecs0 = 0;
	#endif


	#ifdef PROCESSOR_ACCEPT_STATISTICS
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
			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
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

	// QUESTO EVENTO ESEGUE ENTRAMBE: ACCEPT E READ.
	// HO FATTO IL TENTATIVO DI DIVIDERE LE DUE AZIONI, CIOE' UN EVENTO PER
	// ACCEPT ED UN EVENTO PER READ. MI SEMBRAVA UNA BUONA IDEA PERCHÈ LE READ
	// COSTANO A VOLTE 300 400 MILLISECS E PENSAVO CHE SI POTESSE ARRIVARE ALLA
	// READ SOLO QUANDO IL POOL DI SOCKETS MI DICEVA CHE ERA PRONTO PER LEGGERE
	// IN REALTA' DOPO LA ACCEPT BISOGNA CHE LA READ AVVIENE VELOCEMENTE
	// ALTRIMENTI SI HA UN TIMEOUT. DIVIDENDO LE COSE, IN UN MOMENTO DI PICCO,
	// SI AVREBBERO TANTISSIME ACCEPT E LE RELATIVE READ VERREBBERO RITARDATE.

	ulConnectionsAccepted					= 0;

	while (ulConnectionsAccepted < _ulMaxSimultaneousConnectionsToAccept)
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		// accept
		{
			Session_p		pssSession;
			Error					errAcceptConnection;
			unsigned long			ulConnectionsFree;


			if (_pmtFreeSessions -> lock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_LOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			#ifdef VECTORFREESESSIONS
				ulConnectionsFree			= _pvFreeSessions -> size ();
			#else
				ulConnectionsFree			= _phmFreeSessions -> size ();
			#endif

			if (ulConnectionsFree == 0)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_NOTFOUNDSESSIONFREE);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			#ifdef VECTORFREESESSIONS
				pssSession		= *(_pvFreeSessions -> begin ());
			#else
				SessionsHashMap_t:: iterator		it;
				
				it	= _phmFreeSessions -> begin ();
				pssSession		= it -> second;
			#endif

			if ((errAcceptConnection =
				pssSession -> acceptConnection (
				_pssServerSocket, &pciConnectionInformation)) != errNoError)
			{
				if ((long) errAcceptConnection ==
					CM_SESSION_NOTCONNECTIONTOACCEPT_FAILED)
				{
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) errAcceptConnection, __FILE__, __LINE__);

					#ifdef PROCESSOR_ACCEPT_STATISTICS
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

						Message msg = ConnectionsManagerMessages (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_STATISTICS,
							2, "Accept no connection: ", (long)
							(ullLocalExpirationLocalDateTimeInMilliSecs2 -
								ullLocalExpirationLocalDateTimeInMilliSecs1));
						_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
							(const char *) msg, __FILE__, __LINE__);
					}
					#endif

					if (_pmtFreeSessions -> unLock () != errNoError)
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
					// already logged inside the acceptConnection method
					// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					// 	(const char *) errAcceptConnection, __FILE__, __LINE__);

					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_ACCEPTCONNECTION_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pmtFreeSessions -> unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}

// char aaa [256];
// sprintf (aaa, "    ACCEPTED, id: %lu", pciConnectionInformation -> _ulConnectionIdentifier);
// FileIO::appendBuffer ("/tmp/debug.txt", aaa, true);
			{
				Message msg = ConnectionsManagerMessages (
					__FILE__, __LINE__,
					CM_CMPROCESSOR_FREECONNECTIONS,
					1, ulConnectionsFree);
				_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
					(const char *) msg, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_pvFreeSessions -> erase (
					_pvFreeSessions -> begin ());
			#else
				if (!_phmFreeSessions -> Delete (
					(int) (pssSession -> getConnectionIdentifier ())))
				{
					// false means the key was not found and the table
					// was not modified. That should not happen
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_DELETEONFREESESSIONSFAILED,
						1, (long) pssSession -> getConnectionIdentifier ());
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_pmtFreeSessions -> unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			#endif

			if (_pmtFreeSessions -> unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
	
				if ((pciConnectionInformation -> _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, pciConnectionInformation -> _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						(pciConnectionInformation -> _psSession));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) (pciConnectionInformation -> _psSession ->
							getConnectionIdentifier ()),
						(pciConnectionInformation -> _psSession),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}

		ulConnectionsAccepted++;

		#ifdef PROCESSOR_ACCEPT_STATISTICS
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullLocalExpirationLocalDateTimeInMilliSecs2) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "Accept connection accepted: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif

		// it is simulated the ... event
		{
			Event_p						pevLocalEvent;
			ConnectionEvent_p			pevConnection;
			Error_t						errHandleConnection;
			unsigned long				ulInternalTimeInMilliSecsToProcessEvent;


			if (_pesEventsSet -> getFreeEvent (
				CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
				&pevLocalEvent) != errNoError)
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_GETFREEEVENT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if ((pciConnectionInformation -> _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, pciConnectionInformation -> _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						(pciConnectionInformation -> _psSession));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) (pciConnectionInformation -> _psSession ->
							getConnectionIdentifier ()),
						(pciConnectionInformation -> _psSession),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			pevConnection			=
				(ConnectionEvent_p) pevLocalEvent;

			if (pevConnection -> init (
				(const char *) _bProcessorName,	// himself
				// Since we will not call addEvent to the EventsSet,
				// the next parameter will not be used
				(const char *) _bProcessorName,
				CM_EVENT_CONNECTIONREADYTOREAD,
				"CM_EVENT_CONNECTIONREADYTOREAD",
				pciConnectionInformation,
				_ptSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONEVENT_INIT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
					pevConnection) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if ((pciConnectionInformation -> _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, pciConnectionInformation -> _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						(pciConnectionInformation -> _psSession));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) (pciConnectionInformation -> _psSession ->
							getConnectionIdentifier ()),
						(pciConnectionInformation -> _psSession),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			ulInternalTimeInMilliSecsToProcessEvent		= 0;

			if ((errHandleConnection = handleConnectionReadyToReadEvent (
				pevConnection, &ulInternalTimeInMilliSecsToProcessEvent)) !=
				errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_HANDLECONNECTIONREADYTOREADEVENT_FAILED);
				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 	(const char *) err, __FILE__, __LINE__);

				if (pevConnection -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
					pevConnection) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				// handleConnectionReadyToReadEvent will care to
				//	- answer to the Streamer or
				//	- close the connection and add again the Session
				//		into the vector

				return errHandleConnection;
			}

			if (pevConnection -> finish () != errNoError)
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENT_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (_pesEventsSet -> releaseEvent (
					CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
					pevConnection) != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_RELEASEEVENT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			if (_pesEventsSet -> releaseEvent (
				CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
				pevConnection) != errNoError)
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_RELEASEEVENT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (ullLocalExpirationLocalDateTimeInMilliSecs2 -
			ullLocalExpirationLocalDateTimeInMilliSecs1 >
			_ulMaxDelayAcceptableInLoopInMilliSecs)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_TOOTIMETOACCEPTANDREAD,
				2, (unsigned long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
				ullLocalExpirationLocalDateTimeInMilliSecs1),
				ulConnectionsAccepted);
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) err, __FILE__, __LINE__);

		 	break;
		}
	}


	return errNoError;
}


Error CMProcessor:: handleConnectionReadyToReadEvent (
	ConnectionEvent_p pevConnectionEvent,
	unsigned long *pulInternalTimeInMilliSecsToProcessEvent)

{

	Session:: ConnectionInformation_t		ciConnectionInformation;
	#ifdef PROCESSOR_READ_STATISTICS
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs4;
	#endif
	unsigned long long			ullLocalExpirationLocalDateTimeInMilliSecs2;
	unsigned long long			ullLocalExpirationLocalDateTimeInMilliSecs3;

	if (pevConnectionEvent -> getConnectionInformation (
		&ciConnectionInformation) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONEVENT_GETCONNECTIONINFORMATION_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		// if this error will happen, the socket relative to the Streamer
		// Session was deleted from the socketspool and it will not be added
		// again. The consequence is that the Session structure will
		// be never reused, it will remain always allocated and the socket
		// will be never closed.

		return err;
	}

	// read
	{
		Error_t					errRead;
		Error_t					errProxy;
		Boolean_t				bSocketToBeClosedInCaseOfError;
		Boolean_t				bReceivedRequestManaged;


		#ifdef PROCESSOR_READ_STATISTICS
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullLocalExpirationLocalDateTimeInMilliSecs1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}
		#endif

		if ((errRead = (ciConnectionInformation. _psSession) ->
			readRequest (
			&(ciConnectionInformation. _tUTCStartConnectionTime),
			&_bRequestHeader, &_bRequestBody, &_bURL, &_bRequestUserAgent,
			&_bRequestCookie)) != errNoError)
		{
			if ((unsigned long) errRead == SCK_READ_EOFREACHED)
			{
				#ifdef PROCESSOR_READ_STATISTICS
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

					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_STATISTICS,
						2, "Time to read (EOF): ", (long)
						(ullLocalExpirationLocalDateTimeInMilliSecs2 -
							ullLocalExpirationLocalDateTimeInMilliSecs1));
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
						(const char *) msg, __FILE__, __LINE__);
				}
				#endif

				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 	(const char *) errRead, __FILE__, __LINE__);

				if ((ciConnectionInformation. _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, ciConnectionInformation. _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						(ciConnectionInformation. _psSession));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) (ciConnectionInformation. _psSession ->
							getConnectionIdentifier ()),
						(ciConnectionInformation. _psSession),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return errNoError;
			}
			else if ((unsigned long) errRead ==
				WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED)
			{
				// that should be a streamer problem, we have this error when
				// the streamer is like "blocked for some seconds

				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_SESSION_READREQUEST_FAILED,
					1, ciConnectionInformation. _ulConnectionIdentifier);
				// _ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 	(const char *) err, __FILE__, __LINE__);

				#ifdef PROCESSOR_READ_STATISTICS
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

					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_STATISTICS,
						2, "Time to read (HEADER TIMEOUT): ", (long)
						(ullLocalExpirationLocalDateTimeInMilliSecs2 -
							ullLocalExpirationLocalDateTimeInMilliSecs1));
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
						(const char *) msg, __FILE__, __LINE__);
				}
				#endif

				// In this case the socket is added inside the SocketsPool
				// So we don't need to release the connection
				// if ((ciConnectionInformation. _psSession) ->
				// 	releaseConnection (false) != errNoError)
				// {
				// 	Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				// 		CM_SESSION_RELEASECONNECTION_FAILED,
				// 		1, ciConnectionInformation. _ulConnectionIdentifier);
				// 	_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 		(const char *) err, __FILE__, __LINE__);
				// }

				// if (_pmtFreeSessions -> lock () != errNoError)
				// {
				// 	Error err = PThreadErrors (__FILE__, __LINE__,
				// 		THREADLIB_PMUTEX_LOCK_FAILED);
				// 	_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 		(const char *) err, __FILE__, __LINE__);

				//	// return err;
				// }

				// _pvFreeSessions -> insert (
				// 	_pvFreeSessions -> end (),
				// 	(ciConnectionInformation. _psSession));

				// if (_pmtFreeSessions -> unLock () != errNoError)
				// {
				// 	Error err = PThreadErrors (__FILE__, __LINE__,
				// 		THREADLIB_PMUTEX_UNLOCK_FAILED);
				// 	_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				// 		(const char *) err, __FILE__, __LINE__);

				//	// return err;
				// }

				// Anyway, since there was nothing to read, we will not
				// return any error
				// return errRead;
				return errNoError;
			}
			else if ((unsigned long) errRead ==
				WEBTOOLS_HTTPGETTHREAD_BODYTIMEOUTEXPIRED)
			{
				// it seems we do not get never this BODYTIMEOUTEXPIRED error
				// because it is already managed inside
				// the _psSession -> readRequest method

				#ifdef PROCESSOR_READ_STATISTICS
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

					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_STATISTICS,
						2, "Time to read (BODY TIMEOUT): ", (long)
						(ullLocalExpirationLocalDateTimeInMilliSecs2 -
							ullLocalExpirationLocalDateTimeInMilliSecs1));
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
						(const char *) msg, __FILE__, __LINE__);
				}
				#endif

				/*
				// No error because the body is filled and
				// probably the ContentLength was wrong and the
				// read method waiting for more data
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMPROCESSOR_READREQUESTWITHOUTBODY,
					1, (const char *) _bRequestHeader);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if ((ciConnectionInformation. _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, ciConnectionInformation. _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					// return err;
				}

				_pvFreeSessions -> insert (
					_pvFreeSessions -> end (),
					(ciConnectionInformation. _psSession));

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					// return err;
				}

				return err;
				*/
			}
			else if ((unsigned long) errRead ==
				CM_SESSION_REQUESTNOTREADBECAUSEOLDSOCKET)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_SESSION_READREQUEST_FAILED,
					1, ciConnectionInformation. _ulConnectionIdentifier);
				_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
					(const char *) err, __FILE__, __LINE__);

				// or it is closed, or it is opened by another
				// session, in both these cases I don't have to release it
				// Confirmed by a verification

				return errRead;
			}
			else
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_SESSION_READREQUEST_FAILED,
					1, ciConnectionInformation. _ulConnectionIdentifier);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if ((ciConnectionInformation. _psSession) ->
					releaseConnection (false) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_RELEASECONNECTION_FAILED,
						1, ciConnectionInformation. _ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						(ciConnectionInformation. _psSession));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) (ciConnectionInformation. _psSession ->
							getConnectionIdentifier ()),
						(ciConnectionInformation. _psSession),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}

		{
			_bRequestHeader. substitute (CM_SESSION_HTTPNEWLINE, " "); 
			_bRequestBody. substitute (CM_SESSION_NEWLINE, " "); 

			char pEventCreation [EVSET_MAXDATELENGTH];

			pevConnectionEvent -> getCreationDateTime (pEventCreation);

			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_RECEIVEDREQUEST,
				7,
				ciConnectionInformation. _ulConnectionIdentifier,
				pEventCreation,
				(ciConnectionInformation. _psSession) -> _pClientIPAddress,
				(ciConnectionInformation. _psSession) -> _lClientPort,
				_bURL. str(),
				_bRequestHeader. str (),
				_bRequestBody. str ());
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6, (const char *) msg,
				__FILE__, __LINE__);
		}

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		#ifdef PROCESSOR_READ_STATISTICS
		{
			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "Time to read: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif

		bSocketToBeClosedInCaseOfError		= true;
		bReceivedRequestManaged				= false;

		if (_bIsHTTPProxyEnabled)
		{
			if ((errProxy = manageHTTPProxyRequest (&_bURL, &_bRequestHeader,
				&_bRequestBody, &_bRequestUserAgent, &_bRequestCookie,
				ciConnectionInformation. _psSession,
				ciConnectionInformation. _tUTCStartConnectionTime,
				&bSocketToBeClosedInCaseOfError)) != errNoError)
			{
				if ((unsigned long) errProxy == CM_CMPROCESSOR_NOTOBEPROXIED)
				{
					bReceivedRequestManaged		= false;
				}
				else
				{
					bReceivedRequestManaged		= true;

					if (HTTPProxyThread::sendErrorToHTTPClient (
						ciConnectionInformation. _psSession,
						ciConnectionInformation. _tUTCStartConnectionTime,
						&errProxy, _ptSystemTracer) != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_HTTPPROXYTHREAD_SENDERRORTOHTTPCLIENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					// parse error
					if (bSocketToBeClosedInCaseOfError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_PROCESSRECEIVEDREQUEST_FAILED,
							3, ciConnectionInformation. _ulConnectionIdentifier,
							(const char *) _bRequestHeader,
							(const char *) _bRequestBody);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if ((ciConnectionInformation. _psSession) ->
							releaseConnection (true) != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_RELEASECONNECTION_FAILED,
									1,
							ciConnectionInformation. _ulConnectionIdentifier);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (_pmtFreeSessions -> lock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_LOCK_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						#ifdef VECTORFREESESSIONS
							_pvFreeSessions -> insert (
								_pvFreeSessions -> end (),
								(ciConnectionInformation. _psSession));
						#else
							int				iDidInsert;


							_phmFreeSessions -> InsertWithoutDuplication (
								(int) (ciConnectionInformation. _psSession ->
									getConnectionIdentifier ()),
								(ciConnectionInformation. _psSession),
								&iDidInsert);

							if (iDidInsert == 0)
							{
								Error err = ConnectionsManagerErrors (
									__FILE__, __LINE__,
									CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}
						#endif

						if (_pmtFreeSessions -> unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						return err;
					}
				}
			}
			else
			{
				bReceivedRequestManaged		= true;
			}
		}
		else
		{
			bReceivedRequestManaged		= false;
		}

		if (!bReceivedRequestManaged)
		{
			if (processReceivedRequest (&_bURL, &_bRequestHeader,
				&_bRequestBody, &_bRequestUserAgent, &_bRequestCookie,
				ciConnectionInformation. _psSession,
				ciConnectionInformation. _tUTCStartConnectionTime,
				&bSocketToBeClosedInCaseOfError) != errNoError)
			{
				// parse error
				if (bSocketToBeClosedInCaseOfError)
				{
					Error err = ConnectionsManagerErrors (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_PROCESSRECEIVEDREQUEST_FAILED,
						3, ciConnectionInformation. _ulConnectionIdentifier,
						(const char *) _bRequestHeader,
						(const char *) _bRequestBody);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if ((ciConnectionInformation. _psSession) ->
						releaseConnection (true) != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_RELEASECONNECTION_FAILED,
							1,
							ciConnectionInformation. _ulConnectionIdentifier);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (_pmtFreeSessions -> lock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_LOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					#ifdef VECTORFREESESSIONS
						_pvFreeSessions -> insert (
							_pvFreeSessions -> end (),
							(ciConnectionInformation. _psSession));
					#else
						int				iDidInsert;


						_phmFreeSessions -> InsertWithoutDuplication (
							(int) (ciConnectionInformation. _psSession ->
								getConnectionIdentifier ()),
							(ciConnectionInformation. _psSession),
							&iDidInsert);

						if (iDidInsert == 0)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}
					#endif

					if (_pmtFreeSessions -> unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}
		}

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs3) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		*pulInternalTimeInMilliSecsToProcessEvent		=
			ullLocalExpirationLocalDateTimeInMilliSecs3 -
			ullLocalExpirationLocalDateTimeInMilliSecs2;

		#ifdef PROCESSOR_READ_STATISTICS
		{
			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "Time to parse: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs3 -
					ullLocalExpirationLocalDateTimeInMilliSecs2));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif

		#ifdef PROCESSOR_READ_STATISTICS
		{
			if (DateTime:: nowLocalInMilliSecs (
				&ullLocalExpirationLocalDateTimeInMilliSecs4) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "Time to managePlugin: ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs4 -
					ullLocalExpirationLocalDateTimeInMilliSecs3));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}
		#endif
	}


	return errNoError;
}


Error CMProcessor:: handleCheckUnusedSocketsEvent (
	Event_p pevEvent)

{

	unsigned long				ulConnectionIdentifier;
	Boolean_t					bIsOld;
	Boolean_t					bIsFreeSession;
	time_t						tNowInSeconds;
	Error_t						errRelease;
	unsigned long				ulReleasedCounter;
	unsigned long				ulFreeSessionsNumberFromVector;
	unsigned long				ulFreeSessionsNumberFromArray;


	#ifdef PROCESSOR_CHECKUNUSEDSOCKETS_STATISTICS
		unsigned long long		ullLocalDateTimeInMilliSecs1;
		unsigned long long		ullLocalDateTimeInMilliSecs2;

		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	#endif


	tNowInSeconds								= time (NULL);
	ulReleasedCounter							= 0;
	ulFreeSessionsNumberFromArray				= 0;

	for (ulConnectionIdentifier = 0;
		ulConnectionIdentifier < _ulMaxConnections;
		ulConnectionIdentifier++)
	{
		if ((_psSessions [ulConnectionIdentifier]).
			isOlderThan (&tNowInSeconds,
			&_ulMaxConnectionTTLInSeconds, &bIsOld,
			&bIsFreeSession) != errNoError)
		{
			Error err = ConnectionsManagerErrors (
				__FILE__, __LINE__,
				CM_SESSION_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (bIsFreeSession)
		{
			ulFreeSessionsNumberFromArray++;
		}
		else
		{
			if (bIsOld)
			{
				if ((errRelease =
					(_psSessions [ulConnectionIdentifier]).
					releaseConnection (true)) != errNoError)
				{
					if ((unsigned long) errRelease ==
						CM_SESSION_DELETESOCKETFAILED)
						;	// continue;
					else
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_RELEASECONNECTION_FAILED,
							1, ulConnectionIdentifier);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						return err;
					}
				}

				ulReleasedCounter++;

				if (_pmtFreeSessions -> lock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_LOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				#ifdef VECTORFREESESSIONS
					_pvFreeSessions -> insert (
						_pvFreeSessions -> end (),
						&(_psSessions [ulConnectionIdentifier]));
				#else
					int				iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) ((_psSessions [ulConnectionIdentifier]).
							getConnectionIdentifier ()),
						&(_psSessions [ulConnectionIdentifier]),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						// it is too old but it is already in _phmFreeSessions
						// It means _tUTCLastRead is wrong. We will reset it
						if ((_psSessions [ulConnectionIdentifier]). reset () !=
							errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_RESET_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						/*
						if (_pmtFreeSessions -> unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						return err;
						*/
					}
				#endif

				if (_pmtFreeSessions -> unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMPROCESSOR_TTLSOCKETEXPIRED,
						1, ulConnectionIdentifier);
					_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
						(const char *) err, __FILE__, __LINE__);
				}
			}
		}
	}

	if (_pmtFreeSessions -> lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#ifdef VECTORFREESESSIONS
		ulFreeSessionsNumberFromVector		= _pvFreeSessions -> size ();
	#else
		ulFreeSessionsNumberFromVector		= _phmFreeSessions -> size ();
	#endif

	if (_pmtFreeSessions -> unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_SOCKETSEXPIRED,
			4, ulReleasedCounter, (unsigned long)
				(_ulMaxConnections - ulFreeSessionsNumberFromVector),
				ulFreeSessionsNumberFromVector,
				ulFreeSessionsNumberFromArray + ulReleasedCounter);
		_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
			(const char *) err, __FILE__, __LINE__);
	}

	#ifdef PROCESSOR_CHECKUNUSEDSOCKETS_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		Message msg = ConnectionsManagerMessages (
			__FILE__, __LINE__,
			CM_CMPROCESSOR_STATISTICS,
			2, "CheckUnusedSocket time: ", (long)
			(ullLocalDateTimeInMilliSecs2 -
				ullLocalDateTimeInMilliSecs1));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
			(const char *) msg, __FILE__, __LINE__);
	}
	#endif


	return errNoError;
}


Error CMProcessor:: processReceivedRequest (
	Buffer_p pbURL, Buffer_p pbRequestHeader, Buffer_p pbRequestBody,
	Buffer_p pbRequestUserAgent, Buffer_p pbRequestCookie,
	Session_p psSession, time_t tUTCStartConnectionTime,
	Boolean_p pbSocketToBeClosedInCaseOfError)

{


	return errNoError;
}


Error CMProcessor:: manageHTTPProxyRequest (
	Buffer_p pbURL, Buffer_p pbRequestHeader, Buffer_p pbRequestBody,
	Buffer_p pbRequestUserAgent, Buffer_p pbRequestCookie,
	Session_p psSession, time_t tUTCStartConnectionTime,
	Boolean_p pbSocketToBeClosedInCaseOfError)

{

	const char				*pMethodNameStart;
	const char				*pMethodNameEnd;
	Buffer_t				bMethodName;
	ConfigurationItem_t		ciMethodParametersCfgItem;
	long					lMethodParametersNumber;
	long					lMethodParameterIndex;
	Error_t					errCfg;
	char					pParameterName [
		CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH];
	long					lParameterNameLength;
	char					pParameterValue [
		CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH];
	Boolean_t				bIsParameterOptional;
	char					pWebServer [
		SCK_MAXHOSTNAMELENGTH];
	unsigned long			ulWebServerPort;
	Error_t					errGetAvailableModule;



	// setting bMethodName
	{
		// pbURL is like /cms/login?ClientName=CMSGUI&UserName=catalog_giuliano
	
		if ((pMethodNameEnd = strchr (pbURL -> str(), '?')) ==
			(const char *) NULL)
		{
			// no parameters

			if ((pMethodNameStart = strrchr (pbURL -> str(), '/')) ==
				(const char *) NULL)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CMSPROCESSOR_WRONGURIFORMAT,
					1, pbURL -> str());
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			if (bMethodName. setBuffer (pMethodNameStart + 1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}
		else
		{
			pMethodNameStart		= pMethodNameEnd;

			while (pMethodNameStart > pbURL -> str())
			{
				if (*pMethodNameStart == '/')
					break;

				pMethodNameStart--;
			}

			if (bMethodName. setBuffer (pMethodNameStart + 1,
				pMethodNameEnd - pMethodNameStart - 1) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}
	}

	if ((errCfg = _csHTTPProxyCfgSection. getCfgItemByName (
		bMethodName. str (), &ciMethodParametersCfgItem)) != errNoError)
	{
		/*
		_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
			(const char *) errCfg, __FILE__, __LINE__);

		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CMPROCESSOR_NOTOBEPROXIED,
			1, bMethodName. str ());
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errCfg = ciMethodParametersCfgItem. getItemValuesNumber (
		&lMethodParametersNumber)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errCfg, __FILE__, __LINE__);

		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (ciMethodParametersCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_bURLParametersForHTTPProxy. setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (ciMethodParametersCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	// setting of _bHTTPSessionKey
	if (!(pbRequestCookie -> isEmpty ()))
	{
		const char			*pHTTPSessionKeyStart;
		const char			*pHTTPSessionKeyEnd;


		if ((pHTTPSessionKeyStart = strstr (pbRequestCookie -> str (),
			"KEY=")) != (char *) NULL)
		{
			pHTTPSessionKeyStart		+= strlen ("KEY=");

			if ((pHTTPSessionKeyEnd = strchr (pHTTPSessionKeyStart, ';')) ==
				(char *) NULL)
			{
				if (_bHTTPSessionKey. setBuffer (pHTTPSessionKeyStart) !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_SETBUFFER_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (ciMethodParametersCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}
			else
			{
				if (_bHTTPSessionKey. setBuffer (pHTTPSessionKeyStart,
					pHTTPSessionKeyEnd - pHTTPSessionKeyStart) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_SETBUFFER_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (ciMethodParametersCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}
		}

		if (WebUtility:: addURLParameter (&_bURLParametersForHTTPProxy,
			"HTTPSessionKey", _bHTTPSessionKey. str ()) != errNoError)
		{
			Error err = WebToolsErrors (__FILE__, __LINE__,
				WEBTOOLS_WEBUTILITY_ADDURLPARAMETER_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (ciMethodParametersCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (WebUtility:: addURLParameter (&_bURLParametersForHTTPProxy,
		"UserAgent", pbRequestUserAgent -> str ()) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_ADDURLPARAMETER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (ciMethodParametersCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (WebUtility:: addURLParameter (&_bURLParametersForHTTPProxy,
		"ClientIPAddress", psSession -> _pClientIPAddress) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_ADDURLPARAMETER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (ciMethodParametersCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	for (lMethodParameterIndex = 0;
		lMethodParameterIndex < lMethodParametersNumber;
		lMethodParameterIndex++)
	{
		if ((errCfg = ciMethodParametersCfgItem. getItemValue (
			pParameterName, CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH,
			lMethodParameterIndex)) != errNoError ||
			(lParameterNameLength = strlen (pParameterName)) == 0)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errCfg, __FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (ciMethodParametersCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (pParameterName [lParameterNameLength - 1] == '*')
		{
			bIsParameterOptional		= false;
			pParameterName [lParameterNameLength - 1]		= '\0';
		}
		else
		{
			bIsParameterOptional		= true;
		}

		if (WebUtility:: getURLParameterValue (pbURL -> str (),
			pParameterName, pParameterValue,
			CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH) != errNoError)
		{
			if (!bIsParameterOptional)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_WEBUTILITY_GETURLPARAMETERVALUE_FAILED,
					2, pParameterName, pbURL -> str ());
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciMethodParametersCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}
		else
		{
			if (WebUtility:: addURLParameter (&_bURLParametersForHTTPProxy,
				pParameterName, pParameterValue) != errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_WEBUTILITY_ADDURLPARAMETER_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciMethodParametersCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}
	}

	if (ciMethodParametersCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_bURIForHTTPProxy. setBuffer (_pURIHTTPProxyPrefix) != errNoError ||
		_bURIForHTTPProxy. append (bMethodName. str ()) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((errGetAvailableModule =
		_plbHTTPProxyLoadBalancer -> getAvailableModule (
		"WebServers", pWebServer, SCK_MAXHOSTNAMELENGTH,
		&ulWebServerPort)) != errNoError)
	{
		Error err = LoadBalancerErrors (
			__FILE__, __LINE__,
			LB_LOADBALANCER_GETAVAILABLEMODULE_FAILED,
			1, "WebServers");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CMPROCESSOR_HTTPGETPROXY,
			7,
			bMethodName. str (),
			pWebServer,
			ulWebServerPort,
			_pWebServerLocalIPAddress,
			_bURIForHTTPProxy. str (),
			_bURLParametersForHTTPProxy. str (),
			_ulWebServerTimeoutToWaitAnswerInSeconds);
		_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	{
		HTTPProxyThread_p    	phtHTTPProxyThread;


		if ((phtHTTPProxyThread = new HTTPProxyThread_t) ==
			(HTTPProxyThread_p) NULL)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (phtHTTPProxyThread -> init (
			psSession,
			tUTCStartConnectionTime,
			pWebServer,
			ulWebServerPort,
			_bURIForHTTPProxy. str (),
			_bURLParametersForHTTPProxy. str (),
			_ulWebServerTimeoutToWaitAnswerInSeconds,
			_pWebServerLocalIPAddress,
			_ptSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_HTTPPROXYTHREAD_INIT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			delete phtHTTPProxyThread;

			return err;
		}

		if (phtHTTPProxyThread -> start (true) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (phtHTTPProxyThread -> finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_HTTPPROXYTHREAD_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			delete phtHTTPProxyThread;

			return err;
		}
	}


	return errNoError;
}


Error CMProcessor:: handleCheckSocketsPoolEvent (
	Event_p pevEvent)

{

	Error_t					errCheckSocketsStatus;
	#ifdef PROCESSOR_CHECKSOCKETSPOOL_STATISTICS
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs1;
		unsigned long long		ullLocalExpirationLocalDateTimeInMilliSecs2;
		Boolean_t					bStatusChanged;
	#endif


	#ifdef PROCESSOR_CHECKSOCKETSPOOL_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs1) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}
	bStatusChanged				= false;
	#endif

	if ((errCheckSocketsStatus = _pspSocketsPool -> checkSocketsStatus (
		CM_CMPROCESSOR_CHECKSOCKETSSECONDSTOWAIT,
		CM_CMPROCESSOR_CHECKSOCKETSMILLISECONDSTOWAIT)) != errNoError)
	{
		if ((long) errCheckSocketsStatus !=
			SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED &&
			(long) errCheckSocketsStatus !=
			SCK_SOCKETSPOOL_POOLEMPTY)
		{
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errCheckSocketsStatus,
				__FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_CHECKSOCKETSSTATUS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		#ifdef PROCESSOR_CHECKSOCKETSPOOL_STATISTICS
		bStatusChanged				= false;
		#endif
	}

	#ifdef PROCESSOR_CHECKSOCKETSPOOL_STATISTICS
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullLocalExpirationLocalDateTimeInMilliSecs2) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		Message msg;
		if (bStatusChanged)
			msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "CheckSocketsPool time (status changed): ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
		else
			msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CMPROCESSOR_STATISTICS,
				2, "CheckSocketsPool time (status not changed): ", (long)
				(ullLocalExpirationLocalDateTimeInMilliSecs2 -
					ullLocalExpirationLocalDateTimeInMilliSecs1));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
			(const char *) msg, __FILE__, __LINE__);
	}
	#endif


	return errNoError;
}


