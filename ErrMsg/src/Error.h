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


#ifndef Error_h
	#define Error_h

	#include "ErrMsgBase.h"
	#ifdef __MOSYNC__
	#else
		#include <iostream>
	#endif
    #ifdef __QTCOMPILER__
        #include <QtCore/qglobal.h>

        #if defined(ERRMSG_LIBRARY)
            #define ERRMSGSHARED_EXPORT Q_DECL_EXPORT
        #else
            #define ERRMSGSHARED_EXPORT Q_DECL_IMPORT
        #endif
    #else
		#define ERRMSGSHARED_EXPORT
	#endif


	/**
	
		La classe Error contiene un generico errore
		la cui classe eredita dalla classe ErrMsgBase.
	
		Principali funzionalita' della classe Error sono:
	
			- possibilita' di avere stringhe associate agli errori che
				utilizzano una formattazione standard (%s, %d, ...).
				Importante perche' avere ad es. un errore
					'%s file not found in the %s directory'
				con i %s che saranno rimpiazzati risp.te dal
				filename e dalla directory
				da' maggiore informazione rispetto ad un errore
					'file not found'. 
			- possibilita' di avere 'user data' associata ad un errore.
				Infatti, spesso oltre ad un errore una funzione vorrebbe
				comunque ritornare una qualunque informazione all'interno
				di una particolare struttura dati.
			- anche alla istanza errNoError l'utente puo' associare 'user data'
				Anzi e' piu' probabile che quando la funzione non ritorni
				un errore, alla costante errNoError possa essere associata
				una qualche informazione da parte dell'utente. 
			- operatore di cast 'const char *' che ritorna la stringa associata
				all'errore
			- possibilita' di avere il cast a 'long' per avere l'identificativo
				dell'errore
			- opzionalmente e' possibile avere la compatibilita' con la
				classe _Error_ e relativa libreria
	
		Per definire una personalizzata classe di errore e' necessario usare la
		macro
			dclCustomErrorClass([error class name], [max errors])
	
		La libreria ErrMsg permette di gestire in maniera omogenea errori e
		messaggi.

		Ogni errore appartiene ad una data classe di errori specificata
		dall'utente ed e' costituito da una descrizione e da un identificativo
		unico all'interno della classe dell'errore.

		Un esempio di descrizione potrebbe essere 'File not found (%s)',
		dove il %s sara' sostituito dal nome del file quando l'errore viene
		istanziato. La descrizione associata ad ogni errore infatti,
		segue le stesse regole di una stringa data in input alla funzione C
		printf, nel senso che puo' contenere opzioni di formattazione tipo
		%ld, %s, ecc che poi saranno valorizzati in fase di istanziazione
		dell'errore.

		L'utente che vuole gestire gli errori di un dato componente software,
		ad es. un MailServer, creera' una specifica classe di errore che
		potrebbe chiamare 'MailServerErrors' contenente tutti gli errori
		generati dal MailServer.

		La descrizione degli errori si trova all'interno di un unico file
		in modo che si possa avere una visione centralizzata dei possibili
		errori di un componente software ed in modo che si possa facilmente
		gestire l'internazionalizzazione del componente stesso.

		Quando l'errore viene istanziato e' possibile associargli una qualunque
		struttura dati definita dall'utente. In questo modo, l'errore ritornato
		da una funzione, oltre ad indicare il tipo di errore verificatosi
		(ad es. 'File not found'), potrebbe contenere informazioni di ritorno
		della funzione stessa.

		Nel momento in cui si verifica un errore e' possibile chiedere
		alla libreria di generare la stringa associata ad esso. Essa sara'
		composta dalla descrizione utente formattata associata all'errore con
		altre informazioni tipo la classe dell'errore, l'identificativo
		associato, il nome del file in cui l'errore si e' verificato, il
		numero di linea all'interno del file.

		Tutto cio' che riguarda gli errori vale anche per i messaggi.

		L'utente che utilizza la libreria deve creare una specifica classe di
		errore che eredita dalla classe 'ErrMsgBase' e che contiene tutti i
		possibili errori della classe.

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.

	*/
    typedef class ERRMSGSHARED_EXPORT Error {

		private:
			ErrMsgBase_p			_pebCustomError;

			unsigned char       	_pucNoErrorPreAllocatedUserData [
				ERRMSGBASE_PREALLOCATEDUSERDATABYTES];
			unsigned char			*_pucNoErrorUserData;
			unsigned long			_ulNoErrorUserDataBytes;

		public:
			Error (void);

			Error (const ErrMsgBase_t &ebCustomError);

			Error (const Error &err);

			~Error (void);

			operator const char *(void) const;
			const char *str () const;

			operator long (void) const;

			operator unsigned long (void) const;

			Error &operator =(const Error &err);

			Boolean_t operator ==(const Error &err);

			Boolean_t operator !=(const Error &err);

			long getClassName (char *pErrorClassName);

			long getIdentifier (long *plErrorIdentifier) const;

			long getErrorKey (char *pErrorClassName, long *plErrorIdentifier);

			long getUserData (void *pvUserData,
				unsigned long *pulUserDataBytes);

			long setUserData (void *pvUserData, unsigned long ulUserDataBytes);

    } Error_t, *Error_p;

	extern const Error_t	errNoError;

	#define dclCustomErrorClass(ERR_CLASSNAME, ERR_MAXERRORS)	\
		extern ErrMsgBase:: ErrMsgsInfo ERR_CLASSNAME ## Str;	\
		class ERR_CLASSNAME: public ErrMsgBase {	\
			public:	\
				ERR_CLASSNAME (const char *pFileName, unsigned long ulFileLineNumber,	\
					long lErrorIdentifier)	\
					:ErrMsgBase (pFileName, ulFileLineNumber,	\
					lErrorIdentifier)	\
				{	\
					if (buildCustomErrMsg (	\
						ERR_CLASSNAME ## Str,	\
						(unsigned long) ERR_MAXERRORS,	\
						lErrorIdentifier,	\
						(const char *) #ERR_CLASSNAME))	\
					{	\
						std:: cerr << "ErrMsgBase:: buildCustomError failed" << std:: endl;	\
						return;	\
					}	\
				};	\
				ERR_CLASSNAME (const char *pFileName, unsigned long ulFileLineNumber,	\
					long lErrorIdentifier, long lArgumentsNumber, ...)	\
					:ErrMsgBase (pFileName, ulFileLineNumber,	\
					lErrorIdentifier)	\
				{	\
					va_list		vaInputList;	\
					va_start (vaInputList, lArgumentsNumber);	\
					if (buildCustomErrMsg (	\
						ERR_CLASSNAME ## Str,	\
						(unsigned long) ERR_MAXERRORS,	\
						lErrorIdentifier,	\
						(const char *) #ERR_CLASSNAME,	\
						vaInputList,	\
						lArgumentsNumber))	\
					{	\
						std:: cerr << "ErrMsgBase:: buildCustomError failed" << std:: endl;	\
						return;	\
					}	\
					va_end (vaInputList);	\
				};	\
				ERR_CLASSNAME (const char *pFileName, unsigned long ulFileLineNumber,	\
					long lErrorIdentifier, const char *pError)	\
					:ErrMsgBase (pFileName, ulFileLineNumber,	\
					lErrorIdentifier)	\
				{	\
					if (pError != (const char *) NULL)	\
					{	\
						long	lStringBufferBlockNumber;	\
						lStringBufferBlockNumber	= 0;	\
						if (allocMemoryBlockForStringIfNecessary (	\
							&lStringBufferBlockNumber, &_pErrMsg,	\
							(long) strlen (pError)))	\
						{	\
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;	\
							return;	\
						}	\
						else	\
							strcpy (_pErrMsg, pError);	\
					}	\
				};	\
				virtual ~ERR_CLASSNAME ()	\
				{	\
				};	\
				ERR_CLASSNAME &operator = (const ERR_CLASSNAME & /* err */)	\
				{	\
					return *this;	\
				};	\
				virtual long getErrMsgClassName (char *pErrMsgClassName)	\
				{	\
					strcpy (pErrMsgClassName,	\
						(const char *) #ERR_CLASSNAME);	\
					return 0;	\
				};	\
				virtual long duplicate (	\
					ErrMsgBase **perNewErrMsgBase) const	\
				{	\
					*perNewErrMsgBase = new ERR_CLASSNAME (_pFileName,	\
						_ulFileLineNumber, _lErrMsgIdentifier, _pErrMsg);	\
					if (*perNewErrMsgBase == (ERR_CLASSNAME *) NULL)	\
						return 1;	\
					else	\
					{	\
						if (_pucUserData != (unsigned char *) NULL &&	\
							_ulUserDataBytes > 0)	\
						{	\
							if ((*perNewErrMsgBase) -> setUserData (	\
								_pucUserData, _ulUserDataBytes))	\
							{	\
								delete *perNewErrMsgBase;	\
								return 1;	\
							}	\
						}	\
						return 0;	\
					}	\
				};	\
		};

	typedef Error Message, Message_t, *Message_p;
	extern const Message_t	msgNoMessage;
	#define dclCustomMessageClass(MSG_CLASSNAME, MSG_MAXMESSAGES)	\
		dclCustomErrorClass(MSG_CLASSNAME, MSG_MAXMESSAGES)

#endif

