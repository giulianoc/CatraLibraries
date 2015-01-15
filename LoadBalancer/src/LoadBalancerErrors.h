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


#ifndef LoadBalancerErrors_h
	#define LoadBalancerErrors_h

	#include "Error.h"
	#include <iostream>

	enum LoadBalancerErrorsCodes {

		// LoadBalancer
		LB_LOADBALANCER_INIT_FAILED,
		LB_LOADBALANCER_FINISH_FAILED,
		LB_LOADBALANCER_GETAVAILABLEMODULE_FAILED,
		LB_LOADBALANCER_CHECKALLDELIVERYMODULES_FAILED,
		LB_LOADBALANCER_GETSHUTDOWN_FAILED,
		LB_LOADBALANCER_SETSHUTDOWN_FAILED,
		LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED,
		LB_LOADBALANCER_CANCEL_FAILED,
		LB_LOADBALANCER_CONFIGITEMTOOLONG,
		LB_LOADBALANCER_CONFIGITEMSVALUESWRONG,
		LB_LOADBALANCER_BUFFERTOOSHORT,
		LB_LOADBALANCER_MODULENOTAVAILABLE,
		LB_LOADBALANCER_MODULESNUMBERNOTCONSISTENT,
		LB_LOADBALANCER_MODULESNAMENOTFOUND,
		LB_LOADBALANCER_WRONGHTTPGETRESPONSE,
		LB_HTTPGETTHREAD_RUN_FAILED,
		LB_HTTPGETTHREAD_LASTURLCHECKTIMEOUT_WILLPINGAGAINLATER,
		LB_HTTPGETTHREAD_LASTSOCKETCHECKTIMEOUT_WILLPINGAGAINLATER,
		LB_HTTPGETTHREAD_REACHEDMAXREQUESTSNUMBER,

		// common
		LB_NEW_FAILED,
		LB_ACTIVATION_WRONG,

		// Insert here other errors...

		LB_MAXERRORS
	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (LoadBalancerErrors, LB_MAXERRORS)
   
#endif

