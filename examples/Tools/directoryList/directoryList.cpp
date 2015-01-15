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

#include "FileIO.h"
#include <iostream>


int main (int iArgc, char *pArgv [])

{

	Boolean_t					bIsExist;
	FileIO:: Directory_t		dDirectory;
	Error_t						errDir;
	Buffer_t					bDirectoryEntry;
	FileIO:: DirectoryEntryType_t
		detDirectoryEntryType;
	char						pDirectory [1024 + 1];


	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <directory>"
			<< std:: endl;

		return 1;
	}


	strcpy (pDirectory, pArgv [1]);
	#ifdef WIN32
		if (pDirectory [strlen (pDirectory) - 1] != '\\')
			strcat (pDirectory, "\\");
	#else
		if (pDirectory [strlen (pDirectory) - 1] != '/')
			strcat (pDirectory, "/");
	#endif

	if ((errDir = FileIO:: isDirectoryExisting (pDirectory, &bIsExist)) !=
		errNoError)
	{
		std:: cerr << (const char *) errDir << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (!bIsExist)
	{
		std:: cerr << "The directory does not exist" << std:: endl;

		return 1;
	}

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if ((errDir = FileIO:: openDirectory (pDirectory, &dDirectory)) !=
		errNoError)
	{
		std:: cerr << (const char *) errDir << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	while ((errDir = FileIO:: readDirectory (&dDirectory,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		switch (detDirectoryEntryType)
		{
			case FileIO:: TOOLS_FILEIO_REGULARFILE:
				std:: cout << "REGOLAR FILE   "
					<< (const char *) bDirectoryEntry
					<< std:: endl;

				break;
			case FileIO:: TOOLS_FILEIO_DIRECTORY:
				std:: cout << "DIRECTORY      "
					<< (const char *) bDirectoryEntry
					<< std:: endl;

				break;
			#ifdef WIN32
			#else
				case FileIO:: TOOLS_FILEIO_LINKFILE:
					{
						char		pRealPathName [1024];


						std:: cout << "LINK           "
							<< (const char *) bDirectoryEntry;

						if (bDirectoryEntry. insertAt (0,
							pDirectory) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_INSERTAT_FAILED);
							std:: cerr << (const char *) err << std:: endl;

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							return 1;
						}

						if (FileIO:: readLink ((const char *) bDirectoryEntry,
							pRealPathName, 1024) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_READLINK_FAILED);
							std:: cerr << (const char *) err << std:: endl;

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							return 1;
						}

						std:: cout << " TO FILE " << pRealPathName
							<< std:: endl;
					}

					break;
			#endif
			case FileIO:: TOOLS_FILEIO_UNKNOWN:
				std:: cout << "UNKNOWN FILE   "
					<< (const char *) bDirectoryEntry
					<< std:: endl;

				break;
		}
	}

	if ((long) errDir != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
		std:: cerr << (const char *) errDir << std:: endl;

	if (FileIO:: closeDirectory (&dDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bDirectoryEntry. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}

