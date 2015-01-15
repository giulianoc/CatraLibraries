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


#define MAX_ITEMVALUELENGTH				1024 + 1


int main (int argc, char **argv)

{

	ConfigurationFile_t			cfConfiguration;
	char						*pConfigFilePathName;
	char						*pSectionName;
	char						*pItemName;
	char						pItemValue [MAX_ITEMVALUELENGTH];
	Error_t						errParseError;
	Error_t						errGetItemValue;


	{
		const char		*pCopyright = "\n***************************************************************************\ncopyright            : (C) by CatraSoftware\nemail                : catrastreaming-support@catrasoftware.it\n***************************************************************************\n";

		std:: cout << pCopyright << std:: endl;
	}

	if (argc != 4)
	{
		std:: cout << "Main functionalities:"
			<< std:: endl << "\t1. Crypt an item in the configuration file"
			<< std:: endl << std:: endl
			<< "----------------------------------"
			<< std:: endl << std:: endl
			<< "Usage: " << argv [0]
			<< std:: endl
			<< "\t<configuration file path name>"
			<< std:: endl
			<< "\t<section name>"
			<< std:: endl
			<< "\t<item name to crypt>"
			<< std:: endl << std:: endl
			<< std:: endl << "Example: " << argv [0] << " aaa.cfg sect1 item2"
			<< std:: endl;

		return 1;
	}

	pConfigFilePathName			= argv [1];
	pSectionName				= argv [2];
	pItemName					= argv [3];


	if ((errParseError = cfConfiguration. init (pConfigFilePathName,
		"", 20, 5, 20)) != errNoError)
	{
		std:: cout << (const char *) errParseError << std:: endl;
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << cfConfiguration << std:: endl;

	if ((errGetItemValue = cfConfiguration. getItemValue (
		pSectionName, pItemName,
		pItemValue, MAX_ITEMVALUELENGTH)) != errNoError)
	{
		std:: cout << (const char *) errGetItemValue << std:: endl;

		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, pItemName);
		std:: cout << (const char *) err << std:: endl;
		cfConfiguration. finish ();

		return 1;
	}

	std:: cout << "Item value: " << pItemValue << std:: endl;

	if (cfConfiguration. modifyItemValue (pSectionName, pItemName,
		pItemValue, 0, Config:: CFG_ENCRIPTION) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_MODIFYITEMVALUE_FAILED);
		std:: cout << (const char *) err << std:: endl;
		cfConfiguration. finish ();

		return 1;
	}

	std:: cout << cfConfiguration << std:: endl;

	if (cfConfiguration. save () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_SAVE_FAILED);
		std:: cout << (const char *) err << std:: endl;
		cfConfiguration. finish ();

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

