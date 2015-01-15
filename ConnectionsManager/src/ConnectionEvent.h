
#ifndef ConnectionEvent_h
	#define ConnectionEvent_h

	#include "Event.h"
	#include "Tracer.h"
	#include "Session.h"
	#include "ConnectionsManagerErrors.h"

	#define CM_EVENT_CONNECTIONREADYTOREAD			1


	typedef class ConnectionEvent: public Event

	{

		private:
			Boolean_t				_bConnectionInformationInitialized;
			Session:: ConnectionInformation_t
				_ciConnectionInformation;

		protected:
			Tracer_p			_ptSystemTracer;


			ConnectionEvent (const ConnectionEvent &);

			ConnectionEvent &operator = (
				const ConnectionEvent &);

		public:
			ConnectionEvent (void);

			virtual ~ConnectionEvent (void);

			Error init (
				const char *pSource,
				const char *pdestination,
				long lTypeIdentifier,
				const char *pTypeIdentifier,
				Session:: ConnectionInformation_p
				pciConnectionInformation,
				Tracer_p ptTracer);

			virtual Error finish (void);

			Error getConnectionInformation (
				Session:: ConnectionInformation_p
				pciConnectionInformation);

	} ConnectionEvent_t, *ConnectionEvent_p;

#endif

