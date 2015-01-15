
#ifndef CMProcessor_h
	#define CMProcessor_h

	#include "BaseProcessor.h"
	#include "ClientSocket.h"
	#include "ConnectionsManagerErrors.h"
	#include "ConnectionEvent.h"
	#include "LoadBalancer.h"
	#include "my_hash_map.h"
	#include <vector>


	#define CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH			512
	#define CM_CMPROCESSOR_CHECKSOCKETSSECONDSTOWAIT			0
	#define CM_CMPROCESSOR_CHECKSOCKETSMILLISECONDSTOWAIT		5


	#ifdef VECTORFREESESSIONS
	#else
		typedef my_hash_map<int, Session_p,
			IntHasher, IntCmp>
			SessionsHashMap_t, *SessionsHashMap_p;
	#endif

	class CMProcessor: public BaseProcessor
	{
		private:
			Boolean_t					_bIsHTTPProxyEnabled;
			char						_pURIHTTPProxyPrefix [
				CM_CMPROCESSOR_MAXCONFIGURATIONITEMLENGTH];
			ConfigurationSection_t		_csHTTPProxyCfgSection;
			Buffer_t					_bURIForHTTPProxy;
			Buffer_t					_bURLParametersForHTTPProxy;
			Buffer_t					_bHTTPSessionKey;

			unsigned long				_ulMaxSimultaneousConnectionsToAccept;
			unsigned long				_ulMaxDelayAcceptableInLoopInMilliSecs;

			// this mutex is used to sequentialize the activities on
			// the _pvFreeStreamerSessions vector and to 
			// sequentialize also the acceptStreamerConnection and
			// releaseStreamerConnection of the StreamerSession.
			// That because if a Streamer session is added into the vector
			// of the free sessions, it has to be release and viceversa.
			PMutex_t					*_pmtFreeSessions;
			#ifdef VECTORFREESESSIONS
				std:: vector<Session_p>		*_pvFreeSessions;
			#else
				SessionsHashMap_p			_phmFreeSessions;
			#endif
			Session_p					_psSessions;
			unsigned long				_ulMaxConnections;
			unsigned long				_ulMaxConnectionTTLInSeconds;

			CMSocketsPool_p				_pspSocketsPool;
			ServerSocket_p				_pssServerSocket;

			Buffer_t					_bURL;
			Buffer_t					_bRequestHeader;
			Buffer_t					_bRequestBody;
			Buffer_t					_bRequestUserAgent;
			Buffer_t					_bRequestCookie;


			Error handleConnectionToAcceptEvent (
				Event_p pevEvent);

			Error handleConnectionReadyToReadEvent (
				ConnectionEvent_p pevConnectionEvent,
				unsigned long *pulInternalTimeInMilliSecsToProcessEvent);

			Error handleCheckSocketsPoolEvent (Event_p pevEvent);

			Error handleCheckUnusedSocketsEvent (Event_p pevEvent);

			Error manageHTTPProxyRequest (Buffer_p pbURL,
				Buffer_p pbRequestHeader, Buffer_p pbRequestBody,
				Buffer_p pbRequestUserAgent, Buffer_p pbRequestCookie,
				Session_p psSession, time_t tUTCStartConnectionTime,
				Boolean_p pbSocketToBeClosedInCaseOfError);


		protected:
			LoadBalancer_p				_plbHTTPProxyLoadBalancer;
			char						_pWebServerLocalIPAddress [
				SCK_MAXHOSTNAMELENGTH];
			unsigned long			_ulWebServerTimeoutToWaitAnswerInSeconds;


			virtual Error processEvent (Event_p pevEvent,
				unsigned long *pulInternalTimeInMilliSecsToProcessEvent);

			virtual Error processReceivedRequest (Buffer_p pbURL,
				Buffer_p pbRequestHeader, Buffer_p pbRequestBody,
				Buffer_p pbRequestUserAgent, Buffer_p pbRequestCookie,
				Session_p psSession, time_t tUTCStartConnectionTime,
				Boolean_p pbSocketToBeClosedInCaseOfError);


		public:
			CMProcessor (void);

			~CMProcessor (void);

			#ifdef VECTORFREESESSIONS
				virtual Error init (
					unsigned long ulProcessorIdentifier,
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
					Tracer_p ptTracer);
			#else
				virtual Error init (
					unsigned long ulProcessorIdentifier,
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
					Tracer_p ptTracer);
			#endif

			virtual Error finish ();
	} ;

	typedef class CMProcessor
		CMProcessor_t, *CMProcessor_p;

#endif

