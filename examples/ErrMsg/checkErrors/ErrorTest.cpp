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

#include "ErrorTest.h"


ErrMsgBase:: ErrMsgsInfo ErrorTestStr = {


	// main
	ERRORTEST_FUNC1_FAILED,
		"parameter1 %d - parameter2 %ld - parameter3 %c - parameter4 %s parameter5 %f - parameter6 %lf - parameter7 %lu - parameter8 %lld - parameter9 %llu",
	ERRORTEST_FUNC2_FAILED,
		"The func2 function failed (Modules not initialized) %s",
	ERRORTEST_FUNC3_FAILED,
		"The func3 function failed",
	ERRORTEST_FUNC4_FAILED,
		"The func4 function failed"

	// Insert here other errors...

} ;

