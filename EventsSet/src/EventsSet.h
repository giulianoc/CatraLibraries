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

#ifndef EventsSet_h
	#define EventsSet_h


	#include "PMutex.h"
	#include "PCondition.h"
	#include "Event.h"
	#include "Buffer.h"
	#include "my_hash_map.h"
	#include <vector>
	#include <map>


	#define EVSET_MAXDESTINATIONLENGTH						(256 + 1)
	#define EVSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED		500

	#define EVSET_EVENTTYPE_EVENTIDENTIFIER					0

	/**
		La classe EventsSet e' stata pensata per essere usata in un ambiente
		in cui esistono threads che producono eventi e threads che consumano
		eventi.
		Tutti i threads avranno visibilita' di uno o piu' oggetti del tipo
		EventsSet in modo che i threads possano comunicare tramite eventi.
		E' questo il motivo per cui i metodi piu' utilizzati di questa classe,
		addEvent (chiamato dal thread che produce l'evento) e
		getAndRemoveFirstEvent (chiamato dal thread che consuma
		l'evento), sono stati pensati per essere molto veloci.
		Infatti, un oggetto di tipo EventsSet partiziona tutti gli eventi
		inseriti secondo il loro destinatario. Inoltre, per ogni partizione,
		gli eventi sono ordinati secondo la loro chiave. Non possono coesiste
		due eventi con la stessa chiave.
	*/
	typedef class EventsSet

	{

		protected:
			typedef enum EventsSetStatus {
				EVSET_EVENTSSETBUILDED,
				EVSET_EVENTSSETINITIALIZED
			} EventsSetStatus_t, *EventsSetStatus_p;

			struct ltstr
			{
				#ifdef WIN32
					bool operator() (
						__int64 llExpirationLocalDateTimeInMilliSecs1,
						__int64 llExpirationLocalDateTimeInMilliSecs2) const
					{
						return llExpirationLocalDateTimeInMilliSecs1 <
							llExpirationLocalDateTimeInMilliSecs2;
					}
				#else
					bool operator() (unsigned long long
						ullExpirationLocalDateTimeInMilliSecs1,
						unsigned long long
						ullExpirationLocalDateTimeInMilliSecs2) const
					{
						return ullExpirationLocalDateTimeInMilliSecs1 <
							ullExpirationLocalDateTimeInMilliSecs2;
					}
				#endif
			};

			#ifdef WIN32
				typedef std:: multimap<__int64, Event_p, ltstr>
					EventsMultiMap_t, *EventsMultiMap_p;
			#else
				typedef std:: multimap<unsigned long long, Event_p, ltstr>
					EventsMultiMap_t, *EventsMultiMap_p;
			#endif

			typedef struct DestinationEvent
			{
				EventsMultiMap_t		_emEventsMultiMap;

				PCondition_t			_cdAddedEvent;

				Buffer_t				_bDestination;
			} DestinationEvent_t, *DestinationEvent_p;

			/*
			typedef struct BufferHasher: MyHasherModel<Buffer_p>
			{
				public:
					int operator() (Buffer_p const &key) const
					{
						int i, result = 0;
						const int length = strlen (
							(const char *) (*key));
						for(i=0; i<length ; i++)
						{
							result = result*5 + (*key)[(long) i];
						}
						// GENERIC_HASH is defined in my_hash_map.H

						return GENERIC_intHASH(result);
					}
			} BufferHasher_t, *BufferHasher_p;

			typedef struct BufferCmp
			{
				bool operator()(const Buffer_p x, const Buffer_p y) const
				{
					return !strcmp((const char *) (*x), (const char *) (*y));
				}
			} BufferCmp_t, *BufferCmp_p;
			*/

			typedef my_hash_map<Buffer_p, DestinationEvent_p,
				BufferHasher, BufferCmp>
				EventsSetHashMap_t, *EventsSetHashMap_p;

			typedef struct EventTypeInfo
			{
				// This vector contains all the pointers to the Events allocated
				std:: vector<Event_p>		_vPointersToAllocatedEvents;

				// This vector contains the free events
				std:: vector<Event_p>		_vFreeEvents;

				// Contain the number of preallocated events
				unsigned long				_ulPreAllocatedEventsNumber;
			} EventTypeInfo_t, *EventTypeInfo_p;

		protected:
			EventsSetStatus_t			_essEventsSetStatus;
			PMutex_t					_evSetMutex;

			Boolean_t					_bConditionVariablesToBeUsed;

			BufferHasher_p				_psHasher;
			BufferCmp_p					_psComparer;
			EventsSetHashMap_p			_pesmEventsSetHashMap;

			PMutex_t					_mtFreeEvents;
			unsigned long			_ulNumberOfDifferentEventTypeToManage;

			EventTypeInfo_p				_petiEventTypeInfo;

			/*
			#ifdef WIN32
				unsigned long			_ulThreadsWaitingForEvent;
			#endif
			*/


		protected:
			EventsSet (const EventsSet &);

			EventsSet &operator = (const EventsSet &);

			/**
				This method is called each time there is a request
				of a new free event and there is no free event available.
				The default behaviour is to allocate an amount of free events
				(EVSET_DEFAULTNUMBEROFNEWFREEEVENTSALLOCATED) of Event_t type.

				This method can be redefined to allocate the correct event type
				established by the ulEventTypeIndex parameter. Since the library
				cannot know the event type to allocate (because it will be a
				derived class of Event_t) it calls this method to give
				to the user the opportunity to redefine it and allocate the
				correct event.
				If that method is redefined, also the deleteAllocatedEvents
				method must be redefined.

				* ulEventTypeIndex: specifies the event type from which it is
					necessary the allocation
				* pulFreeEventsNumberAllocated: it is an output parameter
					in order to provide to the library the information about
					the number of events allocated
				* pevPreAllocatedEvents: it is an output parameter to provide
					to the library the pointer to the events allocated

				Inside this method it is possible also to put a cap on the
				events to be allocated and if this cap is reached it can
				return the EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE error.
			*/
			virtual Error allocateMoreFreeUserEvents (
				unsigned long ulEventTypeIndex,
				unsigned long *_pulPreAllocatedEventsNumber,
				std:: vector<Event_p> *pvFreeEvents,
				std:: vector<Event_p> *pvPointersToAllocatedEvents);

			/**
				This method is called from the finish method
				to delete the allocated events saved inside the
				pvPointersToAllocatedEvents vector.

				This method can be redefined to delete the correct event type
				established by the ulEventTypeIndex parameter. Since the library
				cannot know the event type to deallocate (because it will be a
				derived class of Event_t) it calls this method to give
				to the user the opportunity to deallocate the correct event.

				If that method is redefined, also the allocateMoreFreeUserEvents
				method must be redefined.

				* ulEventTypeIndex: specifies the event type from which it is
					necessary the allocation
				* pevPreAllocatedEvents: it is the vector containing all the
					pointers to the allocated events by the
					allocateMoreFreeUserEvents method
			*/
			virtual Error deleteAllocatedEvents (unsigned long ulEventTypeIndex,
				std:: vector<Event_p> *pvPointersToAllocatedEvents);

		public:
			EventsSet (void);

			~EventsSet (void);

			/**
				Inizializza un oggetto di tipo EventsSet.
				Un oggetto di tipo EventsSet partiziona tutti gli eventi
				inseriti secondo il loro destinatario.

				This class can preallocates all the events used
				by the application. The ulEventTypeNumberPreAllocated parameter
				indicates how many different type of events must be managed.
				The class needs that information because it will allocates
				an equivalent number of vectors each one to save all the
				free events of the specified type.

				This class calls the allocateFreeUserEvents method each time it
				needs a new free event to allocate.

				* ulEventTypeNumberPreAllocated: it represents the number of all
					the different events type (class derived from Event_t) and
					managed by this class. The default (-1) means that,
					by default, it handles only Event_t event type.
			*/
			Error init (
				Boolean_t bConditionVariablesToBeUsed,
				unsigned long ulEventTypeNumberPreAllocated);

			/**
				This method deallocate also all the events allocated
				by the allocateFreeUserEvents method.
			*/
			Error finish (void);

			/**
				Return a free pre-allocated event.

				The free event required must be of the type represented by
				the ulEventTypeIndex parameter.
			*/
			Error getFreeEvent (unsigned long ulEventTypeIndex,
				Event_p *pevEvent);

			/**
				L'EventsSet riceve l'evento precedentemente avuto
				con getFreeEvent.

				The ulEventTypeIndex parameter specifies the type of the event
				that will be releasing.
			*/
			Error releaseEvent (unsigned long ulEventTypeIndex,
				Event_p pevEvent);

			/**
				Inserisce il puntatore all'evento indicato dal
				parametro pevEvent, nell'insieme.
				Questo metodo potrebbe essere chiamato da un thread che
				vuole 'spedire' un evento ad un altro thread.
				L'EventsSet non supporta chiavi duplicate per cui
				non possono coesistere due eventi con la stessa chiave
			*/
			virtual Error addEvent (Event_p pevEvent);

			/**
				Add a destination to the EventsSet.
				You could add before a destination because if a thread calls
				getAndRemoveFirstEvent with a destination that it is not added
				yet (this tipically happens at the beginning when no events are
				added yet), the getAndRemoveFirstEvent method returns
				immediatelly without waiting any timeout.
				That will cause a big usage of the CPU.
			*/
			Error addDestination (const char *pDestination);

			/**
				Rimuove il puntatore all'evento indicato dal parametro
				pevEvent, dall'insieme.
			
				Nota bene che l'evento non viene de-allocato, ma viene
				semplicemente rimosso il puntatore all'evento, dall'insieme.
			*/
			Error deleteEvent (Event_p pevEvent);

			/**
				Questo metodo ritorna il puntatore al primo evento.
				Ricorda che l'EventsSet è ordinato secondo
				la chiave dell'evento.
			
				Il parametro bBlocking indica se il metodo deve rimanere
				bloccato al suo interno finchè non viene inserito
				un evento nel caso in cui non ci siano eventi nel set.
				I parametri ulSecondsToBlock e ulAdditionalMilliSecondsToBlock
				vengono considerati nel caso che bBlocking sia true.
			*/
			Error getFirstEvent (
				Buffer_p pbDestination, Event_p *pevEvent,
				Boolean_p pbEventExpired,
				Boolean_t bBlocking, unsigned long ulSecondsToBlock,
				unsigned long ulAdditionalMilliSecondsToBlock);

			/**
				Questo metodo ritorna il puntatore al primo evento.
				Ricorda che l'EventsSet è ordinato socondo la chiave dell'evento
				Il metodo rimuove anche l'evento ritornato.
			
				Nota bene che l'evento non viene de-allocato, ma viene
				semplicemente rimosso il puntatore dall'insieme.
			
				Esso potrebbe essere chiamato dal thread che e'
				in attesa di un evento per conoscere la sua prossima
				operazione da fare.
			
				Il parametro bBlocking indica se il metodo deve rimanere
				bloccato al suo interno finchè non viene inserito
				un evento nel caso in cui non ci siano eventi nel set.
				I parametri ulSecondsToBlock e ulAdditionalMilliSecondsToBlock
				vengono considerati nel caso che bBlocking sia true.
			*/
			Error getAndRemoveFirstEvent (
				Buffer_p pbDestination, Event_p *pevEvent,
				Boolean_t bBlocking, unsigned long ulSecondsToBlock,
				unsigned long ulAdditionalMilliSecondsToBlock);

	} EventsSet_t, *EventsSet_p;

#endif

