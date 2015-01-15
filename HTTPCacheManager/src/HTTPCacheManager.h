
#ifndef HTTPCacheManager_h
	#define HTTPCacheManager_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "PMutex.h"
	#include "CacheSaveHttpGetThread.h"
	#include "Tracer.h"
	#include "HTTPCacheManagerErrors.h"
	#include "my_hash_map.h"
	#include <map>
	#include <vector>


	#ifdef WIN32
		typedef class HTTPCacheManager: public WinThread
	#else
		typedef class HTTPCacheManager: public PosixThread
	#endif
	{
		private:
			typedef struct HTTPRequest {
				unsigned long		_ulIdentifier;

				Buffer_t			_bHTTPRequestURL;

				Buffer_t			_bHTTPHost;
				long				_lHTTPPort;
				Buffer_t			_bURI;
				Buffer_t			_bURLParameters;

				Buffer_t			_bHTTPLocalPathName;
			} HTTPRequest_t, *HTTPRequest_p;

			typedef struct HTTPRequestKeyInfo {
				Buffer_t			_bHTTPRequestKey;
				Buffer_t			_bHTTPRequest;

				Boolean_t			_bIsHTTPRequestSuccess;

				time_t				_tTimestamp;
			} HTTPRequestKeyInfo_t, *HTTPRequestKeyInfo_p;

			typedef my_hash_map<Buffer_p, HTTPRequestKeyInfo_p,
				BufferHasher, BufferCmp>
				HTTPRequestKeysHashMap_t, *HTTPRequestKeysHashMap_p;

			struct ltstr
			{
				bool operator() (time_t tUTCEndTimeInSecs1,
					time_t tUTCEndTimeInSecs2) const
				{
					return tUTCEndTimeInSecs1 < tUTCEndTimeInSecs2;
				}
			};

			typedef std:: multimap<time_t, HTTPRequestKeyInfo_p, ltstr>
				HTTPRequestKeysMultiMap_t, *HTTPRequestKeysMultiMap_p;

		private:
			Tracer_p					_ptSystemTracer;

			char						_pLocalIPAddress [
				SCK_MAXHOSTNAMELENGTH];
			unsigned long				_ulTimeoutToWaitAnswerInSeconds;

			Buffer_t					_bLocalPathName;
			Buffer_t					_bCacheDirectory;
			unsigned long				_ulCacheRetentionInHours;
			time_t						_tLastTimeRetentionCheckDone;

			unsigned long				_ulHTTPRequestsQueueSize;
			HTTPRequest_p				_phrHTTPRequests;
			std:: vector<HTTPRequest_p>	_vAvailableHTTPRequests;
			std:: vector<HTTPRequest_p>	_vWaitingQueueHTTPRequests;

			unsigned long				_ulMaxConcurrentHTTPRequest;
			CacheSaveHttpGetThread_p	_pshgtSaveHttpGetThread;

			BufferHasher_p				_phHasher;
			BufferCmp_p					_pcComparer;
			HTTPRequestKeysHashMap_p	_phmHTTPRequestKeys;
			HTTPRequestKeysMultiMap_t	_mmHTTPRequestKeys;
			unsigned long				_ulRetentionInSecondsForHTTPRequestKeys;

			PMutex_t					_mtShutdown;
			PMutex_t					_mtHTTPRequests;
			Boolean_t					_bIsShutdown;

			Error getIsShutdown (Boolean_p pbIsShutdown);

			Error setIsShutdown (Boolean_t bIsShutdown);

			Error getLocalPathName (
				Buffer_p pbHTTPRequest,
				Buffer_p pbHTTPLocalPathName);

			Error isLocalPathNamePresentInCache (
				Buffer_p pbHTTPLocalPathName,
				Boolean_p pbAlreadyInCache);

			Error saveHTTPRequestKey (
				Buffer_p pbHTTPRequestKey,
				Buffer_p pbHTTPRequest,
				Boolean_t bIsHTTPRequestSuccess);

			Error HTTPRequestKeysRetention (void);

			Error logHTTPRequestsRunning (void);

			Error logHTTPRequestsWaiting (void);

		protected:
			virtual Error run (void);

		public:
			HTTPCacheManager (void);

			~HTTPCacheManager (void);

			Error init (
				const char *pCacheDirectory,
				unsigned long ulCacheRetentionInHours,
				unsigned long ulMaxConcurrentHTTPRequest,
				unsigned long ulHTTPRequestsQueueSize,
				const char *pLocalIPAddress,
				unsigned long ulTimeoutToWaitAnswerInSeconds,
				unsigned long ulRetentionInSecondsForHTTPRequestKeys,
				Tracer_p ptSystemTracer);

			virtual Error finish (void);

			virtual Error cancel (void);

			/*
			 * First check if the pbHTTPRequest is already in the cache, then,
			 * if not present, download it and associate it to pbHTTPRequestKey.
			 *	pbHTTPRequest: it is the URL
			 *	pbHTTPRequestKey: it is a key used by HTTPCacheManager to save
			 *		the associated pbHTTPRequest. This association is temporary
			 *		and will be forgot by HTTPCacheManager after
			 *		_ulRetentionInSecondsForHTTPRequestKeys.
			 *		This parameter could be also NULL and, in this case,
			 *		the Key is not saved at all
			 *	pbHTTPLocalPathName: output parameter where it is saved the
			 *		pathname of the content in the cache
			 *	pbAlreadyInCache: output parameter telling if the content
			 *		is in the cache or not
			 */
			Error getLocalPathNameAndCacheContent (
				Buffer_p pbHTTPRequest,
				Buffer_p pbHTTPRequestKey,
				Buffer_p pbHTTPLocalPathName,
				Boolean_p pbAlreadyInCache);

			Error getLocalPathNameUsingHTTPRequestKey (
				Buffer_p pbHTTPRequestKey,
				Buffer_p pbHTTPLocalPathName,
				Boolean_p pbAlreadyInCache);

			Error markHTTPRequestKeyAsFailed (
				Buffer_p pbHTTPRequestKey);

	} HTTPCacheManager_t, *HTTPCacheManager_p;

#endif

