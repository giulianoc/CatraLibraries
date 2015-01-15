
#ifndef BaseProcessor_h
	#define BaseProcessor_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "CMEventsSet.h"
	#include "Scheduler.h"
	#include "ConfigurationFile.h"
	#include "Tracer.h"
	// #include "Session.h"

	#ifdef WIN32
		#define CM_PROCESSOR_SLEEPTIMESECSWAITINGEVENT		1
		#define CM_PROCESSOR_SLEEPTIMEMILLISECSWAITINGEVENT	0
	#else
		#define CM_PROCESSOR_SLEEPTIMESECSWAITINGEVENT		1
		#define CM_PROCESSOR_SLEEPTIMEMILLISECSWAITINGEVENT	0
	#endif

	#define CM_PROCESSOR_MAXTYPEIDENTIFIERLENGTH		128

	#define CM_MAXTRACEFILELENGTH					(1024 + 1)
	#define CM_MAXLONGLENGTH						(256 + 1)
	#define CM_MAXBOOLEANLENGTH						(256 + 1)


	#ifdef WIN32
		typedef class BaseProcessor: public WinThread
	#else
		typedef class BaseProcessor: public PosixThread
	#endif
	{
		private:
			PMutex_t					_mtShutdown;
			Boolean_t					_bIsShutdown;


			Error getIsShutdown (Boolean_p pbIsShutdown);

			Error setIsShutdown (Boolean_t bIsShutdown);

		protected:
			ConfigurationFile_p			_pcfConfiguration;

			unsigned long				_ulProcessorIdentifier;
			Buffer_t					_bProcessorName;
			Scheduler_p					_pscScheduler;
			CMEventsSet_p				_pesEventsSet;
			unsigned long				_ulMaxMilliSecondsToProcessAnEvent;
			Tracer_p					_ptSystemTracer;

			Buffer_t					_bMainProcessor;


			virtual Error run (void);

			virtual Error processEvent (Event_p pevEvent,
				unsigned long *pulInternalTimeInMilliSecsToProcessEvent) = 0;

		public:
			BaseProcessor (void);

			~BaseProcessor (void);

			virtual Error init (
				unsigned long ulProcessorIdentifier,
				const char *pProcessorName,
				ConfigurationFile_p pcfConfiguration,
				Scheduler_p pscScheduler,
				CMEventsSet_p pesEventsSet,
				unsigned long ulMaxMilliSecondsToProcessAnEvent,
				Tracer_p ptTracer);

			virtual Error finish ();

			virtual Error cancel (void);

			static Error startTracer (
				ConfigurationFile_p pcfConfiguration,
				Tracer_p ptTracer, const char *pSectionName);

			static Error stopTracer (Tracer_p ptTracer);

	} BaseProcessor_t, *BaseProcessor_p;

#endif

