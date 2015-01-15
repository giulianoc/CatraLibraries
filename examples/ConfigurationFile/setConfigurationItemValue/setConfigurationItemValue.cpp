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

#include "ConfigurationFile.h"
#include <time.h>

#define CFGFILE_MAXITEMVALUELENGTH			1024 * 10


int main (int argc, char **argv)

{

	ConfigurationFile_t			cfConfiguration;
	char						*pConfigFilePathName;
	char						*pSectionName;
	char						*pItemName;
	char						*pItemValue;
	Error_t						errParseError;
	Error_t						errModifyItemValue;


	if (argc != 5)
	{
		std:: cout << "Usage: " << argv [0] << " <ConfigFilePathName> <SectionName> <ItemName> <ItemValue>" << std:: endl;

		return 1;
	}

	pConfigFilePathName		= argv [1];
	pSectionName			= argv [2];
	pItemName				= argv [3];
	pItemValue				= argv [4];

	if ((errParseError = cfConfiguration. init (pConfigFilePathName,
		"", 20, 5, 32000)) != errNoError)
	{
		std:: cerr << (const char *) errParseError << std:: endl;

		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if ((errModifyItemValue = cfConfiguration. modifyItemValue (
		pSectionName, pItemName, pItemValue,
		0, Config:: CFG_NOENCRIPTION)) != errNoError)
	{
		std:: cerr << (const char *) errModifyItemValue << std:: endl;

		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_MODIFYITEMVALUE_FAILED,
			3, pSectionName, pItemName, pItemValue);
		std:: cerr << (const char *) err << std:: endl;

		if (cfConfiguration. finish () != errNoError)
		{
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (cfConfiguration. save () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_SAVE_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (cfConfiguration. finish () != errNoError)
		{
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (cfConfiguration. finish () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

