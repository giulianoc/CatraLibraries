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


#include "HTTPCacheManagerErrors.h"


ErrMsgBase:: ErrMsgsInfo HTTPCacheManagerErrorsStr = {

	// HTTPCacheManager
	{ HCM_HTTPCACHEMANAGER_INIT_FAILED,		// 0
		"The init function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_FINISH_FAILED,
		"The finish function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_MARKHTTPREQUESTKEYASFAILED_FAILED,
		"The markHTTPRequestKeyAsFailed function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYISMARKEDASFAILED,
		"The HTTPRequestKey %s is marked as failed" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYSRETENTION_FAILED,
		"The HTTPRequestKeysRetention function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSRUNNING_FAILED,
		"The logHTTPRequestsRunning function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSWAITING_FAILED,
		"The logHTTPRequestsWaiting function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_GETLOCALPATHNAMEANDCACHECONTENT_FAILED,
		"The getLocalPathNameAndCacheContent function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_GETLOCALPATHNAMEUSINGHTTPREQUESTKEY_FAILED,
		"The getLocalPathNameUsingHTTPRequestKey function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_GETLOCALPATHNAME_FAILED,
		"The getLocalPathName function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_ISLOCALPATHNAMEPRESENTINCACHE_FAILED,
		"The isLocalPathNamePresentInCache function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED,
		"The saveHTTPRequestKey function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_SETISSHUTDOWN_FAILED,
		"The setIsShutdown function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_GETISSHUTDOWN_FAILED,
		"The getIsShutdown function of the HTTPCacheManager class failed" },
	{ HCM_HTTPCACHEMANAGER_WRONGURI,
		"Wrong URI: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTALREADYINTHEQUEUE,
		"HTTPRequest already in the queue: %s" },
	{ HCM_HTTPCACHEMANAGER_NOMOREHTTPREQUESTAVAILABLE,	// 10
		"No more HTTPRequest available. Queue size: %lu" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYALREADYPRESENT,
		"HTTPRequest Key already present. Key: %s" },
	{ HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYNOTFOUND,
		"HTTPRequest Key not found. Key: %s" },
	{ HCM_HTTPCACHEMANAGER_HASHMAPANDMULTIMAPNOTCONSISTENT,
		"HashMap and MultiMap are not consistent. HashMap size: %lu, MultiMap size: %lu" },

	// common
	{ HCM_NEW_FAILED,
		"The new function failed" },
	{ HCM_ACTIVATION_WRONG,
		"Activation wrong" }

	// Insert here other errors...

} ;

