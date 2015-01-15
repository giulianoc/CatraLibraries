
#include "CMEventsSet.h"
#include "ConnectionsManagerMessages.h"
#include "ConnectionEvent.h"
#include <assert.h>



CMEventsSet:: CMEventsSet (void):
	EventsSet ()

{

}


CMEventsSet:: ~CMEventsSet (void)

{

}


CMEventsSet:: CMEventsSet (const CMEventsSet &)

{

	assert (1==0);
}


CMEventsSet &CMEventsSet:: operator = (
	const CMEventsSet &)

{

	assert (1==0);

	return *this;
}


Error CMEventsSet:: init (
	unsigned long ulNumberOfDifferentEventTypeToManage,
	Tracer_p ptSystemTracer)

{

	_ptSystemTracer			= ptSystemTracer;


	#ifdef NO_CONDITION_VARIABLE
		return EventsSet:: init (false, ulNumberOfDifferentEventTypeToManage);
	#else
		return EventsSet:: init (true, ulNumberOfDifferentEventTypeToManage);
	#endif
}


Error CMEventsSet:: allocateMoreFreeUserEvents (
	unsigned long ulEventTypeIndex,
	unsigned long *_pulPreAllocatedEventsNumber,
	std:: vector<Event_p> *pvFreeEvents,
	std:: vector<Event_p> *pvPointersToAllocatedEvents)

{

	unsigned long			ulFreeEventsNumberAllocated;
	unsigned long			ulEventIndex;


	switch (ulEventTypeIndex)
	{
		case CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				// this event is used to check for new connections and it is
				// not useful to continue to allocate events of this type if
				// already many events are already into the EventsSet ready to
				// be processed
				if (*_pulPreAllocatedEventsNumber >=
					CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED * 1 +
					1)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE,
						1, *_pulPreAllocatedEventsNumber);
					_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				ulFreeEventsNumberAllocated			=
					CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED;

				if ((pevEvents = new Event_t [
					ulFreeEventsNumberAllocated]) == (Event_p) NULL)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_NEW_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				(*_pulPreAllocatedEventsNumber)			+=
					ulFreeEventsNumberAllocated;

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMEVENTSSET_ALLOCATEDMOREEVENT,
						3, "Event for CMServer",
						(unsigned long) (*_pulPreAllocatedEventsNumber),
						(unsigned long)
						((*_pulPreAllocatedEventsNumber) * sizeof (Event_t)));
					_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				pvFreeEvents -> reserve (*_pulPreAllocatedEventsNumber);

				pvPointersToAllocatedEvents -> insert (
					pvPointersToAllocatedEvents -> end (),
					pevEvents);

				for (ulEventIndex = 0;
					ulEventIndex < ulFreeEventsNumberAllocated;
					ulEventIndex++)
				{
					pvFreeEvents -> insert (
						pvFreeEvents -> end (),
						&(pevEvents [ulEventIndex]));
				}
			}

			break;
		case CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				// this event is used to check for new connections and it is
				// not useful to continue to allocate events of this type if
				// already many events are already into the EventsSet ready to
				// be processed
				if (*_pulPreAllocatedEventsNumber >=
		CM_CMEVENTSSET_NUMBEROFNEWFREEEVENTSALLOCATEDFORCHECKUNUSEDSOCKETS *
					1 + 1)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE,
						1, *_pulPreAllocatedEventsNumber);
					_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				ulFreeEventsNumberAllocated			=
		CM_CMEVENTSSET_NUMBEROFNEWFREEEVENTSALLOCATEDFORCHECKUNUSEDSOCKETS;

				if ((pevEvents = new Event_t [
					ulFreeEventsNumberAllocated]) == (Event_p) NULL)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_NEW_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				(*_pulPreAllocatedEventsNumber)			+=
					ulFreeEventsNumberAllocated;

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMEVENTSSET_ALLOCATEDMOREEVENT,
						3, "Event for CheckUnusedSockets",
						(unsigned long) (*_pulPreAllocatedEventsNumber),
						(unsigned long)
						((*_pulPreAllocatedEventsNumber) * sizeof (Event_t)));
					_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				pvFreeEvents -> reserve (*_pulPreAllocatedEventsNumber);

				pvPointersToAllocatedEvents -> insert (
					pvPointersToAllocatedEvents -> end (),
					pevEvents);

				for (ulEventIndex = 0;
					ulEventIndex < ulFreeEventsNumberAllocated;
					ulEventIndex++)
				{
					pvFreeEvents -> insert (
						pvFreeEvents -> end (),
						&(pevEvents [ulEventIndex]));
				}
			}

			break;
		case CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				// this event is used to check the sockets pool and it is
				// not useful to continue to allocate events of this type if
				// already many events are already into the EventsSet ready to
				// be processed
				if (*_pulPreAllocatedEventsNumber >=
					CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED * 1)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE,
						1, *_pulPreAllocatedEventsNumber);
					_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				ulFreeEventsNumberAllocated			=
					CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED;

				if ((pevEvents = new Event_t [
					ulFreeEventsNumberAllocated]) == (Event_p) NULL)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_NEW_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				(*_pulPreAllocatedEventsNumber)			+=
					ulFreeEventsNumberAllocated;

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMEVENTSSET_ALLOCATEDMOREEVENT,
						3, "Event for CheckSocketsPool",
						(unsigned long) (*_pulPreAllocatedEventsNumber),
						(unsigned long)
						((*_pulPreAllocatedEventsNumber) * sizeof (Event_t)));
					_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				pvFreeEvents -> reserve (*_pulPreAllocatedEventsNumber);

				pvPointersToAllocatedEvents -> insert (
					pvPointersToAllocatedEvents -> end (),
					pevEvents);

				for (ulEventIndex = 0;
					ulEventIndex < ulFreeEventsNumberAllocated;
					ulEventIndex++)
				{
					pvFreeEvents -> insert (
						pvFreeEvents -> end (),
						&(pevEvents [ulEventIndex]));
				}
			}

			break;
		case CM_EVENTTYPE_CONNECTIONIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				ConnectionEvent_p
					pevConnectionEvents;


				ulFreeEventsNumberAllocated			=
					CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED;

				if ((pevConnectionEvents =
					new ConnectionEvent_t [
					ulFreeEventsNumberAllocated]) ==
					(ConnectionEvent_p) NULL)
				{
					Error err = EventsSetErrors (__FILE__, __LINE__,
						EVSET_NEW_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					return err;
				}

				(*_pulPreAllocatedEventsNumber)			+=
					ulFreeEventsNumberAllocated;

				{
					Message msg = ConnectionsManagerMessages (
						__FILE__, __LINE__,
						CM_CMEVENTSSET_ALLOCATEDMOREEVENT,
						3, "ConnectionEvent",
						(unsigned long) (*_pulPreAllocatedEventsNumber),
						(unsigned long) ((*_pulPreAllocatedEventsNumber) *
						sizeof (ConnectionEvent_t)));
					_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				pvFreeEvents -> reserve (*_pulPreAllocatedEventsNumber);

				pvPointersToAllocatedEvents -> insert (
					pvPointersToAllocatedEvents -> end (),
					pevConnectionEvents);

				for (ulEventIndex = 0;
					ulEventIndex < ulFreeEventsNumberAllocated;
					ulEventIndex++)
				{
					pvFreeEvents -> insert (
						pvFreeEvents -> end (),
						&(pevConnectionEvents [ulEventIndex]));
				}
			}

			break;
		default:
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_UNKNOWNEVENTTYPE);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
	}


	return errNoError;
}


Error CMEventsSet:: deleteAllocatedEvents (
	unsigned long ulEventTypeIndex,
	std:: vector<Event_p> *pvPointersToAllocatedEvents)

{

	std:: vector<Event_p>:: const_iterator	itPreAllocatedEvents;


	switch (ulEventTypeIndex)
	{
		case CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				for (itPreAllocatedEvents =
					pvPointersToAllocatedEvents -> begin ();
					itPreAllocatedEvents !=
					pvPointersToAllocatedEvents -> end ();
					++itPreAllocatedEvents)
				{
					pevEvents			= (Event_p) (*itPreAllocatedEvents);

					delete [] pevEvents;
				}

			}

			break;
		case CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				for (itPreAllocatedEvents =
					pvPointersToAllocatedEvents -> begin ();
					itPreAllocatedEvents !=
					pvPointersToAllocatedEvents -> end ();
					++itPreAllocatedEvents)
				{
					pevEvents			= (Event_p) (*itPreAllocatedEvents);

					delete [] pevEvents;
				}

			}

			break;
		case CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				Event_p					pevEvents;

				for (itPreAllocatedEvents =
					pvPointersToAllocatedEvents -> begin ();
					itPreAllocatedEvents !=
					pvPointersToAllocatedEvents -> end ();
					++itPreAllocatedEvents)
				{
					pevEvents			= (Event_p) (*itPreAllocatedEvents);

					delete [] pevEvents;
				}

			}

			break;
		case CM_EVENTTYPE_CONNECTIONIDENTIFIER:
			{
				// Important: if you will allocate a type derived from
				// Event_t (next new), the type of the next variable
				// must be the derived type and not Event_p
				ConnectionEvent_p
					pevConnectionEvents;

				for (itPreAllocatedEvents =
					pvPointersToAllocatedEvents -> begin ();
					itPreAllocatedEvents !=
					pvPointersToAllocatedEvents -> end ();
					++itPreAllocatedEvents)
				{
					pevConnectionEvents			=
						(ConnectionEvent_p)
						(*itPreAllocatedEvents);

					delete [] pevConnectionEvents;
				}
			}

			break;
		default:
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_UNKNOWNEVENTTYPE);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
	}

	
	return errNoError;
}

