
#include "CMSocketsPool.h"
// #include "DateTime.h"
#include "ConnectionEvent.h"
#include "ConnectionsManagerMessages.h"
// #include "Session.h"
#include <assert.h>



CMSocketsPool:: CMSocketsPool (void): SocketsPool ()

{

}


CMSocketsPool:: ~CMSocketsPool (void)

{

}


CMSocketsPool:: CMSocketsPool (
	const CMSocketsPool &t)

{
	assert (1 == 0);

}


Error CMSocketsPool:: init (
	CMEventsSet_p pesEventsSet,
	Tracer_p ptTracer)

{

	_pesEventsSet			= pesEventsSet;
	_ptSystemTracer				= ptTracer;

	if (_bMainProcessor. init ("CMProcessor") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (SocketsPool:: init (CM_CMSOCKETSPOOL_MAXSOCKETSNUMBER,
		0, 0, true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_INIT_FAILED);
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


Error CMSocketsPool:: finish (void)

{

	if (SocketsPool:: finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FINISH_FAILED);
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


Error CMSocketsPool:: updateSocketStatus (
	Socket_p pSocket, long lSocketType,
	void *pvSocketData,
	unsigned short usSocketCheckType)

{

	// REMEMBER, you cannot call deleteSocket from the method if
	//  bAllowDeletionSocketFromUpdateMethod (parameter in init)
	//  is false

	switch (lSocketType)
	{
		case CM_CMSOCKETPOOL_CMSOCKET:
			{
				Event_p						pevEvent;
				ConnectionEvent_p		pevConnection;
				Session:: ConnectionInformation_p
					pciConnectionInformation;
				Error_t						errAddSocket;
				Error_t						errDelSocket;


				pciConnectionInformation		=
					((Session:: ConnectionInformation_p) pvSocketData);

				if (usSocketCheckType & SOCKETSTATUS_EXCEPTION)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CMSOCKETSPOOL_EXCEPTIONONSOCKET,
						2,
						pciConnectionInformation -> _psSession ->
						_pClientIPAddress, lSocketType);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}
				else if (usSocketCheckType & SOCKETSTATUS_READ)
				{
					// since select could return a 'ready to read' also in case
					// there is nothing to read, the next check is to avoid to
					// generate a useless CM_EVENT_CONNECTIONREADYTOREAD event
					{
						SocketImpl_p		psiClientSocketImpl;
						Error_t				errSocket;
						Boolean_t			bIsReadyForReading;
						// char				pBuffer [2];
						unsigned long		ulBufferLength;


						if ((errSocket = pSocket -> getSocketImpl (
							&psiClientSocketImpl)) != errNoError)
						{
							Error err = SocketErrors (__FILE__, __LINE__,
								SCK_SOCKETIMPL_READ_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);

							return errSocket;
						}

						ulBufferLength			= 1;

						if ((errSocket = psiClientSocketImpl ->
							isReadyForReading (
							&bIsReadyForReading,
							0,		// ulSecondsToWait
							1000	// ulAdditionalMicrosecondsToWait
							)) != errNoError)
						{
							// if ((unsigned long) errSocket == SCK_NOTHINGTOREAD)
							// {
								// nothing to be read

							// 	return errNoError;
							// }
							// else if ((unsigned long) errSocket ==
							// 	SCK_READ_EOFREACHED)
							// {
							// }
							// else
							// {
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errSocket,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_ISREADYFORREADING_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								return errSocket;
							// }
						}

						if (!bIsReadyForReading)
						{
								return errNoError;
						}

						/*
						if ((errSocket = psiClientSocketImpl -> read (
							pBuffer,
							&ulBufferLength,
							false,	// bReadingCheckToBePerformed
							0,		// ulSecondsToWait
							0,	// ulAdditionalMicrosecondsToWait
							false,	// bOneShotRead
							false	// bRemoveDataFromSocket
							)) != errNoError)
						{
							if ((unsigned long) errSocket == SCK_NOTHINGTOREAD)
							{
								// nothing to be read

								return errNoError;
							}
							else if ((unsigned long) errSocket ==
								SCK_READ_EOFREACHED)
							{
							}
							else
							{
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) errSocket,
									__FILE__, __LINE__);

								Error err = SocketErrors (__FILE__, __LINE__,
									SCK_SOCKETIMPL_READ_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);

								return errSocket;
							}
						}
						*/
					}

					if (_pesEventsSet -> getFreeEvent (
						CMEventsSet:: CM_EVENTTYPE_CONNECTIONIDENTIFIER,
						&pevEvent) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_GETFREEEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						return err;
					}

					pevConnection			=
						(ConnectionEvent_p) pevEvent;

					// we passed the _tUTCStartConnectionTime field as
					// separated field because from now, if will do a read or
					// a write on the StreamerSession we have to be sure that
					// the StreamerSession is not changed
					// (released or new one's)
					if (pevConnection -> init (
						CM_CMSOCKETSPOOL_SOURCE,
						(const char *) _bMainProcessor,
						CM_EVENT_CONNECTIONREADYTOREAD,
						"CM_EVENT_CONNECTIONREADYTOREAD",
						pciConnectionInformation,
						_ptSystemTracer) != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
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

						return err;
					}

					{
						/*
						SocketImpl_p					psiClientSocketImpl;
						pSocket -> getSocketImpl (&psiClientSocketImpl);
						*/
						Message msg = ConnectionsManagerMessages (
							__FILE__, __LINE__,
							CM_SESSION_DELETESOCKET,
							2,
							pciConnectionInformation -> _ulConnectionIdentifier,
							0);
							// (long) (psiClientSocketImpl -> _iFd));
						_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
							(const char *) msg, __FILE__, __LINE__);
					}

					if ((errDelSocket = deleteSocket (
						pSocket, (void **) NULL)) != errNoError)
					{
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) errDelSocket, __FILE__, __LINE__);

						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_DELETESOCKETFAILED,
							1,
							pciConnectionInformation ->
								_ulConnectionIdentifier);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if (pevConnection -> finish () != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_CONNECTIONEVENT_FINISH_FAILED);
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

						return err;
					}

					if (_pesEventsSet -> addEvent (
						pevConnection) != errNoError)
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_ADDEVENT_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						{
							/*
							SocketImpl_p			psiClientSocketImpl;
							pSocket -> getSocketImpl (&psiClientSocketImpl);
							*/
							Message msg = ConnectionsManagerMessages (
								__FILE__, __LINE__,
								CM_SESSION_ADDSOCKET,
								2,
								pciConnectionInformation ->
									_ulConnectionIdentifier, 0);
								// (long) (psiClientSocketImpl -> _iFd));
							_ptSystemTracer -> trace (Tracer:: TRACER_LDBG4,
								(const char *) msg, __FILE__, __LINE__);
						}

						if ((errAddSocket = addSocket (
							SocketsPool:: SOCKETSTATUS_READ |
							SocketsPool:: SOCKETSTATUS_EXCEPTION,
							CM_CMSOCKETPOOL_CMSOCKET,
							pSocket, pvSocketData)) != errNoError)
						{
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) errAddSocket,
								__FILE__, __LINE__);

							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_ADDSOCKETFAILED,
								1,
								pciConnectionInformation ->
								_ulConnectionIdentifier);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (pevConnection -> finish () != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_CONNECTIONEVENT_FINISH_FAILED);
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

						return err;
					}
				}
			}

			break;
		default:
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_WRONGSOCKETTYPE,
					1, lSocketType);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
	}


	return errNoError;
}

