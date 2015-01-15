
#ifndef CheckUnusedSocketsTimes_h
	#define CheckUnusedSocketsTimes_h

	#include "Times.h"
	#include "CMEventsSet.h"
	#include "Tracer.h"

	#define CM_CHECKUNUSEDSOCKETSTIMES_CLASSNAME	"CheckUnusedSocketsTimes"
	#define CM_CHECKUNUSEDSOCKETSTIMES_SOURCE		"CheckUnusedSocketsTimes"


	#define CM_EVENT_CHECKUNUSEDSOCKETS					3


	typedef class CheckUnusedSocketsTimes: public Times

	{
		protected:
			Buffer_t						_bMainProcessor;
			CMEventsSet_p					_pesEventsSet;
			Tracer_p						_ptSystemTracer;

			CheckUnusedSocketsTimes (const CheckUnusedSocketsTimes &t);

		public:
			CheckUnusedSocketsTimes (void);

			virtual ~CheckUnusedSocketsTimes (void);

			Error init (
				unsigned long ulPeriodInMilliSecs,
				CMEventsSet_p pesEventsSet,
				Tracer_p ptTracer);

			virtual Error finish (void);

			virtual Error handleTimeOut (void);

	} CheckUnusedSocketsTimes_t, *CheckUnusedSocketsTimes_p;

#endif

