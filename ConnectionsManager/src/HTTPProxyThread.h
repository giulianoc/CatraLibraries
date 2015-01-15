
#ifndef HTTPProxyThread_h
	#define HTTPProxyThread_h

	#include "HttpGetThread.h"
	#include "Tracer.h"
	#include "Session.h"
	#include "ConnectionsManagerErrors.h"


	typedef class HTTPProxyThread: public HttpGetThread
	{
		private:
			Session_p				_psSession;
			time_t					_tUTCStartConnectionTime;
			Tracer_p				_ptSystemTracer;
			unsigned long			_ulHeaderDataSizeSentToClient;
			unsigned long long		_ullBodyDataSizeSentToClient;


		protected:
			virtual Error closingHttpGet (Error_p perr);

		public:
			HTTPProxyThread (void);

			~HTTPProxyThread (void);

			Error init (
				Session_p psSession,
				time_t tUTCStartConnectionTime,
				const char *pWebServerIPAddress,
				unsigned long ulWebServerPort,
				const char *pURLWithoutParameters,
				const char *pURLParameters,
				unsigned long ulTimeoutInSecs,
				const char *pLocalIPAddress,
				Tracer_p ptSystemTracer);

			virtual Error_t chunkRead (unsigned long ulChunkReadIndex,
				long long llTotalContentLength, const char *pContentType,
				unsigned char *pucBuffer, unsigned long ulBufferDataSize);

			static Error sendErrorToHTTPClient (
				Session_p psSession, time_t tUTCStartConnectionTime,
				Error_p perr, Tracer_p ptSystemTracer);

	} HTTPProxyThread_t, *HTTPProxyThread_p;

#endif

