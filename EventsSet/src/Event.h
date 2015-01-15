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

#ifndef Event_h
	#define Event_h

	#include "EventsSetErrors.h"
	#include "Buffer.h"


	// yyyy-mm-dd hh:mi:ss:milliss
	#define EVSET_MAXDATELENGTH					(24 + 1)


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
	typedef class Event

	{

		private:
			typedef enum EventStatus {
				EVSET_EVENTBUILDED,
				EVSET_EVENTINITIALIZED
			} EventStatus_t, *EventStatus_p;

		private:

		protected:
			EventStatus_t		_esEventStatus;
			Buffer_t			_bSource;
			Buffer_t			_bDestination;
			Buffer_t			_bTypeIdentifier;
			long				_lTypeIdentifier;
			#ifdef WIN32
				__int64			_ullExpirationLocalDateTimeInMilliSecs;
			#else
				unsigned long long	_ullExpirationLocalDateTimeInMilliSecs;
			#endif
			char				_pCreationLocalDateTime [EVSET_MAXDATELENGTH];
			/*
			#ifdef WIN32
				__int64				_ullCreationUTCTimeInMilliSecs;
			#else
			*/
				unsigned long long	_ullCreationUTCTimeInMilliSecs;
			// #endif

			Event (const Event &);

			Event &operator = (const Event &);


		public:
			Event (void);

			/**
				This method calls finish in case
				the object is already initialized.
			*/
			virtual ~Event (void);

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
			/*
			#ifdef WIN32
				Error init (
					const char *pSource,
					const char *pDestination,
					long lTypeIdentifier,
					__int64 ullExpirationLocalDateTimeInMilliSecs);
			#else
			*/
				Error init (
					const char *pSource,
					const char *pDestination,
					long lTypeIdentifier,
					const char *pTypeIdentifier,
					unsigned long long ullExpirationLocalDateTimeInMilliSecs);
			// #endif

			virtual Error finish (void);

			/**
					Ritorna l'identificatore del type.
			*/
			Error getTypeIdentifier (long *plTypeIdentifier);

			/**
					Ritorna la stringa che rappresenta l'identificatore del tipo
			*/
			const char *getTypeIdentifier (void);

			/**
					Ritorna la chiave dell'evento.
			*/
			/*
			#ifdef WIN32
				Error getExpirationLocalDateTimeInMilliSecs (
					__int64 *pullExpirationLocalDateTimeInMilliSecs);
			#else
			*/
				Error getExpirationLocalDateTimeInMilliSecs (
					unsigned long long *pullExpirationLocalDateTimeInMilliSecs);
			// #endif

			/**
					Inizializza la chiave usata per l'ordinamento dell'evento.
					Nell'EventsSet non vengono ammesse chiavi duplicate per
					non ci possono essere due eventi nello stesso EventsSet
					con la stessa chiave
			*/
			#ifdef WIN32
				Error setExpirationLocalDateTimeInMilliSecs (
					__int64 ullExpirationLocalDateTimeInMilliSecs);
			#else
				Error setExpirationLocalDateTimeInMilliSecs (
					unsigned long long ullExpirationLocalDateTimeInMilliSecs);
			#endif

			/**
					Ritorna la data di creazione dell'evento.
			*/
			Error getCreationDateTime (char *pCreationDateTime);

			/**
					Return the point to the source buffer
			*/
			Error getSource (Buffer_p *pbSource);

			/**
					Return the point to the destination buffer
			*/
			Error getDestination (Buffer_p *pbDestination);

	} Event_t, *Event_p;

#endif

