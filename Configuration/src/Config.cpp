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


#include "Config.h"
#include "Encrypt.h"
#include <assert.h>



Config:: Config (void)

{

	_stCfgStatus			= CFG_BUILDED;

}


Config:: ~Config (void)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
		finish ();
}



Config:: Config (const Config &)

{

	assert (1==0);

	// to do

}


Config &Config:: operator = (const Config &)

{

	assert (1==0);

	// to do

	return *this;

}


Error Config:: init (const char *pConfigName,
	Boolean_t bIsCaseSensitive,
	long lCfgSectionsToAllocOnOverflow)

{

	if (_stCfgStatus != CFG_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (pConfigName == (char *) NULL ||
		strlen (pConfigName) > CFG_MAXCONFIGNAMELENGTH - 1 ||
		lCfgSectionsToAllocOnOverflow < 1)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	_bIsCaseSensitive					= bIsCaseSensitive;
	_lCfgSectionsToAllocOnOverflow		= lCfgSectionsToAllocOnOverflow;
	_lCfgSectionsNumber					= 0;
	_lAllocatedCfgSectionsNumber		= _lCfgSectionsToAllocOnOverflow;

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (_configurationMutex. init (PMutex:: MUTEX_FAST) != errNoError)
		#else	// POSIX
			/*
			#if defined(__sparc)	 // SunOs (one of the SunO first version)
				if (_configurationMutex. init (
					PMutex:: MUTEX_PROCESS_PRIVATE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
			*/
			#if defined(__CYGWIN__)
				if (_configurationMutex. init (PMutex:: MUTEX_RECURSIVE) !=
					errNoError)
			#else							// POSIX.1-1996 standard (HPUX 11)
				if (_configurationMutex. init (PMutex:: MUTEX_FAST) !=
					errNoError)
			#endif
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			return err;
		}
	#endif

	if ((_pConfigName = new char [strlen (pConfigName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pConfigName, pConfigName);

	if ((_pcsCfgSections = new ConfigurationSection_t [
		_lAllocatedCfgSectionsNumber]) == (ConfigurationSection_p) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		delete [] (_pConfigName);
		_pConfigName		= (char *) NULL;
		#ifdef _REENTRANT
			if (_configurationMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif


		return err;
	}

	_stCfgStatus		= CFG_INITIALIZED;


	return errNoError;
}


Error Config:: finish (void)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
		_pcsCfgSections [lCfgSectionIndex]. finish ();
	delete [] (_pcsCfgSections);
	_pcsCfgSections		= (ConfigurationSection_p) NULL;

	delete [] (_pConfigName);
	_pConfigName				= (char *) NULL;

	#ifdef _REENTRANT
		if (_configurationMutex. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	#endif

	_stCfgStatus		= CFG_BUILDED;


	return errNoError;
}


Error Config:: appendCfgSection (
	const ConfigurationSection_p pcsCfgSection,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	if (_stCfgStatus != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (pcsCfgSection == (const ConfigurationSection_p) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_lCfgSectionsNumber == _lAllocatedCfgSectionsNumber)
	{
		ConfigurationSection_p		pcsCfgSections;
		long						lCfgSectionIndex;


		pcsCfgSections					= _pcsCfgSections;

		_lAllocatedCfgSectionsNumber	+= _lCfgSectionsToAllocOnOverflow;

		if ((_pcsCfgSections = new ConfigurationSection_t [
			_lAllocatedCfgSectionsNumber]) == (ConfigurationSection_p) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			_pcsCfgSections					= pcsCfgSections;
			_lAllocatedCfgSectionsNumber	-= _lCfgSectionsToAllocOnOverflow;
			#ifdef _REENTRANT
				if (_configurationMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif


			return err;
		}

		for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
			lCfgSectionIndex++)
		{
			if (ConfigurationSection:: copy (
				&(_pcsCfgSections [lCfgSectionIndex]),
				&(pcsCfgSections [lCfgSectionIndex]),
				lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) !=
				errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_COPY_FAILED);

				while (--lCfgSectionIndex >= 0)
					_pcsCfgSections [lCfgSectionIndex]. finish ();
				delete [] (_pcsCfgSections);
				_pcsCfgSections					= (ConfigurationSection_p) NULL;

				_pcsCfgSections					= pcsCfgSections;
				_lAllocatedCfgSectionsNumber -= _lCfgSectionsToAllocOnOverflow;

				#ifdef _REENTRANT
					if (_configurationMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif


				return err;
			}
		}

		for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
			lCfgSectionIndex++)
		{
			if (pcsCfgSections [lCfgSectionIndex]. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);

				#ifdef _REENTRANT
					if (_configurationMutex. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif


				return err;
			}
		}

		delete [] (pcsCfgSections);
		pcsCfgSections				= (ConfigurationSection_p) NULL;
	}

	if (ConfigurationSection:: copy (&(_pcsCfgSections [_lCfgSectionsNumber]),
		pcsCfgSection, lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_COPY_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	_lCfgSectionsNumber++;

	#ifdef _REENTRANT
		if (_configurationMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error Config:: removeCfgSection (
	const char *pSectionName,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	long					lCfgSectionIndex;


	if (_stCfgStatus != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (pSectionName == (const char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND,
			1, pSectionName);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);

				return err;
			}
		#endif

		return err;
	}

	if ((_pcsCfgSections [lCfgSectionIndex]). finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	if (ConfigurationSection:: copy (&(_pcsCfgSections [lCfgSectionIndex]),
		&(_pcsCfgSections [_lCfgSectionsNumber - 1]),
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_COPY_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	if ((_pcsCfgSections [_lCfgSectionsNumber - 1]). finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	_lCfgSectionsNumber--;

	#ifdef _REENTRANT
		if (_configurationMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error Config:: appendCfgItem (const char *pSectionName,
	const ConfigurationItem_p pciCfgItem, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		ConfigurationSection_t			csSection;

		if (csSection. init (pSectionName, "", (char *) NULL,
			_bIsCaseSensitive, lCfgItemsToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_INIT_FAILED);

			return err;
		}

		if (csSection. appendCfgItem (pciCfgItem, lBufferToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (appendCfgSection (&csSection, lCfgItemsToAllocOnOverflow,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (csSection. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);

			return err;
		}
	}
	else
	{
		// section found
		if (_pcsCfgSections [lCfgSectionIndex]. appendCfgItem (pciCfgItem,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error Config:: appendItemValue (const char *pSectionName,
	const char *pItemName, const char *pValue, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		ConfigurationSection_t			csSection;
		ConfigurationItem_t				ciCfgItem;

		if (csSection. init (pSectionName, "", (char *) NULL,
			_bIsCaseSensitive, lCfgItemsToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_INIT_FAILED);

			return err;
		}

		if (ciCfgItem. init (pItemName, "", _bIsCaseSensitive,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_INIT_FAILED);

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (ciCfgItem. appendItemValue (pValue) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);

			if (ciCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
			}

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (csSection. appendCfgItem (&ciCfgItem, lBufferToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			if (ciCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
			}

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (appendCfgSection (&csSection, lCfgItemsToAllocOnOverflow,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

			if (ciCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
			}

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (ciCfgItem. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONITEM_FINISH_FAILED);

			if (csSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			}

			return err;
		}

		if (csSection. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);

			return err;
		}
	}
	else
	{
		// section found
		ConfigurationItem_p				pciCfgItem;
		Error							errGetPtrCfgItemByName;
		long							lErrorIdentifier;


		if ((errGetPtrCfgItemByName = _pcsCfgSections [
			lCfgSectionIndex]. getPtrCfgItemByName (
			pItemName, &pciCfgItem)) != errNoError)
		{
			errGetPtrCfgItemByName. getIdentifier (&lErrorIdentifier);

			if (lErrorIdentifier != CFG_ITEM_NOTFOUND)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_GETPTRCFGITEMBYNAME_FAILED);

				return err;
			}
			else
			{
				// item not found
				ConfigurationItem_t				ciCfgItem;

				if (ciCfgItem. init (pItemName, "", _bIsCaseSensitive,
					lBufferToAllocOnOverflow) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_INIT_FAILED);

					return err;
				}

				if (ciCfgItem. appendItemValue (pValue) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);

					if (ciCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
					}

					return err;
				}

				if (_pcsCfgSections [lCfgSectionIndex]. appendCfgItem (
					&ciCfgItem, lBufferToAllocOnOverflow) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);

					if (ciCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
					}

					return err;
				}

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);

					return err;
				}
			}
		}
		else
		{
			// item found
			if (pciCfgItem -> appendItemValue (pValue) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error Config:: getIsCaseSensitive (Boolean_p pbIsCaseSensitive)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	*pbIsCaseSensitive		= _bIsCaseSensitive;


	return errNoError;
}


Error Config:: getConfigName (char *pConfigName)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	strcpy (pConfigName, _pConfigName);


	return errNoError;
}


Error Config:: modifyConfigName (const char *pConfigName)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (pConfigName == (char *) NULL ||
		strlen (pConfigName) > CFG_MAXCONFIGNAMELENGTH - 1)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_configurationMutex. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	delete [] (_pConfigName);
	_pConfigName		= (char *) NULL;

	if ((_pConfigName = new char [strlen (pConfigName) + 1]) ==
		(char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		#ifdef _REENTRANT
			if (_configurationMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif


		return err;
	}

	strcpy (_pConfigName, pConfigName);

	#ifdef _REENTRANT
		if (_configurationMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error Config:: getSectionsNumber (long *plCfgSectionsNumber)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	*plCfgSectionsNumber		= _lCfgSectionsNumber;


	return errNoError;
}


Error Config:: getCfgSectionByIndex (long lCfgSectionIndex,
	ConfigurationSection_p pcsCfgSection, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (lCfgSectionIndex >= _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	if (ConfigurationSection:: copy (pcsCfgSection,
		&(_pcsCfgSections [lCfgSectionIndex]), lCfgItemsToAllocOnOverflow,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_COPY_FAILED);


		return err;
	}


	return errNoError;
}


Error Config:: getCfgSectionByName (const char *pSectionName,
	ConfigurationSection_p pcsCfgSection, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	if (lCfgSectionIndex < _lCfgSectionsNumber)
	{
		if (ConfigurationSection:: copy (pcsCfgSection,
			&(_pcsCfgSections [lCfgSectionIndex]),
			lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_COPY_FAILED);


			return err;
		}
	}
	else
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND, 1, pSectionName);

		return err;
	}


	return errNoError;
}


Error Config:: getItemsNumber (const char *pSectionName,
	long *plItemsNumber, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByName (pSectionName, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemsNumber (plItemsNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMSNUMBER_FAILED);

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemsNumber (long lSectionIndex,
	long *plItemsNumber,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByIndex (lSectionIndex, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemsNumber (plItemsNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMSNUMBER_FAILED);

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getCfgItemByIndex (const char *pSectionName,
	long lItemIndex, ConfigurationItem_p pciCfgItem,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByName (pSectionName, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByIndex (lItemIndex, pciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getCfgItemByIndex (long lSectionIndex, long lItemIndex,
	ConfigurationItem_p pciCfgItem, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByIndex (lSectionIndex, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByIndex (lItemIndex, pciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getCfgItemByName (const char *pSectionName,
	const char *pItemName, ConfigurationItem_p pciCfgItem,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;
	Error_t							errGeneric;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if ((errGeneric = getCfgSectionByName (pSectionName, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow)) != errNoError)
	{
		// Error err = ConfigurationErrors (__FILE__, __LINE__,
		// 	CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);

		// return err;
		return errGeneric;
	}

	if ((errGeneric = csCfgSection. getCfgItemByName (pItemName, pciCfgItem,
		lBufferToAllocOnOverflow)) != errNoError)
	{
		// Error err = ConfigurationErrors (__FILE__, __LINE__,
		// 	CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED);
		csCfgSection. finish ();

		// return err;
		return errGeneric;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getCfgItemByName (long lSectionIndex,
	const char *pItemName, ConfigurationItem_p pciCfgItem,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByIndex (lSectionIndex, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByName (pItemName, pciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemValuesNumber (const char *pSectionName,
	const char *pItemName, long *plItemValuesNumber,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;
	ConfigurationItem_t				ciCfgItem;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByName (pSectionName, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByName (pItemName, &ciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. getItemValuesNumber (plItemValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
		ciCfgItem. finish ();
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemValuesNumber (long lSectionIndex,
	const char *pItemName, long *plItemValuesNumber,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;
	ConfigurationItem_t				ciCfgItem;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByIndex (lSectionIndex, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByName (pItemName, &ciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYNAME_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. getItemValuesNumber (plItemValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
		ciCfgItem. finish ();
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemValuesNumber (const char *pSectionName,
	long lItemIndex, long *plItemValuesNumber,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;
	ConfigurationItem_t				ciCfgItem;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByName (pSectionName, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByIndex (lItemIndex, &ciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. getItemValuesNumber (plItemValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
		ciCfgItem. finish ();
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemValuesNumber (long lSectionIndex,
	long lItemIndex, long *plItemValuesNumber,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationSection_t			csCfgSection;
	ConfigurationItem_t				ciCfgItem;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (getCfgSectionByIndex (lSectionIndex, &csCfgSection,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

		return err;
	}

	if (csCfgSection. getCfgItemByIndex (lItemIndex, &ciCfgItem,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. getItemValuesNumber (plItemValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
		ciCfgItem. finish ();
		csCfgSection. finish ();

		return err;
	}

	if (ciCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		csCfgSection. finish ();

		return err;
	}

	if (csCfgSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: getItemValue (const char *pSectionName,
	const char *pItemName, char *pItemValue,
	unsigned long ulMaxItemValueLength, long lItemValueIndex,
	SecurityLevel_t slSecurityLevel,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	ConfigurationItem_t				ciCfgItem;
	Error_t							errGeneric;
	Error_t							errGetItemValue;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if ((errGeneric = getCfgItemByName (pSectionName, pItemName, &ciCfgItem,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow)) != errNoError)
	{
		// Error err = ConfigurationErrors (__FILE__, __LINE__,
		// 	CFG_CONFIG_GETCFGITEMBYNAME_FAILED);

		// return err;
		return errGeneric;
	}

	if ((errGetItemValue = ciCfgItem. getItemValue (
		pItemValue, ulMaxItemValueLength,
		lItemValueIndex)) != errNoError)
	{
		// Error err = ConfigurationErrors (__FILE__, __LINE__,
		// 	CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);
		ciCfgItem. finish ();

		// return err;
		return errGetItemValue;
	}

	if (ciCfgItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);

		return err;
	}

	if (slSecurityLevel == CFG_ENCRIPTION &&
		strcmp (pItemValue, ""))
	{
		long			lDecryptedBufferLength;
		char			*pDecryptedBuffer;

		lDecryptedBufferLength		=
			Encrypt:: getDecryptedBufferLength (pItemValue);

		if ((pDecryptedBuffer = new char [lDecryptedBufferLength + 1]) ==
			(char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);

			return err;
		}

		if (Encrypt:: decrypt (pItemValue, pDecryptedBuffer,
			lDecryptedBufferLength + 1) != 0)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_DECRYPT_FAILED);

			delete [] pDecryptedBuffer;

			return err;
		}

		strcpy (pItemValue, pDecryptedBuffer);

		delete [] pDecryptedBuffer;
	}


	return errNoError;
}


Error Config:: modifySectionComment (const char *pSectionName,
	const char *pSectionComment)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND, 1, pSectionName);

		return err;
	}
	else
	{
		// section found
		if (_pcsCfgSections [lCfgSectionIndex]. modifySectionComment (
			pSectionComment) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_MODIFYSECTIONCOMMENT_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error Config:: modifySectionDate (const char *pSectionName,
	const char *pSectionDate)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND, 1, pSectionName);

		return err;
	}
	else
	{
		// section found
		if (_pcsCfgSections [lCfgSectionIndex]. modifySectionDate (
			pSectionDate) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_MODIFYSECTIONDATE_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error Config:: modifyItemComment (const char *pSectionName,
	const char *pItemName, const char *pItemComment)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND, 1, pSectionName);

		return err;
	}
	else
	{
		// section found
		ConfigurationItem_p				pciCfgItem;


		if (_pcsCfgSections [lCfgSectionIndex]. getPtrCfgItemByName (
			pItemName, &pciCfgItem) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_GETPTRCFGITEMBYNAME_FAILED);

			return err;
		}
		else
		{
			// item found
			if (pciCfgItem -> modifyItemComment (pItemComment) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_MODIFYITEMCOMMENT_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error Config:: modifyItemValue (const char *pSectionName,
	const char *pItemName, const char *pNewItemValue,
	long lItemValueIndex, SecurityLevel_t slSecurityLevel)

{

	long		lCfgSectionIndex;


	if (_stCfgStatus  != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (_pcsCfgSections [lCfgSectionIndex] == pSectionName)
			break;
	}

	// section not found
	if (lCfgSectionIndex == _lCfgSectionsNumber)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_SECTION_NOTFOUND, 1, pSectionName);

		return err;
	}
	else
	{
		// section found
		ConfigurationItem_p				pciCfgItem;


		if (_pcsCfgSections [lCfgSectionIndex]. getPtrCfgItemByName (
			pItemName, &pciCfgItem) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_GETPTRCFGITEMBYNAME_FAILED);

			return err;
		}
		else
		{
			// item found
			if (slSecurityLevel == CFG_ENCRIPTION)
			{
				long			lBufferCryptedLength;
				char			*pBufferCrypted;

				lBufferCryptedLength	=
					Encrypt:: getCryptedBufferLength (pNewItemValue);

				if ((pBufferCrypted = new char [lBufferCryptedLength + 1]) ==
					(char *) NULL)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_NEW_FAILED);

					return err;
				}

				if (Encrypt:: encrypt (pNewItemValue, pBufferCrypted,
					lBufferCryptedLength + 1) != 0)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_ENCRYPT_FAILED);

					delete [] pBufferCrypted;

					return err;
				}

				if (pciCfgItem -> modifyItemValue (pBufferCrypted,
					lItemValueIndex) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_MODIFYITEMVALUE_FAILED);

					delete [] pBufferCrypted;

					return err;
				}

				delete [] pBufferCrypted;
			}
			else
			{
				if (pciCfgItem -> modifyItemValue (pNewItemValue,
					lItemValueIndex) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_MODIFYITEMVALUE_FAILED);

					return err;
				}
			}
		}
	}


	return errNoError;
}


Error Config:: copy (Config *pcfgConfig,
	long lCfgSectionsToAllocOnOverflow, long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{

	if (_stCfgStatus != CFG_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgStatus);

		return err;
	}

	if (finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_FINISH_FAILED);

		return err;
	}

	if (Config:: copy (this, pcfgConfig,
		lCfgSectionsToAllocOnOverflow, lCfgItemsToAllocOnOverflow,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_COPY_FAILED);

		return err;
	}


	return errNoError;
}


Error Config:: copy (Config *pcfgDestConfig,
	Config *pcfgSrcConfig, long lCfgSectionsToAllocOnOverflow,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	char					pConfigName [CFG_MAXCONFIGNAMELENGTH];
	long					lCfgSectionsNumber;
	long					lCfgSectionIndex;
	ConfigurationSection_t	csCfgSection;
	Boolean_t				bIsCaseSensitive;


	if (pcfgSrcConfig -> _stCfgStatus != CFG_INITIALIZED ||
		pcfgDestConfig -> _stCfgStatus != CFG_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) pcfgSrcConfig -> _stCfgStatus);

		return err;
	}

	if (pcfgSrcConfig -> getIsCaseSensitive (&bIsCaseSensitive) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETISCASESENSITIVE_FAILED);

		return err;
	}

	if (pcfgSrcConfig -> getConfigName (pConfigName) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCONFIGNAME_FAILED);

		return err;
	}

	if (pcfgDestConfig -> init (pConfigName,
		bIsCaseSensitive, lCfgSectionsToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_INIT_FAILED);

		return err;
	}

	if (pcfgSrcConfig -> getSectionsNumber (&lCfgSectionsNumber) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETSECTIONSNUMBER_FAILED);

		pcfgDestConfig -> finish ();


		return err;
	}

	for (lCfgSectionIndex = 0; lCfgSectionIndex < lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if (pcfgSrcConfig -> getCfgSectionByIndex (lCfgSectionIndex,
			&csCfgSection, lCfgItemsToAllocOnOverflow,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETCFGSECTIONBYINDEX_FAILED);

			pcfgDestConfig -> finish ();


			return err;
		}

		if (pcfgDestConfig -> appendCfgSection (&csCfgSection,
			lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_APPENDCFGSECTION_FAILED);

			csCfgSection. finish ();
			pcfgDestConfig -> finish ();


			return err;
		}

		if (csCfgSection. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);

			pcfgDestConfig -> finish ();


			return err;
		}
	}


	return errNoError;
}


std:: ostream &operator << (std:: ostream &osOutputStream, Config &cfgConfig)

{

	long		lCfgSectionIndex;


	for (lCfgSectionIndex = 0;
		lCfgSectionIndex < cfgConfig. _lCfgSectionsNumber;
		lCfgSectionIndex++)
		osOutputStream << cfgConfig. _pcsCfgSections [lCfgSectionIndex];


	return osOutputStream;
}


Error Config:: write (int iFileDescriptor)

{

	long		lCfgSectionIndex;


	for (lCfgSectionIndex = 0;
		lCfgSectionIndex < _lCfgSectionsNumber;
		lCfgSectionIndex++)
	{
		if ((_pcsCfgSections [lCfgSectionIndex]). write (iFileDescriptor) !=
			errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_WRITE_FAILED);

			return err;
		}
	}


	return errNoError;
}


