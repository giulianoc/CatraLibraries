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


#ifndef Times2_h
#define Times2_h

#include <mutex>
#include "SchedulerErrors.h"

using namespace std;

#define SCH_MAXDATELENGTH				(24 + 1)

#define SCH_MAXSCHEDULELENGTH			(256 + 1)


/**
    Un oggetto di tipo Times puo' essere pensato come un orologio cui
    possono essere inseriti allarmi (timeouts).
    Il tipo Times e' una classe astratta in quanto possiede il metodo
    virtuale puro handleTimeOut.
    Il metodo handleTimeout deve essere ridefinito dall'utente
    in modo che esso possa realizzare le operazioni da eseguire
    quando si verifica un timeout.
    L'utente della libreria deve semplicemente creare una classe
    per ogni Times personalizzato che eredita dalla classe Times e
    ridefinire il metodo handleTimeout.
    E' lo scheduler che automaticamente si preoccupa di gestire gli
    oggetti di tipo Times e chiamare il metodo handleTimeout
    ogni qual volta si verifica un timeout.


    Gestione del cambio ora legale da parte degli oggetti Times

        Cambio 3:00 -> 2:00 (in Italia, notte del 31/10/YYYY)
            1:30 ... 2:00 ... 2:30 ... 3:00=2:00 ... 2:30 ... 3:00 ... 4:00
                    **					***

            In questo caso i timeout non saranno ripetuti, nonostante
            il periodo 2:00 - 3:00 si verifica 2 volte.

            Gli oggetti Times utilizzano la struttura tm ed il
            time_t UTC (tipi del linguaggio C) per i calcoli sulle date.
            Supponiamo che venga settato un timeout
            alle 2:30 di ogni giorno, il timeout della notte incriminata
            (cambio ora legale) sara': 1999-10-31 02:30:00 con il
            corrispondente campo dst (Daylight Saving Time) della
            struttura tm inizializzato a 1 (vedi ** sopra).
            Per calcolare il prossimo timeout l'oggetto Times
            convertira' la data del timeout (1999-10-31 02:30:00) nella
            struttura tm. In questa conversione pero' il campo
            dst (Daylight Saving Time) della struttura tm viene
            inizializzato dalle API C a 0 (vedi *** sopra).
            Per cui le prossime 2:30 saranno il 1999-11-01 02:30:00.

            Cambio 2:00 -> 3:00 (in Italia a Marzo)
                1:30 ... 2:00=3:00 ... 3:30

                Fino alle 1:59 lo scheduler eseguira' normalmente i suoi
                timeout.
                Tutto i timeout tra le 2:00 e le 3:00
                saranno persi in quanto quella notte non si verifichera
                mai ad esempio le 2:30, e cosi' via.
*/
class Times2
{
private:
    typedef enum TimesType {
        SCH_TYPE_PERIODIC,
        SCH_TYPE_CALENDAR
    } TimesType_t, *TimesType_p;

protected:
    typedef enum TimesStatus {
        SCHTIMES_INITIALIZED,
        SCHTIMES_STARTED
    } TimesStatus_t, *TimesStatus_p;

private:
    /**
        Aggiorna il prossimo timeout dell'oggetto
        tenendo conto del periodo.
    */
    Error updateNextPeriodicExpirationDateTime ();

    /**
        Aggiorna il prossimo timeout dell'oggetto
        tenendo conto del calendario specificato dallo schedule.
    */
    void updateNextCalendarExpirationDateTime (bool &lastTimeout);

protected:
    // mutex for the next private and protected variables
    mutex				_mtTimesMutex;
    TimesStatus_t			_schTimesStatus;
    TimesType_t				_ttTimesType;
    string				_className;

    unsigned long			_ulPeriodInMilliSeconds;
    char					_pCalendarSchedule [SCH_MAXSCHEDULELENGTH];

    // the date format is yyyy-mm-dd hh:mi:ss
    Boolean_t		_bNextDaylightSavingTime;
    char			_pNextExpirationDateTime [SCH_MAXDATELENGTH];
    Boolean_t		_bCurrentDaylightSavingTime;
    char			_pCurrentExpirationDateTime [SCH_MAXDATELENGTH];


public:
    /**
        Viene inizializzato un times periodico.
        Inizializza l'oggetto specificando il periodo tra due timeout.
        Il parametro lPeriod e' espresso in secondi.
    */
    Times2 (unsigned long ulPeriodInMilliSeconds = 0, string pClassName = "");

    /**
        Viene inizializzato un times 'calendario'.
        Inizializza l'oggetto specificando la schedulazione all'interno
        del parametro pSchedule.
        Il parametro pSchedule deve essere inizializzato con il seguente
        formato:
            year month monthday weekday hour minute second

        Con questo tipo di schedule, la granularit� si ferma ai secondi
        e non vengono usati i millisecondi.

        I valori dei precedenti elementi costituenti il contenuto
        del parametro pSchedule variano nei seguenti intervalli:
            year			anno
            month			mese di un anno, 1-12
            monthday		giorno di un mese, 1-31
            weekday			giorno della settimana, 0-6, 0=domenica
            hour			ora di un giorno, 0-23
            minute			minuto di un'ora, 0-59
            second			secondo di un minuto, 0-59

        Ogni campo precedente puo' essere:
            - un '*', in tal caso indica qualunque valore
                (esempio minute: * indica che il timeout puo'
                verificarsi in qualunque minuto)
            - un valore (esempio minute: 4 indica che il timeout si
                verifica solo quando i minuti di un ora sia uguali a 4)
            - due valori separati da un trattino (esempio hour: 4-16
                indica che il timeout si puo' verificare solamente
                quando i minuti di un'ora siano all'interno
                dell'intervallo specificato)
            - due o piu' valori separati da virgole
                (esempio minute: 1,15 indica che il timeout si puo'
                verificare solamente quando i minuti di un'ora siano
                1 oppure 15)

        Nota che la specifica dei giorni puo' essere fatta in due 
            campi: monthday e weekday. Se entrambi sono specificati
            essi sono cumulativi. Per specificare i giorni in un solo
            campo inizializzare l'altro campo a '*'.

        Esempio: 	pSchedule	<-	"* * 1,15 1 0 0 0"
            In questo caso si verifica un timeout l'1 ed il 15
            di ogni mese e ogni lunedi'.

        Esempio: 	pSchedule	<-	"2000 * 1,15 1 0 0 0"
            In questo caso si verifica un timeout l'1 ed il 15
            di ogni mese e ogni lunedi' dell'anno 2000.
    */
    Times2 (string schedule, string className = "");

    /**
        Distruttore
    */
    virtual ~Times2 (void);

    /**
        Il metodo start fa si che lo scheduler gestisce gli eventuali
        timeouts dell'oggetto.
        E' possibile anche indicare la data di partenza a partire
        dalla quale il Times viene fatto partire.
        Di default (quando il parametro pStartDateTime e' NULL)
        la data di partenza dell'oggetto di tipo Times e' il momento in
        cui il metodo start viene chiamato.
        Il comportamento del metodo start e' il seguente:
            se il times e' periodico, la prossima data di scadenza sara'
                pStartDateTime + il periodo
            se il times e' calendar, la prossima data di scadenza sara'
                la prossima data dopo pStartDateTime compatibile
                con lo schedule
        Il formato della data del parametro pStartDateTime e':
        yyyy-mm-dd hh:mi:ss:mill.
    */
    virtual Error start (
        const char *pStartDateTime = (const char *) NULL);

    /**
        Questo metodo fa si che lo scheduler non gestisca da ora in poi
        gli eventuali timeouts dell'oggetto.
    */
    Error stop ();

    /**
        Ritorna true se l'oggetto di tipo Times e' in stato started,
        false altrimenti.
    */
    bool isStarted ();

    /**
        Ritorna true se all'oggetto di tipo Times si sia verificato
        almeno un timeout non gestito ancora dallo scheduler,
        false altrimenti.
    */
    bool isExpiredTime ();

    /**
        E' il metodo che deve essere ridefinito per indicare le
        operazioni da eseguire quando si verifica un timeout.
        Esso viene chiamato automaticamente dallo scheduler.

        Nel caso in cui e' l'ultimo timeout che si verifica
        per l'oggetto, lo scheduler thread ha gia' chiamato la
        deactiveTimes sul corrente times, cio' vuol dire che
        lo scheduler thread non ha piu' conoscenza del suddetto times.
        Quindi, all'interno di questo metodo, e' possibile
        se si vuole eseguire la delete dell'oggetto stesso:
            finish ();
            delete this;

        Si puo' capire che si tratta dell'ultimo timeout eseguito
        dall'oggetto, dal valore dello stato dell'oggetto.
        Infatti in questo caso lo stato dell'oggetto e'
        cambiato da SCHTIMES_STARTED a SCHTIMES_INITIALIZED.
            if (_schTimesStatus == Times:: SCHTIMES_INITIALIZED)
            {
                finish ();
                delete this;
            }
        Nel caso in cui l'oggetto Times e' condiviso da altri
        threads oltre lo scheduler
        e' bene accedere alla variabile _schTimesStatus
        all'interno di una 'regione critica' tramite
        il mutex _mtTimesMutex:
            Times:: TimesStatus_t		schTimesStatus;
            _mtTimesMutex. lock ();
            schTimesStatus		= _schTimesStatus;
            _mtTimesMutex. unLock ();
            if (schTimesStatus == Times:: SCHTIMES_INITIALIZED)
            {
                finish ();
                delete this;
            }

        Se si vuole che questo oggetto non sia piu' referenziato
        dallo scheduler thread e' necessario che questo metodo
        ritorni un errore. Anche in questo caso, all'interno di questo
        metodo, e' possibile se si vuole eseguire la delete
        dell'oggetto stesso:
            deleteTimesData ();		 solo per times persistenti
            finish ();
            delete this;
            return errAnError;
    */
    virtual void handleTimeOut (void) = 0;

    /**
        Ritorna il nome della classe dell'oggetto di tipo Times.
    */
    string getClassName ();

    /**
        Ritorna una copia della variabile _pNextExpirationDateTime.
    */
    Error getNextExpirationDateTime (char *pNextExpirationDateTime);

    /**
        Aggiorna il prossimo timeout dell'oggetto.
        Questo metodo viene chiamato dalla libreria dopo
        che si e' verificato un timeout.
        Nel caso in cui e' stato gia' raggiunto l'ultimo
        timeout (nei Times periodici quando il periodo e' 0,
        in quelli calendar si raggiungera' l'ultimo timeout solo
        se l'anno viene specificato) il metodo ritornera' l'errore
        SCH_REACHED_LASTTIMEOUT e lo stato dell'oggetto viene
        cambiato automaticamente da SCHTIMES_STARTED a
        SCHTIMES_INITIALIZED.
    */
    Error updateNextExpirationDateTime (bool &lastTimeout);

    /**
        Metodo statico che permette di sommare alla data pSrcDateTime
        un certo numero di secondi indicato da lSeconds.
        Il parametro lSeconds puo' assumere anche
        un numero di secondi negativo.
        La data indicante il risultato dell'operazione viene
        inserita nel parametro di output pDestDateTime.
        Il formato delle stringhe rappresentanti le date deve essere:
        yyyy-mm-dd hh:mi:ss.
    */
    static Error addMilliSecondsToDateTime (
        char *pDestDateTime, Boolean_p pbDestDaylightSavingTime,
        const char *pSrcDateTime, long lSrcDaylightSavingTime,
        unsigned long ulMilliSeconds);

};

#endif
