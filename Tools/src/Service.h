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

#ifndef Service_h
	#define Service_h

	#ifdef WIN32
		#if _MSC_VER > 1000
			#pragma once
		#endif // _MSC_VER > 1000

		// Exclude rarely-used stuff from Windows headers
		#define WIN32_LEAN_AND_MEAN
	#endif

	#include "Buffer.h"
	#include "ToolsErrors.h"
	#ifdef WIN32
		#include <windows.h>
	#endif

	#ifdef WIN32
		#define TOOLS_SERVICE_CONTROL_USER					128

		#define TOOLS_SERVICE_MAXMODULEFILENAMELENGTH		(512 + 1)
	#else
		#define TOOLS_SERVICE_MAXRUNLEVELDIRECTORYLENGTH	(512 + 1)
		#define TOOLS_SERVICE_MAXPATHNAME					(1024 + 1)
	#endif

	#define TOOLS_SERVICE_TO_INSTALL						"-i"
	#define TOOLS_SERVICE_TO_UNINSTALL						"-u"
	#define TOOLS_SERVICE_TO_START							"-start"
	#define TOOLS_SERVICE_TO_STOP							"-stop"

	/*
	#ifdef WIN32
		#define TOOLS_SERVICE_DEBUGFILE			"C:\\ServiceDebug.log"
	#else
		#define TOOLS_SERVICE_DEBUGFILE			"/tmp/ServiceDebug.log"
	#endif
	*/
	#define TOOLS_SERVICE_MAXSERVICEDEBUGLOG				(128 + 1)

	/**
		This class represent a service.
		Here are the steps to create a service class:
			1. declare a class deriving by Service.
			2. By default the Service just write "I'm running" each two seconds.
				Therefore the derived service class must re-define:
					- the 'onStart' virtual method to start the service.
					- the 'onInit' virtual method (called when the service
						is started)
					- the 'onStop' virtual method (called when the service
						is stopped)
					- ...other virtual methods to manage other events
						if necessary

		Remark: The  service is stopped/finished when the onStart
			method will finish. Therefore the onStart method must return
			only when the service is finished.

		The main of the executable will be like:
			#ifdef WIN32
			#else
				Service:: launchUnixDaemon ();
			#endif

			wsNewService. init ()

			wsNewService. parseArguments (iArgc, pArgv,
				&ulReservedArgumentsNumberFound)

			if (ulReservedArgumentsNumberFound == 0)
				wsNewService. start (iArgc, pArgv)

			wsNewService. finish ()
	*/
	class Service

	{
		public:
			char				_pServiceDebugFile [
				TOOLS_SERVICE_MAXSERVICEDEBUGLOG];

		private:
			Buffer_t			_bServiceDescription;
			#ifdef WIN32
				Buffer_t		_bServiceUserName;
				Buffer_t		_bServicePassword;
				DWORD			_dwStartType;
			#endif
			Boolean_t		_bIsShutdown;

			#ifdef WIN32
				SC_HANDLE		_pscmServiceControlManager;
				SERVICE_STATUS_HANDLE	_sshServiceStatusHandle; 
				Boolean_t		_bServiceRunning;
				Boolean_t		_bServicePaused;

				static Service	*_pThis;

				static DWORD WINAPI handlerServiceEventsEx (
					DWORD dwControl,
					DWORD dwEventType,
					LPVOID lpEventData,
					LPVOID lpContext);

				static VOID WINAPI ServiceMain (
					DWORD dwArgc,
					LPTSTR *lpszArgv);
			#endif

		protected:
			Buffer_t			_bServiceName;


			/**
				Method called when the service is started.
				It must not return until the service is started.
				If the service starts threads, they must be closed before this method is
				finished.
			*/
			virtual Error onStart (void);

			/**
				Method called when the service is stopped. It causes the finishing
				of the onStart method and finishes all the object initialized in onInit.
				Remark:
					If onStart method is something like:
						thread-1. start
						...
						thread-n. start

						thread-n. join
						thread-n-1. cancel
						...
						thread-1. cancel

					the onStop method should be:
						thread-n. cancel
						// wait that the last thread is finished
						{
							thread1.getThreadState (&stRTPThreadState)
							tUTCNow		= time (NULL);
							while (stRTPThreadState == PosixThread:: THREADLIB_STARTED ||
								stRTPThreadState == PosixThread:: THREADLIB_STARTED_AND_JOINED)
							{
								if (time (NULL) - tUTCNow >= 60)
									break;
								PosixThread:: getSleep (1, 0)
								thread1.getThreadState (&stRTPThreadState)
							}
						}

						object-m. finish
						...
						object-1. finish
			*/
			virtual Error onStop (void);

			virtual Error onInstall (void);

			virtual Error onUnInstall (void);

			/**
				Method called before the service is started (onStart method).
				Initialize all the objects of the service.
			*/
			virtual Error onInit (void);

			#ifdef WIN32
				virtual Error onUserControl (DWORD dwControl);

				virtual Error onContinue (void);

				virtual Error onInterrogate (void);

				virtual Error onShutdown (void);

				virtual Error onSessionChange (void);

				virtual Error onPause (void);

				virtual Error onParamChange (void);

				virtual Error onNetBindAdd (void);

				virtual Error onNetBindRemove (void);

				virtual Error onNetBindEnable (void);

				virtual Error onNetBindDisable (void);

				virtual Error onDeviceEvent (DWORD dwEventType,
					LPVOID pvEventData);

				virtual Error onPowerEvent (DWORD dwEventType,
					LPVOID pvEventData);

				virtual Error onHardwareProfileChange (DWORD dwEventType);

				Error setStatus (DWORD dwControl);
			#endif

			#ifdef WIN32
			#else
				/**
					Only on unix/linux environment, this method must be
					re-defined in order to set the command that will be
					used by the script inside the /etc/rc.d/ined.d directory
					to start the service.
				*/
				virtual Error appendStartScriptCommand (
					Buffer_p pbServiceScriptFile);

				/**
					Only on unix/linux environment, this method must be
					re-defined in order to set the command that will be
					used by the script inside the /etc/rc.d/ined.d directory
					to start the service.
				*/
				virtual Error appendStopScriptCommand (
					Buffer_p pbServiceScriptFile);

				/**
					Only on unix/linux environment, this method must be
					re-defined in order to set the command that will be
					used by the script inside the /etc/rc.d/ined.d directory
					to start the service.
				*/
				virtual Error appendStatusScriptCommand (
					Buffer_p pbServiceScriptFile);
			#endif
		public:
			Service (void);

			~Service (void);

			#ifdef WIN32
			#else
				static Error launchUnixDaemon (
					const char *pPIDFilePathName);
			#endif

			/**
				Initialize the  Service. In case of Windows, if service
				user name and password are NULL, the service will use
				the localsystem account.
			*/
			#ifdef WIN32
				Error init (
					const char *pServiceName, const char *pServiceDescription,
					const char *pServiceUserName, const char *pServicePassword,
					DWORD dwStartType = SERVICE_AUTO_START);
			#else
				Error init (
					const char *pServiceName, const char *pServiceDescription);
			#endif

			Error finish (void);

			Error isInstalled (Boolean_p pbIsInstalled);

			Error install (int iArgc, char **pArgv);

			Error unInstall (void);

			/**
				This method parse optional arguments that could be passed to the
				service executable and if it finds 'reserved' parameters
				performs the relative command on the  Service.
				The 'reserved' parameters and the relative  service activity are:
					"-i"				-> install the service
					"-u"				-> uninstall the service
					"-start"			-> start the service
					"-stop"				-> stop the service

				Only for unix/linux environment, the start and the stop
				command just call the start and the stop funcion inside the
				script of the service (/etc/rc.d/init.d/<service name>)
				and wait that the call will finish.
			*/
			Error parseArguments (int argc, char *argv[],
				unsigned long *pulReservedArgumentsNumberFound);

			/**
				This method starts the service calling
				onInit and onStart (and onStop only for unix environment,
				for Windows, onStop is called when the service is stopped
				by the 'services' window)
				and returns when the service is finished.
			*/
			Error start (int iArgc, char **pArgv);
	} ;

	typedef class Service Service_t, *Service_p;

#endif

