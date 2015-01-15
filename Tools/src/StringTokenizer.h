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


#ifndef StringTokenizer_h
	#define StringTokenizer_h


	#include "ToolsErrors.h"
	#include "Buffer.h"


	/**
		Windows non supporta chiamate annidate di strtok
		in quanto tutte utilizzano la stessa memoria statica.
		Nota che Windows non possiede la API strtok_r.
		Usando Windows quindi non usare 2 oggetti di questa
		classe in modo annidato
	*/
	typedef class StringTokenizer

	{
		protected:
			typedef enum StringTokenizerStatus {
				TOOLS_STRINGTOKENIZER_BUILDED,
				TOOLS_STRINGTOKENIZER_INITIALIZED
			} StringTokenizerStatus_t, *StringTokenizerStatus_p;

		private:
			StringTokenizerStatus_t			_stsStringTokenizerStatus;


		protected:
			char					*_pInitialBuffer;
			char					*_pBufferCopy;
			char					*_pDefaultDelimits;
			Boolean_t				_bIsFirstNextToken;
			#ifdef _REENTRANT
				char					*_pLastToken;
			#endif


		public:
			StringTokenizer (void);

			StringTokenizer (const StringTokenizer &);

			/**
				Distrugge il corrente oggetto. Chiama il metodo finish
				nel caso in cui non sia stato chiamato.
			*/
			~StringTokenizer (void);

			Error init (const char *pBuffer, long lBufferLength = -1,
				const char *pDefaultDelimits = " ");

			/**
				Questo metodo e' anche invocato automaticamente dal
				distruttore del corrente oggetto.
				In quest'ultimo caso pero' non e' possibile gestire
				il valore di ritorno.
			*/
			Error finish (void);

			/**
				Il metodo trova il prossimo token e inizializza
				il puntatore pToken.
				Il parametro pCurrentDelimits indica il delimitatore da
				utilizzare per determinare il prossimo token. Se esso
				non viene inizializzato, viene utilizzato il delimitatore
				di default (quello dato in input al metodo init)
			*/
			Error nextToken (const char **pToken,
				const char *pCurrentDelimits = (const char *) NULL);

			Error countTokens (unsigned long *pulTokensNumber);

	} StringTokenizer_t, *StringTokenizer_p;

#endif

