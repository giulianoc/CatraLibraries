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


#include "ConfigurationItem.h"
#include "FileIO.h"
#include <assert.h>



ConfigurationItem:: ConfigurationItem (void)

{

	_stCfgItemStatus		= CFGITEM_BUILDED;

}


ConfigurationItem:: ~ConfigurationItem (void)

{

	if (_stCfgItemStatus  == CFGITEM_INITIALIZED)
		finish ();
}



ConfigurationItem:: ConfigurationItem (const ConfigurationItem &)

{

	assert (1==0);

	// to do

}


ConfigurationItem &ConfigurationItem:: operator = (const ConfigurationItem &)

{

	assert (1==0);

	// to do

	return *this;

}


Error ConfigurationItem:: init (const char *pItemName, const char *pItemComment,
	Boolean_t bIsCaseSensitive, long lBufferToAllocOnOverflow)

{

	long		lBufferIndex;


	if (_stCfgItemStatus  != CFGITEM_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (pItemName == (char *) NULL || pItemComment == (char *) NULL ||
		lBufferToAllocOnOverflow < 1)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	_bIsCaseSensitive			= bIsCaseSensitive;
	_lBufferToAllocOnOverflow	= lBufferToAllocOnOverflow;
	_lValuesNumber				= 0;
	_lAllocatedBuffersNumber	= _lBufferToAllocOnOverflow;

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (_configurationItemMutex. init (PMutex:: MUTEX_FAST) !=
		#else	// POSIX
			/*
			#if defined(__sparc)	 // SunOs (one of the SunO first version)
				if (_configurationItemMutex. init (
					PMutex:: MUTEX_PROCESS_PRIVATE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
			*/
			#if defined(__CYGWIN__)
				if (_configurationItemMutex. init (PMutex:: MUTEX_RECURSIVE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
				if (_configurationItemMutex. init (PMutex:: MUTEX_FAST) !=
			#endif
		#endif
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			return err;
		}
	#endif

	if ((_pItemName = new char [strlen (pItemName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationItemMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif

		return err;
	}

	strcpy (_pItemName, pItemName);

	if ((_pItemComment = new char [strlen (pItemComment) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] (_pItemName);
		_pItemName	= (char *) NULL;
		#ifdef _REENTRANT
			if (_configurationItemMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif

		return err;
	}

	strcpy (_pItemComment, pItemComment);

	if ((_pValues = new char * [_lAllocatedBuffersNumber]) == (char **) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] (_pItemComment);
		_pItemComment	= (char *) NULL;
		delete [] (_pItemName);
		_pItemName	= (char *) NULL;
		#ifdef _REENTRANT
			if (_configurationItemMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif

		return err;
	}

	for (lBufferIndex = 0; lBufferIndex < _lAllocatedBuffersNumber;
		lBufferIndex++)
		_pValues [lBufferIndex]	= (char *) NULL;

	_stCfgItemStatus		= CFGITEM_INITIALIZED;


	return errNoError;
}


Error ConfigurationItem:: finish (void)

{

	long		lValueIndex;


	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	for (lValueIndex = 0; lValueIndex < _lValuesNumber; lValueIndex++)
		delete [] (_pValues [lValueIndex]);
	delete [] (_pValues);
	_pValues				= (char **) NULL;

	delete [] (_pItemName);
	_pItemName					= (char *) NULL;
	delete [] (_pItemComment);
	_pItemComment				= (char *) NULL;

	#ifdef _REENTRANT
		if (_configurationItemMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	#endif

	_stCfgItemStatus		= CFGITEM_BUILDED;


	return errNoError;
}


Boolean_t ConfigurationItem:: operator ==(const char *pItemName)

{
	int			iCompare;


	if (_bIsCaseSensitive)
		iCompare	= strcmp (_pItemName, pItemName);
	else
	{
		#ifdef WIN32
			iCompare	= _stricmp (_pItemName, pItemName);
		#else
			iCompare	= strcasecmp (_pItemName, pItemName);
		#endif
	}

	if (iCompare)
		return false;
	else
		return true;
}


Error ConfigurationItem:: insertItemValue (const char *pValue,
	long lValueIndexToInsert)

{

	long		lValueIndex;


	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (pValue == (const char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationItemMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_lValuesNumber == _lAllocatedBuffersNumber)
	{
		char		**pValues;
		long		lBufferIndex;
		long		lValueIndex;


		pValues							= _pValues;

		_lAllocatedBuffersNumber		+= _lBufferToAllocOnOverflow;

		if ((_pValues = new char * [_lAllocatedBuffersNumber]) ==
			(char **) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			_pValues					= pValues;
			_lAllocatedBuffersNumber	-= _lBufferToAllocOnOverflow;
			#ifdef _REENTRANT
				if (_configurationItemMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif


			return err;
		}

		for (lBufferIndex = 0; lBufferIndex < _lAllocatedBuffersNumber;
			lBufferIndex++)
		{
			if (lBufferIndex < _lValuesNumber)
			{
				if ((_pValues [lBufferIndex] = new char [
					strlen (pValues [lBufferIndex]) + 1]) == (char *) NULL)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_NEW_FAILED);

					while (--lBufferIndex >= 0)
						delete [] (_pValues [lBufferIndex]);
					delete [] (_pValues);
					_pValues					= (char **) NULL;

					_pValues					= pValues;
					_lAllocatedBuffersNumber	-= _lBufferToAllocOnOverflow;

					#ifdef _REENTRANT
						if (_configurationItemMutex. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}
					#endif


					return err;
				}

				strcpy (_pValues [lBufferIndex], pValues [lBufferIndex]);
			}
			else
				_pValues [lBufferIndex]		= (char *) NULL;
		}

		for (lValueIndex = 0; lValueIndex < _lValuesNumber; lValueIndex++)
			delete [] (pValues [lValueIndex]);
		delete [] (pValues);
		pValues			= (char **) NULL;
	}

	for (lValueIndex = _lValuesNumber - 1; lValueIndex >= lValueIndexToInsert;
		lValueIndex--)
	{
		if ((_pValues [lValueIndex + 1] = new char [
			strlen (_pValues [lValueIndex]) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			// error not recovered

			#ifdef _REENTRANT
				if (_configurationItemMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif


			return err;
		}
		
		strcpy (_pValues [lValueIndex + 1], _pValues [lValueIndex]);

		delete [] (_pValues [lValueIndex]);
		_pValues [lValueIndex]		= (char *) NULL;
	}

	lValueIndex++;

	if ((_pValues [lValueIndex] = new char [strlen (pValue) + 1]) ==
		(char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);
		// error not recovered

		#ifdef _REENTRANT
			if (_configurationItemMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	strcpy (_pValues [lValueIndex], pValue);

	_lValuesNumber++;

	#ifdef _REENTRANT
		if (_configurationItemMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationItem:: appendItemValue (const char *pValue)

{

	return (insertItemValue (pValue, _lValuesNumber));
}


Error ConfigurationItem:: appendItemValues (long lValuesNumber,
	const char **pValues)

{

	long		lValueIndex;


	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (lValuesNumber < 1 || pValues == (const char **) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	for (lValueIndex = 0; lValueIndex < lValuesNumber; lValueIndex++)
	{
		if (appendItemValue (pValues [lValueIndex]) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error ConfigurationItem:: getIsCaseSensitive (Boolean_p pbIsCaseSensitive)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	*pbIsCaseSensitive		= _bIsCaseSensitive;


	return errNoError;
}


Error ConfigurationItem:: getItemName (char *pItemName)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	strcpy (pItemName, _pItemName);


	return errNoError;
}


Error ConfigurationItem:: getItemNameLength (long *pItemNameLength)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	*pItemNameLength		= strlen (_pItemName);


	return errNoError;
}


Error ConfigurationItem:: modifyItemName (const char *pItemName)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (pItemName == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationItemMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_pItemName != (char *) NULL)
	{
		delete [] (_pItemName);
		_pItemName		= (char *) NULL;
	}

	if ((_pItemName = new char [strlen (pItemName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationItemMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	strcpy (_pItemName, pItemName);

	#ifdef _REENTRANT
		if (_configurationItemMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationItem:: getItemComment (char *pItemComment)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	strcpy (pItemComment, _pItemComment);


	return errNoError;
}


Error ConfigurationItem:: getItemCommentLength (long *plItemCommentLength)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	*plItemCommentLength		= strlen (_pItemComment);


	return errNoError;
}


Error ConfigurationItem:: modifyItemComment (const char *pItemComment)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (pItemComment == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationItemMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_pItemComment != (char *) NULL)
	{
		delete [] (_pItemComment);
		_pItemComment		= (char *) NULL;
	}

	if ((_pItemComment = new char [strlen (pItemComment) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationItemMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pItemComment, pItemComment);

	#ifdef _REENTRANT
		if (_configurationItemMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationItem:: getItemValuesNumber (long *plValuesNumber)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	*plValuesNumber		= _lValuesNumber;


	return errNoError;
}


Error ConfigurationItem:: getItemValue (
	char *pItemValue, unsigned long ulMaxItemValueLength,
	long lValueIndex)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (lValueIndex >= _lValuesNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	if (strlen (_pValues [lValueIndex]) >= ulMaxItemValueLength)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_BUFFERTOOSHORT,
			1, ulMaxItemValueLength);

		return err;
	}

	strcpy (pItemValue, _pValues [lValueIndex]);


	return errNoError;
}


Error ConfigurationItem:: getItemValueLength (long *plItemValueLength,
	long lValueIndex)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (lValueIndex >= _lValuesNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	*plItemValueLength		= strlen (_pValues [lValueIndex]);


	return errNoError;
}


Error ConfigurationItem:: modifyItemValue (const char *pNewValue,
	long lValueIndexToModify)

{

	if (_stCfgItemStatus  != CFGITEM_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgItemStatus);

		return err;
	}

	if (pNewValue == (const char *) NULL ||
		lValueIndexToModify >= _lValuesNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationItemMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	delete [] (_pValues [lValueIndexToModify]);
	_pValues [lValueIndexToModify]		= (char *) NULL;

	if ((_pValues [lValueIndexToModify] = new char [
		strlen (pNewValue) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);
		// error not recovered

		#ifdef _REENTRANT
			if (_configurationItemMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}
		
	strcpy (_pValues [lValueIndexToModify], pNewValue);

	#ifdef _REENTRANT
		if (_configurationItemMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationItem:: copy (ConfigurationItem *pciDestCfgItem,
	ConfigurationItem *pciSrcCfgItem, long lBufferToAllocOnOverflow)

{

	char		*pItemName;
	char		*pItemComment;
	char		*pValue;
	long		lItemNameLength;
	long		lItemCommentLength;
	long		lItemValueLength;
	long		lValuesNumber;
	long		lValueIndex;
	Boolean_t	bIsCaseSensitive;


	if (pciSrcCfgItem -> _stCfgItemStatus != CFGITEM_INITIALIZED ||
		pciDestCfgItem -> _stCfgItemStatus != CFGITEM_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) pciSrcCfgItem -> _stCfgItemStatus);

		return err;
	}

	if (pciSrcCfgItem -> getIsCaseSensitive (&bIsCaseSensitive) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETISCASESENSITIVE_FAILED);

		return err;
	}

	if (pciSrcCfgItem -> getItemNameLength (&lItemNameLength) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMNAMELENGTH_FAILED);

		return err;
	}

	if (pciSrcCfgItem -> getItemCommentLength (&lItemCommentLength) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMCOMMENTLENGTH_FAILED);

		return err;
	}

	if ((pItemName = new char [lItemNameLength + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		return err;
	}

	if ((pItemComment = new char [lItemCommentLength + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] pItemName;
		pItemName		= (char *) NULL;

		return err;
	}

	if (pciSrcCfgItem -> getItemName (pItemName) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMNAME_FAILED);

		delete [] pItemName;
		pItemName		= (char *) NULL;
		delete [] pItemComment;
		pItemComment		= (char *) NULL;

		return err;
	}

	if (pciSrcCfgItem -> getItemComment (pItemComment) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMCOMMENT_FAILED);

		delete [] pItemName;
		pItemName		= (char *) NULL;
		delete [] pItemComment;
		pItemComment		= (char *) NULL;

		return err;
	}

	if (pciDestCfgItem -> init (pItemName, pItemComment,
		bIsCaseSensitive, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_INIT_FAILED);

		delete [] pItemName;
		pItemName		= (char *) NULL;
		delete [] pItemComment;
		pItemComment		= (char *) NULL;

		return err;
	}

	delete [] pItemName;
	pItemName		= (char *) NULL;
	delete [] pItemComment;
	pItemComment		= (char *) NULL;

	if (pciSrcCfgItem -> getItemValuesNumber (&lValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);

		pciDestCfgItem -> finish ();


		return err;
	}

	for (lValueIndex = 0; lValueIndex < lValuesNumber; lValueIndex++)
	{
		if (pciSrcCfgItem -> getItemValueLength (&lItemValueLength,
			lValueIndex) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_GETITEMVALUELENGTH_FAILED);

			pciDestCfgItem -> finish ();

			return err;
		}

		if ((pValue = new char [lItemValueLength + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			pciDestCfgItem -> finish ();

			return err;
		}

		if (pciSrcCfgItem -> getItemValue (pValue, lItemValueLength + 1,
			lValueIndex) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);

			delete [] pValue;
			pValue			= (char *) NULL;
			pciDestCfgItem -> finish ();


			return err;
		}

		if (pciDestCfgItem -> appendItemValue (pValue) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);

			delete [] pValue;
			pValue			= (char *) NULL;
			pciDestCfgItem -> finish ();


			return err;
		}

		delete [] pValue;
		pValue			= (char *) NULL;
	}


	return errNoError;
}


std:: ostream &operator << (std:: ostream &osOutputStream, ConfigurationItem &ciItem)

{

	if (strcmp (ciItem. _pItemComment, ""))
	{
		char		*pLocalBuffer;
		char		*pItemCommentCurrentNewLine;
		char		*pItemCommentNextNewLine;


		if ((pLocalBuffer = new char [
			strlen (ciItem. _pItemComment) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			return osOutputStream;
		}

		pItemCommentCurrentNewLine		= ciItem. _pItemComment;
		pItemCommentNextNewLine			= ciItem. _pItemComment;

		while ((pItemCommentNextNewLine =
			strchr (pItemCommentCurrentNewLine, '\n')) != (char *) NULL)
		{
			memcpy (pLocalBuffer, pItemCommentCurrentNewLine,
				pItemCommentNextNewLine - pItemCommentCurrentNewLine + 1);
			pLocalBuffer [pItemCommentNextNewLine -
				pItemCommentCurrentNewLine + 1]		= '\0';
			osOutputStream << "\t##" << pLocalBuffer;

			pItemCommentCurrentNewLine		= pItemCommentNextNewLine + 1;
		}

		osOutputStream << "\t##" << pItemCommentCurrentNewLine << std:: endl;

		delete [] pLocalBuffer;
		pLocalBuffer			= (char *) NULL;
	}

	osOutputStream << "	";
	if (ciItem. _lValuesNumber == 0)
	{
		if (strchr (ciItem. _pItemName, ' ') != (char *) NULL ||
			strchr (ciItem. _pItemName, '\t') != (char *) NULL ||
			strchr (ciItem. _pItemName, '"') != (char *) NULL)
			osOutputStream << "\"" << ciItem. _pItemName << "\"";
		else
			osOutputStream << ciItem. _pItemName;
		osOutputStream << " = " << std:: endl;
	}
	else if (ciItem. _lValuesNumber == 1)
	{
		if (strchr (ciItem. _pItemName, ' ') != (char *) NULL ||
			strchr (ciItem. _pItemName, '\t') != (char *) NULL ||
			strchr (ciItem. _pItemName, '"') != (char *) NULL)
			osOutputStream << "\"" << ciItem. _pItemName << "\"";
		else
			osOutputStream << ciItem. _pItemName;
		osOutputStream << " = ";
		if (strchr (ciItem. _pValues [0], ' ') != (char *) NULL ||
			strchr (ciItem. _pValues [0], '\t') != (char *) NULL ||
			strchr (ciItem. _pValues [0], '"') != (char *) NULL)
			osOutputStream << "\"" << ciItem. _pValues [0] << "\"";
		else
			osOutputStream << ciItem. _pValues [0];
		osOutputStream << std:: endl;
	}
	else if (ciItem. _lValuesNumber > 1)
	{
		long		lValueIndex;


		if (strchr (ciItem. _pItemName, ' ') != (char *) NULL ||
			strchr (ciItem. _pItemName, '\t') != (char *) NULL ||
			strchr (ciItem. _pItemName, '"') != (char *) NULL)
			osOutputStream << "\"" << ciItem. _pItemName << "\"";
		else
			osOutputStream << ciItem. _pItemName;

		osOutputStream << " = " << std:: endl << "	" << "{" << std:: endl;

		for (lValueIndex = 0; lValueIndex < ciItem. _lValuesNumber;
			lValueIndex++)
		{
			osOutputStream << "	" << "	";

			if (strchr (ciItem. _pValues [lValueIndex], ' ') !=
				(char *) NULL ||
				strchr (ciItem. _pValues [lValueIndex], '\t') !=
				(char *) NULL ||
				strchr (ciItem. _pValues [lValueIndex], '"') !=
				(char *) NULL)
				osOutputStream << "\""
					<< ciItem. _pValues [lValueIndex] << "\"";
			else
				osOutputStream << ciItem. _pValues [lValueIndex];

			if (lValueIndex < ciItem. _lValuesNumber - 1)
				osOutputStream << ",";

			osOutputStream << std:: endl;
		}

		osOutputStream << "	" << "}" << std:: endl;
	}
	else
		;


	return osOutputStream;
}


Error ConfigurationItem:: write (int iFileDescriptor)

{

	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif


	if (strcmp (_pItemComment, ""))
	{
		char		*pLocalBuffer;
		char		*pItemCommentCurrentNewLine;
		char		*pItemCommentNextNewLine;


		if ((pLocalBuffer = new char [
			strlen (_pItemComment) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			return err;
		}

		pItemCommentCurrentNewLine		= _pItemComment;
		pItemCommentNextNewLine			= _pItemComment;

		while ((pItemCommentNextNewLine =
			strchr (pItemCommentCurrentNewLine, '\n')) != (char *) NULL)
		{
			memcpy (pLocalBuffer, pItemCommentCurrentNewLine,
				pItemCommentNextNewLine - pItemCommentCurrentNewLine);
			pLocalBuffer [pItemCommentNextNewLine -
				pItemCommentCurrentNewLine]		= '\0';

			if (FileIO:: writeChars (iFileDescriptor,
				"\t##", 3, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				delete [] pLocalBuffer;
				pLocalBuffer			= (char *) NULL;

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				pLocalBuffer, strlen (pLocalBuffer),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				delete [] pLocalBuffer;
				pLocalBuffer			= (char *) NULL;

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				delete [] pLocalBuffer;
				pLocalBuffer			= (char *) NULL;

				return err;
			}

			pItemCommentCurrentNewLine		= pItemCommentNextNewLine + 1;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			"\t##", 3, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			delete [] pLocalBuffer;
			pLocalBuffer			= (char *) NULL;

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			pItemCommentCurrentNewLine,
			strlen (pItemCommentCurrentNewLine),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			delete [] pLocalBuffer;
			pLocalBuffer			= (char *) NULL;

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			delete [] pLocalBuffer;
			pLocalBuffer			= (char *) NULL;

			return err;
		}

		delete [] pLocalBuffer;
		pLocalBuffer			= (char *) NULL;
	}

	if (FileIO:: writeChars (iFileDescriptor,
		"\t", 1, &llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		return err;
	}

	if (_lValuesNumber == 0)
	{
		if (strchr (_pItemName, ' ') != (char *) NULL ||
			strchr (_pItemName, '\t') != (char *) NULL ||
			strchr (_pItemName, '"') != (char *) NULL)
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}
		else
		{
			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}

		if (FileIO:: writeChars (iFileDescriptor,
			" = ", 3, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			"\"\"", 2, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}
	}
	else if (_lValuesNumber == 1)
	{
		if (strchr (_pItemName, ' ') != (char *) NULL ||
			strchr (_pItemName, '\t') != (char *) NULL ||
			strchr (_pItemName, '"') != (char *) NULL)
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}
		else
		{
			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}

		if (FileIO:: writeChars (iFileDescriptor,
			" = ", 3, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (strchr (_pValues [0], ' ') != (char *) NULL ||
			strchr (_pValues [0], '\t') != (char *) NULL ||
			strchr (_pValues [0], ',') != (char *) NULL ||
			strchr (_pValues [0], '"') != (char *) NULL)
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				_pValues [0], strlen (_pValues [0]),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}
		else if (!strcmp (_pValues [0], ""))
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\"\"", 2, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}
		else
		{
			if (FileIO:: writeChars (iFileDescriptor,
				_pValues [0], strlen (_pValues [0]),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}
	}
	else if (_lValuesNumber > 1)
	{
		long		lValueIndex;


		if (strchr (_pItemName, ' ') != (char *) NULL ||
			strchr (_pItemName, '\t') != (char *) NULL ||
			strchr (_pItemName, '"') != (char *) NULL)
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (FileIO:: writeChars (iFileDescriptor,
				"\"", 1, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}
		else
		{
			if (FileIO:: writeChars (iFileDescriptor,
				_pItemName, strlen (_pItemName),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}

		if (FileIO:: writeChars (iFileDescriptor,
			" = ", 3, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			"\t{", 2, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		for (lValueIndex = 0; lValueIndex < _lValuesNumber;
			lValueIndex++)
		{
			if (FileIO:: writeChars (iFileDescriptor,
				"\t\t", 2, &llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}

			if (!strcmp (_pValues [lValueIndex], ""))
			{
				if (FileIO:: writeChars (iFileDescriptor,
					"\"\"", 2, &llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}
			}
			else if (strchr (_pValues [lValueIndex], ' ') !=
				(char *) NULL ||
				strchr (_pValues [lValueIndex], '\t') !=
				(char *) NULL ||
				strchr (_pValues [lValueIndex], '"') !=
				(char *) NULL)
			{
				if (FileIO:: writeChars (iFileDescriptor,
					"\"", 1, &llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}

				if (FileIO:: writeChars (iFileDescriptor,
					_pValues [lValueIndex], strlen (_pValues [lValueIndex]),
					&llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}

				if (FileIO:: writeChars (iFileDescriptor,
					"\"", 1, &llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}
			}
			else
			{
				if (FileIO:: writeChars (iFileDescriptor,
					_pValues [lValueIndex], strlen (_pValues [lValueIndex]),
					&llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}
			}

			if (lValueIndex < _lValuesNumber - 1)
			{
				if (FileIO:: writeChars (iFileDescriptor,
					",", 1, &llBytesWritten) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_WRITECHARS_FAILED);

					return err;
				}
			}

			if (FileIO:: writeChars (iFileDescriptor,
				CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				return err;
			}
		}

		if (FileIO:: writeChars (iFileDescriptor,
			"\t}", 2, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			CFG_NEWLINE, strlen (CFG_NEWLINE), &llBytesWritten) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}
	}
	else
		;


	return errNoError;
}

