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


#ifndef ConfigurationItem_h
	#define ConfigurationItem_h

	#ifdef _REENTRANT
		#include "PMutex.h"
	#endif
	#include "iostream"
	#include "ConfigurationErrors.h"


	// #ifdef WIN32
	//	#define CFG_NEWLINE				"\r\n"
	//#else
		#define CFG_NEWLINE				"\n"
	//#endif

	/**
		La classe ConfigurationItem rappresenta un item della configurazione.
		Un oggetto di tipo ConfigurationItem e' caratterizzato dal nome
		dell'item, da un commento associato all'item e da zero o piu' valori
		che l'item assume. I valori che l'item assume possono essere visti
		come valori appartenenti ad una lista op. come valori appartenenti
		ad una tabella (in questo caso e' necessario specificare il numero
		di colonne della tabella).
	*/
	typedef class ConfigurationItem

	{

		private:
			typedef enum CfgItemStatus {
				CFGITEM_BUILDED,
				CFGITEM_INITIALIZED
			} CfgItemStatus_t, *CfgItemStatus_p;

		private:
			CfgItemStatus_t		_stCfgItemStatus;
			Boolean_t			_bIsCaseSensitive;
			char				*_pItemName;
			char				*_pItemComment;
			long				_lBufferToAllocOnOverflow;
			char				**_pValues;
			long				_lValuesNumber;
			long				_lAllocatedBuffersNumber;
			#ifdef _REENTRANT
				PMutex_t				_configurationItemMutex;
			#endif

			friend std:: ostream &operator << (std:: ostream &osOutputStream,
				ConfigurationItem &ciItem);

		protected:
			ConfigurationItem (const ConfigurationItem &);

			ConfigurationItem &operator = (const ConfigurationItem &);

		public:
			/**
				Costruttore.
			*/
			ConfigurationItem ();

			/**
				Distruttore.
			*/
			~ConfigurationItem ();

			/**
				Inizializza l'item indicando un nome ed un commento.
				L'oggetto ConfigurationItem conterra' i valori dell'item
				all'interno di un vettore.
				Il parametro lBufferToAllocOnOverflow indica che
				il vettore sara' inizialmente dimensionato per
				lBufferToAllocOnOverflow possibili valori di un item.
				Ogni qual volta si supera questa soglia saranno eseguite
				le seguenti operazioni:
					1. sara' allocato un vettore avente la
						dimensione precedente + lBufferToAllocOnOverflow
					2. saranno copiati i precedenti valori dell'item nel
						nuovo vettore
					3. sara' deallocato il precedente vettore
			*/
			Error init (const char *pItemName = "",
				const char *pItemComment = "",
				Boolean_t bIsCaseSensitive = true,
				long lBufferToAllocOnOverflow = 20);

			Error finish (void);

			/**
				Questo metodo ritorna true se il nome dell'item indicato
				dal parametro coincide con il nome dell'item dell'oggetto
			*/
			Boolean_t operator ==(const char *pItemName);

			/**
				Viene allocata memoria e viene inserito il valore 
				identificato dal parametro pValue per l'item della
				configurazione.
				Il valore dell'item sara' inserito nella posizione
				identificata dal parametro lValueIndexToInsert.
				Se l'indice lValueIndexToInsert e' maggiore o uguale del numero
				di valori correntemente inseriti per l'item (lValuesNumber),
				pValue sara' inserito nella posizione identificata dall'indice 
				lValuesNumber.
				Se l'indice lValueIndexToInsert e' minore del numero
				di valori correntemente inseriti per l'item (lValuesNumber),
				pValue sara' inserito nella posizione identificata dall'indice
				lValueIndexToInsert e tutti i valori da quell'indice in
				poi saranno slittati di un indice in avanti.
			*/
			Error insertItemValue (const char *pValue,
				long lValueIndexToInsert);

			/**
				Questo metodo coincide con la chiamata
					insertItemValue (pValue, lValuesNumber).
				Vedi ConfigurationItem:: insertItemValue.
			*/
			Error appendItemValue (const char *pValue);

			/**
				Questo metodo aggiunge in append i lValuesNumber valori
				contenuti nel parametro pValues.
				Questo metodo coincide con le chiamate
					appendItemValue (pValue)
				per ogni valore da inserire.
				Vedi ConfigurationItem:: appendItemValue.
			*/
			Error appendItemValues (long lValuesNumber, const char **pValues);

			Error getIsCaseSensitive (Boolean_p pbIsCaseSensitive);

			/**
				Questo metodo ritorna il nome dell'item.
			*/
			Error getItemName (char *pItemName);

			/**
				Questo metodo la lunghezza del nome dell'item.
			*/
			Error getItemNameLength (long *lItemNameLength);

			/**
				Questo metodo modifica il nome dell'item.
			*/
			Error modifyItemName (const char *pItemName);

			/**
				Questo metodo ritorna il commento associato all'item.
			*/
			Error getItemComment (char *pItemComment);

			/**
				Questo metodo ritorna la lunghezza del commento
				associato all'item.
			*/
			Error getItemCommentLength (long *plItemCommentLength);

			/**
				Questo metodo modifica il commento associato all'item.
			*/
			Error modifyItemComment (const char *pItemComment);

			/**
				Questo metodo ritorna il numero dei valori che l'item
				assume.
			*/
			Error getItemValuesNumber (long *plValuesNumber);

			/**
				Questo metodo ritorna il valore dell'item corrispondente
				all'indice identificato dal parametro lValueIndex.
			*/
			Error getItemValue (char *pItemValue,
				unsigned long ulMaxItemValueLength, long lValueIndex = 0);

			/**
				Questo metodo ritorna la lunghezza del valore
				dell'item corrispondente all'indice
				identificato dal parametro lValueIndex.
			*/
			Error getItemValueLength (long *lItemValueLength,
				long lValueIndex = 0);

			/**
				Questo metodo modifica il valore dell'item corrispondente
				all'indice identificato dal parametro lValueIndexToModify.
			*/
			Error modifyItemValue (const char *pNewValue,
				long lValueIndexToModify = 0);

			/**
				Questo metodo salva su file l'item della configurazione.
			*/
			Error write (int iFileDescriptor);

			/**
				Questo metodo statico permette di fare una copia
				di un ConfigurationItem
			*/
			static Error copy (ConfigurationItem *pciDestCfgItem,
				ConfigurationItem *pciSrcCfgItem,
				long lBufferToAllocOnOverflow);

	} ConfigurationItem_t, *ConfigurationItem_p;

#endif
