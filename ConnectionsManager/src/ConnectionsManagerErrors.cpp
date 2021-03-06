
#include "ConnectionsManagerErrors.h"



ErrMsgBase:: ErrMsgsInfo ConnectionsManagerErrorsStr = {

	// ConnectionsManagerService
	{ CM_CONNECTIONSMANAGERSERVICE_ONINITEVENTSSET_FAILED,
		"The method onInitEventsSet of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED,
		"The method onFinishEventsSet of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONINITPROCESSORS_FAILED,
		"The method onInitProcessors of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONFINISHPROCESSORS_FAILED,
		"The method onFinishProcessors of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONSTARTPROCESSORS_FAILED,
		"The method onStartProcessors of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONWAITPROCESSORS_FAILED,
		"The method onWaitProcessors of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED,
		"The method onCancelProcessors of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONSTARTMYTIMES_FAILED,
		"The method onStartMyTimes of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_ONSTOPMYTIMES_FAILED,
		"The method onStopMyTimes of the ConnectionsManagerService class failed" },
	{ CM_CONNECTIONSMANAGERSERVICE_SIGNALRECEIVED,
		"Received the signal: %lu, shutdown..." },

	// CMProcessor
	{ CM_CMPROCESSOR_INIT_FAILED,
		"The method init of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_FINISH_FAILED,
		"The method finish of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_SETISSHUTDOWN_FAILED,
		"The method setIsShutdown of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_GETISSHUTDOWN_FAILED,
		"The method getIsShutdown of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_PROCESSEVENT_FAILED,
		"The method processEvent of the CMProcessor class failed. Processor identifier: %lu" },
	{ CM_CMPROCESSOR_HANDLECONNECTIONTOACCEPTEVENT_FAILED,
		"The method handleConnectionToAcceptEvent of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_HANDLECONNECTIONREADYTOREADEVENT_FAILED,
		"The method handleConnectionReadyToReadEvent of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_HANDLECHECKSOCKETSPOOLEVENT_FAILED,
		"The method handleCheckSocketsPoolEvent of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_HANDLECHECKUNUSEDSOCKETSEVENT_FAILED,
		"The method handleCheckUnusedSocketsEvent of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_PROCESSRECEIVEDREQUEST_FAILED,
		"The method processReceivedRequest of the CMProcessor class failed" },
	{ CM_CMPROCESSOR_EVENTUNKNOWN,
		"Event type unknown (%ld)" },
	{ CM_CMPROCESSOR_NOTFOUNDSESSIONFREE,
		"Not found Session free" },
	{ CM_CMPROCESSOR_ALREADYANSWEREDTOCLIENT,
		"Already answered to Client" },
	{ CM_CMPROCESSOR_READREQUESTWITHOUTBODY,
		"Arrived a method without body. Header: %s" },
	{ CM_CMPROCESSOR_TOOTIMETOACCEPTANDREAD,
		"Too time to accept and read the request, network too slow (?). Time to accept and read (ms), Connections accepted): @%lu	%lu@" },
	{ CM_CMPROCESSOR_TTLSOCKETEXPIRED,
		"Connection identifier: %lu. TTL Socket expired" },
	{ CM_CMPROCESSOR_SOCKETSEXPIRED,
		"Sockets expired released: %lu. Current sockets used: %lu, FreeSessionsNumberFromVector: %lu, FreeSessionsNumberFromArray: %lu"},
	{ CM_CMPROCESSOR_TOOMANYTIMETOMANAGETHEEVENT,
		"Too many time to manage the @%s@ event. Event elapsed time in milliseconds: @%lu@ (InternalTime: @%lu@)" },
	{ CM_CMPROCESSOR_NOTOBEPROXIED,
		"Request no to be proxied: %s" },
	{ CM_CMSPROCESSOR_WRONGURIFORMAT,
		"Wrong URI: %s" },
	{ CM_CMPROCESSOR_DELETEONFREESESSIONSFAILED,
		"Delete on FreeSessions failed. Connection identifier: %ld" },
	{ CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED,
		"Insert on FreeSessions failed" },

	// CheckSocketsPoolTimes
	{ CM_CHECKSOCKETSPOOLTIMES_INIT_FAILED,
		"The method init of the CheckSocketsPoolTimes class failed" },
	{ CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED,
		"The method finish of the CheckSocketsPoolTimes class failed" },
	{ CM_CHECKSOCKETSPOOLTIMES_REACHEDMAXIMUMEVENTSTOALLOCATE,
		"Reached the maximum number of events to check the sockets pool" },

	// CheckServerSocketTimes
	{ CM_CHECKSERVERSOCKETTIMES_INIT_FAILED,
		"The method init of the CheckServerSocketTimes class failed" },
	{ CM_CHECKSERVERSOCKETTIMES_FINISH_FAILED,
		"The method finish of the CheckServerSocketTimes class failed" },
	{ CM_CHECKSERVERSOCKETTIMES_REACHEDMAXIMUMEVENTSTOALLOCATE,
		"Reached the maximum number of events to check server socket" },

	// CheckUnusedSocketsTimes
	{ CM_CHECKUNUSEDSOCKETSTIMES_INIT_FAILED,
		"The method init of the CheckUnusedSocketsTimes class failed" },
	{ CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED,
		"The method finish of the CheckUnusedSocketsTimes class failed" },
	{ CM_CHECKUNUSEDSOCKETSTIMES_REACHEDMAXIMUMEVENTSTOALLOCATE,
		"Reached the maximum number of events to check the unused sockets" },

	// ConnectionEvent
	{ CM_CONNECTIONEVENT_INIT_FAILED,
		"The method init of the ConnectionEvent class failed" },
	{ CM_CONNECTIONEVENT_FINISH_FAILED,
		"The method finish of the ConnectionEvent class failed" },
	{ CM_CONNECTIONEVENT_GETCONNECTIONINFORMATION_FAILED,
		"The method getConnectionInformation of the ConnectionEvent class failed" },
	{ CM_CONNECTIONEVENT_CONNECTIONINFORMATIONNOTINITIALIZED,
		"ConnectionInformation not initialized" },

	// Session
	{ CM_SESSION_INIT_FAILED,
		"The method init of the Session class failed" },
	{ CM_SESSION_FINISH_FAILED,
		"The method finish of the Session class failed" },
	{ CM_SESSION_RESET_FAILED,
		"The method reset of the Session class failed" },
	{ CM_SESSION_ACCEPTCONNECTION_FAILED,
		"The method acceptConnection of the Session class failed" },
	{ CM_SESSION_RELEASECONNECTION_FAILED,
		"Connection identifier: %lu. The method releaseConnection of the Session class failed" },
	{ CM_SESSION_WRITERESPONSE_FAILED,
		"Connection identifier: %lu. The method writeResponse of the Session class failed" },
	{ CM_SESSION_READREQUEST_FAILED,
		"Connection identifier: %lu. The method readRequest of the Session class failed" },
	{ CM_SESSION_NOTCONNECTIONTOACCEPT_FAILED,
		"Not client connection to accept" },
	{ CM_SESSION_RESPONSENOTSENTBECAUSEOLDSOCKET,
		"Connection identifier: %lu. Response not sent because the connection you are referring is not active anymore (closed or reused for other connection). Received start time: %lu, Local start time: %lu" },
	{ CM_SESSION_REQUESTNOTREADBECAUSEOLDSOCKET,
		"Connection identifier: %lu. Request not read because the connection you are referring is not active anymore (closed or reused for other connection). Received start time: %lu, Local start time: %lu" },
	{ CM_SESSION_CONNECTIONFAILED,
		"Connection identifier: %lu. %s" },
	{ CM_SESSION_ADDSOCKETFAILED,
		"Connection identifier: %lu. Add socket to SocketsPool failed" },
	{ CM_SESSION_DELETESOCKETFAILED,
		"Connection identifier: %lu. Delete socket to SocketsPool failed" },
	{ CM_SESSION_FINDOLDCONNECTION,
		"Connection identifier: %lu. Find old connection. Conn. duration: %lu" },

	// CMSocketsPool
	{ CM_CMSOCKETSPOOL_EXCEPTIONONSOCKET,
		"Exception on socket. Client IP address: %s, Socket type: %ld" },

	// HTTPProxyThread
	{ CM_HTTPPROXYTHREAD_INIT_FAILED,
		"The method init of the HTTPProxyThread class failed" },
	{ CM_HTTPPROXYTHREAD_FINISH_FAILED,
		"The method finish of the HTTPProxyThread class failed" },
	{ CM_HTTPPROXYTHREAD_SENDERRORTOHTTPCLIENT_FAILED,
		"The method sendErrorToHTTPClient of the HTTPProxyThread class failed" },
	{ CM_HTTPPROXYTHREAD_HTTPGETINTERNALERROR,
		"HTTPProxy internal error. Error: %s" },

	// common
	{ CM_ACTIVATION_WRONG,
		"Activation wrong" },
	{ CM_CONFIGITEMWRONG,
		"Configuration item value wrong. Section: %s, item: %s" },
	{ CM_WRONGSOAPMETHODCONFIGURATION,
		"The '%s' plugin is configured using the wrong SOAP method" },
	{ CM_SSCANF_FAILED,
		"sscanf failed" },
	{ CM_NEW_FAILED,
		"new failed" }

	// Insert here other errors...

} ;

