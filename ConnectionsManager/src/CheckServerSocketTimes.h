
#ifndef CheckServerSocketTimes_h
	#define CheckServerSocketTimes_h

	#include "Times.h"
	#include "CMEventsSet.h"
	#include "Tracer.h"

	#define CM_CHECKSERVERSOCKETTIMES_CLASSNAME		"CheckServerSocketTimes"
	#define CM_CHECKSERVERSOCKETTIMES_SOURCE		"CheckServerSocketTimes"

	#define CM_EVENT_CONNECTIONTOACCEPT				0


	typedef class CheckServerSocketTimes: public Times

	{
		protected:
			CMEventsSet_p					_pesEventsSet;
			Tracer_p						_ptSystemTracer;
			Buffer_t						_bMainProcessor;


			CheckServerSocketTimes (const CheckServerSocketTimes &t);

		public:
			CheckServerSocketTimes (void);

			virtual ~CheckServerSocketTimes (void);

			Error init (
				unsigned long ulPeriodInMilliSecs,
				CMEventsSet_p pesEventsSet,
				Tracer_p ptTracer);

			virtual Error finish (void);

			virtual Error handleTimeOut (void);

	} CheckServerSocketTimes_t, *CheckServerSocketTimes_p;

#endif

