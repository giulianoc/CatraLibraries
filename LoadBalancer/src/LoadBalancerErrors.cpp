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


#include "LoadBalancerErrors.h"


ErrMsgBase:: ErrMsgsInfo LoadBalancerErrorsStr = {

	// LoadBalancer
	{ LB_LOADBALANCER_INIT_FAILED,
		"The init method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_FINISH_FAILED,
		"The finish method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_GETAVAILABLEMODULE_FAILED,
		"The getAvailableModule method of LoadBalancer class failed. Module name: %s" },
	{ LB_LOADBALANCER_CHECKALLDELIVERYMODULES_FAILED,
		"The checkAllDeliveryModules method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_GETSHUTDOWN_FAILED,
		"The getShutdown method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_SETSHUTDOWN_FAILED,
		"The setShutdown method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_CHECKMODULECOMPLETED_FAILED,
		"The checkModuleCompleted method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_CANCEL_FAILED,
		"The cancel method of LoadBalancer class failed" },
	{ LB_LOADBALANCER_CONFIGITEMTOOLONG,
		"The configuration item is too long. Section: %s, Item: %s, Value: %s, MaxLength: %ul" },
	{ LB_LOADBALANCER_CONFIGITEMSVALUESWRONG,
		"The number of the values inside the configuration item is wrong. Section: %s, Item: %s, values number: %lu" },
	{ LB_LOADBALANCER_BUFFERTOOSHORT,
		"The input buffer parameter is too short" },
	{ LB_LOADBALANCER_MODULENOTAVAILABLE,
		"There is no %s available (up and running)" },
	{ LB_LOADBALANCER_MODULESNUMBERNOTCONSISTENT,
		"The number of Modules is not consistent" },
	{ LB_LOADBALANCER_MODULESNAMENOTFOUND,
		"The ModulesName (%s) was not found" },
	{ LB_LOADBALANCER_WRONGHTTPGETRESPONSE,
		"Wrong HTTP GET Response: %s" },
	{ LB_HTTPGETTHREAD_RUN_FAILED,
		"LoadBalancer HTTP GET URL failed. URL: %s" },
	{ LB_HTTPGETTHREAD_LASTURLCHECKTIMEOUT_WILLPINGAGAINLATER,
		"LoadBalancer HTTP GET URL failed (timeout). URL: %s" },
	{ LB_HTTPGETTHREAD_LASTSOCKETCHECKTIMEOUT_WILLPINGAGAINLATER,
		"LoadBalancer ping failed (timeout). Private Host: %s, Private port: %lu" },
	{ LB_HTTPGETTHREAD_REACHEDMAXREQUESTSNUMBER,
		"LoadBalancer: reached Max Requests Number. URL: %s, Module response: %s" },

	// common
	{ LB_NEW_FAILED,
		"new failed" },
	{ LB_ACTIVATION_WRONG,
		"The activation of the method is wrong" }

	// Insert here other errors...

} ;

