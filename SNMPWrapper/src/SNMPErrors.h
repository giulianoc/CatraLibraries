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


#ifndef SNMPErrors_h
	#define SNMPErrors_h

	#include "Error.h"
	#include <iostream>


	//
	// Defined errors:
	//   
   
	enum SNMPErrorsCodes {

		// Buffer
		SNMP_SNMP_INIT_FAILED,
		SNMP_SNMP_FINISH_FAILED,
		SNMP_SNMP_SENDTRAP_FAILED,
		SNMP_SNMP_NOTENABLED,

		// common
		SNMP_ACTIVATION_WRONG,
		SNMP_SNMP_PDU_CREATE_FAILED,


		// Insert here other errors...

		SNMP_MAX_ERRORS

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (SNMPErrors, SNMP_MAX_ERRORS)
   
#endif

