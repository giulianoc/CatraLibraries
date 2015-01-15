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


#ifndef Config_h
	#define Config_h

	#ifdef _REENTRANT
		#include "PMutex.h"
	#endif
	#include "ConfigurationSection.h"
	#include "ConfigurationErrors.h"


	#define CFG_MAXCONFIGNAMELENGTH					(256 + 1)


	/**
		La classe Config rappresenta una configurazione.
		Un oggetto di tipo Config e' caratterizzato dal nome
		della configurazione e da zero o piu' oggetti di tipo
		ConfigurationSection che a sua volta conterranno oggetti di tipo
		ConfigurationItem.

		La libreria Configuration gestisce una configurazione tipo quella
		contenuta all'interno di un file .ini di Windows in memoria.
		In particolare una configurazione e' costituita da un insieme
		di sezioni ognuna delle quali contiene un insieme di items,
		ognuno dei quali puo' contenere un valore, una lista di valori op.
		una tabella di valori. Ad ogni oggetto della configurazione
		puo' essere associato un commento.

		Un esempio di configurazione puo' essere il seguente:

		#sezione di configurazione per il processo di data collection
		[DATACOLLECTION] (2001/10/20)
			##indica ogni quanto e' necessario eseguire data collection
			##(in minuti)
			DC_SCHEDULE = 10

		#sezione di configurazione per il processo di distribution
		[DISTRIBUTION] (2001/10/20)
			##indica ogni quanto e' necessario distyribuire i dati (in minuti)
			DI_SCHEDULE = 10

		L'uso della libreria e' molto semplice. E' possibile creare
		una configurazione (classe Config) aggiungendo sezioni
		(classe ConfigurationSection) e per ognuna di esse items
		(classe ConfigurationItem). Ogni item puo' essere valorizzato
		con un valore op. con una lista di valori. Inoltre ad ogni sezione
		puo' essere associata una data in modo tale che puo' essere conservata
		la storia di tutti i cambiamenti della configurazione.

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.

	*/
	typedef class Config {

		private:
			typedef enum CfgStatus {
				CFG_BUILDED,
				CFG_INITIALIZED
			} CfgStatus_t, *CfgStatus_p;

		public:
			typedef enum SecurityLevel {
				CFG_NOENCRIPTION,
				CFG_ENCRIPTION
			} SecurityLevel_t, *SecurityLevel_p;

		private:
			CfgStatus_t				_stCfgStatus;
			Boolean_t				_bIsCaseSensitive;
			char					*_pConfigName;
			long					_lCfgSectionsToAllocOnOverflow;
			ConfigurationSection_p	_pcsCfgSections;
			long					_lCfgSectionsNumber;
			long					_lAllocatedCfgSectionsNumber;
			#ifdef _REENTRANT
				PMutex_t			_configurationMutex;
			#endif

		protected:
			Config (const Config &);

			Config &operator = (const Config &);

			friend std:: ostream &operator << (std:: ostream &osOutputStream,
				Config &cfgConfig);

		public:
			/**
				Costruttore.
			*/
			Config ();

			/**
				Distruttore.
			*/
			~Config ();

			/**
				Inizializza la configurazione indicando un nome.
				L'oggetto Config conterra' le ConfigurationSection
				all'interno di un vettore.
				Il parametro lCfgSectionsToAllocOnOverflow indica che
				il vettore sara' inizialmente dimensionato per
				lCfgSectionsToAllocOnOverflow ConfigurationSection.
				Ogni qual volta si supera questa soglia saranno eseguite
				le seguenti operazioni:
					1. sara' allocato un vettore avente la
						dimensione precedente + lCfgSectionsToAllocOnOverflow
					2. saranno copiate le precedenti ConfigurationSection nel
						nuovo vettore
					3. sara' deallocato il precedente vettore
			*/
			Error init (const char *pConfigName = "",
				Boolean_t bIsCaseSensitive = true,
				long lCfgSectionsToAllocOnOverflow = 20);

			Error finish (void);

			/**
				The Section is removed
			*/
			Error removeCfgSection (
				const char *pSectionName,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Viene copiato, con modalita' di append, l'oggetto di tipo
				ConfigurationSection identificato dal parametro
				pcsCfgSection all'interno della configurazione.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error appendCfgSection (
				const ConfigurationSection_p pcsCfgSection,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Viene copiato, con modalita' di append, l'oggetto di tipo
				ConfigurationItem identificato dal parametro
				pciCfgItem all'interno della sezione identificata dal parametro
				pSectionName della configurazione.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error appendCfgItem (const char *pSectionName,
				const ConfigurationItem_p pciCfgItem,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Viene copiato, con modalita' di append, il valore dell'item
				identificato dal parametro pValue.
				La sezione e l'item sono identificati rispettivamente
				dai parametri pSectionName e pItemName.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error appendItemValue (const char *pSectionName,
				const char *pItemName, const char *pValue,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			Error getIsCaseSensitive (Boolean_p pbIsCaseSensitive);

			/**
				Questo metodo ritorna il nome della configurazione.
			*/
			Error getConfigName (char *pConfigName);

			/**
				Questo metodo modifica il nome della configurazione.
			*/
			Error modifyConfigName (const char *pConfigName);

			/**
				Questo metodo ritorna il numero delle sezioni contenute
				all'interno della configurazione.
			*/
			Error getSectionsNumber (long *plSectionsNumber);

			/**
				Questo metodo copia l'oggetto ConfigurationSection specificato
				dall'indice lCfgSectionIndex nell'oggetto specificato dal
				parametro pcsCfgSection.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgSectionByIndex (long lCfgSectionIndex,
				ConfigurationSection_p pcsCfgSection,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia l'oggetto ConfigurationSection specificato
				dal nome pSectionName nell'oggetto specificato dal
				parametro pcsCfgSection.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgSectionByName (const char *pSectionName,
				ConfigurationSection_p pcsCfgSection,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di items contenuti
				nella sezione specificata dal nome pSectionName.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemsNumber (const char *pSectionName,
				long *plItemsNumber, long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di items contenuti
				nella sezione specificata dall'indice lSectionIndex.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemsNumber (long lSectionIndex,
				long *plItemsNumber,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia l'oggetto ConfigurationItem specificato
				dall'indice lItemIndex e dalla sezione identificata
				dal nome pSectionName, nell'oggetto specificato dal
				parametro pciCfgItem.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgItemByIndex (const char *pSectionName,
				long lItemIndex, ConfigurationItem_p pciCfgItem,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia l'oggetto ConfigurationItem specificato
				dall'indice lItemIndex e dalla sezione identificata
				dall'indice lSectionIndex, nell'oggetto specificato dal
				parametro pciCfgItem.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgItemByIndex (long lSectionIndex, long lItemIndex,
				ConfigurationItem_p pciCfgItem,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia l'oggetto ConfigurationItem specificato
				dal nome pItemName e dalla sezione identificata
				dal nome pSectionName, nell'oggetto specificato dal
				parametro pciCfgItem.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgItemByName (const char *pSectionName,
				const char *pItemName, ConfigurationItem_p pciCfgItem,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo copia l'oggetto ConfigurationItem specificato
				dal nome pItemName e dalla sezione identificata
				dall'indice lSectionIndex, nell'oggetto specificato dal
				parametro pciCfgItem.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getCfgItemByName (long lSectionIndex, const char *pItemName,
				ConfigurationItem_p pciCfgItem,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di valori dell'item
				specificato dalla sezione identificata dal nome pSectionName
				e dall'item identificato dal nome pItemName.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemValuesNumber (const char *pSectionName,
				const char *pItemName, long *plItemValuesNumber,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di valori dell'item
				specificato dalla sezione identificata dall'indice lSectionIndex
				e dall'item identificato dal nome pItemName.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemValuesNumber (long lSectionIndex,
				const char *pItemName, long *plItemValuesNumber,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di valori dell'item
				specificato dalla sezione identificata dal nome pSectionName
				e dall'item identificato dall'indice lItemIndex.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemValuesNumber (const char *pSectionName,
				long lItemIndex, long *plItemValuesNumber,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il numero di valori dell'item
				specificato dalla sezione identificata dall'indice lSectionIndex
				e dall'item identificato dall'indice lItemIndex.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemValuesNumber (long lSectionIndex,
				long lItemIndex, long *plItemValuesNumber,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo ritorna il valore associato all'indice
				lItemValueIndex dell'item specificato dalla sezione
				identificata dal nome pSectionName
				e dall'item identificato dal nome pItemName.
				Il parametro slSecurityLevel indica se il valore
				dell'item sia criptato ed in caso affermativo
				viene de-criptato prima che esso sia ritornato.
				Dovendo inizializzare oggetti di tipo
				ConfigurationSection e ConfigurationItem
				sono necessari i parametri lCfgItemsToAllocOnOverflow e
				lBufferToAllocOnOverflow
				(vedi ConfigurationSection:: init e ConfigurationItem:: init).
			*/
			Error getItemValue (const char *pSectionName,
				const char *pItemName, char *pItemValue,
				unsigned long ulMaxItemValueLength, long lItemValueIndex = 0,
				SecurityLevel_t slSecurityLevel = CFG_NOENCRIPTION,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo modifica il commento associato alla sezione
				identificata dal nome pSectionName.
			*/
			Error modifySectionComment (const char *pSectionName,
				const char *pSectionComment);

			/**
				Questo metodo modifica la data associata alla sezione
				identificata dal nome pSectionName.
			*/
			Error modifySectionDate (const char *pSectionName,
				const char *pSectionDate);

			/**
				Questo metodo modifica il commento associato all'item
				identificato dal nome pItemName e dal nome
				della sezione pSectionName.
			*/
			Error modifyItemComment (const char *pSectionName,
				const char *pItemName, const char *pItemComment);

			/**
				Questo metodo modifica il valore associato all'indice
				lItemValueIndex di un item 
				identificato dal nome pItemName e dal nome
				della sezione pSectionName.
				Il parametro slSecurityLevel indica se il valore dell'item
				deve essere criptato.
			*/
			Error modifyItemValue (const char *pSectionName,
				const char *pItemName, const char *pNewItemValue,
				long lItemValueIndex = 0,
				SecurityLevel_t slSecurityLevel = CFG_NOENCRIPTION);

			/**
				Questo metodo salva su file la configurazione.
			*/
			Error write (int iFileDescriptor);

			// setSection
			// delSection
			// delItem

			/**
				Questo metodo permette di sostituire la configurazione
				che l'oggetto rappresenta con la configurazione identificata
				dal parametro pcfgConfig.
				Dovendo inizializzare oggetti di tipo ConfigurationItem,
				ConfigurationSection e configuration sono anche necessari
				i parametri lCfgSectionsToAllocOnOverflow,
				lCfgItemsToAllocOnOverflow e lBufferToAllocOnOverflow
				(vedi Config:: init, ConfigurationSection:: init e
				ConfigurationItem:: init).
			*/
			Error copy (Config *pcfgConfig,
				long lCfgSectionsToAllocOnOverflow = 20,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/**
				Questo metodo statico permette di fare una copia
				di una Config.
				Dovendo inizializzare oggetti di tipo ConfigurationItem,
				ConfigurationSection e configuration sono anche necessari
				i parametri lCfgSectionsToAllocOnOverflow,
				lCfgItemsToAllocOnOverflow e lBufferToAllocOnOverflow
				(vedi Config:: init, ConfigurationSection:: init e
				ConfigurationItem:: init).
			*/
			static Error copy (Config *pcfgDestConfig,
				Config *pcfgSrcConfig,
				long lCfgSectionsToAllocOnOverflow = 20,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

	} Config_t, *Config_p;

#endif
