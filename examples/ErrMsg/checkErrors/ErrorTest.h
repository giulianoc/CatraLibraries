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

#ifndef ErrorTest_h
	#define ErrorTest_h

	#include "Error.h"


	//
	// Defined errors:
	//   

	enum ErrorTestCodes {

		// main
		ERRORTEST_FUNC1_FAILED,
		ERRORTEST_FUNC2_FAILED,
		ERRORTEST_FUNC3_FAILED,
		ERRORTEST_FUNC4_FAILED,

		// Insert here other errors...

		ERRORTEST_MAXERRORS	
	} ;

	// declaration of class error
	dclCustomErrorClass (ErrorTest, ERRORTEST_MAXERRORS)

#endif
