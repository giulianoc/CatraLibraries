
#ifndef Session_h
	#define Session_h

	#include "CMSocketsPool.h"
	#include "ServerSocket.h"
	#include "Tracer.h"

	#define CM_SESSION_HTTPNEWLINE						"\r\n"
	#define CM_SESSION_NEWLINE							"\n"

	typedef struct Session

	{
		public:
			typedef struct ConnectionInformation
			{
				Session					*_psSession;
				unsigned long			_ulConnectionIdentifier;
				time_t					_tUTCStartConnectionTime;
				time_t					_tUTCLastRead;
			} ConnectionInformation_t, *ConnectionInformation_p;

		public:
			char				_pClientIPAddress [SCK_MAXIPADDRESSLENGTH];
			long				_lClientPort;

		private:
			ConnectionInformation_t		_ciConnectionInformation;
			// unsigned long		_ulConnectionIdentifier;
			unsigned long		_ulTimeoutToWaitRequestInSeconds;
			unsigned long		_ulAdditionalMicrosecondsToWait;
			CMSocketsPool_p		_pspSocketsPool;
			CMEventsSet_p		_pesEventsSet;
			PMutex_t			_mtSession;
			Tracer_p			_ptSystemTracer;

			// time_t				_tUTCStartConnectionTime;
			ClientSocket_t		_csClientSocket;

		protected:
			Session (const Session &t);

		public:
			Session (void);

			virtual ~Session (void);

			Error init (
				unsigned long ulConnectionIdentifier,
				CMSocketsPool_p pspSocketsPool,
				CMEventsSet_p pesEventsSet,
				unsigned long ulTimeoutToWaitRequestInSeconds,
				unsigned long ulAdditionalMicrosecondsToWait,
				Tracer_p ptTracer);

			Error finish (void);

			Error reset (void);

			Error acceptConnection (
				ServerSocket_p pssServerSocket,
				ConnectionInformation_p *pciConnectionInformation);

			Error releaseConnection (
				Boolean_t bDeleteFromSocketsPool);

			Error isOlderThan (
				time_t *ptNowInSeconds,
				unsigned long *pulMaxTTLSession,
				Boolean_p pbIsOld, Boolean_p pbIsFreeSession);

			Error writeResponse (
				time_t *ptUTCStartConnectionTime,
				Buffer_p pbResponseHeader,
				Buffer_p pbResponseBody,
				Boolean_t bIsResponseFinished);

			Error readRequest (
				time_t *ptUTCStartConnectionTime,
				Buffer_p pbRequestHeader,
				Buffer_p pbRequestBody,
				Buffer_p pbURL,
				Buffer_p pbRequestUserAgent,
				Buffer_p pbRequestCookie);

			unsigned long getConnectionIdentifier (void);

	} Session_t, *Session_p;

#endif

