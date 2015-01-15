
#ifndef TracerSocketsPool_h
	#define TracerSocketsPool_h

	#include "SocketsPool.h"
	#include "ClientSocket.h"
	#include "TracerServerErrors.h"
	#include "Tracer.h"

	#define TS_TRACERSOCKETSPOOL_MAXSOCKETSNUMBER		(1 + 256)


	#define TS_TRACERSOCKETPOOL_SERVERSOCKET			1
	#define TS_TRACERSOCKETPOOL_CLIENTSOCKET			2

	#define TS_TRACERSOCKETPOOL_MAXTRACELENGTH			(256 + 1)


	typedef class TracerSocketsPool: public SocketsPool

	{
		private:
			Tracer_p					_ptTracer;

			Error readFromClientSocket (
				ClientSocket_p pcsClientSocket,
				Boolean_t bClientSocketToBeAddedInSocketsPool);

		protected:
			TracerSocketsPool (const TracerSocketsPool &t);

			virtual Error updateSocketStatus (Socket_p pSocket,
				long lSocketType, void *pvSocketData,
				unsigned short usSocketCheckType);

		public:
			TracerSocketsPool (void);

			~TracerSocketsPool (void);        

			Error init (
				Tracer_p ptTracer);

			Error finish (void);

	} TracerSocketsPool_t, *TracerSocketsPool_p;

#endif

