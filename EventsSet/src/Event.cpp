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

#include <assert.h>
#include <time.h>
#include <stdio.h>
#ifdef WIN32
	#include "Windows.h"
#endif
#include "DateTime.h"
#include "Event.h"



Event:: Event (void)

{

	_esEventStatus			= EVSET_EVENTBUILDED;

}


Event:: ~Event ()

{

	if (_esEventStatus == EVSET_EVENTINITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENT_FINISH_FAILED);
		}
	}

}


Event:: Event (const Event &)

{

	assert (1==0);
}


Event &Event:: operator = (const Event &)

{

	assert (1==0);

	return *this;
}


/*
#ifdef WIN32
	Error Event:: init (
		const char *pSource,
		const char *pDestination,
		long lTypeIdentifier,
		__int64 ullExpirationLocalDateTimeInMilliSecs)
#else
*/
	Error Event:: init (
		const char *pSource,
		const char *pDestination,
		long lTypeIdentifier,
		const char *pTypeIdentifier,
		unsigned long long ullExpirationLocalDateTimeInMilliSecs)
// #endif

{

	if (pSource == (const char *) NULL ||
		pDestination == (const char *) NULL ||
		pTypeIdentifier == (const char *) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (_esEventStatus != EVSET_EVENTBUILDED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	if (_bSource. init (pSource) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if (_bDestination. init (pDestination) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bSource. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (_bTypeIdentifier. init (pTypeIdentifier) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (_bDestination. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (_bSource. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	_lTypeIdentifier		= lTypeIdentifier;

	_ullExpirationLocalDateTimeInMilliSecs		=
		ullExpirationLocalDateTimeInMilliSecs;

	{
		tm				tmDateTime;
		unsigned long	ulMilliSecs;


		if (DateTime:: get_tm_LocalTime (&tmDateTime, &ulMilliSecs) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);

			if (_bTypeIdentifier. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bDestination. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bSource. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (DateTime:: nowUTCInMilliSecs (&_ullCreationUTCTimeInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			if (_bTypeIdentifier. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bDestination. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bSource. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		// the format date is yyyy-mm-dd-hh24-mi-ss-milliss
		sprintf (_pCreationLocalDateTime,
			"%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday),
			(unsigned long) (tmDateTime. tm_hour),
			(unsigned long) (tmDateTime. tm_min),
			(unsigned long) (tmDateTime. tm_sec),
			ulMilliSecs);
	}

	_esEventStatus			= EVSET_EVENTINITIALIZED;


	return errNoError;
}


Error Event:: finish (void)

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	if (_bTypeIdentifier. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bDestination. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	if (_bSource. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
	}

	_esEventStatus			= EVSET_EVENTBUILDED;


	return errNoError;
}


Error Event:: getTypeIdentifier (long *plTypeIdentifier)

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	*plTypeIdentifier			= _lTypeIdentifier;


	return errNoError;
}


const char *Event:: getTypeIdentifier (void)

{

	return (const char *) _bTypeIdentifier;
}


/*
#ifdef WIN32
	Error Event:: getExpirationLocalDateTimeInMilliSecs (
		__int64 *pullExpirationLocalDateTimeInMilliSecs)
#else
*/
	Error Event:: getExpirationLocalDateTimeInMilliSecs (
		unsigned long long *pullExpirationLocalDateTimeInMilliSecs)
// #endif

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	*pullExpirationLocalDateTimeInMilliSecs		=
		_ullExpirationLocalDateTimeInMilliSecs;


	return errNoError;
}


#ifdef WIN32
	Error Event:: setExpirationLocalDateTimeInMilliSecs (
		__int64 ullExpirationLocalDateTimeInMilliSecs)
#else
	Error Event:: setExpirationLocalDateTimeInMilliSecs (
		unsigned long long ullExpirationLocalDateTimeInMilliSecs)
#endif

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	_ullExpirationLocalDateTimeInMilliSecs		=
		ullExpirationLocalDateTimeInMilliSecs;


	return errNoError;
}


Error Event:: getCreationDateTime (char *pCreationDateTime)

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	strcpy (pCreationDateTime, _pCreationLocalDateTime);


	return errNoError;
}


Error Event:: getSource (Buffer_p *pbSource)

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	*pbSource			= &_bSource;


	return errNoError;
}


Error Event:: getDestination (Buffer_p *pbDestination)

{

	if (_esEventStatus != EVSET_EVENTINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _esEventStatus);

		return err;
	}

	*pbDestination			= &_bDestination;


	return errNoError;
}

