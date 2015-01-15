
#ifndef CheckSocketsPoolTimes_h
	#define CheckSocketsPoolTimes_h

	#include "Times.h"
	#include "CMSocketsPool.h"
	#include "Tracer.h"

	#define CM_CHECKSOCKETSPOOLTIMES_PERIODINMILLISECS	200
	#define CM_CHECKSOCKETSPOOLTIMES_CLASSNAME			"CheckSocketsPoolTimes"
	#define CM_CHECKSOCKETSSTATUS_SECONDSTOWAIT			1
	#define CM_CHECKSOCKETSSTATUS_MICROSECONDSTOWAIT	0

	#define CM_CHECKSOCKETSPOOLTIMES_SOURCE				"CheckSocketsPoolTimes"

	#define CM_EVENT_CHECKSOCKETSPOOL					2


	typedef class CheckSocketsPoolTimes: public Times

	{
		protected:
			CMEventsSet_p					_pesEventsSet;
			Tracer_p						_ptSystemTracer;
			Buffer_t						_bMainProcessor;

			CheckSocketsPoolTimes (const CheckSocketsPoolTimes &t);

		public:
			CheckSocketsPoolTimes (void);

			virtual ~CheckSocketsPoolTimes (void);

			Error init (
				unsigned long ulPeriodInMilliSecs,
				CMEventsSet_p pesEventsSet,
				Tracer_p ptTracer);

			virtual Error finish (void);

			virtual Error handleTimeOut (void);

	} CheckSocketsPoolTimes_t, *CheckSocketsPoolTimes_p;

#endif

