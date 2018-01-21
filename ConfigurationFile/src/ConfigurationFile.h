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


#ifndef ConfigurationFile_h
	#define ConfigurationFile_h

	#include "Config.h"
	#include "ConfigurationFileErrors.h"


	#define CFGFILE_MAXPATHNAMELENGTH		(1024 + 1)

	/**
		La classe ConfigurationFile eredita dalla class Config e
		rappresenta una configurazione gestita all'interno di un file.
		Eredita quindi tutti i metodi della classe Config,
		ridefinisce il metodo init in modo che esso possa leggersi
		la configurazione dal file indicato dal parametro pPathName
		ed il metodo finish, ed inoltre aggiunge il metodo save
		per salvare la configurazione su file.
	
		La classe ConfigurationFile usa la seguente <a href="configurationParser.y#configurationParser" target=classContent>grammatica YACC </a>
		per la sintassi del file di configurazione.

		L'uso della libreria e' molto semplice:

		* viene istanziato un oggetto di tipo ConfigurationFile
			(ConfigurationFile_t cfConfigurationFile)
		* viene inizializzato tramite il 'configuration file path name'
			(cfConfigurationFile. init (pConfigPathName))
		* a questo punto e' possibile richiamare tutti i metodi ereditati
			dall'oggetto Configuration per modificare la configurazione
		* metodo invece messo a disposizione da ConfigurationFile e' 'save'
			che salva la configurazione sul file da cui e' stata caricata
			op. su un nuovo file
		* il metodo finish dealloca la configurazione

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.

	*/
	typedef class ConfigurationFile: public Config {

		public:
			typedef enum CfgFileStatus {
				CFGFILE_BUILDED,
				CFGFILE_INITIALIZED
			} CfgFileStatus_t, *CfgFileStatus_p;

		private:
			CfgFileStatus_t			_stCfgFileStatus;
			char					*_pPathName;

		protected:
			ConfigurationFile (const ConfigurationFile &);

			ConfigurationFile &operator = (const ConfigurationFile &);

		public:
			/**
				Costruttore.
			*/
			ConfigurationFile ();

			/**
				Distruttore. It calls finish() in case
				it was not already called.
			*/
			~ConfigurationFile ();

			CfgFileStatus_t getCfgStatus ();

			/**
				Inizializza la configurazione leggendo il file di configurazione
				indicato dal parametro pPathName.
				Il file di configurazione viene letto 
				eseguendo un solo accesso al file system per cui la lettura
				risulta essere molto veloce anche con file molto grandi.
				Se il file di configurazione risulta essere molto lungo
				e' importante inizializzare correttamente
				i parametri lCfgSectionsToAllocOnOverflow,
				lCfgItemsToAllocOnOverflow e lBufferToAllocOnOverflow
				(vedi Config:: init, ConfigurationSection:: init e
				ConfigurationItem:: init).
			*/
			Error init (const char *pPathName,
				const char *pConfigName = "",
				Boolean_t bIsCaseSensitive = true,
				long lCfgSectionsToAllocOnOverflow = 20,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20,
				Boolean_t bLockFile = false);

			/**
				Inizializza la configurazione copiandola dalla
				configurazione indicata dal parametro pcfgConfig.
				Il parametro pPathName indicante il pathname del file di
				configurazione viene solo memorizzato all'interno dell'oggetto.
				Se la configurazione risulta avere molte sezioni e/o item e/o
				valori di item, e' importante inizializzare correttamente
				i parametri lCfgSectionsToAllocOnOverflow,
				lCfgItemsToAllocOnOverflow e lBufferToAllocOnOverflow
				(vedi Config:: init, ConfigurationSection:: init e
				ConfigurationItem:: init).
			*/
			Error init (const char *pPathName,
				Config_p pcfgConfig,
				const char *pConfigName = "",
				Boolean_t bIsCaseSensitive = true,
				long lCfgSectionsToAllocOnOverflow = 20,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

			/*
			 * It is called automatically when the object is destroyed
			 */
			Error finish (void);

			// it returns the configuration path name
			operator const char * (void) const;

			/**
				Salva la configurazione nel file indicato dal parametro
				pNewPathName.
				Se il parametro e' NULL, la configurazione viene salvata
				nel file indicato dal parametro pPathName
				quando e' stato invocato il metodo di init.
			*/
			Error save (const char *pNewPathName = (const char *) NULL);

			Error addItemAndSave (const char *pSectionName,
				const char *pItemName, const char *pValue,
				long lCfgItemsToAllocOnOverflow = 5,
				long lBufferToAllocOnOverflow = 20);

	} ConfigurationFile_t, *ConfigurationFile_p;

#endif
