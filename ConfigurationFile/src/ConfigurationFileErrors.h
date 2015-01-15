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


#ifndef ConfigurationFileErrors_h
	#define ConfigurationFileErrors_h

	#include "Error.h"
	#include "iostream"


	/**
		Click <a href="ConfigurationFileErrors.C#ConfigurationFileErrors" target=classContent>here</a> for the errors strings.
	*/
	enum ConfigurationFileErrorsCodes {

		// ConfigurationFile
		CFGFILE_FCNTL_FAILED,
		CFGFILE_FILE_NOTRELEASED,
		CFGFILE_CONFIGURATIONFILE_INIT_FAILED,
		CFGFILE_CONFIGURATIONFILE_SAVE_FAILED,
		CFGFILE_CONFIGURATIONFILE_ADDITEMANDSAVE_FAILED,
		CFGFILE_CONFIGURATIONFILE_FINISH_FAILED,
		CFGFILE_CONFIGURATIONTABLE_WRONG,
		CFGFILE_YYPARSER_FAILED,

		// Insert here other errors...

		CFGFILE_MAXERRORS
	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (ConfigurationFileErrors, CFGFILE_MAXERRORS)


#endif

