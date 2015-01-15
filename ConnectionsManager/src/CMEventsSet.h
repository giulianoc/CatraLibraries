
#ifndef CMEventsSet_h
	#define CMEventsSet_h


	#include "ConnectionsManagerErrors.h"
	#include "EventsSet.h"
	#include "Tracer.h"
	#include "vector"

	#ifdef WIN32
		#define CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED		5
	#else
		#define CM_CMEVENTSSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED		20
	#endif
	// #define SB_SBEVENTSSET_NUMBEROFNEWFREEEVENTSALLOCATEDFORSOAPINFO	20
#define CM_CMEVENTSSET_NUMBEROFNEWFREEEVENTSALLOCATEDFORCHECKUNUSEDSOCKETS	\
	5


	typedef class CMEventsSet: public EventsSet

	{
		public:
			enum EventType
			{
				CM_EVENTTYPE_CONNECTIONTOACCEPTIDENTIFIER			= 0,
				CM_EVENTTYPE_CHECKUNUSEDSOCKETSIDENTIFIER,
				CM_EVENTTYPE_CHECKSOCKETSPOOLIDENTIFIER,
				CM_EVENTTYPE_CONNECTIONIDENTIFIER,

				CM_EVENTTYPENUMBER
			} EventType_t, *EventType_p;

		private:


		protected:
			Tracer_p					_ptSystemTracer;


			CMEventsSet (const CMEventsSet &);

			CMEventsSet &operator = (const CMEventsSet &);

			virtual Error allocateMoreFreeUserEvents (
				unsigned long ulEventTypeIndex,
				unsigned long *_pulPreAllocatedEventsNumber,
				std:: vector<Event_p> *pvFreeEvents,
				std:: vector<Event_p> *pvPointersToAllocatedEvents);

			virtual Error deleteAllocatedEvents (unsigned long ulEventTypeIndex,
				std:: vector<Event_p> *pvPointersToAllocatedEvents);
		public:
			CMEventsSet (void);

			~CMEventsSet (void);

			Error init (
				unsigned long ulNumberOfDifferentEventTypeToManage,
				Tracer_p ptTracer);

	} CMEventsSet_t, *CMEventsSet_p;

#endif

