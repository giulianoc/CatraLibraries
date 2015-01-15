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
#include "FileReader.h"
#include <iostream>

#ifdef WIN32
	#define DIRCMP_DIRECTORYSEPARATOR		"\\"
#else
	#define DIRCMP_DIRECTORYSEPARATOR		"/"
#endif

// input files
#ifdef WIN32
	#define DIRCMP_EXCLUDEFILES_PATHNAME		".\\Exclude.txt"
	#define DIRCMP_EXCLUDEEXTENSIONS_PATHNAME	".\\ExcludeExtensions.txt"
#else
	#define DIRCMP_EXCLUDEFILES_PATHNAME		"./Exclude.txt"
	#define DIRCMP_EXCLUDEEXTENSIONS_PATHNAME	"./ExcludeExtensions.txt"
#endif

// output files
#ifdef WIN32
	#define DIRCMP_CHANGEDFILES_PATHNAME		".\\OUT_ChangedFiles.bat"
	#define DIRCMP_FILESNOTPRESENDINB_PATHNAME	".\\OUT_FilesNotPresentInDirB.txt"
#else
	#define DIRCMP_CHANGEDFILES_PATHNAME		"./OUT_ChangedFiles.sh"
	#define DIRCMP_FILESNOTPRESENDINB_PATHNAME	"./OUT_FilesNotPresentInDirB.txt"
#endif


Error isEntryExcluded (const char *pEntry,
	FileReader_p pfrExclude,
	FileReader_p pfrExcludeExtensions,
	Boolean_p pbIsExcluded)

{
	const char				*pExtension;
	#ifdef WIN32
		__int64					llCurrentPosition;
		__int64					ullCharsRead;
	#else
		long long				llCurrentPosition;
		unsigned long long		ullCharsRead;
	#endif
	Error_t				errRead;
	char				pLine [1024 + 1];



	/*
	if (!strcmp (pEntry, "CVS"))
		std:: cout << "1 " << pEntry << std:: endl;
	*/

	*pbIsExcluded				= false;

	if (pfrExclude != (FileReader_p) NULL)
	{
		if (pfrExclude -> seek (0, SEEK_SET, &llCurrentPosition) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_SEEK_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		while ((errRead = pfrExclude -> readLine (pLine, 1024,
			&ullCharsRead)) == errNoError)
		{
			/*
			std:: cout << "2 " << pLine << std:: endl;
			*/
			#ifdef WIN32
				// delete the last char (\r)
				pLine [strlen (pLine) - 1]			= '\0';
			#else
			#endif

			if (!strcmp (pLine, pEntry))
			{
				*pbIsExcluded				= true;

/*
if (strlen (pEntry) > 5 && !strcmp (pEntry + strlen (pEntry) - 5, ".java"))
	std:: cout << "Excluded file from " << pLine << std:: endl;
*/

				break;
			}
		}

		if (errRead != errNoError &&
			(long) errRead != TOOLS_FILEREADER_REACHEDENDOFFILE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_READLINE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return err;
		}

		if (*pbIsExcluded)
			return errNoError;
	}

	if (pfrExcludeExtensions != (FileReader_p) NULL)
	{
		pExtension = strrchr (pEntry, '.');

		if (pExtension != (const char *) NULL)
		{
			pExtension			+= 1;

			if (pfrExcludeExtensions -> seek (0, SEEK_SET,
				&llCurrentPosition) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_SEEK_FAILED);
				std:: cerr << (const char *) err << std:: endl;

				return err;
			}

			while ((errRead = pfrExcludeExtensions -> readLine (pLine, 1024,
				&ullCharsRead)) == errNoError)
			{
				#ifdef WIN32
					// delete the last char (\r)
					pLine [strlen (pLine) - 1]			= '\0';
				#else
				#endif

				if (!strcmp (pLine, pExtension))
				{
					*pbIsExcluded				= true;
	
/*
if (!strcmp (pExtension, "java"))
	std:: cout << "Excluded extension from " << pLine << std:: endl;
*/

					break;
				}
			}
	
			if (errRead != errNoError &&
				(long) errRead != TOOLS_FILEREADER_REACHEDENDOFFILE)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_READLINE_FAILED);
				std:: cerr << (const char *) err << std:: endl;
	
				return err;
			}
		}
	}


	return errNoError;
}


int dirCmp (const char *pDirectoryA, const char *pDirectoryB,
	FileReader_p pfrExclude,
	FileReader_p pfrExcludeExtensions)

{

	FileIO:: Directory_t		dDirectoryA;
	Error_t						errRead;
	Error_t						errReadDirectory;
	Buffer_t					bDirectoryEntry;
	FileIO:: DirectoryEntryType_t
		detDirectoryEntryType;
	Buffer_t					bFilePathNameA;
	Buffer_t					bFilePathNameB;
	Buffer_t					bFileA;
	Buffer_t					bFileB;
	Boolean_t					bIsExcluded;


	if (bFilePathNameA. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (bFilePathNameB. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFileA. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFileB. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFileB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (FileIO:: openDirectory (pDirectoryA, &dDirectoryA) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFileB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	while ((errReadDirectory = FileIO:: readDirectory (&dDirectoryA,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		switch (detDirectoryEntryType)
		{
			case FileIO:: TOOLS_FILEIO_REGULARFILE:
				{
					/*
					if (!strcmp ((const char *) bDirectoryEntry,
						"ServerSocket.h"))
						std:: cout << "1 REGULAR FILE   "
							<< (const char *) bDirectoryEntry
							<< std:: endl;
					*/

					if (isEntryExcluded ((const char *) bDirectoryEntry,
						pfrExclude, pfrExcludeExtensions,
						&bIsExcluded) != errNoError)
					{
						std:: cerr << "isEntryExcluded failed" << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (bIsExcluded)
						continue;

					/*
					if (!strcmp ((const char *) bDirectoryEntry,
						"ServerSocket.h"))
						std:: cout << "2 REGULAR FILE   "
							<< (const char *) bDirectoryEntry
							<< std:: endl;
					*/

					if (!(bFilePathNameA. setBuffer (pDirectoryA) ==
							errNoError &&
						bFilePathNameB. setBuffer (pDirectoryB) == errNoError))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_OPENDIRECTORY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (!(bFilePathNameA. append (
						(const char *) bDirectoryEntry) == errNoError &&
						bFilePathNameB. append (
						(const char *) bDirectoryEntry) == errNoError))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_OPENDIRECTORY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					/*
					std:: cout << "cmp "
						<< (const char *) bFilePathNameA
						<< " "
						<< (const char *) bFilePathNameB
						<< std:: endl;
					*/

					if ((errRead = bFileA. readBufferFromFile (
						(const char *) bFilePathNameA)) != errNoError)
					{
						std:: cerr << (const char *) errRead << std:: endl;
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if ((errRead = bFileB. readBufferFromFile (
							(const char *) bFilePathNameB)) != errNoError)
					{
						if ((unsigned long) errRead ==
							TOOLS_FILEIO_OPEN_FAILED)
						{
							if (FileIO:: appendBuffer (
								DIRCMP_FILESNOTPRESENDINB_PATHNAME,
								(const char *) bFilePathNameA, false) !=
								errNoError ||
								FileIO:: appendBuffer (
									DIRCMP_FILESNOTPRESENDINB_PATHNAME,
									" ", false) != errNoError ||
								FileIO:: appendBuffer (
									DIRCMP_FILESNOTPRESENDINB_PATHNAME,
									(const char *) bFilePathNameB, true) !=
									errNoError
								)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_APPENDBUFFER_FAILED);
								std:: cerr << (const char *) err << std:: endl;

								if (bDirectoryEntry. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_BUFFER_FINISH_FAILED);
									std:: cerr << (const char *) err
										<< std:: endl;
								}

								if (bFileB. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_BUFFER_FINISH_FAILED);
									std:: cerr << (const char *) err
										<< std:: endl;
								}

								if (bFileA. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_BUFFER_FINISH_FAILED);
									std:: cerr << (const char *) err
										<< std:: endl;
								}

								if (bFilePathNameB. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_BUFFER_FINISH_FAILED);
									std:: cerr << (const char *) err
										<< std:: endl;
								}

								if (bFilePathNameA. finish () != errNoError)
								{
									Error err = ToolsErrors (__FILE__, __LINE__,
										TOOLS_BUFFER_FINISH_FAILED);
									std:: cerr << (const char *) err
										<< std:: endl;
								}

								return 1;
							}

							continue;
						}
						else
						{
							std:: cerr << (const char *) errRead << std:: endl;
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
							std:: cerr << (const char *) err << std:: endl;

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFileB. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFileA. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFilePathNameB. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFilePathNameA. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							return 1;
						}
					}

					if (strcmp ((const char *) bFileA, (const char *) bFileB))
					{
						/*
						if (!strcmp ((const char *) bDirectoryEntry,
							"Configuration.vcproj"))
						{
							Buffer_t		bFileToBeVerified;
							bFileToBeVerified. init ();
							bFileToBeVerified. setBuffer (
								(const char *) bDirectoryEntry);
							bFileToBeVerified. append ("_A");
							bFileA. writeBufferOnFile (
								(const char *) bFileToBeVerified);
							bFileToBeVerified. setBuffer (
								(const char *) bDirectoryEntry);
							bFileToBeVerified. append ("_B");
							bFileB. writeBufferOnFile (
								(const char *) bFileToBeVerified);
							bFileToBeVerified. finish ();
						}
						*/

						if (!((FileIO:: appendBuffer (
								DIRCMP_CHANGEDFILES_PATHNAME,
								(const char *) bFilePathNameA, false) ==
								errNoError) &&
							(FileIO:: appendBuffer (
								DIRCMP_CHANGEDFILES_PATHNAME,
								" ", false) == errNoError) &&
							(FileIO:: appendBuffer (
								DIRCMP_CHANGEDFILES_PATHNAME,
								(const char *) bFilePathNameB, true) ==
								errNoError)))
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_APPENDBUFFER_FAILED);
							std:: cerr << (const char *) err << std:: endl;

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFileB. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFileA. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFilePathNameB. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							if (bFilePathNameA. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
								std:: cerr << (const char *) err << std:: endl;
							}

							return 1;
						}
					}
				}

				break;
			case FileIO:: TOOLS_FILEIO_DIRECTORY:
				{
					/*
					std:: cout << "DIRECTORY prima      "
						<< (const char *) bDirectoryEntry
						<< std:: endl;
					*/

					if (isEntryExcluded ((const char *) bDirectoryEntry,
						pfrExclude, pfrExcludeExtensions,
						&bIsExcluded) != errNoError)
					{
						std:: cerr << "isEntryExcluded failed" << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (bIsExcluded)
						continue;

					if (!(bFilePathNameA. setBuffer (pDirectoryA) ==
						errNoError &&
						bFilePathNameB. setBuffer (pDirectoryB) == errNoError))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_OPENDIRECTORY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (!(bFilePathNameA. append (
						(const char *) bDirectoryEntry) == errNoError &&
						bFilePathNameB. append (
						(const char *) bDirectoryEntry) == errNoError))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_OPENDIRECTORY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (!(bFilePathNameA. append (DIRCMP_DIRECTORYSEPARATOR) ==
							errNoError &&
						bFilePathNameB. append (DIRCMP_DIRECTORYSEPARATOR) ==
							errNoError))
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_OPENDIRECTORY_FAILED);
						std:: cerr << (const char *) err << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}

					if (dirCmp ((const char *) bFilePathNameA,
						(const char *) bFilePathNameB,
						pfrExclude, pfrExcludeExtensions))
					{
						std:: cerr << "dirCmp failed" << std:: endl;

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFileA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameB. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						if (bFilePathNameA. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
							std:: cerr << (const char *) err << std:: endl;
						}

						return 1;
					}
				}

				break;
			#ifdef WIN32
			#else
				case FileIO:: TOOLS_FILEIO_LINKFILE:
					{
					/*
						char		pRealPathName [1024];


						std:: cout << "LINK           "
							<< (const char *) bDirectoryEntry;

						if (bDirectoryEntry. insertAt (0,
							pDirectoryA) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_INSERTAT_FAILED);
							std:: cerr << (const char *) err << std:: endl;

							if (FileIO:: closeDirectory (&dDirectoryA) !=
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

							if (FileIO:: closeDirectory (&dDirectoryA) !=
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
					*/
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

	if ((long) errReadDirectory != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
	{
		std:: cerr << (const char *) errReadDirectory << std:: endl;

		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READDIRECTORY_FAILED);
		std:: cerr << (const char *) err << std:: endl;
	}

	if (FileIO:: closeDirectory (&dDirectoryA) != errNoError)
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

		if (bFileB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
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

		if (bFileB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFileB. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFileA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFileA. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFilePathNameB. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFilePathNameB. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFilePathNameA. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;
		}

		return 1;
	}

	if (bFilePathNameA. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}


int main (int iArgc, char *pArgv [])

{

	char						pDirectoryA [1024 + 1];
	char						pDirectoryB [1024 + 1];
	FileReader_t				frExclude;
	FileReader_t				frExcludeExtensions;
	Boolean_t					bFileExist;
	Boolean_t					bFileExcludeExist;
	Boolean_t					bFileExcludeExtensionsExist;


	if (iArgc != 3)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <directory A> <directory B>"
			<< std:: endl;

		return 1;
	}


	strcpy (pDirectoryA, pArgv [1]);
	strcpy (pDirectoryB, pArgv [2]);

	#ifdef WIN32
		if (pDirectoryA [strlen (pDirectoryA) - 1] != '\\')
			strcat (pDirectoryA, DIRCMP_DIRECTORYSEPARATOR);
		if (pDirectoryB [strlen (pDirectoryB) - 1] != '\\')
			strcat (pDirectoryB, DIRCMP_DIRECTORYSEPARATOR);
	#else
		if (pDirectoryA [strlen (pDirectoryA) - 1] != '/')
			strcat (pDirectoryA, DIRCMP_DIRECTORYSEPARATOR);
		if (pDirectoryB [strlen (pDirectoryB) - 1] != '/')
			strcat (pDirectoryB, DIRCMP_DIRECTORYSEPARATOR);
	#endif

	if (FileIO:: isFileExisting (DIRCMP_CHANGEDFILES_PATHNAME,
		&bFileExist) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (bFileExist)
	{
		if (FileIO:: remove (DIRCMP_CHANGEDFILES_PATHNAME) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (FileIO:: isFileExisting (DIRCMP_FILESNOTPRESENDINB_PATHNAME,
		&bFileExist) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (bFileExist)
	{
		if (FileIO:: remove (DIRCMP_FILESNOTPRESENDINB_PATHNAME) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (FileIO:: isFileExisting (DIRCMP_EXCLUDEFILES_PATHNAME,
		&bFileExcludeExist) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	if (bFileExcludeExist)
	{
		Error_t					errInit;
		/*
		Buffer_t				bFile;


		if (bFile. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}

		if (bFile. readBufferFromFile (DIRCMP_EXCLUDEFILES_PATHNAME) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. substitute ("\r\n", "\n") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. writeBufferOnFile (DIRCMP_EXCLUDEFILES_PATHNAME) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
		*/

		if ((errInit = frExclude. init (
			DIRCMP_EXCLUDEFILES_PATHNAME, 1024)) != errNoError)
		{
			std:: cerr << (const char *) errInit << std:: endl;

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_INIT_FAILED,
				1, DIRCMP_EXCLUDEFILES_PATHNAME);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (FileIO:: isFileExisting (DIRCMP_EXCLUDEEXTENSIONS_PATHNAME,
		&bFileExcludeExtensionsExist) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		if (bFileExcludeExist)
		{
			if (frExclude. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}
		}

		return 1;
	}

	if (bFileExcludeExtensionsExist)
	{
		Error_t					errInit;
		/*
		Buffer_t				bFile;


		if (bFile. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}

		if (bFile. readBufferFromFile (DIRCMP_EXCLUDEEXTENSIONS_PATHNAME) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. substitute ("\r\n", "\n") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. writeBufferOnFile (DIRCMP_EXCLUDEEXTENSIONS_PATHNAME) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFile. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}

			return 1;
		}

		if (bFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
		*/

		if ((errInit = frExcludeExtensions. init (
			DIRCMP_EXCLUDEEXTENSIONS_PATHNAME, 1024)) != errNoError)
		{
			std:: cerr << (const char *) errInit << std:: endl;

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_INIT_FAILED,
				1, DIRCMP_EXCLUDEEXTENSIONS_PATHNAME);
			std:: cerr << (const char *) err << std:: endl;

			if (bFileExcludeExist)
			{
				if (frExclude. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEREADER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}

			return 1;
		}
	}

	if (dirCmp (pDirectoryA, pDirectoryB,
		bFileExcludeExist ? &frExclude : (FileReader_p) NULL,
		bFileExcludeExtensionsExist ? &frExcludeExtensions :
			(FileReader_p) NULL))
	{
		std:: cerr << "dirCmp failed" << std:: endl;

		if (bFileExcludeExtensionsExist)
		{
			if (frExcludeExtensions. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}
		}

		if (bFileExcludeExist)
		{
			if (frExclude. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEREADER_FINISH_FAILED);
				std:: cerr << (const char *) err << std:: endl;
			}
		}

		return 1;
	}

	if (bFileExcludeExtensionsExist)
	{
		if (frExcludeExtensions. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			if (bFileExcludeExist)
			{
				if (frExclude. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEREADER_FINISH_FAILED);
					std:: cerr << (const char *) err << std:: endl;
				}
			}

			return 1;
		}
	}

	if (bFileExcludeExist)
	{
		if (frExclude. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			std:: cerr << (const char *) err << std:: endl;

			return 1;
		}
	}


	return 0;
}


