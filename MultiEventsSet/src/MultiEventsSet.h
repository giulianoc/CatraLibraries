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

#ifndef MultiEventsSet_h
#define MultiEventsSet_h


#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>
#include <map>
#include <chrono>
#include "EventsFactory.h"
#include "Event.h"

using namespace std;

/**
    La classe MultiEventsSet e' stata pensata per essere usata in un ambiente
    in cui esistono threads che producono eventi e threads che consumano
    eventi.
    Tutti i threads avranno visibilita' di uno o piu' oggetti del tipo
    MultiEventsSet in modo che i threads possano comunicare tramite eventi.
    E' questo il motivo per cui i metodi piu' utilizzati di questa classe,
    addEvent (chiamato dal thread che produce l'evento) e
    getAndRemoveFirstEvent (chiamato dal thread che consuma
    l'evento), sono stati pensati per essere molto veloci.
    Infatti, un oggetto di tipo MultiEventsSet partiziona tutti gli eventi
    inseriti secondo il loro destinatario. Inoltre, per ogni partizione,
    gli eventi sono ordinati secondo la loro chiave. Non possono coesiste
    due eventi con la stessa chiave.
*/
class MultiEventsSet
{
protected:
    using EventsMultiMap = multimap<chrono::system_clock::time_point, shared_ptr<Event> >;

    struct DestinationEvents
    {
        EventsMultiMap                          _eventsMultiMap;
        condition_variable                      _addedEvent;
        string                                  _destination;
    };

    using MultiEventsSetHashMap = unordered_map<string, shared_ptr<DestinationEvents> >;

protected:
    mutex                       _evSetMutex;
    MultiEventsSetHashMap       _esmMultiEventsSetHashMap;
    shared_ptr<EventsFactory>   _eventsFactory;


public:
    /**
        Inizializza un oggetto di tipo MultiEventsSet.
        Un oggetto di tipo MultiEventsSet partiziona tutti gli eventi
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
    MultiEventsSet (void);

    /**
        Add a destination to the MultiEventsSet.
        You could add before a destination because if a thread calls
        getAndRemoveFirstEvent with a destination that it is not added
        yet (this tipically happens at the beginning when no events are
        added yet), the getAndRemoveFirstEvent method returns
        immediatelly without waiting any timeout.
        That will cause a big usage of the CPU.
    */
    void addDestination (string destination);

    /**
        This method deallocate also all the events allocated
        by the allocateFreeUserEvents method.
    */
    ~MultiEventsSet (void);

    shared_ptr<EventsFactory> getEventsFactory() const {
        return _eventsFactory;
    }

//    void setEventsFactory(EventsFactory _eventsFactory) {
//        this->_eventsFactory = _eventsFactory;
//    }

    /**
        Inserisce il puntatore all'evento indicato dal
        parametro pevEvent, nell'insieme.
        Questo metodo potrebbe essere chiamato da un thread che
        vuole 'spedire' un evento ad un altro thread.
        L'MultiEventsSet non supporta chiavi duplicate per cui
        non possono coesistere due eventi con la stessa chiave
    */
    virtual void addEvent (shared_ptr<Event>& event);

    /**
        Rimuove il puntatore all'evento indicato dal parametro
        pevEvent, dall'insieme.

        Nota bene che l'evento non viene de-allocato, ma viene
        semplicemente rimosso il puntatore all'evento, dall'insieme.
    */
    void deleteEvent (shared_ptr<Event>& event);

    /**
        Questo metodo ritorna il puntatore al primo evento.
        Ricorda che l'MultiEventsSet � ordinato secondo
        la chiave dell'evento.

        Il parametro bBlocking indica se il metodo deve rimanere
        bloccato al suo interno finch� non viene inserito
        un evento nel caso in cui non ci siano eventi nel set.
        I parametri ulSecondsToBlock e ulAdditionalMilliSecondsToBlock
        vengono considerati nel caso che bBlocking sia true.
    */
    shared_ptr<Event> getFirstEvent (
        string destination, bool blocking,
        chrono::milliseconds milliSecondsToBlock,
        bool &eventExpired);

    /**
            Questo metodo ritorna il puntatore al primo evento.
            Ricorda che l'MultiEventsSet � ordinato socondo la chiave dell'evento
            Il metodo rimuove anche l'evento ritornato.

            Nota bene che l'evento non viene de-allocato, ma viene
            semplicemente rimosso il puntatore dall'insieme.

            Esso potrebbe essere chiamato dal thread che e'
            in attesa di un evento per conoscere la sua prossima
            operazione da fare.

            Il parametro bBlocking indica se il metodo deve rimanere
            bloccato al suo interno finch� non viene inserito
            un evento nel caso in cui non ci siano eventi nel set.
            I parametri ulSecondsToBlock e ulAdditionalMilliSecondsToBlock
            vengono considerati nel caso che bBlocking sia true.
    */
    shared_ptr<Event> getAndRemoveFirstEvent (
        string destination, bool blocking,
        chrono::milliseconds milliSecondsToBlock);
};

#endif
