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


#ifndef LoadBalancer_h
	#define LoadBalancer_h

	#include "ConfigurationFile.h"
	#include "ClientSocket.h"
	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "Tracer.h"
	#include "LoadBalancerErrors.h"

	#define LB_LOADBALANCER_MAXITEMLENGTH			(512 + 1)


	#ifdef WIN32
		typedef class LoadBalancer: public WinThread
	#else
		typedef class LoadBalancer: public PosixThread
	#endif
	{
		private:
			typedef struct DeliveryModule
			{
				char				_pPrivateHost [
					SCK_MAXHOSTNAMELENGTH];
				unsigned long		_ulPrivatePort;
				char				_pPublicHost [
					SCK_MAXHOSTNAMELENGTH];
				unsigned long		_ulPublicPort;
				Buffer_t			_bURLToGetCurrentRequestsNumber;
				unsigned long		_ulMaxRequestsNumber;

				time_t				_tLastCheckTimestamp;
				PMutex_t			_mtSocketStatus;
				Boolean_t			_bCheckResultSuccessful;

				Boolean_t			_bWasLastErrorTimeout;

			} DeliveryModule_t, *DeliveryModule_p;

			typedef struct GroupModules
			{
				// i.e.: in case the configuration section is
				// "Modules_Streamers", the _pModulesName will be just
				// "Streamers"
				char				_pModulesName [
					LB_LOADBALANCER_MAXITEMLENGTH];
				unsigned long		_ulModulesNumber;
				std:: vector<DeliveryModule_p>		_vModules;

                PMutex_t			_mtLastModuleSelected;
				std:: vector<DeliveryModule_p>:: const_iterator
					_itLastModuleSelected;

				GroupModules ()
				{
					_ulModulesNumber		= 0;
				}

			} GroupModules_t, *GroupModules_p;

		protected:
			Tracer_p					_ptSystemTracer;

		private:
			Boolean_t					_bShutdown;
			PMutex_t					_mtLoadBalancer;
			Boolean_t					_bLoadBalancerEnabled;
			char						_pLoadBalancerLocalIPAddress [
				SCK_MAXIPADDRESSLENGTH];
			unsigned long				_ulPingSocketTimeoutInSeconds;
			unsigned long		_ulCheckLoadBalancerModulesPeriodInMilliSecs;

			// All the delivery modules considered only one time
			std:: vector<DeliveryModule_p>		_vModulesForAllGroups;

			GroupModules_p				_pgmGroupsModules;
			unsigned long				_ulModulesGroupsNumber;
			std:: vector<GroupModules_p>	_vModulesGroups;


			Error setShutdown (Boolean_t bShutdown);

			Error getShutdown (Boolean_p pbShutdown);

		protected:
			virtual Error checkModuleCompleted (
				const char *pError,
				const char *pPrivateHost,
				unsigned long ulPrivatePort,
				const char *pPublicHost,
				unsigned long ulPublicPort,
				Boolean_t bCheckResultSuccessful,
				Boolean_t bWasLastErrorTimeout,
				unsigned long ulCheckElapsedTimeInSecs);

			virtual Error run (void);

		public:
			LoadBalancer (void);

			~LoadBalancer (void);

			Error init (
				ConfigurationFile_p pcfConfiguration,
				const char *pLoadBalancerSectionName,
				Tracer_p ptSystemTracer);

			virtual Error finish ();

			virtual Error cancel (void);

			Error checkAllDeliveryModules ();

			// i.e.: in case the configuration section is
			// "Modules_Streamers", the pModulesName will be just
			// "Streamers"
			// pPublicHost and pulPublicPort are output parameters
			// pPrivateHost and pulPrivatePort are optional output parameters
			Error getAvailableModule (const char *pModulesName,
				char *pPublicHost, unsigned long ulPublicHostBufferLength,
				unsigned long *pulPublicPort,
				char *pPrivateHost = (char *) NULL,
				unsigned long ulPrivateHostBufferLength = 0,
				unsigned long *pulPrivatePort = (unsigned long *) NULL);

	} LoadBalancer_t, *LoadBalancer_p;

#endif

