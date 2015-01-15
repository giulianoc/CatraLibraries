#include <assert.h>
#ifdef WIN32
	#include "Windows.h"
#endif
#include "ConnectionEvent.h"
#include "DateTime.h"


ConnectionEvent:: ConnectionEvent (void): Event ()

{

}


ConnectionEvent:: ~ConnectionEvent (void)

{

}


ConnectionEvent:: ConnectionEvent (
	const ConnectionEvent &)

{

	assert (1==0);
}


ConnectionEvent &ConnectionEvent:: operator = (
	const ConnectionEvent &)

{

	assert (1==0);

	return *this;
}


Error ConnectionEvent:: init (
	const char *pSource,
	const char *pDestination,
	long lTypeIdentifier,
	const char *pTypeIdentifier,
	Session:: ConnectionInformation_p pciConnectionInformation,
	Tracer_p ptTracer)

{

	unsigned long long	ullLocalExpirationLocalDateTimeInMilliSecs;


	// _pciConnectionInformation is NULL in case of new connection
	if (pciConnectionInformation ==
		(Session:: ConnectionInformation_p) NULL)
	{
		_bConnectionInformationInitialized			= false;
	}
	else
	{
		_bConnectionInformationInitialized			= true;
		_ciConnectionInformation		= *pciConnectionInformation;
	}
	_ptSystemTracer					= ptTracer;

	if (DateTime:: nowLocalInMilliSecs (
		&ullLocalExpirationLocalDateTimeInMilliSecs) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (Event:: init (pSource, pDestination, lTypeIdentifier,
		pTypeIdentifier, ullLocalExpirationLocalDateTimeInMilliSecs) !=
		errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error ConnectionEvent:: finish (void)

{

	if (Event:: finish () != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	_bConnectionInformationInitialized			= false;


	return errNoError;
}


Error ConnectionEvent:: getConnectionInformation (
	Session:: ConnectionInformation_p pciConnectionInformation)

{

	if (!_bConnectionInformationInitialized)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONEVENT_CONNECTIONINFORMATIONNOTINITIALIZED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	*pciConnectionInformation			= _ciConnectionInformation;


	return errNoError;
}

