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


#include "ErrMsgBase.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef __MOSYNC__
#else
	#include <iostream>
#endif


ErrMsgBase:: ErrMsgBase (const char *pFileName, unsigned long ulFileLineNumber,
	long lErrMsgIdentifier)

{

	if (strlen (pFileName) >= ERRMSGBASE_MAXFILENAMELENGTH)
	{
		strncpy (_pFileName, pFileName, ERRMSGBASE_MAXFILENAMELENGTH - 1);
		_pFileName [ERRMSGBASE_MAXFILENAMELENGTH - 1]		= '\0';
	}
	else
		strcpy (_pFileName, pFileName);

	_ulFileLineNumber		= ulFileLineNumber;
	_lErrMsgIdentifier		= lErrMsgIdentifier;

	_pErrMsg			= (char *) NULL;

	_pucUserData		= (unsigned char *) NULL;
	_ulUserDataBytes	= 0;
}


ErrMsgBase:: ~ErrMsgBase (void)

{

	destroy ();
}


long ErrMsgBase:: buildCustomErrMsg (
	ErrMsgInfo_p peiErrMsgInfo, unsigned long ulMaxErrMsgNumber,
	long lErrMsgIdentifier, const char *pErrMsgClassName)

{

	#if defined(ERRMSGBASE_FAST)
		if (buildMessage (pErrMsgClassName, lErrMsgIdentifier,
			(peiErrMsgInfo [lErrMsgIdentifier]. _pErrMsg)))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: buildMessage failed" << std:: endl;
			#endif

			return 1;
		}
	#else
		unsigned long		ulErrMsgIndex;
		Boolean_t			bIsFound;


		bIsFound	= false;
		for (ulErrMsgIndex = 0; ulErrMsgIndex < ulMaxErrMsgNumber;
			ulErrMsgIndex++)
		{
			if (peiErrMsgInfo [ulErrMsgIndex].
				_lIdentifier == lErrMsgIdentifier)
			{
				bIsFound	= true;

				break;
			}
		}

		if (bIsFound)
		{
			if (buildMessage (pErrMsgClassName, lErrMsgIdentifier,
				(peiErrMsgInfo [ulErrMsgIndex]. _pErrMsg)))
			{
				#ifdef __MOSYNC__
				#else
					std:: cerr << "ErrMsgBase:: buildMessage failed"
						<< std:: endl;
				#endif

				return 1;
			}
		}
	#endif


	return 0;
}


long ErrMsgBase:: buildCustomErrMsg (
	ErrMsgInfo_p peiErrMsgInfo, unsigned long ulMaxErrMsgNumber,
	long lErrMsgIdentifier, const char *pErrMsgClassName,
	va_list vaInputList, long lArgumentsNumber)

{

	#if defined(ERRMSGBASE_FAST)
		if (buildMessage (pErrMsgClassName, lErrMsgIdentifier,
			(peiErrMsgInfo [lErrMsgIdentifier]. _pErrMsg),
			vaInputList, lArgumentsNumber))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: buildMessage failed" << std:: endl;
			#endif

			return 1;
		}
	#else
		unsigned long		ulErrMsgIndex;
		Boolean_t			bIsFound;


		bIsFound	= false;
		for (ulErrMsgIndex = 0; ulErrMsgIndex < ulMaxErrMsgNumber;
			ulErrMsgIndex++)
		{
			if (peiErrMsgInfo [ulErrMsgIndex].
				_lIdentifier == lErrMsgIdentifier)
			{
				bIsFound	= true;

				break;
			}
		}

		if (bIsFound)
		{
			if (buildMessage (pErrMsgClassName, lErrMsgIdentifier,
				(peiErrMsgInfo [ulErrMsgIndex]. _pErrMsg),
				vaInputList, lArgumentsNumber))
			{
				#ifdef __MOSYNC__
				#else
					std:: cerr << "ErrMsgBase:: buildMessage failed"
						<< std:: endl;
				#endif

				return 1;
			}
		}
	#endif


	return 0;
}


ErrMsgBase:: operator const char *()

{

	return _pErrMsg;
}


Boolean_t ErrMsgBase:: operator ==(const ErrMsgBase *perErrMsgBase)

{

	if (_lErrMsgIdentifier == perErrMsgBase -> _lErrMsgIdentifier)
		return true;
	else
		return false;
}


long ErrMsgBase:: operator !=(const ErrMsgBase *perErrMsgBase)	

{

	if (_lErrMsgIdentifier != perErrMsgBase -> _lErrMsgIdentifier)	
		return true;
	else
		return false;

}


ErrMsgBase &ErrMsgBase:: operator =(const ErrMsgBase &erErrMsgBase)

{

	long	lStringBufferBlockNumber;


	if (destroy ())
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "ErrMsgBase:: destroy failed" << std:: endl;
		#endif

		return *this;
	}

	if (strlen (erErrMsgBase. _pFileName) >= ERRMSGBASE_MAXFILENAMELENGTH)
	{
		strncpy (_pFileName, erErrMsgBase. _pFileName,
			ERRMSGBASE_MAXFILENAMELENGTH - 1);
		_pFileName [ERRMSGBASE_MAXFILENAMELENGTH - 1]		= '\0';
	}
	else
		strcpy (_pFileName, erErrMsgBase. _pFileName);

	_ulFileLineNumber		= erErrMsgBase. _ulFileLineNumber;
	_lErrMsgIdentifier		= erErrMsgBase. _lErrMsgIdentifier;

	lStringBufferBlockNumber	= 0;
	if (erErrMsgBase. _pErrMsg != (const char *) NULL)
	{
		if (allocMemoryBlockForStringIfNecessary (
			&lStringBufferBlockNumber, &_pErrMsg,
			strlen (erErrMsgBase. _pErrMsg)))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr
					<< "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed"
					<< std:: endl;
			#endif

			return *this;
		}
		strcpy (_pErrMsg, erErrMsgBase. _pErrMsg);
	}

	if (!(erErrMsgBase. _pucUserData == (unsigned char *) NULL ||
		erErrMsgBase. _ulUserDataBytes == 0))
	{
		if (setUserData (erErrMsgBase. _pucUserData,
			erErrMsgBase. _ulUserDataBytes))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: setUserData failed" << std:: endl;
			#endif

			return *this;
		}
	}


	return *this;
}


ErrMsgBase:: ErrMsgBase (const ErrMsgBase_t &erErrMsgBase)

{

	_pErrMsg			= (char *) NULL;
	_pFileName [0]		= '\0';
	_ulFileLineNumber	= 0;
	_pucUserData		= (unsigned char *) NULL;
	_ulUserDataBytes	= 0;
	_lErrMsgIdentifier	= 0;

	*this		= erErrMsgBase;

}


long ErrMsgBase:: setUserData (
	void *pvUserData, unsigned long ulUserDataBytes)

{

	unsigned char		*pucLocalUserData;


	if (pvUserData == (void *) NULL)
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "activation wrong" << std:: endl;
		#endif

		return 1;
	}

	if (ulUserDataBytes <= ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
	{
		pucLocalUserData		= _pucPreAllocatedUserData;
	}
	else
	{
		if ((pucLocalUserData = new unsigned char [ulUserDataBytes]) ==
			(void *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "new failed" << std:: endl;
			#endif

			return 1;
		}
	}

	// if the object contains user data and the user data was allocated by 'new'
	if (_ulUserDataBytes > 0 &&
		_ulUserDataBytes > ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
	{
		// delete of the old allocated user data
		delete [] _pucUserData;
		_pucUserData		= (unsigned char *) NULL;
		_ulUserDataBytes	= 0;
	}

	_pucUserData				= pucLocalUserData;

	memcpy (_pucUserData, pvUserData, ulUserDataBytes);

	_ulUserDataBytes		= ulUserDataBytes;


	return 0;
}


long ErrMsgBase:: getUserData (
	void *pvUserData, unsigned long *pulUserDataBytes)

{

	if (pvUserData == (void *) NULL ||
		pulUserDataBytes == (unsigned long *) NULL)
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "activation wrong" << std:: endl;
		#endif

		return 1;
	}

	*pulUserDataBytes		= _ulUserDataBytes;
	if (_ulUserDataBytes != 0)
		memcpy (pvUserData, _pucUserData, _ulUserDataBytes);


	return 0;
}


long ErrMsgBase:: getErrMsgIdentifier (long *plErrMsgIdentifier)

{

	*plErrMsgIdentifier		= _lErrMsgIdentifier;


	return 0;
}


long ErrMsgBase:: buildMessage (const char *pErrMsgClassName,
	long lErrMsgIdentifier,
	const char *pPredefinedMessage)

{

	long		lStringBufferBlockNumber;
	long		lMaxErrorCodeLength;
	long		lMaxFileLineLength;

	if (pErrMsgClassName == (const char *) NULL ||
		pPredefinedMessage == (const char *) NULL)
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "activation wrong" << std:: endl;
		#endif

		return 1;
	}

	lStringBufferBlockNumber		= 0;
	lMaxErrorCodeLength				= 16;
	lMaxFileLineLength				= 16;

	if (allocMemoryBlockForStringIfNecessary (
		&lStringBufferBlockNumber, &_pErrMsg,
		7 /* strlen ("Class: ") */ + strlen (pErrMsgClassName) +
		8 /* strlen (", Code: ") */ + lMaxErrorCodeLength +
		8 /* strlen (", File: ") */ + strlen (_pFileName) +
		8 /* strlen (", Line: ") */ + lMaxFileLineLength +
		7 /* strlen (", Msg: ") */ + strlen (pPredefinedMessage)))
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
		#endif

		return 1;
	}

	sprintf (_pErrMsg,
		"Class: %s, Code: %ld, File: %s, Line: %lu, Msg: %s",
		pErrMsgClassName, lErrMsgIdentifier,
		_pFileName, _ulFileLineNumber,
		pPredefinedMessage);


	return 0;
}


long ErrMsgBase:: buildMessage (const char *pErrMsgClassName,
	long lErrMsgIdentifier,
	const char *pPredefinedMessage,
	va_list vaInputList, long lArgumentsNumber)

{

	#if defined(ERRMSGBASE_FAST)
		long		lStringBufferBlockNumber;
		long		lMaxErrorCodeLength;
		long		lMaxFileLineLength;

		if (pErrMsgClassName == (const char *) NULL ||
			pPredefinedMessage == (const char *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "activation wrong" << std:: endl;
			#endif

			return 1;
		}

		lStringBufferBlockNumber		= 0;
		lMaxErrorCodeLength				= 16;
		lMaxFileLineLength				= 16;

		if (allocMemoryBlockForStringIfNecessary (
			&lStringBufferBlockNumber, &_pErrMsg,
			7 /* strlen ("Class: ") */ + strlen (pErrMsgClassName) +
			8 /* strlen (", Code: ") */ + lMaxErrorCodeLength +
			8 /* strlen (", File: ") */ + strlen (_pFileName) +
			8 /* strlen (", Line: ") */ + lMaxFileLineLength +
			7 /* strlen (", Msg: ") */ + strlen (pPredefinedMessage)))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
			#endif

			return 1;
		}

		sprintf (_pErrMsg,
			"Class: %s, Code: %ld, File: %s, Line: %lu, Msg: %s",
			pErrMsgClassName, lErrMsgIdentifier,
			_pFileName, _ulFileLineNumber,
			pPredefinedMessage);
	#else
		long		lStringBufferBlockNumber;
		long		lMaxErrorCodeLength;
		long		lMaxFileLineLength;
		long		lModulesCounter;
		const char	*pPredefinedMessageNext;
		const char	*pPredefinedMessagePrev;
		char		*pError;
		long		lErrorCurrentLength;


		lStringBufferBlockNumber		= 0;
		lMaxErrorCodeLength				= 16;
		lMaxFileLineLength				= 16;

		if (pErrMsgClassName == (const char *) NULL ||
			pPredefinedMessage == (const char *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "activation wrong" << std:: endl;
			#endif

			return 1;
		}

		if (allocMemoryBlockForStringIfNecessary (
			&lStringBufferBlockNumber, &pError,
			7 /* strlen ("Class: ") */ + strlen (pErrMsgClassName) +
			8 /* strlen (", Code: ") */ + lMaxErrorCodeLength +
			8 /* strlen (", File: ") */ + strlen (_pFileName) +
			8 /* strlen (", Line: ") */ + lMaxFileLineLength +
			7 /* strlen (", Msg: ") */))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
			#endif

			return 1;
		}

		sprintf (pError,
			"Class: %s, Code: %ld, File: %s, Line: %lu, Msg: ",
			pErrMsgClassName, lErrMsgIdentifier, _pFileName, _ulFileLineNumber);

		lErrorCurrentLength	= strlen (pError);

		lModulesCounter			= 0;
		pPredefinedMessagePrev	= pPredefinedMessage;
		pPredefinedMessageNext	= pPredefinedMessage;
		while ((pPredefinedMessageNext =
			strchr (pPredefinedMessagePrev, '%')) !=
			(char *) NULL)
		{
			lModulesCounter++;

			if (allocMemoryBlockForStringIfNecessary (
				&lStringBufferBlockNumber, &pError,
				pPredefinedMessageNext - pPredefinedMessagePrev))
			{
				#ifdef __MOSYNC__
				#else
					std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
				#endif

				delete [] (pError);
				pError	= (char *) NULL;

				return 1;
			}

			strncat (pError, pPredefinedMessagePrev,
				pPredefinedMessageNext - pPredefinedMessagePrev);
			lErrorCurrentLength	+=
				(pPredefinedMessageNext - pPredefinedMessagePrev);
			pError [lErrorCurrentLength]	= '\0';

			if (*(pPredefinedMessageNext + 1) == '%')
			{
				// Se si tratta di %%

				if (allocMemoryBlockForStringIfNecessary (
					&lStringBufferBlockNumber, &pError, 1))
				{
					#ifdef __MOSYNC__
					#else
						std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
					#endif

					delete [] (pError);
					pError	= (char *) NULL;

					return 1;
				}

				strcat (pError, "%");
				lErrorCurrentLength	+= 1;

				pPredefinedMessageNext	+= 2;
				pPredefinedMessagePrev	= pPredefinedMessageNext;
			}
			else if (lModulesCounter > lArgumentsNumber)
			{
				// Se viene trovato il % ma non il corrispondente
				//	argomento di input

				if (allocMemoryBlockForStringIfNecessary (
					&lStringBufferBlockNumber, &pError, 1))
				{
					#ifdef __MOSYNC__
					#else
						std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
					#endif

					delete [] (pError);
					pError	= (char *) NULL;

					return 1;
				}

				strcat (pError, "%");
				lErrorCurrentLength	+= 1;

				pPredefinedMessageNext	+= 1;
				pPredefinedMessagePrev	= pPredefinedMessageNext;
			}
			else
			{
				if (*(pPredefinedMessageNext + 1) == 'd' ||
					(*(pPredefinedMessageNext + 1) == 'l' &&
					*(pPredefinedMessageNext + 2) == 'd'))
				{
					long		lValue;
					char		pValue [ERRMSGBASE_MAXLONGLENGTH];
					long		lValueLength;

					lValue			= (long) va_arg (vaInputList, long);
					sprintf (pValue, "%ld", lValue);
					lValueLength	= strlen (pValue);

					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, lValueLength))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}
					strcat (pError, pValue);
					lErrorCurrentLength	+= lValueLength;

					if (*(pPredefinedMessageNext + 1) == 'l')
						pPredefinedMessageNext	+= 3;
					else
						pPredefinedMessageNext	+= 2;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
				else if (*(pPredefinedMessageNext + 1) == 'l' &&
					*(pPredefinedMessageNext + 2) == 'u')
				{
					unsigned long		ulValue;
					char				pValue [ERRMSGBASE_MAXLONGLENGTH];
					long				lValueLength;

					ulValue			= (unsigned long) va_arg (vaInputList,
						unsigned long);
					sprintf (pValue, "%lu", ulValue);
					lValueLength	= strlen (pValue);

					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, lValueLength))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}
					strcat (pError, pValue);
					lErrorCurrentLength	+= lValueLength;

					pPredefinedMessageNext	+= 3;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
				#ifdef WIN32
					else if (*(pPredefinedMessageNext + 1) == 'l' &&
						*(pPredefinedMessageNext + 2) == 'l' &&
						*(pPredefinedMessageNext + 3) == 'd')
					{
						__int64				llValue;
						char				pValue [ERRMSGBASE_MAXLONGLENGTH];
						long				lValueLength;

						llValue		= (__int64) va_arg (vaInputList, __int64);
						sprintf (pValue, "%I64d", llValue);
						lValueLength	= strlen (pValue);

						if (allocMemoryBlockForStringIfNecessary (
							&lStringBufferBlockNumber, &pError, lValueLength))
						{
							#ifdef __MOSYNC__
							#else
								std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
							#endif

							delete [] (pError);
							pError	= (char *) NULL;

							return 1;
						}
						strcat (pError, pValue);
						lErrorCurrentLength	+= lValueLength;

						pPredefinedMessageNext	+= 4;
						pPredefinedMessagePrev	= pPredefinedMessageNext;
					}
				#else
					else if (*(pPredefinedMessageNext + 1) == 'l' &&
						*(pPredefinedMessageNext + 2) == 'l' &&
						*(pPredefinedMessageNext + 3) == 'd')
					{
						long long			llValue;
						char				pValue [ERRMSGBASE_MAXLONGLENGTH];
						long				lValueLength;

						llValue			=
							(long long) va_arg (vaInputList, long long);
						sprintf (pValue, "%lld", llValue);
						lValueLength	= strlen (pValue);

						if (allocMemoryBlockForStringIfNecessary (
							&lStringBufferBlockNumber, &pError, lValueLength))
						{
							#ifdef __MOSYNC__
							#else
								std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
							#endif

							delete [] (pError);
							pError	= (char *) NULL;

							return 1;
						}
						strcat (pError, pValue);
						lErrorCurrentLength	+= lValueLength;

						pPredefinedMessageNext	+= 4;
						pPredefinedMessagePrev	= pPredefinedMessageNext;
					}
				#endif
				#ifdef WIN32
					// in WIN32 doesn't exist a type like 'unsigned long long'
					else if (*(pPredefinedMessageNext + 1) == 'l' &&
						*(pPredefinedMessageNext + 2) == 'l' &&
						*(pPredefinedMessageNext + 3) == 'u')
					{
						__int64				ullValue;
						char				pValue [ERRMSGBASE_MAXLONGLENGTH];
						long				lValueLength;

						ullValue			= (__int64) va_arg (vaInputList,
							__int64);
						sprintf (pValue, "%I64d", ullValue);
						lValueLength	= strlen (pValue);

						if (allocMemoryBlockForStringIfNecessary (
							&lStringBufferBlockNumber, &pError, lValueLength))
						{
							#ifdef __MOSYNC__
							#else
								std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
							#endif

							delete [] (pError);
							pError	= (char *) NULL;

							return 1;
						}
						strcat (pError, pValue);
						lErrorCurrentLength	+= lValueLength;

						pPredefinedMessageNext	+= 4;
						pPredefinedMessagePrev	= pPredefinedMessageNext;
					}
				#else
					else if (*(pPredefinedMessageNext + 1) == 'l' &&
						*(pPredefinedMessageNext + 2) == 'l' &&
						*(pPredefinedMessageNext + 3) == 'u')
					{
						unsigned long long	ullValue;
						char				pValue [ERRMSGBASE_MAXLONGLENGTH];
						long				lValueLength;

						ullValue			=
							(unsigned long long) va_arg (vaInputList,
							unsigned long long);
						sprintf (pValue, "%llu", ullValue);
						lValueLength	= strlen (pValue);

						if (allocMemoryBlockForStringIfNecessary (
							&lStringBufferBlockNumber, &pError, lValueLength))
						{
							#ifdef __MOSYNC__
							#else
								std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
							#endif

							delete [] (pError);
							pError	= (char *) NULL;

							return 1;
						}
						strcat (pError, pValue);
						lErrorCurrentLength	+= lValueLength;

						pPredefinedMessageNext	+= 4;
						pPredefinedMessagePrev	= pPredefinedMessageNext;
					}
				#endif
				else if (*(pPredefinedMessageNext + 1) == 'f' ||
					(*(pPredefinedMessageNext + 1) == 'l' &&
					*(pPredefinedMessageNext + 2) == 'f'))
				{
					double		dValue;
					char		pValue [ERRMSGBASE_MAXFLOATLENGTH];
					long		lValueLength;

					dValue			= (double) va_arg (vaInputList, double);
					sprintf (pValue, "%lf", dValue);
					lValueLength	= strlen (pValue);

					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, lValueLength))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}
					strcat (pError, pValue);
					lErrorCurrentLength	+= lValueLength;

					if (*(pPredefinedMessageNext + 1) == 'l')
						pPredefinedMessageNext	+= 3;
					else
						pPredefinedMessageNext	+= 2;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
				else if (*(pPredefinedMessageNext + 1) == 'c')
				{
					char		cValue;
					long		lValueLength;

					cValue			= (char) va_arg (vaInputList, int);
					lValueLength	= 1;

					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, lValueLength))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}
					pError [lErrorCurrentLength]	= cValue;
					pError [lErrorCurrentLength + 1]	= '\0';
					lErrorCurrentLength	+= lValueLength;

					pPredefinedMessageNext	+= 2;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
				else if (*(pPredefinedMessageNext + 1) == 's')
				{
					char		*pValue;
					long		lValueLength;

					pValue			= (char *) va_arg (vaInputList, char *);
					if (pValue == (char *) NULL)
						lValueLength	=
							strlen (ERRMSGBASE_NULLPOINTERMESSAGE);
					else
						lValueLength	= strlen (pValue);

					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, lValueLength))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}

					if (pValue == (char *) NULL)
						strcat (pError, ERRMSGBASE_NULLPOINTERMESSAGE);
					else
						strcat (pError, pValue);
					lErrorCurrentLength	+= lValueLength;

					pPredefinedMessageNext	+= 2;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
				else
				{
					if (allocMemoryBlockForStringIfNecessary (
						&lStringBufferBlockNumber, &pError, 1))
					{
						#ifdef __MOSYNC__
						#else
							std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
						#endif

						delete [] (pError);
						pError	= (char *) NULL;

						return 1;
					}

					strcat (pError, "%");
					lErrorCurrentLength	+= 1;

					pPredefinedMessageNext	+= 1;
					pPredefinedMessagePrev	= pPredefinedMessageNext;
				}
			}
		}

		if (allocMemoryBlockForStringIfNecessary (
			&lStringBufferBlockNumber, &pError,
			strlen (pPredefinedMessagePrev)))
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "ErrMsgBase:: allocMemoryBlockForStringIfNecessary failed" << std:: endl;
			#endif

			delete [] (pError);
			pError	= (char *) NULL;

			return 1;
		}

		strcat (pError, pPredefinedMessagePrev);
		lErrorCurrentLength			+= strlen (pPredefinedMessagePrev);

		_pErrMsg					= pError;
	#endif


	return 0;
}


long ErrMsgBase:: destroy (void)

{

	_pFileName [0]		= '\0';

	if (_pErrMsg != (char *) NULL)
	{
		delete [] _pErrMsg;
		_pErrMsg	= (char *) NULL;
	}

	// if the object contains user data and the user data was allocated by 'new'
	if (_ulUserDataBytes > 0 &&
		_ulUserDataBytes > ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
	{
		delete [] _pucUserData;
		_pucUserData		= (unsigned char *) NULL;
		_ulUserDataBytes	= 0;
	}

	return 0;
}


long ErrMsgBase:: allocMemoryBlockForStringIfNecessary (
	long *plStringBufferBlockNumber,
	char **pStringBuffer, long lLengthToInsert)

{

	long		lStringBufferLength;


	if (*plStringBufferBlockNumber != 0)
		lStringBufferLength		= strlen (*pStringBuffer);
	else
		lStringBufferLength		= 0;

	if (*plStringBufferBlockNumber == 0 ||
		lStringBufferLength + lLengthToInsert + 1 >=
		(*plStringBufferBlockNumber) * ERRMSGBASE_BLOCKLENGTH)
	{
		char			*pLocalStringBuffer;


		do
		{
			(*plStringBufferBlockNumber)	+= 1;
		}
		while (lStringBufferLength + lLengthToInsert >=
			(*plStringBufferBlockNumber) * ERRMSGBASE_BLOCKLENGTH);

		if ((pLocalStringBuffer = new char [
			(*plStringBufferBlockNumber) * ERRMSGBASE_BLOCKLENGTH]) ==
			(char *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "new failed" << std:: endl;
			#endif

			return 1;
		}

		if (lStringBufferLength != 0)
		{
			strcpy (pLocalStringBuffer, *pStringBuffer);
			delete [] (*pStringBuffer);
		}
		else
			strcpy (pLocalStringBuffer, "");

		*pStringBuffer = pLocalStringBuffer;
	}


	return 0;
}

