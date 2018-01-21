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


#ifndef ErrMsgBase_h
	#define ErrMsgBase_h

	#include "string.h"
	#include <stdarg.h>


	#ifndef Boolean_t
		typedef bool    Boolean_t;
	#endif

	#ifndef Boolean_p
		typedef bool    *Boolean_p;
	#endif

	#define ERRMSGBASE_MAXFILENAMELENGTH	(128 + 1)
	#define ERRMSGBASE_BLOCKLENGTH			32
	#define ERRMSGBASE_MAXFLOATLENGTH		(32 + 1)
	#define ERRMSGBASE_MAXLONGLENGTH		(32 + 1)

	#define ERRMSGBASE_PREALLOCATEDUSERDATABYTES		32

	#define ERRMSGBASE_NULLPOINTERMESSAGE		"<null pointer>"

	/**

		La classe ErrMsgBase rappresenta la classe di base per ciascun tipo
		di errore (e/o messaggio).
		Qualunque classe di errore (e/o messaggio) deve ereditare da ErrMsgBase.
		Essa realizza le funzioni di base comuni ad ogni classe di errore
		(e/o messaggio).
	
		Il diagramma che segue descrive la struttura statica della libreria
		ErrMsg in termini di oggetti e relazioni tra di loro:
	
		<CENTER> <IMG SRC="./ErrMsgObjectModel.gif"> </CENTER>
	
	*/

	class ErrMsgBase {

		public:
			typedef struct  ErrMsgInfo {
				long			_lIdentifier;
				const char		*_pErrMsg;
			} ErrMsgInfo_t, *ErrMsgInfo_p, ErrMsgsInfo [];

		private:
			long destroy (void);

		protected:
			char				_pFileName [ERRMSGBASE_MAXFILENAMELENGTH];
			unsigned long		_ulFileLineNumber;

			unsigned char		_pucPreAllocatedUserData [
				ERRMSGBASE_PREALLOCATEDUSERDATABYTES];
			unsigned char		*_pucUserData;
			unsigned long		_ulUserDataBytes;

			long				_lErrMsgIdentifier;
			char				*_pErrMsg;

			long allocMemoryBlockForStringIfNecessary (
				long *plStringBufferBlockNumber,
				char **pStringBuffer, long lLengthToInsert);

			long buildCustomErrMsg (
				ErrMsgInfo_p peiErrMsgInfo, unsigned long ulMaxErrMsgNumber,
				long lErrMsgIdentifier, const char *pErrMsgClassName);

			long buildCustomErrMsg (
				ErrMsgInfo_p peiErrMsgInfo, unsigned long ulMaxErrMsgNumber,
				long lErrMsgIdentifier, const char *pErrMsgClassName,
				va_list vaInputList,
				long lArgumentsNumber = 0);

			long buildMessage (const char *pErrMsgClassName,
				long lErrMsgIdentifier,
				const char *pPredefinedErrorMessage);

			long buildMessage (const char *pErrMsgClassName,
				long lErrMsgIdentifier,
				const char *pPredefinedErrorMessage,
				va_list vaInputList,
				long lArgumentsNumber = 0);

		public:
			ErrMsgBase (const char *pFileName, unsigned long ulFileLineNumber,
				long lErrMsgIdentifier);

			ErrMsgBase (const ErrMsgBase &erErrMsgBase);

			virtual ~ErrMsgBase (void);

			ErrMsgBase &operator =(const ErrMsgBase &erErrMsgBase);

			long getUserData (void *pvUserData,
				unsigned long *pulUserDataBytes);

			long setUserData (void *pvUserData, unsigned long ulUserDataBytes);

			long getErrMsgIdentifier (long *plErrMsgIdentifier);

			operator const char *();

			Boolean_t operator ==(const ErrMsgBase *perErrMsgBase);

			long operator !=(const ErrMsgBase *perErrMsgBase);

			virtual long getErrMsgClassName (char *pErrMsgClassName) = 0;

			virtual long duplicate (ErrMsgBase **perNewErrMsgBase) const = 0;

	} ;

	typedef class ErrMsgBase ErrMsgBase_t, *ErrMsgBase_p;


#endif

