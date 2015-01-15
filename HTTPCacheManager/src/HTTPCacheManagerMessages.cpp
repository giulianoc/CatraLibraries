
#include "HTTPCacheManagerMessages.h"


ErrMsgBase:: ErrMsgsInfo HTTPCacheManagerMessagesStr = {

    // HTTPCacheManager
	{ HCM_HTTPCACHEMANAGER_HTTPGETINITIALIZATION,
		"HTTPRequest ID: %lu. Initialization HTTP GET thread. WebServerIPAddress: %s, WebServerPort: %lu, LocalIPAddress: %s, URI: %s, URL parameters: %s, Timeout: %lu" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTWAITINGTOBEUPLOADED,
		"HTTPRequest ID: %lu. HTTPRequest Waiting to be uploaded. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s, URL parameters: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTRUNNING,
		"HTTPRequest ID: %lu. HTTPRequest Running. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s, URL parameters: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPGETRUNNING,
		"Running HTTP GET thread. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPGETALREADYRUNNING,
		"HTTP GET thread already running. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPGETALREADYINWAITINGQUEUE,
		"HTTP GET thread already in the waiting queue. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPGETWAITING,
		"Waiting HTTP GET thread. URL: %s" },
	{ HCM_HTTPCACHEMANAGER_ALREADYINCACHE,
		"URL %s is already in cache (%s)" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYADDEDTOHASHMAP,
		"HTTPRequestKey %s added into the HashMap associated to the HTTPRequestURL %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYADDEDTOMULTIMAP,
		"HTTPRequestKey %s added into the MultiMap associated to the HTTPRequestURL %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYREMOVEDFROMMULTIMAP,
		"HTTPRequestKey %s removed from the MultiMap. Associated HTTPRequestURL: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYREMOVEDFROMHASHMAP,
		"HTTPRequestKey %s removed from the HashMap. Associated HTTPRequestURL: %s" },
	{ HCM_HTTPCACHEMANAGER_FOUNDHTTPREQUESTKEY,
		"Found HTTPRequestKey %s associated to the HTTPRequestURL %s. IsHTTPRequestSuccess: %ld" },
	{ HCM_HTTPCACHEMANAGER_REACHEDMAXCONCURRENCY,
		"Reached the HTTPRequests Max Concurrency: %lu" },

	// CacheSaveHttpGetThread
	{ HCM_CACHESAVEHTTPGETTHREAD_HTTPREQUESTFINISHED,
		"HTTPRequest ID: %lu. HTTPRequest Finished. WebServerIPAddress: %s, WebServerPort: %lu, URI: %s" },

	// Insert here other errors...

} ;

