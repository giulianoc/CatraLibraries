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

#include "DateTime.h"
#include <assert.h>
#ifdef WIN32
	#include <winsock2.h>
#endif
#include "EventsSet.h"
#include "FileIO.h"


EventsSet:: EventsSet (void)

{

	_essEventsSetStatus			= EVSET_EVENTSSETBUILDED;

}


EventsSet:: ~EventsSet (void)

{

}


EventsSet:: EventsSet (const EventsSet &)

{

	assert (1==0);
}


EventsSet &EventsSet:: operator = (const EventsSet &)

{

	assert (1==0);

	return *this;
}


Error EventsSet:: init (
	Boolean_t bConditionVariablesToBeUsed,
	unsigned long ulNumberOfDifferentEventTypeToManage)

{

	if (_essEventsSetStatus != EVSET_EVENTSSETBUILDED ||
		ulNumberOfDifferentEventTypeToManage <= 0)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		return err;
	}

	_ulNumberOfDifferentEventTypeToManage		=
		ulNumberOfDifferentEventTypeToManage;
	_bConditionVariablesToBeUsed				= bConditionVariablesToBeUsed;

	// prepare the data to save the event type info
	{
		unsigned long			ulDifferentEventTypeIndex;


		if ((_petiEventTypeInfo = new EventTypeInfo_t [
			_ulNumberOfDifferentEventTypeToManage]) ==
			(EventTypeInfo_p) NULL)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_NEW_FAILED);

			return err;
		}

		for (ulDifferentEventTypeIndex = 0;
			ulDifferentEventTypeIndex < _ulNumberOfDifferentEventTypeToManage;
			ulDifferentEventTypeIndex++)
			(_petiEventTypeInfo [ulDifferentEventTypeIndex]).
				_ulPreAllocatedEventsNumber		= 0;
	}

	#if defined(__CYGWIN__)
		if (_mtFreeEvents. init (
			PMutex:: MUTEX_RECURSIVE) != errNoError)
	#else							// POSIX.1-1996 standard (HPUX 11)
		if (_mtFreeEvents. init (
			PMutex:: MUTEX_RECURSIVE) != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		delete [] _petiEventTypeInfo;
		_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

		return err;
	}

	#if defined(__CYGWIN__)
		if (_evSetMutex. init (PMutex:: MUTEX_RECURSIVE) != errNoError)
	#else							// POSIX.1-1996 standard (HPUX 11)
		if (_evSetMutex. init (PMutex:: MUTEX_RECURSIVE) != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		if (_mtFreeEvents. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete [] _petiEventTypeInfo;
		_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

		return err;
	}

	if ((_psHasher = new BufferHasher_t) == (BufferHasher_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_NEW_FAILED);

		if (_evSetMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		if (_mtFreeEvents. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete [] _petiEventTypeInfo;
		_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

		return err;
	}

	if ((_psComparer = new BufferCmp_t) == (BufferCmp_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_NEW_FAILED);

		delete _psHasher;

		if (_evSetMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		if (_mtFreeEvents. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete [] _petiEventTypeInfo;
		_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

		return err;
	}

	if ((_pesmEventsSetHashMap =
		new EventsSetHashMap_t (100, *_psHasher, *_psComparer)) ==
		(EventsSetHashMap_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_NEW_FAILED);

		delete _psComparer;
		delete _psHasher;

		if (_evSetMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		if (_mtFreeEvents. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete [] _petiEventTypeInfo;
		_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

		return err;
	}

	_essEventsSetStatus		= EVSET_EVENTSSETINITIALIZED;


	return errNoError;
}


Error EventsSet:: finish (void)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	Buffer_p						pbDestination;
	DestinationEvent_p				pedDestinationEvents;
	// EventsMultiMap_t:: iterator		itEvents;
	// Event_p							pevEvent;
	// #ifdef WIN32
	// 	__int64			llExpirationLocalDateTimeInMilliSecs;
	// #else
	// 	long long		llExpirationLocalDateTimeInMilliSecs;
	// #endif


	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		return err;
	}

	{
		if (_evSetMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			// return err;
		}

		for (itDestinations = _pesmEventsSetHashMap -> begin ();
			itDestinations != _pesmEventsSetHashMap -> end ();
			++itDestinations)
		{
			pbDestination					= itDestinations -> first;

			pedDestinationEvents	= itDestinations -> second;

			if (pbDestination -> finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (_bConditionVariablesToBeUsed)
			{
				if ((pedDestinationEvents -> _cdAddedEvent).
					finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PCONDITION_FINISH_FAILED);
				}
			}

			/*
			for (itEvents = (pedDestinationEvents -> _emEventsMultiMap).
				begin ();
				itEvents != (pedDestinationEvents -> _emEventsMultiMap). end ();
				++itEvents)
			{
				llExpirationLocalDateTimeInMilliSecs		= itEvents -> first;

				pevEvent			= itEvents -> second;

				if (pevEvent -> finish () != errNoError)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENT_FINISH_FAILED);

					// return err;
				}

				delete pevEvent;
			}
			*/
			(pedDestinationEvents -> _emEventsMultiMap). clear ();

			delete pedDestinationEvents;
			pedDestinationEvents		= (DestinationEvent_p) NULL;
		}

		_pesmEventsSetHashMap -> clear ();

		delete _pesmEventsSetHashMap;

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		delete _psComparer;
		delete _psHasher;

		if (_evSetMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	}

	{
		unsigned long						ulDifferentEventTypeIndex;


		if (_mtFreeEvents. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			// return err;
		}

		for (ulDifferentEventTypeIndex = 0;
			ulDifferentEventTypeIndex < _ulNumberOfDifferentEventTypeToManage;
			ulDifferentEventTypeIndex++)
		{
			(_petiEventTypeInfo [ulDifferentEventTypeIndex]).
				_vFreeEvents. clear ();

			(_petiEventTypeInfo [ulDifferentEventTypeIndex]).
				_ulPreAllocatedEventsNumber			= 0;

			if (deleteAllocatedEvents (ulDifferentEventTypeIndex,
				&((_petiEventTypeInfo [ulDifferentEventTypeIndex]).
				_vPointersToAllocatedEvents)) != errNoError)
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_DELETEALLOCATEDEVENTS_FAILED);

				// if (_mtFreeEvents. unLock () != errNoError)
				// {
				// 	Error err = PThreadErrors (__FILE__, __LINE__,
				// 		THREADLIB_PMUTEX_UNLOCK_FAILED);
				// }

				// return err;
			}

			(_petiEventTypeInfo [ulDifferentEventTypeIndex]).
				_vPointersToAllocatedEvents. clear ();
		}

		if (_mtFreeEvents. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	}

	if (_mtFreeEvents. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
	}

	delete [] _petiEventTypeInfo;
	_petiEventTypeInfo		= (EventTypeInfo_p) NULL;

	_essEventsSetStatus		= EVSET_EVENTSSETBUILDED;


	return errNoError;
}


Error EventsSet:: getFreeEvent (
	unsigned long ulEventTypeIndex,
	Event_p *pevEvent)

{
	if (_mtFreeEvents. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

/*
char aaa[1024];
sprintf (aaa, "\ngetFreeEvent %lu. Prima Size: %lu\n", ulEventTypeIndex,
	(unsigned long) ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size ())
);
ptTracer -> trace (Tracer:: TRACER_LINFO,
	aaa, __FILE__, __LINE__);
*/

	if (ulEventTypeIndex >= _ulNumberOfDifferentEventTypeToManage)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_EVENTTYPEOVERCOMENUMBEROFDIFFERENTEVENTTYPES,
			ulEventTypeIndex, _ulNumberOfDifferentEventTypeToManage);

		if (_mtFreeEvents. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size () == 0)
	{
		Error_t					errAllocate;


		if ((errAllocate = allocateMoreFreeUserEvents (ulEventTypeIndex,
			&((_petiEventTypeInfo [ulEventTypeIndex]).
			_ulPreAllocatedEventsNumber),
			&((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents),
			&((_petiEventTypeInfo [ulEventTypeIndex]).
			_vPointersToAllocatedEvents))) != errNoError)
		{
			// Error err = EventsSetErrors (__FILE__, __LINE__,
			// 	EVSET_EVENTSSET_ALLOCATEMOREFREEUSEREVENTS_FAILED);

			if (_mtFreeEvents. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return errAllocate;
		}

		if ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size () == 0)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_NOFREEEVENTSALLOCATED);

			if (_mtFreeEvents. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}

	*pevEvent			= *((_petiEventTypeInfo [ulEventTypeIndex]).
		_vFreeEvents. begin ());

	(_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. erase (
		(_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. begin ());

/*
sprintf (aaa, "\ngetFreeEvent %lu. Dopo Size: %lu\n", ulEventTypeIndex,
	(unsigned long) ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size ())
);
ptTracer -> trace (Tracer:: TRACER_LINFO,
	aaa, __FILE__, __LINE__);
*/

	if (_mtFreeEvents. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: releaseEvent (
	unsigned long ulEventTypeIndex,
	Event_p pevEvent)

{

	if (_mtFreeEvents. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

/*
char aaa[1024];
sprintf (aaa, "\nreleaseEvent %lu. Prima Size: %lu\n", ulEventTypeIndex,
	(unsigned long) ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size ())
);
ptTracer -> trace (Tracer:: TRACER_LINFO,
	aaa, __FILE__, __LINE__);
*/

	if (ulEventTypeIndex >= _ulNumberOfDifferentEventTypeToManage)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_EVENTTYPEOVERCOMENUMBEROFDIFFERENTEVENTTYPES,
			ulEventTypeIndex, _ulNumberOfDifferentEventTypeToManage);

		if (_mtFreeEvents. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	(_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. insert (
		(_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. end (),
		pevEvent);

/*
sprintf (aaa, "\nreleaseEvent %lu. Dopo Size: %lu\n", ulEventTypeIndex,
	(unsigned long) ((_petiEventTypeInfo [ulEventTypeIndex]). _vFreeEvents. size ())
);
ptTracer -> trace (Tracer:: TRACER_LINFO,
	aaa, __FILE__, __LINE__);
*/

	if (_mtFreeEvents. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: allocateMoreFreeUserEvents (
	unsigned long ulEventTypeIndex,
	unsigned long *_pulPreAllocatedEventsNumber,
	std:: vector<Event_p> *pvFreeEvents,
	std:: vector<Event_p> *pvPointersToAllocatedEvents)

{

	unsigned long			ulFreeEventsNumberAllocated;
	unsigned long			ulEventIndex;


	switch (ulEventTypeIndex)
	{
		case EVSET_EVENTTYPE_EVENTIDENTIFIER:
			{
				// Important: if you will allocate a type derived
				//	from Event_t (next new), the type of the next variable
				//	must be the derived type and not Event_p
				Event_p				pevEvents;


				ulFreeEventsNumberAllocated			=
					EVSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED;

				if ((pevEvents = new Event_t [ulFreeEventsNumberAllocated]) ==
					(Event_p) NULL)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_NEW_FAILED);

					return err;
				}

				(*_pulPreAllocatedEventsNumber)			+=
					ulFreeEventsNumberAllocated;

				pvFreeEvents -> reserve (*_pulPreAllocatedEventsNumber);

				pvPointersToAllocatedEvents -> insert (
					pvPointersToAllocatedEvents -> end (),
					pevEvents);

				for (ulEventIndex = 0;
					ulEventIndex < ulFreeEventsNumberAllocated;
					ulEventIndex++)
				{
					pvFreeEvents -> insert (pvFreeEvents -> end (),
						&(pevEvents [ulEventIndex]));
				}
			}

			break;
		default:
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_UNKNOWNEVENTTYPE);

			return err;
	}

	
	return errNoError;
}


Error EventsSet:: deleteAllocatedEvents (
	unsigned long ulEventTypeIndex,
	std:: vector<Event_p> *pvPointersToAllocatedEvents)

{

	std:: vector<Event_p>:: const_iterator	itPreAllocatedEvents;


	switch (ulEventTypeIndex)
	{
		case EVSET_EVENTTYPE_EVENTIDENTIFIER:
			{
				// Important: if you will allocate a type derived
				//	from Event_t (next new), the type of the next variable
				//	must be the derived type and not Event_p
				Event_p				pevPreAllocatedEvents;


				for (itPreAllocatedEvents =
					pvPointersToAllocatedEvents -> begin ();
					itPreAllocatedEvents !=
					pvPointersToAllocatedEvents -> end ();
					++itPreAllocatedEvents)
				{
					pevPreAllocatedEvents			= *itPreAllocatedEvents;

					delete [] pevPreAllocatedEvents;
				}
			}

			break;
		default:
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_UNKNOWNEVENTTYPE);

			return err;
	}

	
	return errNoError;
}


Error EventsSet:: addEvent (Event_p pevEvent)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	EventsMultiMap_t:: iterator		itEvents;
	DestinationEvent_p				pedDestinationEvents;
	/*
	#ifdef WIN32
		__int64				ullExpirationLocalDateTimeInMilliSecs;
		__int64				ullExpirationDateTimeInMilliSecsOfHeadEvent;
	#else
	*/
		unsigned long long	ullExpirationLocalDateTimeInMilliSecs;
		unsigned long long	ullExpirationDateTimeInMilliSecsOfHeadEvent;
	// #endif
	unsigned long			ulMultiMapSize;
	Event_p					pevHeadEvent;
	Buffer_p				pbDestination;


	if (pevEvent == (Event_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (pevEvent -> getDestination (
		&pbDestination) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_GETDESTINATION_FAILED);

		return err;
	}

	if (pevEvent -> getExpirationLocalDateTimeInMilliSecs (
		&ullExpirationLocalDateTimeInMilliSecs) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED);

		return err;
	}

	if (_evSetMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	itDestinations		= _pesmEventsSetHashMap -> find (pbDestination);

	if (itDestinations == _pesmEventsSetHashMap -> end ())
	{
		if ((pedDestinationEvents = new DestinationEvent_t) ==
			(DestinationEvent_p) NULL)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_NEW_FAILED);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (pedDestinationEvents -> _bDestination. init (
			(const char *) (*pbDestination)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			delete pedDestinationEvents;
			pedDestinationEvents		= (DestinationEvent_p) NULL;

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (_bConditionVariablesToBeUsed)
		{
			if ((pedDestinationEvents -> _cdAddedEvent). init (
				PCondition:: COND_DEFAULT) != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PCONDITION_INIT_FAILED);

				if (pedDestinationEvents -> _bDestination. finish () !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				delete pedDestinationEvents;
				pedDestinationEvents		= (DestinationEvent_p) NULL;

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}

		_pesmEventsSetHashMap -> insert (
			&(pedDestinationEvents -> _bDestination), pedDestinationEvents);
	}
	else
		pedDestinationEvents			= itDestinations -> second;

	ulMultiMapSize						=
		(pedDestinationEvents -> _emEventsMultiMap). size ();
/*
static long lMaxMultiMapSize = -1;
if (lMaxMultiMapSize == -1)
	lMaxMultiMapSize			= ulMultiMapSize;

if (ulMultiMapSize > lMaxMultiMapSize)
{
	lMaxMultiMapSize			= ulMultiMapSize;
char pBuffer [1024];
sprintf (pBuffer, "EVENTSSET. MultiMap max size: %lu\n", ulMultiMapSize);
#ifdef WIN32
FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
#else
FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
#endif
}
*/

	if (ulMultiMapSize > 0)
	{
		pevHeadEvent			=
			((pedDestinationEvents -> _emEventsMultiMap). begin ()) -> second;

		if (pevHeadEvent -> getExpirationLocalDateTimeInMilliSecs (
			&ullExpirationDateTimeInMilliSecsOfHeadEvent) != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}
	else
		pevHeadEvent			= (Event_p) NULL;

	#ifdef WIN32
		(pedDestinationEvents -> _emEventsMultiMap).
			insert (std:: pair<__int64, Event_p>(
			ullExpirationLocalDateTimeInMilliSecs, pevEvent));
	#else
		(pedDestinationEvents -> _emEventsMultiMap).
			insert (std:: pair<unsigned long long, Event_p>(
			ullExpirationLocalDateTimeInMilliSecs, pevEvent));
	#endif

	if (pevHeadEvent == (Event_p) NULL)
	{
		if (_bConditionVariablesToBeUsed)
		{
			// Every destination has his own condition variable.
			// When an event is added it is sufficient that just one
			// thread waiting the event for that 'destination' is waken up.
			// This is the reason it is used signal and not broadcast.
			if ((pedDestinationEvents -> _cdAddedEvent). signal () !=
				errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PCONDITION_SIGNAL_FAILED);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}
	}
	else
	{
		if (ullExpirationLocalDateTimeInMilliSecs <
			ullExpirationDateTimeInMilliSecsOfHeadEvent)
		{
			if (_bConditionVariablesToBeUsed)
			{
				// Every destination has his own condition variable.
				// When an event is added it is sufficient that just one
				// thread waiting the event for that 'destination' is waken up.
				// This is the reason it is used signal and not broadcast.
				if ((pedDestinationEvents -> _cdAddedEvent). signal () !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PCONDITION_SIGNAL_FAILED);

					if (_evSetMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}

					return err;
				}
			}
		}
	}

	if (_evSetMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: addDestination (const char *pDestination)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	DestinationEvent_p				pedDestinationEvents;


	if (pDestination == (const char *) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (_evSetMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if ((pedDestinationEvents = new DestinationEvent_t) ==
		(DestinationEvent_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_NEW_FAILED);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (pedDestinationEvents -> _bDestination. init (
		pDestination) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		delete pedDestinationEvents;
		pedDestinationEvents		= (DestinationEvent_p) NULL;

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	itDestinations		= _pesmEventsSetHashMap -> find (
		&(pedDestinationEvents -> _bDestination));

	if (itDestinations != _pesmEventsSetHashMap -> end ())
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_DESTINATIONALREADYADDED,
			1, pDestination);

		if (pedDestinationEvents -> _bDestination. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		delete pedDestinationEvents;
		pedDestinationEvents		= (DestinationEvent_p) NULL;

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	if (_bConditionVariablesToBeUsed)
	{
		if ((pedDestinationEvents -> _cdAddedEvent). init (
			PCondition:: COND_DEFAULT) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PCONDITION_INIT_FAILED);

			if (pedDestinationEvents -> _bDestination. finish () !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			delete pedDestinationEvents;
			pedDestinationEvents		= (DestinationEvent_p) NULL;

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}

	_pesmEventsSetHashMap -> insert (
		&(pedDestinationEvents -> _bDestination), pedDestinationEvents);

	if (_evSetMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: deleteEvent (Event_p pevEvent)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	DestinationEvent_p				pedDestinationEvents;
	EventsMultiMap_t:: iterator		itEvents;
	/*
	#ifdef WIN32
		__int64		ullExpirationLocalDateTimeInMilliSecs;
	#else
	*/
		unsigned long long	ullExpirationLocalDateTimeInMilliSecs;
	// #endif
	Buffer_p				pbDestination;


	if (pevEvent == (Event_p) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (pevEvent -> getDestination (
		&pbDestination) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_GETDESTINATION_FAILED);

		return err;
	}

	if (pevEvent -> getExpirationLocalDateTimeInMilliSecs (
		&ullExpirationLocalDateTimeInMilliSecs) != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED);

		return err;
	}

	if (_evSetMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	itDestinations		= _pesmEventsSetHashMap -> find (pbDestination);

	if (itDestinations == _pesmEventsSetHashMap -> end ())
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_DESTINATIONNOTFOUND,
			1, (const char *) (*pbDestination));

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	pedDestinationEvents			= itDestinations -> second;

	itEvents			=
		(pedDestinationEvents -> _emEventsMultiMap).
		find (ullExpirationLocalDateTimeInMilliSecs);

	if (itEvents == (pedDestinationEvents ->
		_emEventsMultiMap). end ())
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_EVENTNOTFOUND,
			1, ullExpirationLocalDateTimeInMilliSecs);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	(pedDestinationEvents -> _emEventsMultiMap).
		erase (itEvents);

	if (_evSetMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: getFirstEvent (
	Buffer_p pbDestination, Event_p *pevEvent,
	Boolean_p pbEventExpired,
	Boolean_t bBlocking, unsigned long ulSecondsToBlock,
	unsigned long ulAdditionalMilliSecondsToBlock)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	DestinationEvent_p				pedDestinationEvents;
	EventsMultiMap_t:: iterator		itEvents;


	if (pbDestination == (Buffer_p) NULL ||
		pevEvent == (Event_p *) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (_evSetMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	itDestinations		= _pesmEventsSetHashMap -> find (pbDestination);

	if (itDestinations == _pesmEventsSetHashMap -> end ())
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_DESTINATIONNOTFOUND,
			1, (const char *) (*pbDestination));

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	pedDestinationEvents			= itDestinations -> second;

	if ((pedDestinationEvents ->
		_emEventsMultiMap). begin () ==
		(pedDestinationEvents ->
		_emEventsMultiMap). end ())
	{
		if (_bConditionVariablesToBeUsed)
		{
			if (bBlocking)
			{
				Error_t				errTimedWait;


				if ((errTimedWait = (pedDestinationEvents -> _cdAddedEvent).
					timedWait (&_evSetMutex, ulSecondsToBlock,
					ulAdditionalMilliSecondsToBlock)) != errNoError)
				{
					int					i_errno;
					unsigned long		ulUserDataBytes;


					if ((long) errTimedWait == THREADLIB_COND_TIMEDWAIT_FAILED)
					{
						errTimedWait. getUserData (&i_errno, &ulUserDataBytes);

						#if defined(__hpux) && defined(_CMA__HP)
							if (i_errno == 11)	// EAGAIN
						#elif WIN32
							if (i_errno == WSAETIMEDOUT)
						#else
							if (i_errno == ETIMEDOUT)
						#endif
						{
							// time expired
							Error err = EventsSetErrors (__FILE__, __LINE__,
								EVSET_EVENTSSET_NOEVENTSFOUND);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}
						else
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return errTimedWait;
						}
					}
					else
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

						if (_evSetMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}

						return errTimedWait;
					}
				}
				else
				{
					// an event must be added

					if ((pedDestinationEvents ->
						_emEventsMultiMap). begin () ==
						(pedDestinationEvents ->
						_emEventsMultiMap). end ())
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

						if (_evSetMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}

						return err;
					}
				}
			}
			else
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_NOEVENTSFOUND);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}
		else
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_NOEVENTSFOUND);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}

	itEvents		= (pedDestinationEvents -> _emEventsMultiMap).
		begin ();
	*pevEvent		= itEvents -> second;

	{
		/*
		#ifdef WIN32
			__int64		ullEventExpirationLocalDateTimeInMilliSecs;
			__int64		ullNowLocalInMilliSecs;
		#else
		*/
			unsigned long long	ullEventExpirationLocalDateTimeInMilliSecs;
			unsigned long long	ullNowLocalInMilliSecs;
		// #endif


		if ((*pevEvent) -> getExpirationLocalDateTimeInMilliSecs (
			&ullEventExpirationLocalDateTimeInMilliSecs) != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (ullNowLocalInMilliSecs < ullEventExpirationLocalDateTimeInMilliSecs)
			*pbEventExpired			= false;
		else
			*pbEventExpired			= true;
	}

	if (_evSetMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error EventsSet:: getAndRemoveFirstEvent (
	Buffer_p pbDestination, Event_p *pevEvent,
	Boolean_t bBlocking, unsigned long ulSecondsToBlock,
	unsigned long ulAdditionalMilliSecondsToBlock)

{

	EventsSetHashMap_t:: iterator	itDestinations;
	DestinationEvent_p				pedDestinationEvents;
	EventsMultiMap_t:: iterator		itEvents;
	/*
	#ifdef WIN32
		__int64				ullInitialLocalInMilliSecs;
	#else
	*/
		unsigned long long	ullInitialLocalInMilliSecs;
	// #endif
	/*
	#ifdef WIN32
		__int64				ullNowLocalInMilliSecs;
	#else
	*/
		unsigned long long	ullNowLocalInMilliSecs;
	// #endif
	unsigned long			ulMilliSecondsToWait;
	unsigned long			ulRemainingSecondsToWait;
	unsigned long			ulRemainingMilliSecondsToWait;
	long					lRemainingTimeInMilliSecs;



	if (pbDestination == (Buffer_p) NULL ||
		pevEvent == (Event_p *) NULL)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_ACTIVATION_WRONG);

		return err;
	}

	if (bBlocking)
	{
		if (DateTime:: nowLocalInMilliSecs (
			&ullInitialLocalInMilliSecs) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

			return err;
		}
	}

	ulMilliSecondsToWait		=
		(ulSecondsToBlock * 1000) + ulAdditionalMilliSecondsToBlock;

	if (_evSetMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_essEventsSetStatus != EVSET_EVENTSSETINITIALIZED)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_OPERATION_NOTALLOWED,
			1, (long) _essEventsSetStatus);

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	itDestinations		= _pesmEventsSetHashMap -> find (pbDestination);

	if (itDestinations == _pesmEventsSetHashMap -> end ())
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_DESTINATIONNOTFOUND,
			1, (const char *) (*pbDestination));

		if (_evSetMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	pedDestinationEvents			= itDestinations -> second;

	if ((pedDestinationEvents ->
		_emEventsMultiMap). begin () ==
		(pedDestinationEvents ->
		_emEventsMultiMap). end ())
	{
		if (_bConditionVariablesToBeUsed)
		{
			if (bBlocking)
			{
				Error_t				errTimedWait;
				Boolean_t			bSignalReceivedButEventNotPresent;


				ulRemainingSecondsToWait		= ulSecondsToBlock;

				ulRemainingMilliSecondsToWait	=
					ulAdditionalMilliSecondsToBlock;

				bSignalReceivedButEventNotPresent				= true;

				while (bSignalReceivedButEventNotPresent)
				{
					// the signal API (addEvent method) could unblock more
					// than one threads. In this case the event cannot be
					// found because the previous thread took it
					if ((errTimedWait = (pedDestinationEvents -> _cdAddedEvent).
						timedWait (&_evSetMutex, ulRemainingSecondsToWait,
						ulRemainingMilliSecondsToWait)) != errNoError)
					{
						int					i_errno;
						unsigned long		ulUserDataBytes;


						if ((long) errTimedWait ==
							THREADLIB_COND_TIMEDWAIT_FAILED)
						{
							errTimedWait. getUserData (
								&i_errno, &ulUserDataBytes);

							#if defined(__hpux) && defined(_CMA__HP)
								if (i_errno == 11)	// EAGAIN
							#elif WIN32
								if (i_errno == WSAETIMEDOUT)
							#else
								if (i_errno == ETIMEDOUT)
							#endif
							{
								// also if we have a timeout, I'll do the check
								// to verify the timeout is expired properly

								if (DateTime:: nowLocalInMilliSecs (
									&ullNowLocalInMilliSecs) != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

									if (_evSetMutex. unLock () != errNoError)
									{
										Error err = PThreadErrors (
											__FILE__, __LINE__,
											THREADLIB_PMUTEX_UNLOCK_FAILED);
									}

									return err;
								}

								lRemainingTimeInMilliSecs			= (long) (
									ulMilliSecondsToWait -
									((unsigned long) (ullNowLocalInMilliSecs -
									ullInitialLocalInMilliSecs)));

								if (lRemainingTimeInMilliSecs <= 0)
								{
									// blocking time expired

									Error err = EventsSetErrors (
										__FILE__, __LINE__,
										EVSET_EVENTSSET_NOEVENTSFOUND);

									if (_evSetMutex. unLock () != errNoError)
									{
										Error err = PThreadErrors (
											__FILE__, __LINE__,
											THREADLIB_PMUTEX_UNLOCK_FAILED);
									}

									return err;
								}

								ulRemainingSecondsToWait		=
									(unsigned long)
									(lRemainingTimeInMilliSecs / 1000);

								ulRemainingMilliSecondsToWait	=
									lRemainingTimeInMilliSecs -
									ulRemainingSecondsToWait * 1000;

// char pBuffer [1024];
// sprintf (pBuffer,
// 	"EVENTSSET. TIMEOUT Remaining (secs-millisecs): %lu-%lu Input: %lu-%lu\n",
// 	ulRemainingSecondsToWait, ulRemainingMilliSecondsToWait,
// 	ulSecondsToBlock, ulAdditionalMilliSecondsToBlock);
// #ifdef WIN32
// 	FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
// #else
//	FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
// #endif

								continue;
							}
							else
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

								if (_evSetMutex. unLock () != errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_UNLOCK_FAILED);
								}

								return errTimedWait;
							}
						}
						else
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return errTimedWait;
						}
					}

					// an event must be added. Since we work with multi thereads
					// I do the check in any case

					if ((pedDestinationEvents ->
						_emEventsMultiMap). begin () ==
						(pedDestinationEvents ->
						_emEventsMultiMap). end ())
					{
						if (DateTime:: nowLocalInMilliSecs (
							&ullNowLocalInMilliSecs) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (
									__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}

						lRemainingTimeInMilliSecs			= (long) (
							ulMilliSecondsToWait -
							((unsigned long) (ullNowLocalInMilliSecs -
							ullInitialLocalInMilliSecs)));

						if (lRemainingTimeInMilliSecs <= 0)
						{
							// blocking time expired

							Error err = EventsSetErrors (__FILE__, __LINE__,
								EVSET_EVENTSSET_NOEVENTSFOUND);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (
									__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}

						ulRemainingSecondsToWait		= (unsigned long)
							(lRemainingTimeInMilliSecs / 1000);

						ulRemainingMilliSecondsToWait	=
							lRemainingTimeInMilliSecs -
							ulRemainingSecondsToWait * 1000;

// char pBuffer [1024];
// sprintf (pBuffer,
// 	"EVENTSSET. NOEVENTS Remaining (secs-millisecs): %lu-%lu Input: %lu-%lu\n",
// 	ulRemainingSecondsToWait, ulRemainingMilliSecondsToWait,
//	ulSecondsToBlock, ulAdditionalMilliSecondsToBlock);
//#ifdef WIN32
//	FileIO:: appendBuffer ("C:\\times.txt", pBuffer, false);
//#else
//	FileIO:: appendBuffer ("/tmp/times.txt", pBuffer, false);
//#endif

					}
					else
						bSignalReceivedButEventNotPresent				= false;
				}
			}
			else
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_NOEVENTSFOUND);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}
		else
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_NOEVENTSFOUND);

			if (_evSetMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}
	}

	// sure there is an event inside the set but, after the call of timedWait
	// of the condition, the event could not be present anymore

	{
		/*
		#ifdef WIN32
			__int64				ullEventExpirationLocalDateTimeInMilliSecs;
		#else
		*/
			unsigned long long	ullEventExpirationLocalDateTimeInMilliSecs;
		// #endif
		Boolean_t				bFoundExpiredEvent;
		Error_t					errTimedWait;
		Boolean_t				bEventToBeReleasedAfterTimeout;


		bFoundExpiredEvent			= false;

		while (!bFoundExpiredEvent)
		{
			itEvents		= (pedDestinationEvents -> _emEventsMultiMap).
				begin ();
			if (itEvents == (pedDestinationEvents -> _emEventsMultiMap). end ())
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_NOEVENTSFOUND);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}

			*pevEvent		= itEvents -> second;

			if ((*pevEvent) -> getExpirationLocalDateTimeInMilliSecs (
				&ullEventExpirationLocalDateTimeInMilliSecs) != errNoError)
			{
				Error err = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}

			if (DateTime:: nowLocalInMilliSecs (&ullNowLocalInMilliSecs) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);

				if (_evSetMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}

			if (ullNowLocalInMilliSecs <
				ullEventExpirationLocalDateTimeInMilliSecs)
			{
				if (_bConditionVariablesToBeUsed)
				{
					if (bBlocking)
					{
						lRemainingTimeInMilliSecs			= (long) (
							ulMilliSecondsToWait - ((unsigned long)
							(ullNowLocalInMilliSecs -
							ullInitialLocalInMilliSecs)));

						if (lRemainingTimeInMilliSecs <= 0)
						{
							// blocking time expired

							Error err = EventsSetErrors (__FILE__, __LINE__,
								EVSET_EVENTSSET_EVENTNOTEXPIREDYET);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}

						// timeWait con il min tra...
						if ((long) (ullEventExpirationLocalDateTimeInMilliSecs -
							ullNowLocalInMilliSecs) <=
							lRemainingTimeInMilliSecs)
						{
							ulRemainingSecondsToWait		= (unsigned long)
								((ullEventExpirationLocalDateTimeInMilliSecs -
								ullNowLocalInMilliSecs) / 1000);

							ulRemainingMilliSecondsToWait	=
								(ullEventExpirationLocalDateTimeInMilliSecs -
								ullNowLocalInMilliSecs) -
								ulRemainingSecondsToWait * 1000;

							bEventToBeReleasedAfterTimeout			= true;
						}
						else
						{
							ulRemainingSecondsToWait		= (unsigned long)
								(lRemainingTimeInMilliSecs / 1000);

							ulRemainingMilliSecondsToWait	=
								lRemainingTimeInMilliSecs -
								ulRemainingSecondsToWait * 1000;

							bEventToBeReleasedAfterTimeout			= false;
						}

						if ((errTimedWait =
							(pedDestinationEvents -> _cdAddedEvent).
							timedWait (&_evSetMutex, ulRemainingSecondsToWait,
							ulRemainingMilliSecondsToWait)) != errNoError)
						{
							int					i_errno;
							unsigned long		ulUserDataBytes;


							if ((long) errTimedWait ==
								THREADLIB_COND_TIMEDWAIT_FAILED)
							{
								errTimedWait. getUserData (&i_errno,
									&ulUserDataBytes);

								#if defined(__hpux) && defined(_CMA__HP)
									if (i_errno == 11)	// EAGAIN
								#elif WIN32
									if (i_errno == WSAETIMEDOUT)
								#else
									if (i_errno == ETIMEDOUT)
								#endif
								{
									// time expired

									;
								}
								else
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

									if (_evSetMutex. unLock () != errNoError)
									{
										Error err = PThreadErrors (
											__FILE__, __LINE__,
											THREADLIB_PMUTEX_UNLOCK_FAILED);
									}

									return errTimedWait;
								}
							}
							else
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PCONDITION_TIMEDWAIT_FAILED);

								if (_evSetMutex. unLock () != errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_UNLOCK_FAILED);
								}

								return errTimedWait;
							}
						}

						/*
						if (!bEventToBeReleasedAfterTimeout &&
							(long) errTimedWait ==
							THREADLIB_COND_TIMEDWAIT_FAILED)
						{
							// blocking time expired and no signal received

							Error err = EventsSetErrors (__FILE__, __LINE__,
								EVSET_EVENTSSET_EVENTNOTEXPIREDYET);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}
						else if (bEventToBeReleasedAfterTimeout &&
							(long) errTimedWait ==
							THREADLIB_COND_TIMEDWAIT_FAILED)
						{
							bFoundExpiredEvent		= true;
						}
						// else if errTimedWait == errNoError means there was a
						// signal and we have another event expiring before
						// the current one

						if ((pedDestinationEvents ->
							_emEventsMultiMap). begin () ==
							(pedDestinationEvents ->
							_emEventsMultiMap). end ())
						{
							// shouldn't happen that case
							Error err = EventsSetErrors (__FILE__, __LINE__,
								EVSET_EVENTSSET_NOEVENTSFOUND);

							if (_evSetMutex. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}

							return err;
						}
						*/

						// we have to check if we still have an event available
						continue;
					}
					else
					{
						Error err = EventsSetErrors (__FILE__, __LINE__,
							EVSET_EVENTSSET_EVENTNOTEXPIREDYET);

						if (_evSetMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}

						return err;
					}
				}
				else
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_EVENTNOTEXPIREDYET);

					if (_evSetMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}

					return err;
				}
			}
			else
				bFoundExpiredEvent		= true;
		}
	}

	(pedDestinationEvents -> _emEventsMultiMap).
		erase (itEvents);

	if (_evSetMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}



	return errNoError;
}

