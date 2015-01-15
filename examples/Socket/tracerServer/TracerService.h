
#ifndef TracerService_h
	#define TracerService_h

	#include "TracerServerErrors.h"
	#include "ConfigurationFile.h"
	#include "Tracer.h"
	#include "ServerSocket.h"
	#include "TracerSocketsPool.h"
	#include "Service.h"

	#define TS_MAXLONGLENGTH					128
	#define TS_MAXTRACEFILELENGTH				256


	#define TS_TRACERSERVICE_SERVICENAME	\
		"tracerServer"

	#define TS_TRACERSERVICE_CONFPATHNAMEENVIRONMENTVARIABLE	\
		"TRACERSERVER_CONF_PATHNAME"

	#ifdef WIN32
	#else
		#define TS_TRACERSERVICE_PIDFILEPATHNAME	\
			"/tmp/tracerServer.pid"
	#endif


	class TracerService: public Service
	{
		private:
			char						*_pConfigurationPathName;
			char						*_pPidPathName;
			char						*_pServiceVersion;

			ConfigurationFile_t			_cfConfiguration;
			Tracer_t					_tTracer;
			time_t						_tServerStartTime;

			char						_pLocalIPAddress [
				SCK_MAXIPADDRESSLENGTH];
			unsigned long				_ulServerPort;
			ServerSocket_t				_ssServerSocket;
			TracerSocketsPool_t			_tspTracerSocketsPool;

			// unsigned long				_ulTimeoutToWaitRequestInSeconds;
			// unsigned long				_ulAdditionalMicrosecondsToWait;


			Error finishTheService (void);

			Error startTracer (
				ConfigurationFile_p pcfConfiguration,
				Tracer_p ptTracer, const char *pSectionName);

			Error stopTracer (Tracer_p ptTracer);


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

			virtual Error onStop (void);

			virtual Error onStart (void);

		public:
			TracerService (void);

			~TracerService (void);

			Error init (const char *pServiceName,
				const char *pServiceVersion,
				const char *pConfigurationPathName,
				const char *pPidPathName);

			Error finish (void);

			Error cancel (void);
	} ;

	typedef class TracerService
		TracerService_t, *TracerService_p;

#endif

