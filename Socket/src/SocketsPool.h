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


#ifndef SocketsPool_h
	#define SocketsPool_h

	/*
	#ifdef WIN32
	#else
		#include <CatraLibraries_OSConfig.h>
		#ifdef HAVE_POLL
			#include <sys/poll.h>
		#endif
	#endif
	*/

	#include "SocketErrors.h"
	#include "SocketImpl.h"
	#include "Socket.h"
	#ifdef _REENTRANT
		#ifdef WIN32
			#include "WinThread.h"
		#else
			#include "PosixThread.h"
		#endif
		#include "PMutex.h"
	#endif
	#include "my_hash_map.h"
	#include <vector>


	/**
		This class implements a pool of sockets.
		Here is the steps to use this class:
			1. initialize the instance (init method)
			2. add the sockets to be monitored (addSocket method)
			3. call checkSocketsStatus every time you have to check the status
				of the sockets belonging to the pool

		The step no. 3 must be done every time you need to check the status
		of the sockets.

		The SocketsPool could be also used as a thread. In that way, himself
		could call the checkSocketsStatus method on your behalf
		according a period specified by the ulCheckSocketsPoolPeriodInSeconds
		and ulAdditionalCheckSocketsPoolPeriodInMilliSeconds parameters of the
		init method. If you need to use the SocketsPool as a thread the steps
		will be:
			1. initialize the instance (init method)
			2. add the sockets to be monitored (addSocket method)
			3. call the start method to run the thread. It will call
				automaticaly, according the period specified
				by the parameters of the init, the checkSocketsStatus method.
	*/
		#ifdef WIN32
			typedef class SocketsPool: public WinThread
		#else
			typedef class SocketsPool: public PosixThread
		#endif
		{
		private:
			typedef enum SocketsPoolStatus {
				SOCKETSPOOL_BUILDED,
				SOCKETSPOOL_INITIALIZED
			} SocketsPoolStatus_t, *SocketsPoolStatus_p;

			typedef struct SocketInfo
			{
				Socket_p				_psSocket;
				void					*_pvData;
				unsigned short			_usSocketCheckType;
				long					_lSocketType;

				// To avoid to include the CatraLibraries_OSConfig.h
				// in an header file (that will cause many redefined warnings
				// during the compilation of the catrastreaming project due to
				// the CatraStreaming_OSConfig.h header)
				// we have to declare as 'void *' the _ppfDescriptor private
				// member
				// #ifdef HAVE_POLL
					// important for deleteSocket and checkSocketsStatus
					// struct pollfd			*_ppfDescriptor;
					void						*_ppfDescriptor;
				// #endif
			} SocketInfo_t, *SocketInfo_p;

			typedef my_hash_map<int, SocketInfo_p, IntHasher, IntCmp>
				SocketsInfoHashMap_t, *SocketsInfoHashMap_p;

			typedef struct SocketToBeUpdated
			{
				unsigned short			_usLocalSocketCheckType;
				SocketInfo_p			_psiSocketInfo;
			} SocketToBeUpdated_t, *SocketToBeUpdated_p;

		public:
			typedef enum CheckSocketStatusType {
				SOCKETSTATUS_READ		= 0x01,
				SOCKETSTATUS_WRITE		= 0x02,
				SOCKETSTATUS_EXCEPTION	= 0x04
			} CheckSocketStatusType_t, *CheckSocketStatusType_p;

		private:
			SocketsPoolStatus_t		_ssSocketsPoolStatus;
			IntHasher_p				_phHasher;
			IntCmp_p				_pcComparer;
			SocketsInfoHashMap_p	_psiSocketsInfoSetByFileDescriptor;

			// next hash map is used to save the port for the sockets
			// by his local port in order to find it subsequently in a fast way
			// It cannot be applicable to the client socket opened through
			// the accept method, because his local port will be -1
			SocketsInfoHashMap_p	_psiSocketsInfoSetBySocketPort;
			unsigned long			_ulMaxSocketsNumber;
			#ifdef _REENTRANT
				PMutex_t			_mtSocketsPool;
				PMutex_t			_mtShutdown;
				Boolean_t			_bIsShutdown;
				unsigned long		_ulCheckSocketsPoolPeriodInSeconds;
				unsigned long
					_ulAdditionalCheckSocketsPoolPeriodInMilliSeconds;
			#endif

			Boolean_t				_bAllowDeletionSocketFromUpdateMethod;
			SocketInfo_p			_psiSocketsInfo;
			std:: vector<SocketInfo_p>	_vFreeSocketsInfo;

			// To avoid to include the CatraLibraries_OSConfig.h
			// in an header file (that will cause many redefined warnings
			// during the compilation of the catrastreaming project)
			// we have to declare as 'void *' the _ppfDescriptors private member
			// #ifdef HAVE_POLL
				// struct pollfd		*_ppfDescriptors;
				void					*_ppfDescriptors;
			// #endif

			Error setIsShutdown (Boolean_t bIsShutdown);

			Error getIsShutdown (Boolean_p pbIsShutdown);

		protected:
			SocketsPool (const SocketsPool &t);

			virtual Error run (void);

			/**
				called when checkSocketsStatus is called and the socket
				updates his status.

				Only if the bAllowDeletionSocketFromUpdateMethod parameter
				in init method is true, inside
				updateSocketStatus it is possible to call
				deletedSocket (pSocket).
				The reason is that if inside updateSocketStatus will not be
				used the deleteSocket method, the algorithm can be more
				efficient because a loop could be completely skipped.
			*/
			virtual Error updateSocketStatus (Socket_p psSocket,
				long lSocketType, void *pvSocketData,
				unsigned short usSocketCheckType) = 0;

		public:
			SocketsPool (void);

			~SocketsPool (void);        

			/**
				Initialize the Sockets Pool.
				ulMaxSocketsNumber - the max number of Sockets this class
					can manage
				ulCheckSocketsPoolPeriodInSeconds - if it is called the start
					method to run the SocketsPool thread, this parameter
					represents the period to call the
					checkSocketsStatus. Otherwise it is not used.
				ulAdditionalCheckSocketsPoolPeriodInMilliSeconds -
					additional MilliSeconds to be added to the above parameter.
				bAllowDeletionSocketFromUpdateMethod - If true inside the 
					updateSocketStatus method it is possible to call
					deleteSocket, if it is false you cannot delete a socket
					from the updateSocketStatus method.
					The reason we have this flag is that if inside
					updateSocketStatus will not be
					used the deleteSocket method, the algorithm can be more
					efficient because a loop could be completely skipped.
			*/
			Error init (unsigned long ulMaxSocketsNumber,
				unsigned long ulCheckSocketsPoolPeriodInSeconds,
				unsigned long ulAdditionalCheckSocketsPoolPeriodInMilliSeconds,
				Boolean_t bAllowDeletionSocketFromUpdateMethod);

			Error finish (void);

			virtual Error cancel (void);

			/**
				The checkSocketsStatus method checks if there is a socket
				with the status changed (according the usSocketCheckType
				parameter of the addSocket method) and,
				only in case it is changed, call the virtual updateSocketStatus
				method.
				This method returns the SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED
				error if, during the period specified by the two parameters,
				the sockets belonging to the pool are not changed.
			*/
			Error checkSocketsStatus (
				unsigned long ulSecondsToWait,
				unsigned long ulAdditionalMilliSecsToWait);

			/**
				Add the socket (pSocket parameter) to the Sockets Pool
				specifying if must be checked for reading, writing or
				for exception (usSocketCheckType parameter).

				- usSocketCheckType is an OR between SOCKETSTATUS_READ,
					SOCKETSTATUS_WRITE and SOCKETSTATUS_EXCEPTION
				- lSocketType. Since you could add to the Sockets Pool many
					different type of Sockets, this parameter could be used
					by the derived class to understand which type of Socket
					changed his status.
				- pSocket - socket to be observed
				- pvSocketData - pointer to data associated with the socket.
					This class just save the pointer.
			*/
			Error addSocket (
				unsigned short usSocketCheckType,
				long lSocketType,
				Socket_p pSocket, void *pvSocketData);

			/**
				Delete the socket from the SocketsPool.
				- pSocket - socket to be deleted from the SocketsPool
				- pvSocketData - output parameter returning the data
					associated to the Socket. Could be also NULL.
			*/
			Error deleteSocket (
				Socket_p pSocket, void **pvSocketData);

			Error findServerSocket (
				unsigned long ulServerPort,
				Socket_p *psSocket, void **pvSocketData);

	} SocketsPool_t, *SocketsPool_p;

#endif

