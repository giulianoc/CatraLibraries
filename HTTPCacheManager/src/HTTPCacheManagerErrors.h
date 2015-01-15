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


#ifndef HTTPCacheManagerErrors_h
	#define HTTPCacheManagerErrors_h

	#include "Error.h"
	#include <iostream>


	//
	// Defined errors:
	//   
   
	enum HTTPCacheManagerErrorsCodes {

		// HTTPCacheManager
		HCM_HTTPCACHEMANAGER_INIT_FAILED,		// 0
		HCM_HTTPCACHEMANAGER_FINISH_FAILED,
		HCM_HTTPCACHEMANAGER_MARKHTTPREQUESTKEYASFAILED_FAILED,
		HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYISMARKEDASFAILED,
		HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYSRETENTION_FAILED,
		HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSRUNNING_FAILED,
		HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSWAITING_FAILED,
		HCM_HTTPCACHEMANAGER_GETLOCALPATHNAMEANDCACHECONTENT_FAILED,
		HCM_HTTPCACHEMANAGER_GETLOCALPATHNAMEUSINGHTTPREQUESTKEY_FAILED,
		HCM_HTTPCACHEMANAGER_GETLOCALPATHNAME_FAILED,
		HCM_HTTPCACHEMANAGER_ISLOCALPATHNAMEPRESENTINCACHE_FAILED,
		HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED,
		HCM_HTTPCACHEMANAGER_SETISSHUTDOWN_FAILED,
		HCM_HTTPCACHEMANAGER_GETISSHUTDOWN_FAILED,
		HCM_HTTPCACHEMANAGER_WRONGURI,
		HCM_HTTPCACHEMANAGER_HTTPREQUESTALREADYINTHEQUEUE,
		HCM_HTTPCACHEMANAGER_NOMOREHTTPREQUESTAVAILABLE,	// 10
		HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYALREADYPRESENT,
		HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYNOTFOUND,
		HCM_HTTPCACHEMANAGER_HASHMAPANDMULTIMAPNOTCONSISTENT,

		// common
		HCM_NEW_FAILED,
		HCM_ACTIVATION_WRONG,

		// Insert here other errors...

		HCM_MAX_ERRORS

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (HTTPCacheManagerErrors, HCM_MAX_ERRORS)
   
#endif

