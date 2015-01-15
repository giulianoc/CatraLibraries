/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/

#ifndef ClientSocketsPool_h
	#define ClientSocketsPool_h


	#include "PMutex.h"
	#include "Buffer.h"
	#include "ClientSocket.h"
	#include "my_hash_map.h"
	#include <vector>
	#include "my_hash_map.h"


	typedef class ClientSocketsPool

	{

		protected:
			typedef enum ClientSocketsPoolStatus {
				CSP_BUILDED,
				CSP_INITIALIZED
			} ClientSocketsPoolStatus_t, *ClientSocketsPoolStatus_p;

			typedef struct ClientSocketsInfo
			{
				Buffer_t						_bRemoteIPAddress;

				ClientSocket_p					_pcsClientSockets;

				std:: vector<ClientSocket_p>	_vFreeClientSockets;

			} ClientSocketsInfo_t, *ClientSocketsInfo_p;

			typedef my_hash_map<Buffer_p, ClientSocketsInfo_p,
				BufferHasher, BufferCmp>
				ClientSocketsInfoHashMap_t, *ClientSocketsInfoHashMap_p;

		protected:
			ClientSocketsPoolStatus_t	_cssClientSocketsPoolStatus;

			BufferHasher_p				_phHasher;
			BufferCmp_p					_pcComparer;
			ClientSocketsInfoHashMap_p	_phmClientSocketsInfo;
			PMutex_t					_mtClientSocketsInfo;

			unsigned long				_ulReceivingTimeoutInSeconds;
			unsigned long			_ulReceivingAdditionalTimeoutInMicroSeconds;
			unsigned long				_ulSendingTimeoutInSeconds;
			unsigned long			_ulSendingAdditionalTimeoutInMicroSeconds;
			Boolean_t					_bReuseAddr;
			char						_pLocalAddress [SCK_MAXIPADDRESSLENGTH];
			long						_lRemotePort;
			unsigned long				_ulClientSocketsToBeAllocatedPerType;
			long						_lTTLInSeconds;

		protected:
			ClientSocketsPool (const ClientSocketsPool &);

			ClientSocketsPool &operator = (const ClientSocketsPool &);

		public:
			ClientSocketsPool (void);

			~ClientSocketsPool (void);

			Error init (
				unsigned long ulReceivingTimeoutInSeconds,
				unsigned long ulReceivingAdditionalTimeoutInMicroSeconds,
				unsigned long ulSendingTimeoutInSeconds,
				unsigned long ulSendingAdditionalTimeoutInMicroSeconds,
				Boolean_t bReuseAddr,
				const char *pLocalAddress,
				long lRemotePort,
				unsigned long ulClientSocketsToBeAllocatedPerType,
				long lTTLInSeconds);

			Error finish (void);

			Error getFreeClientSocket (Buffer_p pbRemoteIPAddress,
				ClientSocket_p *pcsClientSocket);

			Error getFreeClientSocket (
				const char *pRemoteIPAddress,
				ClientSocket_p *pcsClientSocket);

			Error releaseClientSocket (Buffer_p pbRemoteIPAddress,
				ClientSocket_p pcsClientSocket,
				Boolean_t bSocketToBeClosed = false);

			Error releaseClientSocket (
				const char *pRemoteIPAddress,
				ClientSocket_p pcsClientSocket,
				Boolean_t bSocketToBeClosed = false);

	} ClientSocketsPool_t, *ClientSocketsPool_p;

#endif

