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


#include "ConfigurationFileErrors.h"


ErrMsgBase:: ErrMsgsInfo ConfigurationFileErrorsStr = {

	// ConfigurationFile
	{ CFGFILE_FCNTL_FAILED,
		"The fcntl function failed (errno: %d)" },
	{ CFGFILE_FILE_NOTRELEASED,
		"The \"%s\" file is not released" },
	{ CFGFILE_CONFIGURATIONFILE_INIT_FAILED,
		"The init method of the ConfigurationFile class failed. PathFileName: %s" },
	{ CFGFILE_CONFIGURATIONFILE_SAVE_FAILED,
		"The save method of the ConfigurationFile class failed" },
	{ CFGFILE_CONFIGURATIONFILE_ADDITEMANDSAVE_FAILED,
		"The addItemAndSave method of the ConfigurationFile class failed" },
	{ CFGFILE_CONFIGURATIONFILE_FINISH_FAILED,
		"The finish method of the ConfigurationFile class failed" },
	{ CFGFILE_CONFIGURATIONTABLE_WRONG,
		"The columns of the table in the configuration file is wrong" },
	{ CFGFILE_YYPARSER_FAILED,
		"The yyparse function failed (line: %d - Message error: '%s')" }

	// Insert here other errors...

} ;

