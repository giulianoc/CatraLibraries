
#ifndef ConnectionsManagerService_h
	#define ConnectionsManagerService_h

	#include "ConnectionsManagerErrors.h"
	#include "ConfigurationFile.h"
	#include "Tracer.h"
	#include "CMEventsSet.h"
	#include "CMSocketsPool.h"
	#include "CheckSocketsPoolTimes.h"
	#include "CheckServerSocketTimes.h"
	#include "CheckUnusedSocketsTimes.h"
	#include "Scheduler.h"
	#include "LoadBalancer.h"
	#include "ServerSocket.h"
	#include "CMProcessor.h"
	#include "Service.h"


	// #define SB_MAXSERVICEUSERNAME				(256 + 1)
	// #define SB_MAXSERVICEPASSWORD				(256 + 1)

	// #define CMS_SERVICEBROKERFRAMEWORK_MAXLICENSELENGTH	1024

	// #define CMS_SERVICEBROKERFRAMEWORK_MAXCLIENTS		100

	// #define CMS_SERVICEBROKERFRAMEWORK_MAXPLUGINITEMNAMELENGTH	(128 + 1)

	// #define CMS_SERVICEBROKERFRAMEWORK_CONFPATHNAMEENVIRONMENTVARIABLE	\
	// 	"SERVICEBROKER_CONF_PATHNAME"

	// #define CMS_SERVICEBROKERFRAMEWORK_SERVICENAME	\
	// 	"ServiceBrokerFramework"
	// #define CMS_SERVICEBROKERFRAMEWORK_SERVICEDESCRIPTION	\
	// 	"Service Broker Framework"

	class ConnectionsManagerService: public Service
	{
		private:
			char						*_pConfigurationPathName;
			char						*_pPidPathName;
			char						*_pServiceVersion;

			ConfigurationFile_t			_cfConfiguration;
			Tracer_t					_tSystemTracer;
			time_t						_tServerStartTime;

			char						_pLocalIPAddress [
				SCK_MAXIPADDRESSLENGTH];
			unsigned long				_ulServerPort;
			ServerSocket_t				_ssServerSocket;

			CMProcessor_p				_pspProcessor;
			unsigned long				_ulProcessorsNumber;

			unsigned long				_ulSchedulerSleepTimeInMilliSecs;
			CheckSocketsPoolTimes_t		_cspCheckSocketsPoolTimes;
			unsigned long				_ulCheckSocketsPoolPeriodInMilliSecs;
			CheckServerSocketTimes_t	_cssCheckServerSocketTimes;
            unsigned long				_ulCheckServerSocketPeriodInMilliSecs;
			CheckUnusedSocketsTimes_t	_cssCheckUnusedSocketsTimes;
            unsigned long				_ulCheckUnusedSocketsPeriodInSeconds;

            unsigned long				_ulTimeoutToAcceptNewConnectionsInSeconds;

			Boolean_t					_bIsHTTPProxyEnabled;
			LoadBalancer_t				_lbHTTPProxyLoadBalancer;

			CMEventsSet_p				_pesEventsSet;
			Scheduler_t					_scScheduler;
			CMSocketsPool_t				_spSocketsPool;

			unsigned long				_ulTimeoutToWaitRequestInSeconds;
			unsigned long				_ulAdditionalMicrosecondsToWait;
			#ifdef VECTORFREESESSIONS
				std:: vector<Session_p>		_vFreeSessions;
			#else
				// HashMap will allow us to guarantee unique keys
				// and we will make sure the same session is not added
				// in the Free Structure
				IntHasher_p					_phHasher;
				IntCmp_p					_pcComparer;
				SessionsHashMap_p			_phmFreeSessions;
			#endif
			PMutex_t					_mtFreeSessions;
			Session_p					_psSessions;
			unsigned long				_ulMaxConnections;


			Error finishTheService (void);

		protected:
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

			virtual Error onInit (void);

			virtual Error onInitEventsSet (
				CMEventsSet_p *pesEventsSet,
				Tracer_p ptSystemTracer);

			virtual Error onFinishEventsSet (
				CMEventsSet_p *pesEventsSet,
				Tracer_p ptSystemTracer);

			#ifdef VECTORFREESESSIONS
				virtual Error onInitProcessors (
					CMProcessor_p *pspProcessor,
					unsigned long ulProcessorsNumber,
					ConfigurationFile_p pcfConfiguration,
					Scheduler_p pscScheduler,
					CMEventsSet_p pesEventsSet,
					Boolean_t bIsHTTPProxyEnabled,
					LoadBalancer_p plbHTTPProxyLoadBalancer,
					unsigned long ulMaxSimultaneousConnectionsToAccept,
					unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
					std:: vector<Session_p> *pvFreeSessions,
					PMutex_p pmtFreeSessions,
					Session_p psSessions,
					unsigned long ulMaxConnections,
					unsigned long ulMaxConnectionTTLInSeconds,
					ServerSocket_p pssServerSocket,
					CMSocketsPool_p pspSocketsPool,
					unsigned long ulMaxMilliSecondsToProcessAnEvent,
					Tracer_p ptSystemTracer);
			#else
				virtual Error onInitProcessors (
					CMProcessor_p *pspProcessor,
					unsigned long ulProcessorsNumber,
					ConfigurationFile_p pcfConfiguration,
					Scheduler_p pscScheduler,
					CMEventsSet_p pesEventsSet,
					Boolean_t bIsHTTPProxyEnabled,
					LoadBalancer_p plbHTTPProxyLoadBalancer,
					unsigned long ulMaxSimultaneousConnectionsToAccept,
					unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
					SessionsHashMap_p phmFreeSessions,
					PMutex_p pmtFreeSessions,
					Session_p psSessions,
					unsigned long ulMaxConnections,
					unsigned long ulMaxConnectionTTLInSeconds,
					ServerSocket_p pssServerSocket,
					CMSocketsPool_p pspSocketsPool,
					unsigned long ulMaxMilliSecondsToProcessAnEvent,
					Tracer_p ptSystemTracer);
			#endif

			virtual Error onFinishProcessors (
				CMProcessor_p *pspProcessor,
				unsigned long ulProcessorsNumber,
				Tracer_p ptSystemTracer);

			virtual Error onStartProcessors (
				CMProcessor_p pspProcessor,
				unsigned long ulProcessorsNumber,
				ConfigurationFile_p pcfConfiguration,
				Tracer_p ptSystemTracer);

			virtual Error onWaitProcessors (
				CMProcessor_p pspProcessor,
				unsigned long ulProcessorsNumber,
				Tracer_p ptSystemTracer);

			virtual Error onCancelProcessors (
				CMProcessor_p pspProcessor,
				unsigned long ulProcessorsNumber,
				Tracer_p ptSystemTracer);

			virtual Error onStartMyTimes (
				ConfigurationFile_p pcfConfiguration,
				Scheduler_p pscScheduler,
				CMEventsSet_p pesEventsSet,
				Tracer_p ptSystemTracer);

			virtual Error onStopMyTimes (
				Tracer_p ptSystemTracer);

			virtual Error onStop (void);

			virtual Error onStart (void);

		public:
			ConnectionsManagerService (void);

			~ConnectionsManagerService (void);

			Error init (const char *pServiceName,
				const char *pServiceVersion,
				const char *pConfigurationPathName,
				const char *pPidPathName);

			Error finish (void);

			Error cancel (void);
	} ;

	typedef class ConnectionsManagerService
		ConnectionsManagerService_t, *ConnectionsManagerService_p;


#endif

