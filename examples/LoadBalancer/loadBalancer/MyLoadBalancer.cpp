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

#include "MyLoadBalancer.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>


MyLoadBalancer:: MyLoadBalancer (void): LoadBalancer ()

{

}


MyLoadBalancer:: ~MyLoadBalancer (void)

{

}


Error MyLoadBalancer:: checkModuleCompleted (
	const char *pError,
	const char *pPrivateHost,
	unsigned long ulPrivatePort,
	const char *pPublicHost,
	unsigned long ulPublicPort,
	Boolean_t bCheckResultSuccessful,
	Boolean_t bWasLastErrorTimeout,
	unsigned long ulCheckElapsedTimeInSecs)

{

	// if (ulCheckElapsedTimeInSecs >= 1 || !bCheckResultSuccessful)
	{
		char		pMessage [1024];

		sprintf (pMessage, "Error: %s, %s:%lu, CheckResultSuccessful: %ld, WasLastErrorTimeout: %ld, CheckElapsedTimeInSecs: %lu",
			pError,
			pPrivateHost,
			ulPrivatePort,
			(long) bCheckResultSuccessful,
			(long) bWasLastErrorTimeout,
			ulCheckElapsedTimeInSecs);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			pMessage, __FILE__, __LINE__);

		/*
		std:: cout << pPrivateHost << ":" << ulPrivatePort
			<< ", CheckResultSuccessful: " << bCheckResultSuccessful
			<< ", WasLastErrorTimeout: " << bWasLastErrorTimeout
			<< ", CheckElapsedTimeInSecs: " << ulCheckElapsedTimeInSecs
			<< std:: endl;
		*/
	}


	return errNoError;
}

