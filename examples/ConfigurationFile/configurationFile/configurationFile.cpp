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



int main (int argc, char **argv)

{

	ConfigurationFile_t			cfConfiguration;
	long						lIndex;
	char						*pConfigFilePathName;
	char						*pNewConfigFilePathName;
	Error_t						errParseError;
	Error_t						errSave;
	time_t						lUtcTimeNow;


	if (argc != 3)
	{
		std:: cout << "Usage: " << argv [0] << " <pConfigFilePathName> <pNewConfigFilePathName>" << std:: endl;

		return 1;
	}

	pConfigFilePathName			= argv [1];
	pNewConfigFilePathName		= argv [2];

	std:: cout << "Args: " << pConfigFilePathName << ", " << pNewConfigFilePathName << std:: endl;


	// to test the memory leak
	for (lIndex = 0; lIndex < 1; lIndex++)
	{
		std:: cout << "Start init" << std:: endl;
		lUtcTimeNow		= time (NULL);
		if ((errParseError = cfConfiguration. init (pConfigFilePathName,
			"", 20, 5, 32000)) != errNoError)
		{
			std:: cout << (const char *) errParseError << std:: endl;
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
		std:: cout << "End init (secondi: " << time (NULL) - lUtcTimeNow << ")" << std:: endl;
//		std:: cout << cfConfiguration;
		std:: cout << "Index: " << lIndex << std:: endl;

		std:: cout << "start save" << std:: endl;
		lUtcTimeNow		= time (NULL);
		if ((errSave = cfConfiguration. save (pNewConfigFilePathName)) !=
			errNoError)
		{
			std:: cout << (const char *) errSave << std:: endl;
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_SAVE_FAILED);
			std:: cout << (const char *) err << std:: endl;
			cfConfiguration. finish ();

			return 1;
		}
		std:: cout << "end save" << std:: endl;
		std:: cout << "End save (secondi: " << time (NULL) - lUtcTimeNow << ")" << std:: endl;

		if (cfConfiguration. finish () != errNoError)
		{
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}


	return 0;
}

