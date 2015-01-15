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


#include "LoadBalancer.h"
#include "DateTime.h"
#include "HttpGetThread.h"
#include <stdlib.h>



#ifdef WIN32
	LoadBalancer:: LoadBalancer (void): WinThread ()
#else
	LoadBalancer:: LoadBalancer (void): PosixThread ()
#endif

{

}


LoadBalancer:: ~LoadBalancer (void)

{

}


Error LoadBalancer:: init (
	ConfigurationFile_p pcfConfiguration,
	const char *pLoadBalancerSectionName,
	Tracer_p ptSystemTracer)

{

	char					pConfigurationBuffer [
		LB_LOADBALANCER_MAXITEMLENGTH];
	Error_t					errGetItemValue;
	long					lItemValuesNumber;
	long					lGroupIndex;
	std:: vector<DeliveryModule_p>:: const_iterator		itDeliveryModule;
	DeliveryModule_p		pdmDeliveryModule;



	if (pcfConfiguration == (ConfigurationFile_p) NULL ||
		ptSystemTracer == (Tracer_p) NULL)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_ACTIVATION_WRONG);

		return err;
	}

	_ptSystemTracer			= ptSystemTracer;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (
		pLoadBalancerSectionName, "Enabled", pConfigurationBuffer,
		LB_LOADBALANCER_MAXITEMLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pLoadBalancerSectionName, "Enabled");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);


		return errGetItemValue;
	}
	#ifdef WIN32
		if (!stricmp (pConfigurationBuffer, "true"))
	#else
		if (!strcasecmp (pConfigurationBuffer, "true"))
	#endif
		_bLoadBalancerEnabled			= true;
	else
		_bLoadBalancerEnabled			= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (
		pLoadBalancerSectionName, "PingSocketTimeoutInSeconds",
		pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pLoadBalancerSectionName, "PingSocketTimeoutInSeconds");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);


		return errGetItemValue;
	}
	_ulPingSocketTimeoutInSeconds			=
		strtoul (pConfigurationBuffer, (char **) NULL, 10);

	if ((errGetItemValue = pcfConfiguration -> getItemValue (
		pLoadBalancerSectionName, "LocalIPAddress", pConfigurationBuffer,
		LB_LOADBALANCER_MAXITEMLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pLoadBalancerSectionName, "LocalIPAddress");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errGetItemValue;
	}

	if (strlen (pConfigurationBuffer) >= SCK_MAXIPADDRESSLENGTH)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_LOADBALANCER_CONFIGITEMTOOLONG,
			4, pLoadBalancerSectionName, "LocalIPAddress",
			pConfigurationBuffer, (unsigned long) SCK_MAXIPADDRESSLENGTH);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	strcpy (_pLoadBalancerLocalIPAddress, pConfigurationBuffer);

	if ((errGetItemValue = pcfConfiguration -> getItemValue (
		pLoadBalancerSectionName, "CheckLoadBalancerModulesPeriodInMilliSecs",
		pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pLoadBalancerSectionName,
			"CheckLoadBalancerModulesPeriodInMilliSecs");
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return errGetItemValue;
	}
	_ulCheckLoadBalancerModulesPeriodInMilliSecs    =
		strtoul (pConfigurationBuffer, (char **) NULL, 10);

	// look for how many group of Modules we have to load balancing
	// and initialize pvModulesGroups
	{
		ConfigurationSection_t			csSrcCfgSection;
		ConfigurationItem_t				ciCfgItem;
		long							lItemsNumber;
		long							lItemIndex;
		long							ulModuleIndex;
		Boolean_t						bModulesFound;
		char							pItemName [
			LB_LOADBALANCER_MAXITEMLENGTH];
		Error_t							errGetCfgItem;
		char							pPrivateHost [SCK_MAXHOSTNAMELENGTH];
		unsigned long					ulPrivatePort;
		char							pPublicHost [SCK_MAXHOSTNAMELENGTH];
		unsigned long					ulPublicPort;
		Buffer_t						bURLToGetCurrentRequestsNumber;
		unsigned long					ulMaxRequestsNumber;


		_ulModulesGroupsNumber			= 0;

		if (pcfConfiguration -> getCfgSectionByName (
			pLoadBalancerSectionName, &csSrcCfgSection) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETCFGSECTIONBYNAME_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (csSrcCfgSection. getCfgItemsNumber (&lItemsNumber) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_GETCFGITEMSNUMBER_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (csSrcCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		for (lItemIndex = 0; lItemIndex < lItemsNumber; lItemIndex++)
		{
			if (csSrcCfgSection. getCfgItemByIndex (
				lItemIndex, &ciCfgItem) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			if (ciCfgItem. getItemName (pItemName) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_GETITEMNAME_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			if (!strncmp (pItemName, "Modules_", strlen ("Modules_")))
				_ulModulesGroupsNumber++;

			if (ciCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}
		}

		if (_ulModulesGroupsNumber == 0)
		{
			if (csSrcCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			return errNoError;
		}

		if ((_pgmGroupsModules = new GroupModules_t [_ulModulesGroupsNumber]) ==
			(GroupModules_p) NULL)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (csSrcCfgSection. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONSECTION_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		_vModulesForAllGroups. clear ();

		_vModulesGroups. clear ();
		_vModulesGroups. reserve (_ulModulesGroupsNumber);

		for (lGroupIndex = 0, lItemIndex = 0;
			lGroupIndex < (long) _ulModulesGroupsNumber;
			lGroupIndex++)
		{
			bModulesFound			= false;

			for (; lItemIndex < lItemsNumber; lItemIndex++)
			{
				if ((errGetCfgItem = csSrcCfgSection. getCfgItemByIndex (
					lItemIndex, &ciCfgItem)) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_GETCFGITEMBYINDEX_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (csSrcCfgSection. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONSECTION_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					while (--lGroupIndex >= 0)
					{
						if ((_pgmGroupsModules [lGroupIndex]).
							_ulModulesNumber == 0)
							continue;

						if ((_pgmGroupsModules [lGroupIndex]).
							_mtLastModuleSelected. finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
					}

					for (itDeliveryModule = _vModulesForAllGroups. begin ();
						itDeliveryModule != _vModulesForAllGroups. end ();
						++itDeliveryModule)
					{
						pdmDeliveryModule		=
							(DeliveryModule_p) (*itDeliveryModule);

						if ((pdmDeliveryModule -> _mtSocketStatus).
							finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						delete pdmDeliveryModule;
					}
					_vModulesForAllGroups. clear ();

					delete [] _pgmGroupsModules;
					_vModulesGroups. clear ();

					return errGetCfgItem;
				}

				if (ciCfgItem. getItemName (pItemName) != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_GETITEMNAME_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (ciCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (csSrcCfgSection. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONSECTION_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					while (--lGroupIndex >= 0)
					{
						if ((_pgmGroupsModules [lGroupIndex]).
							_ulModulesNumber == 0)
							continue;

						if ((_pgmGroupsModules [lGroupIndex]).
							_mtLastModuleSelected. finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
					}

					for (itDeliveryModule = _vModulesForAllGroups. begin ();
						itDeliveryModule != _vModulesForAllGroups. end ();
						++itDeliveryModule)
					{
						pdmDeliveryModule		=
							(DeliveryModule_p) (*itDeliveryModule);

						if ((pdmDeliveryModule -> _mtSocketStatus).
							finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						delete pdmDeliveryModule;
					}
					_vModulesForAllGroups. clear ();

					delete [] _pgmGroupsModules;
					_vModulesGroups. clear ();

					return err;
				}

				if (!strncmp (pItemName, "Modules_", strlen ("Modules_")))
				{
					bModulesFound			= true;

					lItemIndex++;

					break;
				}

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (csSrcCfgSection. finish () != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONSECTION_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					while (--lGroupIndex >= 0)
					{
						if ((_pgmGroupsModules [lGroupIndex]).
							_ulModulesNumber == 0)
							continue;

						if ((_pgmGroupsModules [lGroupIndex]).
							_mtLastModuleSelected. finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
					}

					for (itDeliveryModule = _vModulesForAllGroups. begin ();
						itDeliveryModule != _vModulesForAllGroups. end ();
						++itDeliveryModule)
					{
						pdmDeliveryModule		=
							(DeliveryModule_p) (*itDeliveryModule);

						if ((pdmDeliveryModule -> _mtSocketStatus).
							finish () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						delete pdmDeliveryModule;
					}
					_vModulesForAllGroups. clear ();

					delete [] _pgmGroupsModules;
					_vModulesGroups. clear ();


					return err;
				}
			}

			if (!bModulesFound)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_MODULESNUMBERNOTCONSISTENT);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				while (--lGroupIndex >= 0)
				{
					if ((_pgmGroupsModules [lGroupIndex]).
						_ulModulesNumber == 0)
						continue;

					if ((_pgmGroupsModules [lGroupIndex]).
						_mtLastModuleSelected. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
				}

				for (itDeliveryModule = _vModulesForAllGroups. begin ();
					itDeliveryModule != _vModulesForAllGroups. end ();
					++itDeliveryModule)
				{
					pdmDeliveryModule		=
						(DeliveryModule_p) (*itDeliveryModule);

					if ((pdmDeliveryModule -> _mtSocketStatus).
						finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pdmDeliveryModule;
				}
				_vModulesForAllGroups. clear ();

				delete [] _pgmGroupsModules;
				_vModulesGroups. clear ();


				return err;
			}

			strcpy ((_pgmGroupsModules [lGroupIndex]). _pModulesName,
				pItemName + strlen ("Modules_"));

			if (ciCfgItem. getItemValuesNumber (&lItemValuesNumber) !=
				errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_GETITEMVALUESNUMBER_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				while (--lGroupIndex >= 0)
				{
					if ((_pgmGroupsModules [lGroupIndex]).
						_ulModulesNumber == 0)
						continue;

					if ((_pgmGroupsModules [lGroupIndex]).
						_mtLastModuleSelected. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
				}

				for (itDeliveryModule = _vModulesForAllGroups. begin ();
					itDeliveryModule != _vModulesForAllGroups. end ();
					++itDeliveryModule)
				{
					pdmDeliveryModule		=
						(DeliveryModule_p) (*itDeliveryModule);

					if ((pdmDeliveryModule -> _mtSocketStatus).
						finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pdmDeliveryModule;
				}
				_vModulesForAllGroups. clear ();

				delete [] _pgmGroupsModules;
				_vModulesGroups. clear ();

				return err;
			}

			if (lItemValuesNumber % 6 != 0)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CONFIGITEMSVALUESWRONG,
					3, pLoadBalancerSectionName, pItemName,
					(unsigned long) lItemValuesNumber);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (ciCfgItem. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONITEM_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				while (--lGroupIndex >= 0)
				{
					if ((_pgmGroupsModules [lGroupIndex]).
						_ulModulesNumber == 0)
						continue;

					if ((_pgmGroupsModules [lGroupIndex]).
						_mtLastModuleSelected. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
				}

				for (itDeliveryModule = _vModulesForAllGroups. begin ();
					itDeliveryModule != _vModulesForAllGroups. end ();
					++itDeliveryModule)
				{
					pdmDeliveryModule		=
						(DeliveryModule_p) (*itDeliveryModule);

					if ((pdmDeliveryModule -> _mtSocketStatus).
						finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pdmDeliveryModule;
				}
				_vModulesForAllGroups. clear ();

				delete [] _pgmGroupsModules;
				_vModulesGroups. clear ();

				return err;
			}

			// PrivateHost, PrivatePort, PublicHost, PublicPort,
			// URIToGetCurrentRequestsNumber, MaxRequestsNumber
			(_pgmGroupsModules [lGroupIndex]). _ulModulesNumber		=
				lItemValuesNumber / 6;

			if ((_pgmGroupsModules [lGroupIndex]). _ulModulesNumber != 0)
			{
				(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
				(_pgmGroupsModules [lGroupIndex]). _vModules. reserve (
					(_pgmGroupsModules [lGroupIndex]). _ulModulesNumber);

				for (ulModuleIndex = 0; ulModuleIndex <
					(_pgmGroupsModules [lGroupIndex]). _ulModulesNumber;
					ulModuleIndex++)
				{
					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						ulModuleIndex * 6) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					if (strlen (pConfigurationBuffer) >= SCK_MAXHOSTNAMELENGTH)
					{
						Error err = LoadBalancerErrors (__FILE__, __LINE__,
							LB_LOADBALANCER_CONFIGITEMTOOLONG,
							4, pLoadBalancerSectionName, "<ItemName>",
							pConfigurationBuffer,
							(unsigned long) SCK_MAXHOSTNAMELENGTH);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					strcpy (pPrivateHost, pConfigurationBuffer);

					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						(ulModuleIndex * 6) + 1) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					ulPrivatePort		=
						strtoul (pConfigurationBuffer, (char **) NULL, 10);

					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						(ulModuleIndex * 6) + 2) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					if (strlen (pConfigurationBuffer) >= SCK_MAXHOSTNAMELENGTH)
					{
						Error err = LoadBalancerErrors (__FILE__, __LINE__,
							LB_LOADBALANCER_CONFIGITEMTOOLONG,
							4, pLoadBalancerSectionName, "<ItemName>",
							pConfigurationBuffer,
							(unsigned long) SCK_MAXHOSTNAMELENGTH);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					strcpy (pPublicHost, pConfigurationBuffer);

					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						(ulModuleIndex * 6) + 3) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					ulPublicPort		=
						strtoul (pConfigurationBuffer, (char **) NULL, 10);

					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						(ulModuleIndex * 6) + 4) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					if (bURLToGetCurrentRequestsNumber. setBuffer (
						pConfigurationBuffer) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_SETBUFFER_FAILED);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					if (ciCfgItem. getItemValue (
						pConfigurationBuffer, LB_LOADBALANCER_MAXITEMLENGTH,
						(ulModuleIndex * 6) + 5) != errNoError)
					{
						Error err = ConfigurationErrors (__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_GETITEMVALUE_FAILED);

						if (ciCfgItem. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONITEM_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (csSrcCfgSection. finish () != errNoError)
						{
							Error err = ConfigurationErrors (__FILE__, __LINE__,
								CFG_CONFIGURATIONSECTION_FINISH_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						// since the current element of the vector
						// has to be considered:
						lGroupIndex++;

						while (--lGroupIndex >= 0)
						{
							if ((_pgmGroupsModules [lGroupIndex]).
								_ulModulesNumber == 0)
								continue;

							if ((_pgmGroupsModules [lGroupIndex]).
								_mtLastModuleSelected. finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							(_pgmGroupsModules [lGroupIndex]). _vModules.
								clear ();
						}

						for (itDeliveryModule = _vModulesForAllGroups. begin ();
							itDeliveryModule != _vModulesForAllGroups. end ();
							++itDeliveryModule)
						{
							pdmDeliveryModule		=
								(DeliveryModule_p) (*itDeliveryModule);

							if ((pdmDeliveryModule -> _mtSocketStatus).
								finish () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							delete pdmDeliveryModule;
						}
						_vModulesForAllGroups. clear ();

						delete [] _pgmGroupsModules;
						_vModulesGroups. clear ();

						return err;
					}

					ulMaxRequestsNumber		=
						strtoul (pConfigurationBuffer, (char **) NULL, 10);

					// look for pPrivateHost, ulPrivatePort and
					// bURLToGetCurrentRequestsNumber if already present
					// Consider that in some case the URL could refer a port
					// different by the one specified above (i.e.:
					// http port (8081) and URL referring to billingAgent (8082)
					for (itDeliveryModule = _vModulesForAllGroups. begin ();
						itDeliveryModule != _vModulesForAllGroups. end ();
						++itDeliveryModule)
					{
						pdmDeliveryModule		=
							(DeliveryModule_p) (*itDeliveryModule);

						if (!strcmp (pdmDeliveryModule -> _pPrivateHost,
								pPrivateHost) &&
							pdmDeliveryModule -> _ulPrivatePort ==
								ulPrivatePort &&
							!strcmp (pdmDeliveryModule -> _pPublicHost,
								pPublicHost) &&
							pdmDeliveryModule -> _ulPublicPort ==
								ulPublicPort &&
							!strcmp (pdmDeliveryModule ->
								_bURLToGetCurrentRequestsNumber. str(),
								bURLToGetCurrentRequestsNumber. str())
							)
						{

							break;
						}
					}

					if (itDeliveryModule == _vModulesForAllGroups. end ())
					{
						// DeliveryModule not found

						if ((pdmDeliveryModule = new DeliveryModule_t) ==
							(DeliveryModule_p) NULL)
						{
							Error err = LoadBalancerErrors (__FILE__, __LINE__,
								LB_NEW_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);

							if (ciCfgItem. finish () != errNoError)
							{
								Error err = ConfigurationErrors (
									__FILE__, __LINE__,
									CFG_CONFIGURATIONITEM_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							if (csSrcCfgSection. finish () != errNoError)
							{
								Error err = ConfigurationErrors (
									__FILE__, __LINE__,
									CFG_CONFIGURATIONSECTION_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							// since the current element of the vector
							// has to be considered:
							lGroupIndex++;

							while (--lGroupIndex >= 0)
							{
								if ((_pgmGroupsModules [lGroupIndex]).
									_ulModulesNumber == 0)
									continue;

								if ((_pgmGroupsModules [lGroupIndex]).
									_mtLastModuleSelected. finish () !=
									errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								(_pgmGroupsModules [lGroupIndex]). _vModules.
									clear ();
							}

							for (itDeliveryModule =
								_vModulesForAllGroups. begin ();
								itDeliveryModule !=
									_vModulesForAllGroups. end ();
								++itDeliveryModule)
							{
								pdmDeliveryModule		=
									(DeliveryModule_p) (*itDeliveryModule);

								if ((pdmDeliveryModule -> _mtSocketStatus).
									finish () != errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								delete pdmDeliveryModule;
							}
							_vModulesForAllGroups. clear ();

							delete [] _pgmGroupsModules;
							_vModulesGroups. clear ();

							return err;
						}

						strcpy (pdmDeliveryModule -> _pPrivateHost,
							pPrivateHost);
						pdmDeliveryModule -> _ulPrivatePort		=
							ulPrivatePort;
						strcpy (pdmDeliveryModule -> _pPublicHost,
							pPublicHost);
						pdmDeliveryModule -> _ulPublicPort		=
							ulPublicPort;
						pdmDeliveryModule -> _bURLToGetCurrentRequestsNumber=
							bURLToGetCurrentRequestsNumber;
						pdmDeliveryModule -> _ulMaxRequestsNumber	=
							ulMaxRequestsNumber;
						pdmDeliveryModule -> _bCheckResultSuccessful		=
							false;

						pdmDeliveryModule -> _bWasLastErrorTimeout	= false;

						if ((pdmDeliveryModule -> _mtSocketStatus). init (
							PMutex:: MUTEX_RECURSIVE) != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_INIT_FAILED);
							_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);

							delete pdmDeliveryModule;

							if (ciCfgItem. finish () != errNoError)
							{
								Error err = ConfigurationErrors (
									__FILE__, __LINE__,
									CFG_CONFIGURATIONITEM_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							if (csSrcCfgSection. finish () != errNoError)
							{
								Error err = ConfigurationErrors (
									__FILE__, __LINE__,
									CFG_CONFIGURATIONSECTION_FINISH_FAILED);
								_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}

							// since the current element of the vector
							// has to be considered:
							lGroupIndex++;

							while (--lGroupIndex >= 0)
							{
								if ((_pgmGroupsModules [lGroupIndex]).
									_ulModulesNumber == 0)
									continue;

								if ((_pgmGroupsModules [lGroupIndex]).
									_mtLastModuleSelected. finish () !=
									errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								(_pgmGroupsModules [lGroupIndex]).
									_vModules. clear ();
							}

							for (itDeliveryModule =
								_vModulesForAllGroups. begin ();
								itDeliveryModule !=
									_vModulesForAllGroups. end ();
								++itDeliveryModule)
							{
								pdmDeliveryModule		=
									(DeliveryModule_p) (*itDeliveryModule);

								if ((pdmDeliveryModule -> _mtSocketStatus).
									finish () != errNoError)
								{
									Error err = PThreadErrors (
										__FILE__, __LINE__,
										THREADLIB_PMUTEX_FINISH_FAILED);
									_ptSystemTracer -> trace (
										Tracer:: TRACER_LERRR,
										(const char *) err, __FILE__, __LINE__);
								}

								delete pdmDeliveryModule;
							}
							_vModulesForAllGroups. clear ();

							delete [] _pgmGroupsModules;
							_vModulesGroups. clear ();

							return err;
						}

						_vModulesForAllGroups. insert (
							_vModulesForAllGroups. end (), pdmDeliveryModule);
					}

					((_pgmGroupsModules [lGroupIndex]). _vModules). insert (
						((_pgmGroupsModules [lGroupIndex]). _vModules). end (),
						pdmDeliveryModule);
				}

				(_pgmGroupsModules [lGroupIndex]). _itLastModuleSelected	=
					((_pgmGroupsModules [lGroupIndex]). _vModules). begin ();

				if ((_pgmGroupsModules [lGroupIndex]).
					_mtLastModuleSelected. init (PMutex:: MUTEX_RECURSIVE) !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_INIT_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (ciCfgItem. finish () != errNoError)
					{
						Error err = ConfigurationErrors (
							__FILE__, __LINE__,
							CFG_CONFIGURATIONITEM_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (csSrcCfgSection. finish () != errNoError)
					{
						Error err = ConfigurationErrors (
							__FILE__, __LINE__,
							CFG_CONFIGURATIONSECTION_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					// since the current element of the vector
					// has to be considered:
					lGroupIndex++;

					while (--lGroupIndex >= 0)
					{
						if ((_pgmGroupsModules [lGroupIndex]).
							_ulModulesNumber == 0)
							continue;

						if ((_pgmGroupsModules [lGroupIndex]).
							_mtLastModuleSelected. finish () !=
							errNoError)
						{
							Error err = PThreadErrors (
								__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (
								Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						(_pgmGroupsModules [lGroupIndex]).
							_vModules. clear ();
					}

					for (itDeliveryModule =
						_vModulesForAllGroups. begin ();
						itDeliveryModule !=
							_vModulesForAllGroups. end ();
						++itDeliveryModule)
					{
						pdmDeliveryModule		=
							(DeliveryModule_p) (*itDeliveryModule);

						if ((pdmDeliveryModule -> _mtSocketStatus).
							finish () != errNoError)
						{
							Error err = PThreadErrors (
								__FILE__, __LINE__,
								THREADLIB_PMUTEX_FINISH_FAILED);
							_ptSystemTracer -> trace (
								Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						delete pdmDeliveryModule;
					}
					_vModulesForAllGroups. clear ();

					delete [] _pgmGroupsModules;
					_vModulesGroups. clear ();

					return err;
				}
			}

			if (ciCfgItem. finish () != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_CONFIGURATIONITEM_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (csSrcCfgSection. finish () != errNoError)
				{
					Error err = ConfigurationErrors (__FILE__, __LINE__,
						CFG_CONFIGURATIONSECTION_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				// since the current element of the vector
				// has to be considered:
				lGroupIndex++;

				while (--lGroupIndex >= 0)
				{
					if ((_pgmGroupsModules [lGroupIndex]).
						_ulModulesNumber == 0)
						continue;

					if ((_pgmGroupsModules [lGroupIndex]).
						_mtLastModuleSelected. finish () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
				}

				for (itDeliveryModule = _vModulesForAllGroups. begin ();
					itDeliveryModule != _vModulesForAllGroups. end ();
					++itDeliveryModule)
				{
					pdmDeliveryModule		=
						(DeliveryModule_p) (*itDeliveryModule);

					if ((pdmDeliveryModule -> _mtSocketStatus). finish () !=
						errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_FINISH_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					delete pdmDeliveryModule;
				}
				_vModulesForAllGroups. clear ();

				delete [] _pgmGroupsModules;
				_vModulesGroups. clear ();

				return err;
			}
		}

		if (csSrcCfgSection. finish () != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIGURATIONSECTION_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			for (lGroupIndex = 0;
				lGroupIndex < _ulModulesGroupsNumber;
				lGroupIndex++)
			{
				if ((_pgmGroupsModules [lGroupIndex]).
					_ulModulesNumber == 0)
					continue;

				if ((_pgmGroupsModules [lGroupIndex]).
					_mtLastModuleSelected. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
			}

			for (itDeliveryModule = _vModulesForAllGroups. begin ();
				itDeliveryModule != _vModulesForAllGroups. end ();
				++itDeliveryModule)
			{
				pdmDeliveryModule		=
					(DeliveryModule_p) (*itDeliveryModule);

				if ((pdmDeliveryModule -> _mtSocketStatus). finish () !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				delete pdmDeliveryModule;
			}
			_vModulesForAllGroups. clear ();

			delete [] _pgmGroupsModules;
			_vModulesGroups. clear ();

			return err;
		}
	}

	_bShutdown			= false;

	if (_mtLoadBalancer. init (PMutex:: MUTEX_RECURSIVE) !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

		for (lGroupIndex = 0;
			lGroupIndex < _ulModulesGroupsNumber;
			lGroupIndex++)
		{
			if ((_pgmGroupsModules [lGroupIndex]).
				_ulModulesNumber == 0)
				continue;

			if ((_pgmGroupsModules [lGroupIndex]).
				_mtLastModuleSelected. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
		}

		for (itDeliveryModule = _vModulesForAllGroups. begin ();
			itDeliveryModule != _vModulesForAllGroups. end ();
			++itDeliveryModule)
		{
			pdmDeliveryModule		=
				(DeliveryModule_p) (*itDeliveryModule);

			if ((pdmDeliveryModule -> _mtSocketStatus). finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			delete pdmDeliveryModule;
		}
		_vModulesForAllGroups. clear ();

		delete [] _pgmGroupsModules;
		_vModulesGroups. clear ();

		return err;
	}

	if (_bLoadBalancerEnabled)
		checkAllDeliveryModules ();

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtLoadBalancer. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		for (lGroupIndex = 0;
			lGroupIndex < _ulModulesGroupsNumber;
			lGroupIndex++)
		{
			if ((_pgmGroupsModules [lGroupIndex]).
				_ulModulesNumber == 0)
				continue;

			if ((_pgmGroupsModules [lGroupIndex]).
				_mtLastModuleSelected. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			(_pgmGroupsModules [lGroupIndex]). _vModules. clear ();
		}

		for (itDeliveryModule = _vModulesForAllGroups. begin ();
			itDeliveryModule != _vModulesForAllGroups. end ();
			++itDeliveryModule)
		{
			pdmDeliveryModule		=
				(DeliveryModule_p) (*itDeliveryModule);

			if ((pdmDeliveryModule -> _mtSocketStatus). finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			delete pdmDeliveryModule;
		}
		_vModulesForAllGroups. clear ();

		delete [] _pgmGroupsModules;
		_vModulesGroups. clear ();

		return err;
	}


	return errNoError;
}


Error LoadBalancer:: finish ()

{

	unsigned long						ulGroupIndex;
	std:: vector<DeliveryModule_p>:: const_iterator		itDeliveryModule;
	DeliveryModule_p				pdmDeliveryModule;


	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_mtLoadBalancer. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_ulModulesGroupsNumber > 0)
	{
		for (ulGroupIndex = 0;
			ulGroupIndex < _ulModulesGroupsNumber;
			ulGroupIndex++)
		{
			if ((_pgmGroupsModules [ulGroupIndex]). _ulModulesNumber == 0)
				continue;

			if ((_pgmGroupsModules [ulGroupIndex]).
				_mtLastModuleSelected. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			(_pgmGroupsModules [ulGroupIndex]). _vModules. clear ();
		}

		for (itDeliveryModule = _vModulesForAllGroups. begin ();
			itDeliveryModule != _vModulesForAllGroups. end ();
			++itDeliveryModule)
		{
			pdmDeliveryModule		=
				(DeliveryModule_p) (*itDeliveryModule);

			if ((pdmDeliveryModule -> _mtSocketStatus). finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			delete pdmDeliveryModule;
		}
		_vModulesForAllGroups. clear ();

		delete [] _pgmGroupsModules;
		_vModulesGroups. clear ();
	}


	return errNoError;
}


Error LoadBalancer:: run (void)

{

	Boolean_t					bShutdown;
	// time_t						tStartTime;
	// time_t						tEndTime;
	unsigned long				ulSecondsToSleep;
	unsigned long				ulAdditionalMicroSecondsToSleep;


	bShutdown			= false;

	if (setShutdown (bShutdown) != errNoError)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_LOADBALANCER_SETSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	ulSecondsToSleep						=
		_ulCheckLoadBalancerModulesPeriodInMilliSecs / 1000;

	ulAdditionalMicroSecondsToSleep			=
		(_ulCheckLoadBalancerModulesPeriodInMilliSecs -
		(ulSecondsToSleep * 1000)) * 1000;

	while (!bShutdown)
	{
		#ifdef WIN32
			if (WinThread:: getSleep (
				ulSecondsToSleep, ulAdditionalMicroSecondsToSleep) !=
				errNoError)
		#else
			if (PosixThread:: getSleep (ulSecondsToSleep,
				ulAdditionalMicroSecondsToSleep) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		// tStartTime				= time (NULL);

		if (checkAllDeliveryModules () != errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_CHECKALLDELIVERYMODULES_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		// tEndTime				= time (NULL);

		if (getShutdown (&bShutdown) != errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_GETSHUTDOWN_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}


	return errNoError;
}


Error LoadBalancer:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t	stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif


	if (setShutdown (true) != errNoError)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_LOADBALANCER_SETSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (getThreadState (&stThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	tUTCNow					= time (NULL);

	while (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		if (time (NULL) - tUTCNow >=
			(_ulCheckLoadBalancerModulesPeriodInMilliSecs / 1000) + 1)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (0, 1000) != errNoError)
		#else
			if (PosixThread:: getSleep (0, 1000) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (getThreadState (&stThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}

	if (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		#ifdef WIN32
			// no cancel for Windows threads
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		#endif
	}


	return errNoError;
}


Error LoadBalancer:: checkAllDeliveryModules ()

{

	std:: vector<DeliveryModule_p>:: const_iterator		itDeliveryModule;
	DeliveryModule_p				pdmDeliveryModule;
	Boolean_t						bLocalCheckResultSuccessful;
	Boolean_t						bLocalWasLastErrorTimeout;


	for (itDeliveryModule = _vModulesForAllGroups. begin ();
		itDeliveryModule != _vModulesForAllGroups. end ();
		++itDeliveryModule)
	{
		pdmDeliveryModule		=
			(DeliveryModule_p) (*itDeliveryModule);

		if ((pdmDeliveryModule -> _mtSocketStatus). lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (checkModuleCompleted (
				(const char *) err,
				pdmDeliveryModule -> _pPrivateHost,
				pdmDeliveryModule -> _ulPrivatePort,
				pdmDeliveryModule -> _pPublicHost,
				pdmDeliveryModule -> _ulPublicPort,
				pdmDeliveryModule -> _bCheckResultSuccessful,
				pdmDeliveryModule -> _bWasLastErrorTimeout,
				// Next value does not have much sense
				// since there was no check
				time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
				errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

			}

			continue;
		}

		if (pdmDeliveryModule -> _bWasLastErrorTimeout &&
			time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp <= 5 * 60)
		{
			// in case the last error was a timeout,
			// we will check again after 5 minutes

			bLocalCheckResultSuccessful		=
				pdmDeliveryModule -> _bCheckResultSuccessful;
			bLocalWasLastErrorTimeout	=
				pdmDeliveryModule -> _bWasLastErrorTimeout;

			if ((pdmDeliveryModule -> _mtSocketStatus). unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (checkModuleCompleted (
					(const char *) err,
					pdmDeliveryModule -> _pPrivateHost,
					pdmDeliveryModule -> _ulPrivatePort,
					pdmDeliveryModule -> _pPublicHost,
					pdmDeliveryModule -> _ulPublicPort,
					bLocalCheckResultSuccessful,
					bLocalWasLastErrorTimeout,
					// Next value does not have much sense
					// since there was no check
					time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
					errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				continue;
			}

			if (checkModuleCompleted (
				(const char *) errNoError,
				pdmDeliveryModule -> _pPrivateHost,
				pdmDeliveryModule -> _ulPrivatePort,
				pdmDeliveryModule -> _pPublicHost,
				pdmDeliveryModule -> _ulPublicPort,
				bLocalCheckResultSuccessful,
				bLocalWasLastErrorTimeout,
				// Next value does not have much sense
				// since there was no check
				time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
				errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			continue;
		}

		pdmDeliveryModule -> _tLastCheckTimestamp		= time (NULL);

		if (strcmp (
			pdmDeliveryModule -> _bURLToGetCurrentRequestsNumber. str (), ""))
		{
			HttpGetThread_t		hgtHttpGetThread;
			Error_t				errHTTPGet;
			Buffer_t			bHttpGetHeaderResponse;
			Buffer_t			bHttpGetBodyResponse;
			unsigned long		ulCurrentRequestNumber;


			if ((errHTTPGet = hgtHttpGetThread. init (
				pdmDeliveryModule -> _bURLToGetCurrentRequestsNumber. str (),
				(const char *) NULL,	// Cookie
				"LoadBalancer",			// UserAgent
				_ulPingSocketTimeoutInSeconds,
				0,
				_ulPingSocketTimeoutInSeconds,
				0,
				_pLoadBalancerLocalIPAddress)) != errNoError)
			{
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errHTTPGet, __FILE__, __LINE__);

				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_INIT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				pdmDeliveryModule -> _bCheckResultSuccessful	= false;

				pdmDeliveryModule -> _bWasLastErrorTimeout		= false;

				if ((pdmDeliveryModule -> _mtSocketStatus). unLock () !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (checkModuleCompleted (
					(const char *) err,
					pdmDeliveryModule -> _pPrivateHost,
					pdmDeliveryModule -> _ulPrivatePort,
					pdmDeliveryModule -> _pPublicHost,
					pdmDeliveryModule -> _ulPublicPort,
					pdmDeliveryModule -> _bCheckResultSuccessful,
					pdmDeliveryModule -> _bWasLastErrorTimeout,
					time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
					errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				continue;
			}

			if ((errHTTPGet = hgtHttpGetThread. run (
				&bHttpGetHeaderResponse, &bHttpGetBodyResponse,
				(Buffer_p) NULL, (Buffer_p) NULL, (Buffer_p) NULL)) !=
				errNoError)
			{
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errHTTPGet, __FILE__, __LINE__);

				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_HTTPGETTHREAD_RUN_FAILED,
					1, pdmDeliveryModule ->
						_bURLToGetCurrentRequestsNumber. str ());
				_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
					(const char *) err, __FILE__, __LINE__);


				pdmDeliveryModule -> _bCheckResultSuccessful	= false;

				if ((unsigned long) errHTTPGet ==
					WEBTOOLS_HTTPGETTHREAD_HEADERTIMEOUTEXPIRED)
				{
					Error errLocal = LoadBalancerErrors (__FILE__, __LINE__,
						LB_HTTPGETTHREAD_LASTURLCHECKTIMEOUT_WILLPINGAGAINLATER,
						1, pdmDeliveryModule ->
							_bURLToGetCurrentRequestsNumber. str ());
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errLocal, __FILE__, __LINE__);

					pdmDeliveryModule -> _bWasLastErrorTimeout		= true;
				}
				else
				{
					pdmDeliveryModule -> _bWasLastErrorTimeout		= false;
				}

				if (hgtHttpGetThread. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						WEBTOOLS_HTTPGETTHREAD_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if ((pdmDeliveryModule -> _mtSocketStatus). unLock () !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (checkModuleCompleted (
					(const char *) err,
					pdmDeliveryModule -> _pPrivateHost,
					pdmDeliveryModule -> _ulPrivatePort,
					pdmDeliveryModule -> _pPublicHost,
					pdmDeliveryModule -> _ulPublicPort,
					pdmDeliveryModule -> _bCheckResultSuccessful,
					pdmDeliveryModule -> _bWasLastErrorTimeout,
					time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
					errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				continue;
			}

			if (hgtHttpGetThread. finish () != errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_HTTPGETTHREAD_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				pdmDeliveryModule -> _bCheckResultSuccessful	= false;

				pdmDeliveryModule -> _bWasLastErrorTimeout		= false;

				if ((pdmDeliveryModule -> _mtSocketStatus). unLock () !=
					errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (checkModuleCompleted (
					(const char *) err,
					pdmDeliveryModule -> _pPrivateHost,
					pdmDeliveryModule -> _ulPrivatePort,
					pdmDeliveryModule -> _pPublicHost,
					pdmDeliveryModule -> _ulPublicPort,
					pdmDeliveryModule -> _bCheckResultSuccessful,
					pdmDeliveryModule -> _bWasLastErrorTimeout,
					time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
					errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				continue;
			}

			if (!strncmp (bHttpGetBodyResponse. str (), "Concurrent users: ",
				strlen ("Concurrent users: ")))
			{
				// XDM Format: "Concurrent users: XXXX"
				const char			*pCurrentRequestNumber;


				pCurrentRequestNumber		= bHttpGetBodyResponse. str () +
					strlen ("Concurrent users: ");
				ulCurrentRequestNumber		=
					strtoul (pCurrentRequestNumber, (char **) NULL, 10);

				/*
				std:: cout << "bHttpGetHeaderResponse: "
					<< bHttpGetHeaderResponse. str ()
					<< std:: endl
					<< "bHttpGetBodyResponse: " << bHttpGetBodyResponse. str ()
					<< std:: endl
					<< "ulCurrentRequestNumber: " << ulCurrentRequestNumber
					<< std:: endl
					<< std:: endl;
				*/

				if (ulCurrentRequestNumber >=
					pdmDeliveryModule -> _ulMaxRequestsNumber)
				{
					Error errLocal = LoadBalancerErrors (__FILE__, __LINE__,
						LB_HTTPGETTHREAD_REACHEDMAXREQUESTSNUMBER,
						2, pdmDeliveryModule ->
							_bURLToGetCurrentRequestsNumber. str (),
							bHttpGetBodyResponse. str ());
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errLocal, __FILE__, __LINE__);

					pdmDeliveryModule -> _bCheckResultSuccessful	= false;

					pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
				}
				else
				{
					pdmDeliveryModule -> _bCheckResultSuccessful	= true;

					pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
				}
			}
			else if (strstr (bHttpGetBodyResponse. str (),
				"Concurrent Users (Total)") != (char *) NULL &&
				strchr (bHttpGetBodyResponse. str (), '=') != (char *) NULL)
			{
				// Format: "Concurrent Users (Total)      =      1\n..."
				/* In particular XSS returns:
					Concurrent Users (Total)      =      1
					Concurrent Users (MOD)        =      1
					Concurrent Users (Playlist)   =      0
					Concurrent Users (Live)       =      0
					Concurrent Users (FCS)        =      0
					Concurrent Users (FTS)        =      0
				*/

				const char		*pEqualChar;
				const char		*pEndNumber;
				char			pCurrentRequestNumber [256];


				pEqualChar		= strchr (bHttpGetBodyResponse. str (), '=');
				pEqualChar++;
				if ((pEndNumber = strchr (pEqualChar, '\n')) == (char *) NULL ||
					pEndNumber - pEqualChar >= 256)
				{
					pdmDeliveryModule -> _bCheckResultSuccessful	= true;

					pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
				}
				else
				{
					strncpy (pCurrentRequestNumber, pEqualChar,
						pEndNumber - pEqualChar);
					pCurrentRequestNumber [pEndNumber - pEqualChar]		= '\0';
					ulCurrentRequestNumber		=
						strtoul (pCurrentRequestNumber, (char **) NULL, 10);

					if (ulCurrentRequestNumber >=
						pdmDeliveryModule -> _ulMaxRequestsNumber)
					{
						Error errLocal = LoadBalancerErrors (__FILE__, __LINE__,
							LB_HTTPGETTHREAD_REACHEDMAXREQUESTSNUMBER,
							2, pdmDeliveryModule ->
								_bURLToGetCurrentRequestsNumber. str (),
								bHttpGetBodyResponse. str ());
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) errLocal, __FILE__, __LINE__);

						pdmDeliveryModule -> _bCheckResultSuccessful	= false;

						pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
					}
					else
					{
						pdmDeliveryModule -> _bCheckResultSuccessful	= true;

						pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
					}
				}
			}
			else
			{
				pdmDeliveryModule -> _bCheckResultSuccessful	= true;

				pdmDeliveryModule -> _bWasLastErrorTimeout	= false;
			}
		}
		else
		{
			ClientSocket_t					csClientSocket;
			Error_t							errSocket;


			if ((errSocket = csClientSocket. init (SocketImpl:: STREAM,
				_ulPingSocketTimeoutInSeconds, 0,
				_ulPingSocketTimeoutInSeconds, 0,
				0,
				true, _pLoadBalancerLocalIPAddress,
				pdmDeliveryModule -> _pPrivateHost,
				pdmDeliveryModule -> _ulPrivatePort)) != errNoError)
			{
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errSocket, __FILE__, __LINE__);

				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_CLIENTSOCKET_INIT_FAILED,
					2, pdmDeliveryModule -> _pPrivateHost,
					pdmDeliveryModule -> _ulPrivatePort);
				_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
					(const char *) err, __FILE__, __LINE__);

				pdmDeliveryModule -> _bCheckResultSuccessful	= false;

				if ((unsigned long) errSocket == SCK_CONNECT_EINPROGRESSFAILED)
				{
					Error errLocal = LoadBalancerErrors (__FILE__, __LINE__,
					LB_HTTPGETTHREAD_LASTSOCKETCHECKTIMEOUT_WILLPINGAGAINLATER,
						2, pdmDeliveryModule -> _pPrivateHost,
						pdmDeliveryModule -> _ulPrivatePort);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errLocal, __FILE__, __LINE__);

					pdmDeliveryModule -> _bWasLastErrorTimeout		= true;
				}
				else
				{
					pdmDeliveryModule -> _bWasLastErrorTimeout		= false;
				}
			}
			else
			{
				pdmDeliveryModule -> _bCheckResultSuccessful		= true;

				pdmDeliveryModule -> _bWasLastErrorTimeout	= false;

				if (csClientSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_CLIENTSOCKET_FINISH_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					pdmDeliveryModule -> _bCheckResultSuccessful	= false;

					if ((pdmDeliveryModule -> _mtSocketStatus). unLock () !=
						errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (checkModuleCompleted (
						(const char *) err,
						pdmDeliveryModule -> _pPrivateHost,
						pdmDeliveryModule -> _ulPrivatePort,
						pdmDeliveryModule -> _pPublicHost,
						pdmDeliveryModule -> _ulPublicPort,
						pdmDeliveryModule -> _bCheckResultSuccessful,
						pdmDeliveryModule -> _bWasLastErrorTimeout,
						time (NULL) -
							pdmDeliveryModule -> _tLastCheckTimestamp) !=
						errNoError)
					{
						Error err = LoadBalancerErrors (__FILE__, __LINE__,
							LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					continue;
				}
			}
		}

		bLocalCheckResultSuccessful		=
			pdmDeliveryModule -> _bCheckResultSuccessful;
		bLocalWasLastErrorTimeout	=
			pdmDeliveryModule -> _bWasLastErrorTimeout;

		if ((pdmDeliveryModule -> _mtSocketStatus). unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (checkModuleCompleted (
				(const char *) err,
				pdmDeliveryModule -> _pPrivateHost,
				pdmDeliveryModule -> _ulPrivatePort,
				pdmDeliveryModule -> _pPublicHost,
				pdmDeliveryModule -> _ulPublicPort,
				bLocalCheckResultSuccessful,
				bLocalWasLastErrorTimeout,
				time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
				errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			continue;
		}

		if (checkModuleCompleted (
			(const char *) errNoError,
			pdmDeliveryModule -> _pPrivateHost,
			pdmDeliveryModule -> _ulPrivatePort,
			pdmDeliveryModule -> _pPublicHost,
			pdmDeliveryModule -> _ulPublicPort,
			bLocalCheckResultSuccessful,
			bLocalWasLastErrorTimeout,
			time (NULL) - pdmDeliveryModule -> _tLastCheckTimestamp) !=
			errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			continue;
		}
	}


	return errNoError;
}


Error LoadBalancer:: checkModuleCompleted (
	const char *pError,
	const char *pPrivateHost,
	unsigned long ulPrivatePort,
	const char *pPublicHost,
	unsigned long ulPublicPort,
	Boolean_t bCheckResultSuccessful,
	Boolean_t bWasLastErrorTimeout,
	unsigned long ulCheckElapsedTimeInSecs)

{

	return errNoError;
}


Error LoadBalancer:: getAvailableModule (const char *pModulesName,
	char *pPublicHost, unsigned long ulPublicHostBufferLength,
	unsigned long *pulPublicPort,
	char *pPrivateHost, unsigned long ulPrivateHostBufferLength,
	unsigned long *pulPrivatePort)

{

	unsigned long						ulGroupIndex;
	unsigned long						ulModuleIndex;
	std:: vector<DeliveryModule_p>:: const_iterator		itDeliveryModule;
	DeliveryModule_p					pdmDeliveryModule;
	Boolean_t							bModuleFound;
	Boolean_t							bLocalCheckResultSuccessful;


	if (!_bLoadBalancerEnabled ||
		pPublicHost == (char *) NULL ||
		pulPublicPort == (unsigned long *) NULL)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_ACTIVATION_WRONG);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	for (ulGroupIndex = 0;
		ulGroupIndex < _ulModulesGroupsNumber;
		ulGroupIndex++)
	{
		if (!strcmp ((_pgmGroupsModules [ulGroupIndex]). _pModulesName,
			pModulesName))
			break;
	}

	if (ulGroupIndex == _ulModulesGroupsNumber)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_LOADBALANCER_MODULESNAMENOTFOUND,
			1, pModulesName);
		_ptSystemTracer -> trace (Tracer:: TRACER_LWRNG,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((_pgmGroupsModules [ulGroupIndex]). _mtLastModuleSelected. lock () !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	bModuleFound			= false;

	for (ulModuleIndex = 0; ulModuleIndex <
		(_pgmGroupsModules [ulGroupIndex]). _ulModulesNumber;
		ulModuleIndex++)
	{
		pdmDeliveryModule	= (DeliveryModule_p)
			(*((_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected));

		if ((pdmDeliveryModule -> _mtSocketStatus). lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if ((_pgmGroupsModules [ulGroupIndex]). _mtLastModuleSelected.
				unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		bLocalCheckResultSuccessful		=
			pdmDeliveryModule -> _bCheckResultSuccessful;

		if ((pdmDeliveryModule -> _mtSocketStatus). unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if ((_pgmGroupsModules [ulGroupIndex]). _mtLastModuleSelected.
				unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (bLocalCheckResultSuccessful)
		{
			if (strlen (pdmDeliveryModule -> _pPublicHost) >=
				ulPublicHostBufferLength ||
				(pPrivateHost != (char *) NULL &&
				strlen (pdmDeliveryModule -> _pPrivateHost) >=
				ulPrivateHostBufferLength))
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_BUFFERTOOSHORT);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if ((_pgmGroupsModules [ulGroupIndex]). _mtLastModuleSelected.
					unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			strcpy (pPublicHost, pdmDeliveryModule -> _pPublicHost);
			*pulPublicPort		= pdmDeliveryModule -> _ulPublicPort;
			if (pPrivateHost != (char *) NULL)
			{
				strcpy (pPrivateHost, pdmDeliveryModule -> _pPrivateHost);
			}
			if (pulPrivatePort != (unsigned long *) NULL)
			{
				*pulPrivatePort		= pdmDeliveryModule -> _ulPrivatePort;
			}

			((_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected)++;

			if ((_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected ==
				((_pgmGroupsModules [ulGroupIndex]). _vModules). end ())
			{
				(_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected	=
				((_pgmGroupsModules [ulGroupIndex]). _vModules). begin ();
			}

			bModuleFound			= true;

			break;
		}

		((_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected)++;

		if ((_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected ==
			((_pgmGroupsModules [ulGroupIndex]). _vModules). end ())
		{
			(_pgmGroupsModules [ulGroupIndex]). _itLastModuleSelected	=
			((_pgmGroupsModules [ulGroupIndex]). _vModules). begin ();
		}
	}

	if ((_pgmGroupsModules [ulGroupIndex]). _mtLastModuleSelected. unLock () !=
		errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (!bModuleFound)
	{
		Error err = LoadBalancerErrors (__FILE__, __LINE__,
			LB_LOADBALANCER_MODULENOTAVAILABLE,
			1, pModulesName);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error LoadBalancer:: getShutdown (Boolean_p pbShutdown)

{

	if (_mtLoadBalancer. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	*pbShutdown			= _bShutdown;

	if (_mtLoadBalancer. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error LoadBalancer:: setShutdown (Boolean_t bShutdown)

{

	if (_mtLoadBalancer. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	_bShutdown				= bShutdown;

	if (_mtLoadBalancer. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


