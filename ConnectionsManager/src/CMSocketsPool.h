
#ifndef CMSocketsPool_h
	#define CMSocketsPool_h

	#include "SocketsPool.h"
	// #include "ServerSocket.h"
	#include "CMEventsSet.h"
	#include "Tracer.h"

	#define CM_CMSOCKETSPOOL_MAXSOCKETSNUMBER		(1024 * 10)

	#define CM_CMSOCKETSPOOL_SOURCE				"CMSocketsPool"

	#define CM_CMSOCKETPOOL_CMSOCKET				1



	typedef class CMSocketsPool: public SocketsPool

	{
		private:
			CMEventsSet_p				_pesEventsSet;
			Buffer_t					_bMainProcessor;
			Tracer_p					_ptSystemTracer;

		protected:
			CMSocketsPool (const CMSocketsPool &t);

			virtual Error updateSocketStatus (Socket_p pSocket,
				long lSocketType, void *pvSocketData,
				unsigned short usSocketCheckType);

		public:
			CMSocketsPool (void);

			~CMSocketsPool (void);        

			Error init (
				CMEventsSet_p pesEventsSet,
				Tracer_p ptTracer);

			Error finish (void);

	} CMSocketsPool_t, *CMSocketsPool_p;

#endif

