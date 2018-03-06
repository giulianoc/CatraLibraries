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

#ifndef Event2_h
#define Event2_h

#include <string>
#include <chrono>

using namespace std;


/**
    La classe Event e' stata pensata per essere usata in un ambiente
    in cui esistono threads che producono eventi e threads che consumano
    eventi.
    L'oggetto evento diventa, quindi, l'elemento attraverso il quale i
    threads possono scambiarsi informazioni.
    Naturalmente, in caso di particolari eventi, e' possibile creare
    una classe che eredita dalla classe Event e la specializza secondo
    particolari esigenze.
    Ogni evento e' caratterizzato da:
            un oggetto sorgente che rappresenta colui che ha spedito l'evento
            un oggetto destinatario che rappresenta colui a cui e'
                    destinato l'evento
            un identificatore del tipo dell'evento
            una chiave univoca usata per l'ordinamento degli eventi

    Gli eventi vengono gestiti esclusivamente in memoria ed il loro
    identificativo unico e' rappresentato dal puntatore all'evento.
*/
class Event2
{
private:
    // _typeIdentifier, _identifier
    pair<long,long>         _eventKey;
    string                  _source;
    string                  _destination;

    chrono::system_clock::time_point   _expirationTimePoint;
    chrono::system_clock::time_point   _startProcessingTime;

public:
    friend ostream& operator << (ostream& os, const Event2& event);

    /**
        Questo metodo inizializza l'evento.
        Parametri
                pSource: indica colui che spedisce l'evento
                        che in genere coincide con colui che alloca
                        l'evento in memoria.
                lTypeIdentifier: rappresenta l'identificatore del tipo
                        dell'evento (ad es. EV_GETDATA potrebbe indicare
                        un evento per collezionare dati).
                        Non rappresenta un identificatore unico dell'evento
                        infatti possono coesistere piu' di un evento
                        con uguale lTypeIdentifier.
                ullExpirationLocalDateTimeInMilliSecs:
                        rappresenta la chiave usata per l'ordinamento
                        degli eventi.
    */
    Event2 () { };

    virtual ~Event2 (void) {  };

    pair<long, long> getEventKey() const {
        return _eventKey;
    }

    void setEventKey(pair<long, long> eventKey) {
        this->_eventKey = eventKey;
    }

    string getDestination() const {
        return _destination;
    }

    void setDestination(string _destination) {
        this->_destination = _destination;
    }

    string getSource() const {
        return _source;
    }

    void setSource(string _source) {
        this->_source = _source;
    }

    chrono::system_clock::time_point getStartProcessingTime() const {
        return _startProcessingTime;
    }

    void setStartProcessingTime(chrono::system_clock::time_point _startProcessingTime) {
        this->_startProcessingTime = _startProcessingTime;
    }

    chrono::system_clock::time_point getExpirationTimePoint() const {
        return _expirationTimePoint;
    }

    void setExpirationTimePoint(chrono::system_clock::time_point _expirationTimePoint) {
        this->_expirationTimePoint = _expirationTimePoint;
    }
};

#endif

