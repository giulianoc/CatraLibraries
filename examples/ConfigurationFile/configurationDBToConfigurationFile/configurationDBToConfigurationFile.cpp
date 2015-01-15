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

#include "ConfigurationDB.h"
#include "ConfigurationFile.h"


int main (int argc, char **argv)

{

	ConfigurationDB_t			cfgConfigurationDB;
	ConfigurationFile_t			cfgConfigurationFile;
	char						*pInstance;
	char						*pUser;
	char						*pPassword;
	char						*pNewConfigFilePathName;


	if (argc != 5)
	{
		cout << "Usage: configurationDB <pInstance> <pUser> <pPassword> <pNewConfigFilePathName>" << endl;

		return 1;
	}

	pInstance					= argv [1];
	pUser						= argv [2];
	pPassword					= argv [3];
	pNewConfigFilePathName		= argv [4];


	cout << "Loading configuration from DB ..." << endl;
	if (cfgConfigurationDB. init (pInstance, pUser, pPassword, "",
		20, 5, 32000) != errNoError)
	{
		Error err = ConfigurationDBErrors (__FILE__, __LINE__,
			CFGDB_CONFIGURATIONDB_INIT_FAILED);
		cout << (const char *) err << endl;

		return 1;
	}

	if (cfgConfigurationFile. init (pNewConfigFilePathName,
		&cfgConfigurationDB, "", 20, 5, 32000) != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_INIT_FAILED);
		cout << (const char *) err << endl;
		cfgConfigurationDB. finish ();

		return 1;
	}

//	cout << "ConfigurationDB: " << endl << cfgConfigurationDB;

	cout << "Saving configuration on file ..." << endl;
	if (cfgConfigurationFile. save () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_SAVE_FAILED);
		cout << (const char *) err << endl;
		cfgConfigurationFile. finish ();
		cfgConfigurationDB. finish ();

		return 1;
	}

	if (cfgConfigurationDB. finish () != errNoError)
	{
		Error err = ConfigurationDBErrors (__FILE__, __LINE__,
			CFGDB_CONFIGURATIONDB_FINISH_FAILED);
		cout << (const char *) err << endl;
		cfgConfigurationFile. finish ();

		return 1;
	}

//	cout << "ConfigurationFile: " << endl << cfgConfigurationFile;

	if (cfgConfigurationFile. finish () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
		cout << (const char *) err << endl;

		return 1;
	}


	return 0;
}
