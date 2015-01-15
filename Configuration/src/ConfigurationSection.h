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


#ifndef ConfigurationSection_h
	#define ConfigurationSection_h

	#ifdef _REENTRANT
		#include "PMutex.h"
	#endif
	#include "ConfigurationItem.h"
	#include "ConfigurationErrors.h"


	#ifndef CFG_SECTIONDATELENGTH
		#define CFG_SECTIONDATELENGTH		(10 + 1)
	#endif


	/**
		La classe ConfigurationSection rappresenta una sezione
		della configurazione.
		Un oggetto di tipo ConfigurationSection e' caratterizzato dal nome
		della sezione, da un commento associato alla sezione e da
		zero o piu' oggetti di tipo ConfigurationItem.
	*/
	typedef class ConfigurationSection

	{

		private:
			typedef enum CfgSectionStatus {
				CFGSECTION_BUILDED,
				CFGSECTION_INITIALIZED
			} CfgSectionStatus_t, *CfgSectionStatus_p;

		private:
			CfgSectionStatus_t		_stCfgSectionStatus;
			Boolean_t				_bIsCaseSensitive;
			char					*_pSectionName;
			char					*_pSectionComment;
			char					*_pSectionDate;
			long					_lCfgItemsToAllocOnOverflow;
			ConfigurationItem_p		_pciCfgItems;
			long					_lCfgItemsNumber;
			long					_lAllocatedCfgItemNumber;
			#ifdef _REENTRANT
				PMutex_t			_configurationSectionMutex;
			#endif

			friend std:: ostream &operator << (std:: ostream &osOutputStream,
				ConfigurationSection &csSection);

		protected:
			ConfigurationSection (const ConfigurationSection &);

			ConfigurationSection &operator = (const ConfigurationSection &);

		public:
			/**
				Costruttore.
			*/
			ConfigurationSection ();

			/**
				Distruttore.
			*/
			~ConfigurationSection ();

			/**
				Inizializza la sezione indicando un nome, un commento ed una
				data. Il parametro pSectionDate puo' essere null oppure
				deve avere il formato: 'YYYY/MM/DD'.
				L'oggetto ConfigurationSection conterra' i ConfigurationItem
				all'interno di un vettore.
				Il parametro lCfgItemsToAllocOnOverflow indica che
				il vettore sara' inizialmente dimensionato per
				lCfgItemsToAllocOnOverflow ConfigurationItem.
				Ogni qual volta si supera questa soglia saranno eseguite
				le seguenti operazioni:
					1. sara' allocato un vettore avente la
						dimensione precedente + lCfgItemsToAllocOnOverflow
					2. saranno copiati i precedenti ConfigurationItem nel
						nuovo vettore
					3. sara' deallocato il precedente vettore
			*/
			Error init (const char *pSectionName = "",
				const char *pSectionComment = "",
				const char *pSectionDate = (char *) NULL,
				Boolean_t bIsCaseSensitive = true,
				long lCfgItemsToAllocOnOverflow = 5);

			Error finish (void);

			/**
				Questo metodo ritorna true se il nome della sezione indicato
				dal parametro coincide con il nome della sezione dell'oggetto
			*/
			Boolean_t operator ==(const char *pSectionName);

			/**
				Viene copiato, con modalita' di append, l'oggetto di tipo
				ConfigurationItem identificato dal parametro
				pciCfgItem all'interno della sezione.
			*/
			Error appendCfgItem (const ConfigurationItem_p pciCfgItem,
				long lBufferToAllocOnOverflow = 20);

			Error getIsCaseSensitive (Boolean_p pbIsCaseSensitive);

			/**
				Ritorna il nome della sezione.
			*/
			Error getSectionName (char *pSectionName);

			/**
				Ritorna la lunghezza del nome della sezione.
			*/
			Error getSectionNameLength (long *plSectionNameLength);

			/**
				Questo metodo ritorna il nome della sezione.
			*/
			Error modifySectionName (const char *pSectionName);

			/**
				Questo metodo ritorna il commento associato alla sezione.
			*/
			Error getSectionComment (char *pSectionComment);

			/**
				Questo metodo ritorna la lunghezza del commento
				associato alla sezione.
			*/
			Error getSectionCommentLength (long *plSectionCommentLength);

			/**
				Questo metodo modifica il commento associato alla sezione.
			*/
			Error modifySectionComment (const char *pSectionComment);

			/**
				Questo metodo ritorna la data associata alla sezione.
			*/
			Error getSectionDate (char *pSectionDate);

			/**
				Questo metodo modifica la data associata alla sezione.
				Il parametro pSectionDate deve avere il formato: 'YYYY/MM/DD'.
			*/
			Error modifySectionDate (const char *pSectionDate);

			/**
				Questo metodo ritorna il numero di configuration item
				che la sezione correntemente contiene.
			*/
			Error getCfgItemsNumber (long *plCfgItemsNumber);

			/**
				Questo metodo copia nel ConfigurationItem, identificato
				dal parametro pciCfgItem, l'item che si trova nella
				posizione indicata dal parametro lCfgItemIndex.
				Dovendo quindi inizializzare un ConfigurationItem
				e' anche necessario il parametro lBufferToAllocOnOverflow
				(vedi ConfigurationItem:: init).
			*/
			Error getCfgItemByIndex (long lCfgItemIndex,
				ConfigurationItem_p pciCfgItem,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia nel ConfigurationItem, identificato
				dal parametro pciCfgItem, l'item dal nome indicato dal
				parametro pItemName.
				Dovendo quindi inizializzare un ConfigurationItem
				e' anche necessario il parametro lBufferToAllocOnOverflow
				(vedi ConfigurationItem:: init).
			*/
			Error getCfgItemByName (const char *pItemName,
				ConfigurationItem_p pciCfgItem,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo inizializza il puntatore pciCfgItem in modo
				che lo faccia puntare al ConfigurationItem di nome indicato
				dal parametro pItemName.
				Attenzione che se si modifica il ConfigurationItem ritornato
				da questo metodo, si modifica il ConfigurationItem interno alla
				sezione.
			*/
			Error getPtrCfgItemByName (const char *pItemName,
				ConfigurationItem_p *pciCfgItem);

			/**
				Questo metodo salva su file la sezione della configurazione.
			*/
			Error write (int iFileDescriptor);

			/**
				Questo metodo statico permette di fare una copia
				di un ConfigurationSection.
				Dovendo inizializzare oggetti di tipo ConfigurationItem
				e ConfigurationSection sono anche necessari i parametri
				lCfgItemsToAllocOnOverflow e lBufferToAllocOnOverflow
				(vedi ConfigurationItem:: init e ConfigurationSection:: init).
			*/
			static Error copy (ConfigurationSection *pcsDestCfgSection,
				ConfigurationSection *pcsSrcCfgSection,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo statico ritorna true se la data indicata
				dall'argomento pSectionDate e' valida, false
				altrimenti
			*/
			static Boolean_t isValidDate (const char *pSectionDate);

	} ConfigurationSection_t, *ConfigurationSection_p;

#endif

