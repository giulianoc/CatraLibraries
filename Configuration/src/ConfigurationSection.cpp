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



#include "ConfigurationSection.h"
#include "FileIO.h"
#include <assert.h>
#include <stdio.h>



ConfigurationSection:: ConfigurationSection (void)

{

	_stCfgSectionStatus		= CFGSECTION_BUILDED;

}


ConfigurationSection:: ~ConfigurationSection (void)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
		finish ();
}



ConfigurationSection:: ConfigurationSection (const ConfigurationSection &)

{

	assert (1==0);

	// to do

}


ConfigurationSection &ConfigurationSection:: operator = (
	const ConfigurationSection &)

{

	assert (1==0);

	// to do

	return *this;

}


Error ConfigurationSection:: init (const char *pSectionName,
	const char *pSectionComment, const char *pSectionDate,
	Boolean_t bIsCaseSensitive, long lCfgItemsToAllocOnOverflow)

{

	if (_stCfgSectionStatus  != CFGSECTION_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (pSectionName == (char *) NULL || pSectionComment == (char *) NULL ||
		lCfgItemsToAllocOnOverflow < 1 ||
		(pSectionDate != (char *) NULL && isValidDate (pSectionDate) == false))
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	_bIsCaseSensitive					= bIsCaseSensitive;
	_lCfgItemsToAllocOnOverflow			= lCfgItemsToAllocOnOverflow;
	_lCfgItemsNumber					= 0;
	_lAllocatedCfgItemNumber			= _lCfgItemsToAllocOnOverflow;

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (_configurationSectionMutex. init (PMutex:: MUTEX_FAST) !=
		#else	// POSIX
			/*
			#if defined(__sparc)	 // SunOs (one of the SunO first version)
				if (_configurationSectionMutex. init (
					PMutex:: MUTEX_PROCESS_PRIVATE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
			*/
			#if defined(__CYGWIN__)
				if (_configurationSectionMutex. init (
					PMutex:: MUTEX_RECURSIVE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
				if (_configurationSectionMutex. init (PMutex:: MUTEX_FAST) !=
			#endif
		#endif
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			return err;
		}
	#endif

	if ((_pSectionName = new char [strlen (pSectionName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationSectionMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pSectionName, pSectionName);

	if ((_pSectionComment = new char [strlen (pSectionComment) + 1]) ==
		(char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] (_pSectionName);
		_pSectionName	= (char *) NULL;
		#ifdef _REENTRANT
			if (_configurationSectionMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pSectionComment, pSectionComment);

	if (pSectionDate != (char *) NULL)
	{
		if ((_pSectionDate = new char [CFG_SECTIONDATELENGTH]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			delete [] (_pSectionComment);
			_pSectionComment	= (char *) NULL;
			delete [] (_pSectionName);
			_pSectionName	= (char *) NULL;
			#ifdef _REENTRANT
				if (_configurationSectionMutex. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
				}
			#endif


			return err;
		}

		strcpy (_pSectionDate, pSectionDate);
	}
	else
		_pSectionDate		= (char *) NULL;

	if ((_pciCfgItems = new ConfigurationItem_t [_lAllocatedCfgItemNumber]) ==
		(ConfigurationItem_p) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] (_pSectionComment);
		_pSectionComment	= (char *) NULL;
		delete [] (_pSectionName);
		_pSectionName	= (char *) NULL;
		#ifdef _REENTRANT
			if (_configurationSectionMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif


		return err;
	}

	_stCfgSectionStatus		= CFGSECTION_INITIALIZED;


	return errNoError;
}


Error ConfigurationSection:: finish (void)

{

	long		lCfgItemIndex;


	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber; lCfgItemIndex++)
		_pciCfgItems [lCfgItemIndex]. finish ();
	delete [] (_pciCfgItems);
	_pciCfgItems			= (ConfigurationItem_p) NULL;

	delete [] (_pSectionName);
	_pSectionName					= (char *) NULL;
	delete [] (_pSectionComment);
	_pSectionComment				= (char *) NULL;

	if (_pSectionDate != (char *) NULL)
	{
		delete [] (_pSectionDate);
		_pSectionDate				= (char *) NULL;
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	#endif

	_stCfgSectionStatus		= CFGSECTION_BUILDED;


	return errNoError;
}


Boolean_t ConfigurationSection:: operator ==(const char *pSectionName)

{

	int				iCompare;


	if (_bIsCaseSensitive)
		iCompare	= strcmp (_pSectionName, pSectionName);
	else
	{
		#ifdef WIN32
			iCompare	= _stricmp (_pSectionName, pSectionName);
		#else
			iCompare	= strcasecmp (_pSectionName, pSectionName);
		#endif
	}

	if (iCompare)
		return false;
	else
		return true;
}


Error ConfigurationSection:: appendCfgItem (
	const ConfigurationItem_p pciCfgItem, long lBufferToAllocOnOverflow)

{

	if (_stCfgSectionStatus != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (pciCfgItem == (const ConfigurationItem_p) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_lCfgItemsNumber == _lAllocatedCfgItemNumber)
	{
		ConfigurationItem_p		pciCfgItems;
		long					lCfgItemIndex;


		pciCfgItems						= _pciCfgItems;

		_lAllocatedCfgItemNumber		+= _lCfgItemsToAllocOnOverflow;

		if ((_pciCfgItems = new ConfigurationItem_t [
			_lAllocatedCfgItemNumber]) == (ConfigurationItem_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			_pciCfgItems				= pciCfgItems;
			_lAllocatedCfgItemNumber	-= _lCfgItemsToAllocOnOverflow;
			#ifdef _REENTRANT
				if (_configurationSectionMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif


			return err;
		}

		for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber;
			lCfgItemIndex++)
		{
			if (ConfigurationItem:: copy (
				&(_pciCfgItems [lCfgItemIndex]),
				&(pciCfgItems [lCfgItemIndex]), lBufferToAllocOnOverflow) !=
				errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_COPY_FAILED);

				while (--lCfgItemIndex >= 0)
					_pciCfgItems [lCfgItemIndex]. finish ();
				delete [] (_pciCfgItems);
				_pciCfgItems				= (ConfigurationItem_p) NULL;

				_pciCfgItems				= pciCfgItems;
				_lAllocatedCfgItemNumber	-= _lCfgItemsToAllocOnOverflow;

				#ifdef _REENTRANT
					if (_configurationSectionMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif


				return err;
			}
		}

		for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber;
			lCfgItemIndex++)
		{
			if (pciCfgItems [lCfgItemIndex]. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);

				#ifdef _REENTRANT
					if (_configurationSectionMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif


				return err;
			}
		}

		delete [] (pciCfgItems);
		pciCfgItems				= (ConfigurationItem_p) NULL;
	}

	if (ConfigurationItem:: copy (&(_pciCfgItems [_lCfgItemsNumber]),
		pciCfgItem, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_COPY_FAILED);

		#ifdef _REENTRANT
			if (_configurationSectionMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	_lCfgItemsNumber++;

	#ifdef _REENTRANT
		if (_configurationSectionMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationSection:: getIsCaseSensitive (Boolean_p pbIsCaseSensitive)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	*pbIsCaseSensitive		= _bIsCaseSensitive;


	return errNoError;
}


Error ConfigurationSection:: getSectionName (char *pSectionName)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	strcpy (pSectionName, _pSectionName);


	return errNoError;
}


Error ConfigurationSection:: getSectionNameLength (long *plSectionNameLength)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	*plSectionNameLength		= strlen (_pSectionName);


	return errNoError;
}


Error ConfigurationSection:: modifySectionName (const char *pSectionName)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (pSectionName == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_pSectionName != (char *) NULL)
	{
		delete [] (_pSectionName);
		_pSectionName		= (char *) NULL;
	}

	if ((_pSectionName = new char [strlen (pSectionName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationSectionMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pSectionName, pSectionName);

	#ifdef _REENTRANT
		if (_configurationSectionMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationSection:: getSectionComment (char *pSectionComment)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	strcpy (pSectionComment, _pSectionComment);


	return errNoError;
}


Error ConfigurationSection:: getSectionCommentLength (
	long *plSectionCommentLength)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	*plSectionCommentLength		= strlen (_pSectionComment);


	return errNoError;
}


Error ConfigurationSection:: modifySectionComment (const char *pSectionComment)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (pSectionComment == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_pSectionComment != (char *) NULL)
	{
		delete [] (_pSectionComment);
		_pSectionComment		= (char *) NULL;
	}

	if ((_pSectionComment = new char [strlen (pSectionComment) + 1]) ==
		(char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationSectionMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pSectionComment, pSectionComment);

	#ifdef _REENTRANT
		if (_configurationSectionMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationSection:: getSectionDate (char *pSectionDate)

{

	if (_stCfgSectionStatus != CFGSECTION_INITIALIZED ||
		pSectionDate == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (_pSectionDate == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTIONDATE_NOTFOUND);

		return err;
	}

	strcpy (pSectionDate, _pSectionDate);


	return errNoError;
}


Error ConfigurationSection:: modifySectionDate (const char *pSectionDate)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (pSectionDate == (char *) NULL)
	{
		if (_pSectionDate != (char *) NULL)
		{
			delete [] _pSectionDate;
			_pSectionDate		= (char *) NULL;
		}
	}
	else
	{
		if (isValidDate (pSectionDate)  == false)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_ACTIVATION_WRONG);

			#ifdef _REENTRANT
				if (_configurationSectionMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif


			return err;
		}

		if (_pSectionDate == (char *) NULL)
		{
			if ((_pSectionDate = new char [CFG_SECTIONDATELENGTH]) ==
				(char *) NULL)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_NEW_FAILED);

				#ifdef _REENTRANT
					if (_configurationSectionMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif


				return err;
			}
		}

		strcpy (_pSectionDate, pSectionDate);
	}

	#ifdef _REENTRANT
		if (_configurationSectionMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationSection:: getCfgItemsNumber (long *plCfgItemsNumber)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	*plCfgItemsNumber		= _lCfgItemsNumber;


	return errNoError;
}


Error ConfigurationSection:: getCfgItemByIndex (long lCfgItemIndex,
	ConfigurationItem_p pciCfgItem, long lBufferToAllocOnOverflow)

{

	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	if (lCfgItemIndex >= _lCfgItemsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	if (ConfigurationItem:: copy (pciCfgItem,
		&(_pciCfgItems [lCfgItemIndex]), lBufferToAllocOnOverflow) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_COPY_FAILED);


		return err;
	}


	return errNoError;
}


Error ConfigurationSection:: getCfgItemByName (const char *pItemName,
	ConfigurationItem_p pciCfgItem, long lBufferToAllocOnOverflow)

{

	long		lCfgItemIndex;


	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber;
		lCfgItemIndex++)
	{
		if (_pciCfgItems [lCfgItemIndex] == pItemName)
			break;
	}

	if (lCfgItemIndex < _lCfgItemsNumber)
	{
		if (ConfigurationItem:: copy (pciCfgItem,
			&(_pciCfgItems [lCfgItemIndex]), lBufferToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_COPY_FAILED);


			return err;
		}
	}
	else
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ITEM_NOTFOUND, 1, pItemName);

		return err;
	}


	return errNoError;
}


Error ConfigurationSection:: getPtrCfgItemByName (const char *pItemName,
	ConfigurationItem_p *pciCfgItem)

{

	long		lCfgItemIndex;


	if (_stCfgSectionStatus  != CFGSECTION_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgSectionStatus);

		return err;
	}

	for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber;
		lCfgItemIndex++)
	{
		if (_pciCfgItems [lCfgItemIndex] == pItemName)
			break;
	}

	if (lCfgItemIndex < _lCfgItemsNumber)
		*pciCfgItem		= &(_pciCfgItems [lCfgItemIndex]);
	else
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ITEM_NOTFOUND, 1, pItemName);

		return err;
	}


	return errNoError;
}


Error ConfigurationSection:: copy (ConfigurationSection *pcsDestCfgSection,
	ConfigurationSection *pcsSrcCfgSection, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	char					*pSectionName;
	long					lSectionNameLength;
	char					*pSectionComment;
	long					lSectionCommentLength;
	char					pSectionDate [CFG_SECTIONDATELENGTH];
	long					lCfgItemsNumber;
	long					lCfgItemIndex;
	ConfigurationItem_t		ciCfgItem;
	Error					errGetSectionDate;
	Boolean_t				bIsCaseSensitive;


	if (pcsSrcCfgSection -> _stCfgSectionStatus != CFGSECTION_INITIALIZED ||
		pcsDestCfgSection -> _stCfgSectionStatus != CFGSECTION_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) pcsSrcCfgSection -> _stCfgSectionStatus);

		return err;
	}

	if (pcsSrcCfgSection -> getIsCaseSensitive (&bIsCaseSensitive) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETISCASESENSITIVE_FAILED);

		return err;
	}

	if (pcsSrcCfgSection -> getSectionNameLength (&lSectionNameLength) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETSECTIONNAMELENGTH_FAILED);

		return err;
	}

	if (pcsSrcCfgSection -> getSectionCommentLength (&lSectionCommentLength) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETSECTIONCOMMENTLENGTH_FAILED);

		return err;
	}

	if ((pSectionName = new char [lSectionNameLength + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		return err;
	}

	if ((pSectionComment = new char [lSectionCommentLength + 1]) ==
		(char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] pSectionName;
		pSectionName		= (char *) NULL;

		return err;
	}

	if (pcsSrcCfgSection -> getSectionName (pSectionName) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETSECTIONNAME_FAILED);

		delete [] pSectionName;
		pSectionName		= (char *) NULL;
		delete [] pSectionComment;
		pSectionComment		= (char *) NULL;

		return err;
	}

	if (pcsSrcCfgSection -> getSectionComment (pSectionComment) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETSECTIONCOMMENT_FAILED);

		delete [] pSectionName;
		pSectionName		= (char *) NULL;
		delete [] pSectionComment;
		pSectionComment		= (char *) NULL;

		return err;
	}

	errGetSectionDate	= pcsSrcCfgSection -> getSectionDate (pSectionDate);
	if (errGetSectionDate == errNoError)
	{
		if (pcsDestCfgSection -> init (pSectionName, pSectionComment,
			pSectionDate, bIsCaseSensitive, lCfgItemsToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_INIT_FAILED);

			delete [] pSectionName;
			pSectionName		= (char *) NULL;
			delete [] pSectionComment;
			pSectionComment		= (char *) NULL;


			return err;
		}
	}
	else if (errGetSectionDate != errNoError &&
		(long) errGetSectionDate == CFG_CONFIGURATIONSECTIONDATE_NOTFOUND)
	{
		if (pcsDestCfgSection -> init (pSectionName, pSectionComment,
			(char *) NULL, bIsCaseSensitive, lCfgItemsToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_INIT_FAILED);

			delete [] pSectionName;
			pSectionName		= (char *) NULL;
			delete [] pSectionComment;
			pSectionComment		= (char *) NULL;


			return err;
		}
	}
	else
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETSECTIONDATE_FAILED);

		delete [] pSectionName;
		pSectionName		= (char *) NULL;
		delete [] pSectionComment;
		pSectionComment		= (char *) NULL;


		return err;
	}

	delete [] pSectionName;
	pSectionName		= (char *) NULL;
	delete [] pSectionComment;
	pSectionComment		= (char *) NULL;

	if (pcsSrcCfgSection -> getCfgItemsNumber (&lCfgItemsNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMSNUMBER_FAILED);

		pcsDestCfgSection -> finish ();


		return err;
	}

	for (lCfgItemIndex = 0; lCfgItemIndex < lCfgItemsNumber; lCfgItemIndex++)
	{
		if (pcsSrcCfgSection -> getCfgItemByIndex (lCfgItemIndex, &ciCfgItem,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);

			pcsDestCfgSection -> finish ();


			return err;
		}

		if (pcsDestCfgSection -> appendCfgItem (&ciCfgItem,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			ciCfgItem. finish ();
			pcsDestCfgSection -> finish ();


			return err;
		}

		if (ciCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);

			pcsDestCfgSection -> finish ();


			return err;
		}
	}


	return errNoError;
}


std:: ostream &operator << (std:: ostream &osOutputStream, ConfigurationSection &csSection)

{

	long		lCfgItemIndex;


	if (strcmp (csSection. _pSectionComment, ""))
	{
		char		*pLocalBuffer;
		char		*pSectionCommentCurrentNewLine;
		char		*pSectionCommentNextNewLine;


		if ((pLocalBuffer = new char [
			strlen (csSection. _pSectionComment) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			return osOutputStream;
		}

		pSectionCommentCurrentNewLine		= csSection. _pSectionComment;
		pSectionCommentNextNewLine			= csSection. _pSectionComment;

		while ((pSectionCommentNextNewLine =
			strchr (pSectionCommentCurrentNewLine, '\n')) != (char *) NULL)
		{
			memcpy (pLocalBuffer, pSectionCommentCurrentNewLine,
				pSectionCommentNextNewLine - pSectionCommentCurrentNewLine + 1);
			pLocalBuffer [pSectionCommentNextNewLine -
				pSectionCommentCurrentNewLine + 1]		= '\0';
			osOutputStream << "#" << pLocalBuffer;

			pSectionCommentCurrentNewLine	= pSectionCommentNextNewLine + 1;
		}

		osOutputStream << "#" << pSectionCommentCurrentNewLine << std:: endl;

		delete [] pLocalBuffer;
		pLocalBuffer			= (char *) NULL;
	}

	osOutputStream << "[";
	if (strchr (csSection. _pSectionName, ' ') != (char *) NULL ||
		strchr (csSection. _pSectionName, '\t') != (char *) NULL ||
		strchr (csSection. _pSectionName, '"') != (char *) NULL)
		osOutputStream << "\"" << csSection. _pSectionName << "\"";
	else
		osOutputStream << csSection. _pSectionName;
	osOutputStream << "]";

	if (csSection. _pSectionDate != (char *) NULL)
		osOutputStream << " (" << csSection. _pSectionDate << ")";

	osOutputStream << std:: endl;

	for (lCfgItemIndex = 0; lCfgItemIndex < csSection. _lCfgItemsNumber;
			lCfgItemIndex++)
		osOutputStream << csSection. _pciCfgItems [lCfgItemIndex] << std:: endl;


	return osOutputStream;
}


Error ConfigurationSection:: write (int iFileDescriptor)

{

	long		lCfgItemIndex;
	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif


	if (strcmp (_pSectionComment, ""))
	{
		char		*pLocalBuffer;
		char		*pSectionCommentCurrentNewLine;
		char		*pSectionCommentNextNewLine;


		if ((pLocalBuffer = new char [
			strlen (_pSectionComment) + 1]) == (char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			return err;
		}

		pSectionCommentCurrentNewLine		= _pSectionComment;
		pSectionCommentNextNewLine			= _pSectionComment;

		while ((pSectionCommentNextNewLine =
			strchr (pSectionCommentCurrentNewLine, '\n')) != (char *) NULL)
		{
			memcpy (pLocalBuffer, pSectionCommentCurrentNewLine,
				pSectionCommentNextNewLine - pSectionCommentCurrentNewLine);
			pLocalBuffer [pSectionCommentNextNewLine -
				pSectionCommentCurrentNewLine]		= '\0';

			if (FileIO:: writeChars (iFileDescriptor,
				"#", 1, &llBytesWritten) != errNoError)
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

			pSectionCommentCurrentNewLine	= pSectionCommentNextNewLine + 1;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			"#", 1, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			delete [] pLocalBuffer;
			pLocalBuffer			= (char *) NULL;

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			pSectionCommentCurrentNewLine,
			strlen (pSectionCommentCurrentNewLine),
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
		"[", 1, &llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		return err;
	}
	if (strchr (_pSectionName, ' ') != (char *) NULL ||
		strchr (_pSectionName, '\t') != (char *) NULL ||
		strchr (_pSectionName, '"') != (char *) NULL)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			"\"", 1, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			_pSectionName, strlen (_pSectionName),
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
			_pSectionName, strlen (_pSectionName),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}
	}

	if (FileIO:: writeChars (iFileDescriptor,
		"]", 1, &llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		return err;
	}

	if (_pSectionDate != (char *) NULL)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			" (", 2, &llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			_pSectionDate, strlen (_pSectionDate),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			return err;
		}

		if (FileIO:: writeChars (iFileDescriptor,
			")", 1, &llBytesWritten) != errNoError)
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

	for (lCfgItemIndex = 0; lCfgItemIndex < _lCfgItemsNumber;
			lCfgItemIndex++)
	{
		if ((_pciCfgItems [lCfgItemIndex]). write (iFileDescriptor) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_WRITE_FAILED);

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


	return errNoError;
}


Boolean_t ConfigurationSection:: isValidDate (const char *pSectionDate)

{

	long		lYear;
	long		lMonth;
	long		lDay;


	if (pSectionDate == (char *) NULL ||
		strlen (pSectionDate) != CFG_SECTIONDATELENGTH - 1)
		return false;

	if (sscanf (pSectionDate, "%4ld/%2ld/%2ld",
		&lYear, &lMonth, &lDay) != 3)
		return false;

	if (lYear < 1900 || lMonth < 1 || lMonth > 12 ||
		lDay < 1 || lDay > 31)
		return false;


	return true;
}

