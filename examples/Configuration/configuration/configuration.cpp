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

#include "Config.h"

#define XXX_MAXITEMNAMELENGTH		256 + 1


int main (void)

{

	ConfigurationItem_t			ciItem;
	ConfigurationSection_t		csSection;
	Config_t					cfgConfiguration;
	long						lItemsNumber;
	long						lItemValuesNumber;
	char						pItemValue [XXX_MAXITEMNAMELENGTH];
	Error_t						errGetItemValue;


	if (ciItem. init ("item1Name", "item1Comment") != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ciItem. appendItemValue ("item1Value1") != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ciItem. appendItemValue ("item1Value2") != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ciItem. appendItemValue ("item1Value3") != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_APPENDITEMVALUE_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << ciItem;

	if (csSection. init ("section1Name", "section1Comment", "2001/12/30") !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (csSection. appendCfgItem (&ciItem) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_APPENDCFGITEM_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ciItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << csSection;

	if (cfgConfiguration. init ("configuration1Name") != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (cfgConfiguration. appendCfgSection (&csSection) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_APPENDCFGSECTION_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (cfgConfiguration. getItemsNumber ("section1Name", &lItemsNumber) !=
		errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMSNUMBER_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (cfgConfiguration. getCfgItemByName ("section1Name", "item1Name",
		&ciItem) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETCFGITEMBYNAME_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}
	std:: cout << ciItem;
	if (ciItem. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONITEM_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}
	if (cfgConfiguration. getItemValuesNumber ("section1Name", "item1Name",
		&lItemValuesNumber) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUESNUMBER_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}
	if ((errGetItemValue = cfgConfiguration. getItemValue (
		"section1Name", "item1Name",
		pItemValue, XXX_MAXITEMNAMELENGTH)) != errNoError)
	{
		std:: cout << (const char *) errGetItemValue << std:: endl;

		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, "section1Name", "item1Name");
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (csSection. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIGURATIONSECTION_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << cfgConfiguration;

	if (cfgConfiguration. finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_FINISH_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}
