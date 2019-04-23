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
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <fstream>
#include <thread>
#ifdef WIN32
	#include <io.h>
	#include <direct.h>
	#include <shobjidl.h>
	#include <shlguid.h>
#else
	#include <unistd.h>
	#ifdef __APPLE__
		#include <sys/mount.h>
	#else
		#include <sys/vfs.h>
	#endif
#endif
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#ifdef USEGZIPLIB
	#include <zlib.h>
#endif

//using namespace std;
//#include <iostream>

FileIO:: FileIO (void)

{

}


FileIO:: ~FileIO (void)

{

}



FileIO:: FileIO (const FileIO &)

{

	assert (1==0);

	// to do

}


FileIO &FileIO:: operator = (const FileIO &)

{

	assert (1==0);

	// to do

	return *this;

}


Error FileIO:: getFileNameFromPathFileName (const char *pPathFileName,
	Boolean_t bExtension, Buffer_p pbFileName, const char *pDirectorySeparator)

{

	const char			*pStartFileName;
	const char			*pEndFileName;
	char				cLocalDirectorySeparator;


	if (pDirectorySeparator != (const char *) NULL)
		cLocalDirectorySeparator		= pDirectorySeparator [0];
	else
	{
		#ifdef WIN32
			cLocalDirectorySeparator		= '\\';
		#else
			cLocalDirectorySeparator		= '/';
		#endif
	}

	if ((pStartFileName = strrchr (pPathFileName, cLocalDirectorySeparator)) ==
		(const char *) NULL)
		pStartFileName		= pPathFileName;
	else
		pStartFileName++;

	if (bExtension)
	{
		if (pbFileName -> setBuffer(pStartFileName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}
	else
	{
		if ((pEndFileName = strrchr (pStartFileName, '.')) ==
			(const char *) NULL)
		{
			if (pbFileName -> setBuffer (pStartFileName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}
		}
		else
		{
			if (pbFileName -> setBuffer(pStartFileName,
				pEndFileName - pStartFileName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error FileIO:: replaceUnsafeFileNameChars (Buffer_p pbFileName)

{

	unsigned long				ulBufferLength;
	unsigned long				ulBufferIndex;
	char						cChar;


	ulBufferLength			= (unsigned long) (*pbFileName);

	for (ulBufferIndex = 0; ulBufferIndex < ulBufferLength; ulBufferIndex++)
	{
		if (pbFileName -> getChar (&cChar, ulBufferIndex) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_GETCHAR_FAILED);

			return err;
		}

		if ((cChar >= 'a' && cChar <= 'z') ||
			(cChar >= 'A' && cChar <= 'Z') ||
			(cChar >= '0' && cChar <= '9') ||
			cChar == '_' ||
			cChar == '-' ||
			cChar == '.')
		{
			;	// char ok
		}
		else
		{
			if (pbFileName -> setChar (ulBufferIndex, '_') != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETCHAR_FAILED);

				return err;
			}
		}
	}


	return errNoError;
}


Error FileIO:: openDirectory (const char *pDirectoryPathName,
	Directory_p pdDirectory)

{
	
	unsigned long			ulDirectoryPathNameLength;


	if (pDirectoryPathName == (const char *) NULL ||
		pdDirectory == (Directory_p) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	ulDirectoryPathNameLength		= strlen (pDirectoryPathName);

	if ((pdDirectory -> _pPathName = new char [
		ulDirectoryPathNameLength + 1]) == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}
	strcpy (pdDirectory -> _pPathName, pDirectoryPathName);

	#ifdef WIN32
		if ((pdDirectory -> _pPathName) [ulDirectoryPathNameLength - 1] == '\\')
			(pdDirectory -> _pPathName) [ulDirectoryPathNameLength - 1]	= '\0';
	#else
		if ((pdDirectory -> _pPathName) [ulDirectoryPathNameLength - 1] == '/')
			(pdDirectory -> _pPathName) [ulDirectoryPathNameLength - 1]	= '\0';
	#endif

	#ifdef WIN32
		pdDirectory -> _lIdentifier			= -1;
	#else
		Error						errEntryType;
		DirectoryEntryType_t		detDirectoryEntryType;


		if ((errEntryType = FileIO:: getDirectoryEntryType (
			pdDirectory -> _pPathName,
			&detDirectoryEntryType)) != errNoError)
		{

			return errEntryType;
		}

		if (detDirectoryEntryType == TOOLS_FILEIO_LINKFILE)
		{
			char				pRealPathName [512];
			Error_t				errRead;


			if ((errRead = FileIO:: readLink (pdDirectory -> _pPathName,
				pRealPathName, 512)) != errNoError)
			{

				return errRead;
			}

			if ((pdDirectory -> _pdDir = opendir (pRealPathName)) ==
				(DIR *) NULL)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_OPENDIR_FAILED,
					2, pRealPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				delete [] pdDirectory -> _pPathName;
				pdDirectory -> _pPathName		= (char *) NULL;

				return err;
			}
		}
		else
		{
			if ((pdDirectory -> _pdDir = opendir (pdDirectory -> _pPathName)) ==
				(DIR *) NULL)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_OPENDIR_FAILED,
					2, pdDirectory -> _pPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				delete [] pdDirectory -> _pPathName;
				pdDirectory -> _pPathName		= (char *) NULL;

				return err;
			}
		}
	#endif


	return errNoError;
}

shared_ptr<FileIO::Directory> FileIO::openDirectory (string directoryPathName)
{
    Error errFileIO;
    shared_ptr<Directory> directory = make_shared<Directory>();
    
    if ((errFileIO = FileIO:: openDirectory (directoryPathName.c_str(),
	directory.get())) != errNoError)
    {
        throw runtime_error(string("FileIO::openDirectory failed: ")
                + (const char *) errFileIO);
    }
    
    return directory;    
}

Error FileIO:: readDirectory (Directory_p pdDirectory,
	Buffer_p pbDirectoryEntry,
	DirectoryEntryType_p pdetDirectoryEntryType)

{

	if (pdDirectory == (Directory_p) NULL ||
		pbDirectoryEntry == (Buffer_p) NULL ||
		pdetDirectoryEntryType == (DirectoryEntryType_p) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		struct _finddata_t			fdDirectoryEntry;
		long						lFindNextReturn;
		Error_t						errGetDirectoryEntryType;


		if (pdDirectory -> _lIdentifier == -1)
		{
			if (pbDirectoryEntry -> setBuffer (pdDirectory -> _pPathName) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append ("\\*") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if ((pdDirectory -> _lIdentifier = _findfirst (
				(const char *) (*pbDirectoryEntry),
				&fdDirectoryEntry)) == -1L)
			{
				if (errno == ENOENT)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_DIRECTORYFILESFINISHED);

					return err;
				}
				else
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FINDFIRST_FAILED,
						1, (const char *) (*pbDirectoryEntry));

					return err;
				}
			}

			lFindNextReturn				= 10;

			while (!strcmp (fdDirectoryEntry. name, ".") ||
				!strcmp (fdDirectoryEntry. name, ".."))
			{
				if ((lFindNextReturn = _findnext (pdDirectory -> _lIdentifier,
					&fdDirectoryEntry)) == -1)
					break;
			}

			if (lFindNextReturn == -1)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_DIRECTORYFILESFINISHED);

				return err;
			}

			if (pbDirectoryEntry -> setBuffer (pdDirectory -> _pPathName) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append (fdDirectoryEntry. name) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if ((errGetDirectoryEntryType = FileIO:: getDirectoryEntryType (
				(const char *) (*pbDirectoryEntry),
				pdetDirectoryEntryType)) != errNoError)
			{

				return errGetDirectoryEntryType;
			}

			if (pbDirectoryEntry -> setBuffer (fdDirectoryEntry. name) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}
		}
		else
		{
			while ((lFindNextReturn = _findnext (pdDirectory -> _lIdentifier,
				&fdDirectoryEntry)) == -1)
			{
				if (!strcmp (fdDirectoryEntry. name, ".") ||
					!strcmp (fdDirectoryEntry. name, ".."))
					continue;
				else
					break;
			}

			if (lFindNextReturn == -1)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_DIRECTORYFILESFINISHED);

				return err;
			}

			if (pbDirectoryEntry -> setBuffer (pdDirectory -> _pPathName) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append (fdDirectoryEntry. name) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if ((errGetDirectoryEntryType = FileIO:: getDirectoryEntryType (
				(const char *) (*pbDirectoryEntry),
				pdetDirectoryEntryType)) != errNoError)
			{

				return errGetDirectoryEntryType;
			}

			if (pbDirectoryEntry -> setBuffer (fdDirectoryEntry. name) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}
		}
	#else
		struct dirent				*pdDirent;
		Error_t						errGetDirectoryEntryType;


		while ((pdDirent = readdir (pdDirectory -> _pdDir)) !=
			(struct dirent *) NULL)
		{
			if (!strcmp (pdDirent -> d_name, ".") ||
				!strcmp (pdDirent -> d_name, ".."))
				continue;

			if (pbDirectoryEntry -> setBuffer (pdDirectory -> _pPathName) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if (pbDirectoryEntry -> append (pdDirent -> d_name) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				return err;
			}

			if ((errGetDirectoryEntryType = FileIO:: getDirectoryEntryType (
				(const char *) (*pbDirectoryEntry),
				pdetDirectoryEntryType)) != errNoError)
			{
				int					iErrno;
				unsigned long		ulUserDataBytes;


				errGetDirectoryEntryType. getUserData (&iErrno,
					&ulUserDataBytes);
				if ((unsigned long) errGetDirectoryEntryType ==
					TOOLS_LSTAT_FAILED &&
					iErrno == ENOENT)	// No such file or directory
				{
					// it means the file/directory was removed after
					// the above readdir is called

					continue;
				}

				return errGetDirectoryEntryType;
			}

			if (pbDirectoryEntry -> setBuffer (pdDirent -> d_name) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_SETBUFFER_FAILED);

				return err;
			}

			break;
		}

		if (pdDirent == (struct dirent *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_DIRECTORYFILESFINISHED);

			return err;
		}

	#endif


	return errNoError;
}

string FileIO:: readDirectory (shared_ptr<Directory> directory,
    DirectoryEntryType_p pdetDirectoryEntryType)

{
    Error errFileIO;
    Buffer_t directoryEntry;
    
    
    if ((errFileIO = FileIO:: readDirectory (directory.get(),
        &directoryEntry, pdetDirectoryEntryType)) != errNoError)
    {
        if ((long) errFileIO == TOOLS_FILEIO_DIRECTORYFILESFINISHED)
            throw DirectoryListFinished();
        else
            throw runtime_error(string("FileIO::readDirectory failed: ")
                + (const char *) errFileIO);
    }
    
    return string(directoryEntry.str());
}

Error FileIO:: closeDirectory (Directory_p pdDirectory)

{

	if (pdDirectory == (Directory_p) NULL ||
		pdDirectory -> _pPathName == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	delete [] pdDirectory -> _pPathName;
	pdDirectory -> _pPathName		= (char *) NULL;

	#ifdef WIN32
		if (pdDirectory -> _lIdentifier != -1)
		{
			_findclose (pdDirectory -> _lIdentifier);

			pdDirectory -> _lIdentifier		= -1;
		}
	#else
		if (pdDirectory -> _pdDir == (DIR *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		if (closedir (pdDirectory -> _pdDir) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_CLOSEDIR_FAILED);

			return err;
		}

		pdDirectory -> _pdDir			= (DIR *) NULL;
	#endif


	return errNoError;
}

void FileIO:: closeDirectory (shared_ptr<Directory> directory)

{
    Error errFileIO;
    
    if ((errFileIO = FileIO:: closeDirectory (directory.get())) != errNoError)
    {
        throw runtime_error(string("FileIO::closeDirectory failed: ")
            + (const char *) errFileIO);
    }    
}

Error FileIO:: getWorkingDirectory (char *pWorkingDirectory,
	unsigned long ulBufferLength)

{

	if (pWorkingDirectory == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		if (_getcwd (pWorkingDirectory, ulBufferLength) == (char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETCWD_FAILED, 1, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}
	#else
		if (getcwd (pWorkingDirectory, ulBufferLength) == (char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETCWD_FAILED, 1, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}
	#endif


	return errNoError;
}


Error FileIO:: isDirectoryExisting (const char *pDirectoryPathName,
	Boolean_p pbExist)

{

	#ifdef WIN32
		Buffer_t					bDirectory;
		struct _finddata_t			fdDirectoryEntry;
		long						lIdentifier;


		if (bDirectory. init (pDirectoryPathName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}

		if (bDirectory. append ("\\*") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (bDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if ((lIdentifier = _findfirst (
			(const char *) bDirectory,
			&fdDirectoryEntry)) == -1L)
		{
			if (errno == ENOENT)
				*pbExist			= false;
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FINDFIRST_FAILED,
					1, (const char *) bDirectory);

				if (bDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
		else
		{
			*pbExist			= true;

			_findclose (lIdentifier);
		}

		if (bDirectory. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
			
			return err;
		}
	#else
		Directory_t			dDirectory;
		Error_t				errDir;


//                cout << "idDirectoryExisting"
//                        << ", pDirectoryPathName: " << pDirectoryPathName
//                        << endl;
		if ((errDir = FileIO:: openDirectory (pDirectoryPathName,
			&dDirectory)) != errNoError)
		{
			int					iErrno;
			unsigned long		ulUserDataBytes;


			errDir. getUserData (&iErrno, &ulUserDataBytes);
			if (iErrno == ENOENT || iErrno == ENOTDIR)
			{
				*pbExist			= false;
			}
			else
				return errDir;
		}
		else
		{
			*pbExist				= true;

			if ((errDir = FileIO:: closeDirectory (&dDirectory)) != errNoError)
			{
				return errDir;
			}
		}
	#endif


	return errNoError;
}

bool FileIO:: directoryExisting (string directoryPathName)
{
    Error errFileIO;
    bool bExist;
    
    if ((errFileIO = FileIO::isDirectoryExisting (directoryPathName.c_str(),
	&bExist)) != errNoError)
    {
        throw runtime_error(string("FileIO::isDirectoryExisting failed: ")
                + (const char *) errFileIO);
    }
    
    return bExist;
}

Error FileIO:: getDirectoryEntryType (const char *pPathName,
	DirectoryEntryType_p pdetDirectoryEntryType)

{

	#ifdef WIN32
		struct _stat						sFileInfo;


		if (_stat (pPathName, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		if (sFileInfo. st_mode & _S_IFDIR)
			*pdetDirectoryEntryType			= TOOLS_FILEIO_DIRECTORY;
		else if (sFileInfo. st_mode & _S_IFREG)
			*pdetDirectoryEntryType			= TOOLS_FILEIO_REGULARFILE;
		else
			*pdetDirectoryEntryType			= TOOLS_FILEIO_UNKNOWN;
	#else
		struct stat							sFileInfo;


		if (lstat (pPathName, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		if (S_ISDIR (sFileInfo. st_mode))
			*pdetDirectoryEntryType			= TOOLS_FILEIO_DIRECTORY;
		else if (S_ISLNK (sFileInfo. st_mode))
			*pdetDirectoryEntryType			= TOOLS_FILEIO_LINKFILE;
		else if (S_ISREG (sFileInfo. st_mode))
			*pdetDirectoryEntryType			= TOOLS_FILEIO_REGULARFILE;
		else
			*pdetDirectoryEntryType			= TOOLS_FILEIO_UNKNOWN;
	#endif	


	return errNoError;
}

FileIO::DirectoryEntryType_t FileIO:: getDirectoryEntryType (string pathName)

{
    Error errFileIO;
    DirectoryEntryType_t detDirectoryEntryType;
    
    if ((errFileIO = FileIO:: getDirectoryEntryType (pathName.c_str(),
	&detDirectoryEntryType)) != errNoError)
    {
        throw runtime_error(string("FileIO::getDirectoryEntryType failed: ")
                + (const char *) errFileIO);
    }

    return detDirectoryEntryType;
}


Error FileIO:: getFileSystemInfo (const char *pPathName,
	unsigned long long *pullUsedInKB,
	unsigned long long *pullAvailableInKB,
	long *plPercentUsed)

{

	if (pPathName == (const char *) NULL ||
		pullUsedInKB == (unsigned long long *) NULL ||
		pullAvailableInKB == (unsigned long long *) NULL ||
		plPercentUsed == (long *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		DWORD				dwSectPerClust;
		DWORD				dwBytesPerSect;
		DWORD				dwFreeClusters;
		DWORD				dwTotalClusters;
		unsigned __int64	i64TotalBytes;
		unsigned __int64	i64FreeBytes;
		char				szDrive [4];
		const char			*pszRootPathName;


		if (pPathName [1] == ':')
		{
			szDrive [0]		= pPathName [0];
			szDrive [1]		= ':';
			szDrive [2]		= '\\';
			szDrive [3]		= '\0';

			pszRootPathName	= szDrive;
		}
		else
			pszRootPathName		= pPathName;

		if (GetDiskFreeSpace (
			pszRootPathName,
			&dwSectPerClust,
			&dwBytesPerSect,
			&dwFreeClusters,
			&dwTotalClusters) == 0)
		 {
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETDISKFREESPACE_FAILED,
				1, (long) GetLastError ());

			return err;
		}

        /* force 64-bit math */ 
        i64TotalBytes		= (__int64) dwTotalClusters * dwSectPerClust * dwBytesPerSect;
        i64FreeBytes		= (__int64) dwFreeClusters * dwSectPerClust * dwBytesPerSect;

		*pullUsedInKB		= (i64TotalBytes / 1024) - (i64FreeBytes / 1024);
		*pullAvailableInKB	= (i64FreeBytes / 1024);
		*plPercentUsed		= (long) (*pullUsedInKB * 100.0 / (*pullUsedInKB + *pullAvailableInKB)) + 0.5;
	#else
		struct statfs			sStatfs;
		long long				llBlocksUsed;


		if (statfs (pPathName, &sStatfs) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STATFS_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		if (sStatfs. f_blocks > 0)
		{
			llBlocksUsed			= sStatfs. f_blocks - sStatfs. f_bfree;
			*plPercentUsed		= (long)
				((llBlocksUsed * 100.0 / (llBlocksUsed + sStatfs. f_bavail)) +
				0.5);

			*pullUsedInKB			= llBlocksUsed *
				(sStatfs. f_bsize / 1024.0);
			*pullAvailableInKB		= sStatfs. f_bavail *
				(sStatfs. f_bsize / 1024.0);
		}
		else
		{
			// no blocks available

			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STATFS_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}
	#endif


	return errNoError;
}

void FileIO:: getFileSystemInfo (string pathName,
	unsigned long long *pullUsedInKB,
	unsigned long long *pullAvailableInKB,
	long *plPercentUsed)

{
    Error errFileIO;

    if ((errFileIO = FileIO:: getFileSystemInfo (pathName.c_str(),
	pullUsedInKB, pullAvailableInKB, plPercentUsed)) != errNoError)
    {
        throw runtime_error(string("FileIO::getFileSystemInfo failed: ")
                + (const char *) errFileIO);
    }
}


Error FileIO:: getDirectorySizeInBytes (const char *pDirectoryPathName,
	unsigned long long *pullDirectorySizeInBytes)

{
	return FileIO:: getDirectoryUsage (pDirectoryPathName,
		pullDirectorySizeInBytes);
}


unsigned long long FileIO:: getDirectorySizeInBytes (string directoryPathName)
{
    Error errFileIO;
    unsigned long long  ullDirectorySizeInBytes;
    
    if ((errFileIO = FileIO:: getDirectorySizeInBytes (directoryPathName.c_str(),
	&ullDirectorySizeInBytes)) != errNoError)
    {
        throw runtime_error(string("FileIO::getDirectorySizeInBytes failed: ")
                + (const char *) errFileIO);
    }
    
    return ullDirectorySizeInBytes;
}

Error FileIO:: getDirectoryUsage (const char *pDirectoryPathName,
	unsigned long long *pullDirectoryUsageInBytes)

{

	unsigned long					ulFileSizeInBytes;
	Boolean_t						bIsExist;
	FileIO:: Directory_t			dDirectory;
	Error_t							errFileIO;
	Buffer_t						bDirectoryEntry;
	FileIO:: DirectoryEntryType_t	detDirectoryEntryType;
	Buffer_t						bDirectoryPathName;


	if (pDirectoryPathName == (const char *) NULL ||
		pullDirectoryUsageInBytes == (unsigned long long *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (bDirectoryPathName. init (pDirectoryPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	#ifdef WIN32
		if (pDirectoryPathName [strlen (pDirectoryPathName) - 1] != '\\')
		{
			if (bDirectoryPathName. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#else
		if (pDirectoryPathName [strlen (pDirectoryPathName) - 1] != '/')
		{
			if (bDirectoryPathName. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#endif

	if ((errFileIO = FileIO:: isDirectoryExisting (
		(const char *) bDirectoryPathName,
		&bIsExist)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (!bIsExist)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_DIRECTORYNOTEXISTING,
			1, pDirectoryPathName);

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (FileIO:: getFileSizeInBytes (
		(const char *) bDirectoryPathName,
		&ulFileSizeInBytes, false) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_GETFILESIZEINBYTES_FAILED,
			1, (const char *) bDirectoryEntry);

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	*pullDirectoryUsageInBytes			= ulFileSizeInBytes;

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if ((errFileIO = FileIO:: openDirectory (
		(const char *) bDirectoryPathName, &dDirectory)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	while ((errFileIO = FileIO:: readDirectory (&dDirectory,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		switch (detDirectoryEntryType)
		{
			case FileIO:: TOOLS_FILEIO_REGULARFILE:
			#ifdef WIN32
			#else
				case FileIO:: TOOLS_FILEIO_LINKFILE:
			#endif
				{
					if (bDirectoryEntry. insertAt (0,
						(const char *) bDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if (FileIO:: getFileSizeInBytes (
						(const char *) bDirectoryEntry,
						&ulFileSizeInBytes, false) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_GETFILESIZEINBYTES_FAILED,
							1, (const char *) bDirectoryEntry);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					(*pullDirectoryUsageInBytes)			+=
						ulFileSizeInBytes;
				}

				break;
			case FileIO:: TOOLS_FILEIO_DIRECTORY:
				{
					unsigned long long		ullLocalDirectoryUsageInBytes;


					if (bDirectoryEntry. insertAt (0,
						(const char *) bDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = FileIO:: getDirectoryUsage (
						(const char *) bDirectoryEntry,
						&ullLocalDirectoryUsageInBytes)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_GETDIRECTORYUSAGE_FAILED,
							1, (const char *) bDirectoryEntry);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}

					(*pullDirectoryUsageInBytes)			+=
						ullLocalDirectoryUsageInBytes;
				}

				break;
			case FileIO:: TOOLS_FILEIO_UNKNOWN:
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_UNKNOWNFILETYPE,
						2, (const char *) bDirectoryPathName,
						(const char *) bDirectoryEntry);

					if (FileIO:: closeDirectory (&dDirectory) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
					}

					if (bDirectoryEntry. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					if (bDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return err;
				}

				break;
		}
	}

	if ((long) errFileIO != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
	{
		if (FileIO:: closeDirectory (&dDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
		}

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (FileIO:: closeDirectory (&dDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDirectoryEntry. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}

unsigned long long FileIO:: getDirectoryUsage (string directoryPathName)

{
    Error errFileIO;
    unsigned long long  ullDirectorySizeInBytes;
    
    if ((errFileIO = FileIO:: getDirectoryUsage (directoryPathName.c_str(),
	&ullDirectorySizeInBytes)) != errNoError)
    {
        if ((unsigned long) errFileIO == TOOLS_FILEIO_DIRECTORYNOTEXISTING)
            throw DirectoryNotExisting();
        else
            throw runtime_error(string("FileIO::getDirectoryUsage failed: ")
                + (const char *) errFileIO);
    }
    
    return ullDirectorySizeInBytes;
}

Error FileIO:: moveDirectory (const char *pSrcPathName,
	const char *pDestPathName, int mDirectoryMode)

{

	Boolean_t						bIsExist;
	FileIO:: Directory_t			dDirectory;
	Error_t							errFileIO;
	Buffer_t						bDirectoryEntry;
	FileIO:: DirectoryEntryType_t	detDirectoryEntryType;
	Buffer_t						bSrcDirectoryPathName;
	Buffer_t						bDestDirectoryPathName;


	if (pSrcPathName == (const char *) NULL ||
		pDestPathName == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (bSrcDirectoryPathName. init (pSrcPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	#ifdef WIN32
		if (pSrcPathName [strlen (pSrcPathName) - 1] != '\\')
		{
			if (bSrcDirectoryPathName. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#else
		if (pSrcPathName [strlen (pSrcPathName) - 1] != '/')
		{
			if (bSrcDirectoryPathName. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#endif

	if ((errFileIO = FileIO:: isDirectoryExisting (
		(const char *) bSrcDirectoryPathName,
		&bIsExist)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (!bIsExist)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_DIRECTORYNOTEXISTING,
			1, (const char *) bSrcDirectoryPathName);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDestDirectoryPathName. init (pDestPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	#ifdef WIN32
		if (pDestPathName [strlen (pDestPathName) - 1] != '\\')
		{
			if (bDestDirectoryPathName. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDestDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#else
		if (pDestPathName [strlen (pDestPathName) - 1] != '/')
		{
			if (bDestDirectoryPathName. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDestDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#endif

	if ((errFileIO = FileIO:: isDirectoryExisting (
		(const char *) bDestDirectoryPathName,
		&bIsExist)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (!bIsExist)
	{
		if (createDirectory ((const char *) bDestDirectoryPathName,
			mDirectoryMode, true, false) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
				1, (const char *) bDestDirectoryPathName);

			if (bDestDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bSrcDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if ((errFileIO = FileIO:: openDirectory (
		(const char *) bSrcDirectoryPathName, &dDirectory)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	while ((errFileIO = FileIO:: readDirectory (&dDirectory,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		switch (detDirectoryEntryType)
		{
			case FileIO:: TOOLS_FILEIO_REGULARFILE:
				{
					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = moveFile (
						(const char *) bDirectoryEntry,
						(const char *) bDestDirectoryPathName)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_MOVEFILE_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				break;
			#ifdef WIN32
			#else
				case FileIO:: TOOLS_FILEIO_LINKFILE:
				{
					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = moveLink (
						(const char *) bDirectoryEntry,
						(const char *) bDestDirectoryPathName)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_MOVELINK_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				break;
			#endif
			case FileIO:: TOOLS_FILEIO_DIRECTORY:
				{
					Buffer_t			bNewDestDirectoryPathName;


					if (bNewDestDirectoryPathName. init (
						(const char *) bDestDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INIT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if (bNewDestDirectoryPathName. append (
						(const char *) bDirectoryEntry) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_APPEND_FAILED);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = moveDirectory (
						(const char *) bDirectoryEntry,
						(const char *) bNewDestDirectoryPathName,
						mDirectoryMode)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_MOVEDIRECTORY_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}

					if (bNewDestDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}
				}

				break;
			case FileIO:: TOOLS_FILEIO_UNKNOWN:
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_UNKNOWNFILETYPE,
						2, (const char *) bSrcDirectoryPathName,
						(const char *) bDirectoryEntry);

					if (FileIO:: closeDirectory (&dDirectory) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
					}

					if (bDirectoryEntry. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					if (bDestDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					if (bSrcDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return err;
				}

				break;
		}
	}

	if ((long) errFileIO != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
	{
		if (FileIO:: closeDirectory (&dDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
		}

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (FileIO:: closeDirectory (&dDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if ((errFileIO = FileIO:: removeDirectory (
		(const char *) bSrcDirectoryPathName, false)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_REMOVEDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (bDirectoryEntry. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDestDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bSrcDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


void FileIO:: moveDirectory (string srcPathName,
	string destPathName, int mDirectoryMode)
{
    
    Error errFileIO;
    
    if ((errFileIO = FileIO:: moveDirectory (srcPathName.c_str(),
	destPathName.c_str(), mDirectoryMode)) != errNoError)
    {
        throw runtime_error(string("FileIO::moveDirectory failed: ")
                + (const char *) errFileIO);
    }
}


Error FileIO:: copyDirectory (const char *pSrcPathName,
	const char *pDestPathName, int mDirectoryMode)

{

	Boolean_t						bIsExist;
	FileIO:: Directory_t			dDirectory;
	Error_t							errFileIO;
	Buffer_t						bDirectoryEntry;
	FileIO:: DirectoryEntryType_t	detDirectoryEntryType;
	Buffer_t						bSrcDirectoryPathName;
	Buffer_t						bDestDirectoryPathName;


	if (pSrcPathName == (const char *) NULL ||
		pDestPathName == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (bSrcDirectoryPathName. init (pSrcPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	#ifdef WIN32
		if (pSrcPathName [strlen (pSrcPathName) - 1] != '\\')
		{
			if (bSrcDirectoryPathName. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#else
		if (pSrcPathName [strlen (pSrcPathName) - 1] != '/')
		{
			if (bSrcDirectoryPathName. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#endif

	if ((errFileIO = FileIO:: isDirectoryExisting (
		(const char *) bSrcDirectoryPathName,
		&bIsExist)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (!bIsExist)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_DIRECTORYNOTEXISTING,
			1, (const char *) bSrcDirectoryPathName);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDestDirectoryPathName. init (pDestPathName) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	#ifdef WIN32
		if (pDestPathName [strlen (pDestPathName) - 1] != '\\')
		{
			if (bDestDirectoryPathName. append ("\\") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDestDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#else
		if (pDestPathName [strlen (pDestPathName) - 1] != '/')
		{
			if (bDestDirectoryPathName. append ("/") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);

				if (bDestDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				if (bSrcDirectoryPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
	#endif

	if ((errFileIO = FileIO:: isDirectoryExisting (
		(const char *) bDestDirectoryPathName,
		&bIsExist)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (!bIsExist)
	{
		if (createDirectory ((const char *) bDestDirectoryPathName,
			mDirectoryMode, true, false) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
				1, (const char *) bDestDirectoryPathName);

			if (bDestDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bSrcDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}
	}

	if (bDirectoryEntry. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if ((errFileIO = FileIO:: openDirectory (
		(const char *) bSrcDirectoryPathName, &dDirectory)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPENDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	while ((errFileIO = FileIO:: readDirectory (&dDirectory,
		&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
	{
		switch (detDirectoryEntryType)
		{
			case FileIO:: TOOLS_FILEIO_REGULARFILE:
				{
					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = copyFile (
						(const char *) bDirectoryEntry,
						(const char *) bDestDirectoryPathName)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_COPYFILE_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				break;
			#ifdef WIN32
			#else
				case FileIO:: TOOLS_FILEIO_LINKFILE:
				{
					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = createLink (
						(const char *) bDirectoryEntry,
						(const char *) bDestDirectoryPathName, true, false,
						mDirectoryMode)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CREATELINK_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				break;
			#endif
			case FileIO:: TOOLS_FILEIO_DIRECTORY:
				{
					Buffer_t			bNewDestDirectoryPathName;


					if (bNewDestDirectoryPathName. init (
						(const char *) bDestDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INIT_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if (bNewDestDirectoryPathName. append (
						(const char *) bDirectoryEntry) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_APPEND_FAILED);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if (bDirectoryEntry. insertAt (0,
						(const char *) bSrcDirectoryPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_INSERTAT_FAILED);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					if ((errFileIO = copyDirectory (
						(const char *) bDirectoryEntry,
						(const char *) bNewDestDirectoryPathName,
						mDirectoryMode)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_COPYDIRECTORY_FAILED,
							2, (const char *) bSrcDirectoryPathName,
							(const char *) bDestDirectoryPathName);

						if (bNewDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}

					if (bNewDestDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDestDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bSrcDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}
				}

				break;
			case FileIO:: TOOLS_FILEIO_UNKNOWN:
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_UNKNOWNFILETYPE,
						2, (const char *) bSrcDirectoryPathName,
						(const char *) bDirectoryEntry);

					if (FileIO:: closeDirectory (&dDirectory) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
					}

					if (bDirectoryEntry. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					if (bDestDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					if (bSrcDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return err;
				}

				break;
		}
	}

	if ((long) errFileIO != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
	{
		if (FileIO:: closeDirectory (&dDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
		}

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errFileIO;
	}

	if (FileIO:: closeDirectory (&dDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDirectoryEntry. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bDestDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bDestDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		if (bSrcDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bSrcDirectoryPathName. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}

void FileIO:: copyDirectory (string srcPathName,
	string destPathName, int mDirectoryMode)
{
    Error errFileIO;
    
    if ((errFileIO = FileIO:: copyDirectory (srcPathName.c_str(),
	destPathName.c_str(), mDirectoryMode)) != errNoError)
    {
        throw runtime_error(string("FileIO::copyDirectory failed: ")
                + (const char *) errFileIO);
    }
}

#ifdef WIN32
#else
	Error FileIO:: changePermission (const char *pPathName, mode_t mMode)

	{

		if (chmod (pPathName, mMode) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_CHMOD_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}


		return errNoError;
	}
#endif


#ifdef WIN32
	Error FileIO:: readLink (const char *pLinkPathName,
		char *pRealPathName,
		unsigned long ulRealPathNameLength)

	{

		HRESULT					hResult;
		IShellLink				*pShellLink;
		IPersistFile			*pPersistFile;
		WCHAR					wszLinkPathName [MAX_PATH]; 
		WIN32_FIND_DATA			wfd;
 

		// Get a pointer to the IShellLink interface.
		// It is assumed that CoInitialize has already been called. 
		hResult = CoCreateInstance (CLSID_ShellLink, NULL,
			CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*) &pShellLink);

		if (!SUCCEEDED (hResult))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_COCREATEINSTANCE_FAILED);

			return err;
		}

		// Get a pointer to the IPersistFile interface.
		hResult = pShellLink -> QueryInterface (IID_IPersistFile,
			(void**) &pPersistFile); 

		if (!SUCCEEDED(hResult)) 
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SHELLLINK_QUERYINTERFACE_FAILED);

			return err;
		}

		// Ensure that the string is Unicode. 
		MultiByteToWideChar(CP_ACP, 0, pLinkPathName, -1,
			wszLinkPathName, MAX_PATH);

		// Add code here to check return value from MultiByteWideChar
		// for success.

		// Load the shortcut. 
		hResult = pPersistFile -> Load (wszLinkPathName, STGM_READ);

		if (!SUCCEEDED(hResult)) 
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_PERSISTFILE_LOAD_FAILED);

			return err;
		}

		// Resolve the link
		/*
		hResult = pShellLink -> Resolve(hwnd, 0);

		if (!SUCCEEDED(hResult)) 
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_RESOLVE_FAILED);

			return err;
		}
		*/

		// Get the path to the link target
		hResult = pShellLink -> GetPath (pRealPathName, MAX_PATH,
			(WIN32_FIND_DATA*) &wfd, SLGP_SHORTPATH);

		if (!SUCCEEDED(hResult)) 
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SHELLLINK_GETPATH_FAILED);

			return err;
		}

		/*
		hResult = StringCbCopy(lpszPath, iPathBufferSize, szGotPath);
		if (SUCCEEDED(hres))
		{
			// Handle success
		}
		else
		{
			// Handle the error
		}
		*/

		// Release the pointer to the IPersistFile interface. 
		pPersistFile -> Release(); 

		// Release the pointer to the IShellLink interface. 
		pShellLink -> Release(); 


		return errNoError;
	}
#else
	Error FileIO:: readLink (const char *pLinkPathName,
		char *pRealPathName,
		unsigned long ulRealPathNameLength)

	{
		int					iCharsCount;


		if ((iCharsCount = readlink (pLinkPathName, pRealPathName,
			ulRealPathNameLength)) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_READLINK_FAILED, 2, pLinkPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		if (iCharsCount >= ulRealPathNameLength)
		{
			// readlink truncate the content
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_BUFFERTOOSMALL);

			return err;
		}

		pRealPathName [iCharsCount]			= '\0';

		if (pRealPathName [0] != '/')
		{
			// The real path is not absolute. Add the relative path
			//	contained in pLinkPathName

			const char			*pLinkPathNameDirectoryEnd;
			unsigned long		ulCharsNumberToAdd;


			pLinkPathNameDirectoryEnd		= strrchr (pLinkPathName, '/');

			if (pLinkPathNameDirectoryEnd != (const char *) NULL)
			{
				ulCharsNumberToAdd		=
					pLinkPathNameDirectoryEnd - pLinkPathName + 1;

				if (iCharsCount + ulCharsNumberToAdd >=
					ulRealPathNameLength)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_BUFFERTOOSMALL);

					return err;
				}

				memmove (pRealPathName + ulCharsNumberToAdd,
					pRealPathName, iCharsCount + 1);
				memcpy (pRealPathName, pLinkPathName,
					ulCharsNumberToAdd);
			}
		}


		return errNoError;
	}
#endif


Error FileIO:: createDirectory (const char *pPathName,
	int mMode, Boolean_t bNoErrorIfExists, Boolean_t bRecursive)

{

//    cout << "createDirectory"
//            << ", pPathName: " << pPathName
//            << endl;
            
	if (pPathName == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (bRecursive)
	{
		const char			*pLastSlash;


		#ifdef WIN32
			if ((pLastSlash = strrchr (pPathName, '\\')) !=
				(const char *) NULL && pPathName != pLastSlash)
		#else
			if ((pLastSlash = strrchr (pPathName, '/')) !=
				(const char *) NULL && pPathName != pLastSlash)
		#endif
		{
			Buffer_t			bPreviousDirectory;
			Boolean_t			bIsExist;
			Error_t				errFileIO;


			if (bPreviousDirectory. init (pPathName,
				pLastSlash - pPathName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);

				return err;
			}

//    cout << "isDirectoryExisting"
//            << ", bPreviousDirectory: " << (const char *) bPreviousDirectory
//            << endl;
			if ((errFileIO = FileIO:: isDirectoryExisting (
				(const char *) bPreviousDirectory, &bIsExist)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

				if (bPreviousDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return errFileIO;
			}

//    cout << "isDirectoryExisting"
//            << ", bIsExist: " << bIsExist
//            << endl;
			if (!bIsExist)
			{
				if ((errFileIO = FileIO:: createDirectory (
					(const char *) bPreviousDirectory,
					mMode, true, bRecursive)) != errNoError)
				{
					int					iErrno;
					unsigned long		ulUserDataBytes;


					errFileIO. getUserData (&iErrno, &ulUserDataBytes);
					if (iErrno == EEXIST)
					{
						// it means the directory already exist, may be
						// it was created after the above check
					}
					else
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
							1, (const char *) bPreviousDirectory);

						if (bPreviousDirectory. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}
			}

			if (bPreviousDirectory. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);

				return err;
			}
		}
	}

	#ifdef WIN32
		if (_mkdir (pPathName) == -1)
		{
			if (bNoErrorIfExists && errno == EEXIST)
			{
			}
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_MKDIR_FAILED,
					2, pPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}
	#else
		if (mkdir (pPathName, mMode) == -1)
		{
			if (bNoErrorIfExists && errno == EEXIST)
			{
			}
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_MKDIR_FAILED,
					2, pPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}
	#endif


	return errNoError;
}

void FileIO:: createDirectory (string pathName,
	int mMode, bool bNoErrorIfExists, bool bRecursive)
{
    Error       errFileIO;
    
    if ((errFileIO = FileIO::createDirectory (pathName.c_str(),
	mMode, bNoErrorIfExists, bRecursive)) != errNoError)
    {
        throw runtime_error(string("FileIO::createDirectory failed: ")
                + (const char *) errFileIO);
    }
}

Error FileIO:: removeDirectory (const char *pPathName,
	Boolean_t bRemoveRecursively)

{

	if (bRemoveRecursively)
	{
		Boolean_t						bIsExist;
		Buffer_t						bDirectoryPathName;
		FileIO:: Directory_t			dDirectory;
		Error_t							errFileIO;
		Buffer_t						bDirectoryEntry;
		FileIO:: DirectoryEntryType_t	detDirectoryEntryType;


		if (pPathName == (const char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		if (bDirectoryPathName. init (pPathName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}

		#ifdef WIN32
			if (pPathName [strlen (pPathName) - 1] != '\\')
			{
				if (bDirectoryPathName. append ("\\") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					if (bDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return err;
				}
			}
		#else
			if (pPathName [strlen (pPathName) - 1] != '/')
			{
				if (bDirectoryPathName. append ("/") != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);

					if (bDirectoryPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return err;
				}
			}
		#endif

		if ((errFileIO = FileIO:: isDirectoryExisting (
			(const char *) bDirectoryPathName,
			&bIsExist)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return errFileIO;
		}

		if (!bIsExist)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_DIRECTORYNOTEXISTING,
				1, (const char *) bDirectoryPathName);

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (bDirectoryEntry. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if ((errFileIO = FileIO:: openDirectory (
			(const char *) bDirectoryPathName, &dDirectory)) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPENDIRECTORY_FAILED);

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return errFileIO;
		}

		while ((errFileIO = FileIO:: readDirectory (&dDirectory,
			&bDirectoryEntry, &detDirectoryEntryType)) == errNoError)
		{
			switch (detDirectoryEntryType)
			{
				case FileIO:: TOOLS_FILEIO_REGULARFILE:
					{
						if (bDirectoryEntry. insertAt (0,
							(const char *) bDirectoryPathName) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_INSERTAT_FAILED);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return err;
						}

						if ((errFileIO = remove ((const char *) bDirectoryEntry)) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_REMOVE_FAILED,
								1, (const char *) bDirectoryPathName);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return errFileIO;
						}
					}

					break;
				#ifdef WIN32
				#else
					case FileIO:: TOOLS_FILEIO_LINKFILE:
					{
						if (bDirectoryEntry. insertAt (0,
							(const char *) bDirectoryPathName) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_INSERTAT_FAILED);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return err;
						}

						if ((errFileIO = remove ((const char *) bDirectoryEntry)) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_REMOVE_FAILED,
								1, (const char *) bDirectoryPathName);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return errFileIO;
						}
					}

					break;
				#endif
				case FileIO:: TOOLS_FILEIO_DIRECTORY:
					{
					if (bDirectoryEntry. insertAt (0,
							(const char *) bDirectoryPathName) != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_INSERTAT_FAILED);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return err;
						}

						if ((errFileIO = removeDirectory (
							(const char *) bDirectoryEntry, true)) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_REMOVEDIRECTORY_FAILED);

							if (FileIO:: closeDirectory (&dDirectory) !=
								errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
							}

							if (bDirectoryEntry. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							if (bDirectoryPathName. finish () != errNoError)
							{
								Error err = ToolsErrors (__FILE__, __LINE__,
									TOOLS_BUFFER_FINISH_FAILED);
							}

							return errFileIO;
						}
					}

					break;
				case FileIO:: TOOLS_FILEIO_UNKNOWN:
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_UNKNOWNFILETYPE,
							2, (const char *) bDirectoryPathName,
							(const char *) bDirectoryEntry);

						if (FileIO:: closeDirectory (&dDirectory) !=
							errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
						}

						if (bDirectoryEntry. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						if (bDirectoryPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}

					break;
			}
		}

		if ((long) errFileIO != TOOLS_FILEIO_DIRECTORYFILESFINISHED)
		{
			if (FileIO:: closeDirectory (&dDirectory) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);
			}

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return errFileIO;
		}

		if (FileIO:: closeDirectory (&dDirectory) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSEDIRECTORY_FAILED);

			if (bDirectoryEntry. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (bDirectoryEntry. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);

			if (bDirectoryPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (bDirectoryPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);

			return err;
		}
	}

	#ifdef WIN32
		if (_rmdir (pPathName) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_RMDIR_FAILED,
				2, pPathName, (long) errno);

			return err;
		}
	#else
		if (rmdir (pPathName) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_RMDIR_FAILED,
				2, pPathName, (long) errno);

			return err;
		}
	#endif


	return errNoError;
}

void FileIO:: removeDirectory (string pathName, bool removeRecursively)
{
    Error errFileIO;
    
    if ((errFileIO = FileIO:: removeDirectory (pathName.c_str(), removeRecursively)) != errNoError)
    {
        throw runtime_error(string("FileIO::removeDirectory failed: ")
                + (const char *) errFileIO);
    }
}

#ifdef USEGZIPLIB
	Error FileIO:: gzip (const char *pUnCompressedPathName,
		Boolean_t bSourceFileToBeRemoved,
		long lSizeOfEachBlockToGzip)

	{

		Buffer_t			bCompressedPathName;
		#ifdef WIN32
			__int64			llUncompressedBytesRead;
		#else
			long long		llUncompressedBytesRead;
		#endif
		int					iSrcFileDescriptor;
		unsigned char		*pUncompressedBuffer;
		gzFile				gzfGZipFile;


		if (bCompressedPathName. init (pUnCompressedPathName) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}

		if (bCompressedPathName. append (".gz") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			if (bCompressedPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (FileIO:: open (pUnCompressedPathName,
			O_RDONLY, &iSrcFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED,
				1, pUnCompressedPathName);

			if (bCompressedPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if ((pUncompressedBuffer = new unsigned char [
			lSizeOfEachBlockToGzip * 1024]) == (unsigned char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			if (FileIO:: close (iSrcFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
			}

			if (bCompressedPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if ((gzfGZipFile = gzopen ((const char *) bCompressedPathName,
			"wb+")) == (gzFile) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GZOPEN_FAILED);

			delete [] pUncompressedBuffer;

			if (FileIO:: close (iSrcFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
			}

			if (bCompressedPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		do
		{
			#ifdef WIN32
				if (FileIO:: readChars (iSrcFileDescriptor,
					(char *) pUncompressedBuffer,
					(__int64) (lSizeOfEachBlockToGzip * 1024),
					&llUncompressedBytesRead) != errNoError)
			#else
				if (FileIO:: readChars (iSrcFileDescriptor,
					(char *) pUncompressedBuffer,
					(unsigned long long) (lSizeOfEachBlockToGzip * 1024),
					&llUncompressedBytesRead) != errNoError)
			#endif
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_READCHARS_FAILED);

				gzclose (gzfGZipFile);

				delete [] pUncompressedBuffer;

				if (FileIO:: close (iSrcFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
				}

				if (bCompressedPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			if (llUncompressedBytesRead == 0)
				continue;

			if (gzwrite (gzfGZipFile, pUncompressedBuffer,
				llUncompressedBytesRead) == 0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_GZWRITE_FAILED);

				gzclose (gzfGZipFile);

				delete [] pUncompressedBuffer;

				if (FileIO:: close (iSrcFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
				}

				if (bCompressedPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}
		#ifdef WIN32
			while (llUncompressedBytesRead ==
				(__int64) (lSizeOfEachBlockToGzip * 1024));
		#else
			while (llUncompressedBytesRead ==
				(long long) (lSizeOfEachBlockToGzip * 1024));
		#endif

		// The gzclose function closes iDestFileDescriptor too
		gzclose (gzfGZipFile);

		delete [] pUncompressedBuffer;

		if (FileIO:: close (iSrcFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);

			if (bCompressedPathName. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_FINISH_FAILED);
			}

			return err;
		}

		if (bCompressedPathName. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);

			return err;
		}

		if (bSourceFileToBeRemoved)
		{
			Error_t				errDelete;


			if ((errDelete = FileIO:: remove (
				pUnCompressedPathName)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_REMOVE_FAILED);

				return err;
			}
		}


		return errNoError;
	}
#endif


Error FileIO:: getFileTime (const char *pPathName,
	time_t *ptLastModificationTime)

{

	#ifdef WIN32
		struct _stat								sFileInfo;
		int											iFileDescriptor;
		Error										errOpen;


		if ((errOpen = FileIO:: open (pPathName, O_RDONLY,
			&iFileDescriptor)) != errNoError)
		{
			/*
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED);

			return err;
			*/
			return errOpen;
		}

		if (_fstat (iFileDescriptor, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FSTAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		*ptLastModificationTime				= sFileInfo. st_mtime;

		if (FileIO:: close (iFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);

			return err;
		}
	#else
		struct stat										sFileInfo;


		if (lstat (pPathName, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		*ptLastModificationTime				= sFileInfo. st_mtime;
	#endif


	return errNoError;
}

chrono::system_clock::time_point FileIO:: getFileTime (string pathName)

{
    time_t tLastModificationTime;
    Error errFileIO;
    
    if ((errFileIO = FileIO::getFileTime (pathName.c_str(), &tLastModificationTime)) != errNoError)
    {
        int			iErrno;
        unsigned long		ulUserDataBytes;


        errFileIO. getUserData (&iErrno, &ulUserDataBytes);
        if (iErrno != ENOENT)	// ENOENT: file not found
        {
            throw FileNotExisting();
        }   
        else
        {
            throw runtime_error(string ("FileIO::getFileTime failed")
                    + ", iErrno: " + to_string(iErrno)
                    );
        }
    }
    
    return chrono::system_clock::from_time_t(tLastModificationTime);
}


Error FileIO:: getFileSizeInBytes (const char *pPathName,
	unsigned long *pulFileSize, Boolean_t bInCaseOfLinkHasItToBeRead)

{

	#ifdef WIN32
		struct _stat								sFileInfo;
		int											iFileDescriptor;
		Error										errOpen;


		if ((errOpen = FileIO:: open (pPathName, O_RDONLY,
			&iFileDescriptor)) != errNoError)
		{
			/*
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED);

			return err;
			*/
			return errOpen;
		}

		if (_fstat (iFileDescriptor, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FSTAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		*pulFileSize						= sFileInfo. st_size;

		if (FileIO:: close (iFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);

			return err;
		}
	#else
		struct stat										sFileInfo;


		if (lstat (pPathName, &sFileInfo) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData ((void *) (&errno), sizeof (int));

			return err;
		}

		if (bInCaseOfLinkHasItToBeRead && S_ISLNK (sFileInfo. st_mode))
		{
			Error_t			errRead;
			char			pRealPathName [512];


			if ((errRead = FileIO:: readLink (pPathName,
				pRealPathName, 512)) != errNoError)
			{

				return errRead;
			}

			if (lstat (pRealPathName, &sFileInfo) == -1)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}

		*pulFileSize				= sFileInfo. st_size;
	#endif


	return errNoError;
}

unsigned long FileIO:: getFileSizeInBytes (string pathName,
	bool inCaseOfLinkHasItToBeRead)
{
    Error errFileIO;
    unsigned long   ulFileSize;
    
    if ((errFileIO = FileIO:: getFileSizeInBytes (pathName.c_str(),
	&ulFileSize, inCaseOfLinkHasItToBeRead)) != errNoError)
    {
        throw runtime_error(string("FileIO::getFileSizeInBytes failed: ")
                + (const char *) errFileIO);
    }
    
    return ulFileSize;
}

Error FileIO:: isFileExisting (const char *pPathName, Boolean_p pbExist,
	long maxMillisecondsToWait, long milliSecondsWaitingBetweenChecks)

{

	/*
	 * 	previous implementation.
	 * 	The problem was that it returns true also in case of a directory
	#ifdef WIN32
		if (::_access (pPathName, 0) == -1)
	#else
		if (::access (pPathName, F_OK))
	#endif
	{
		*pbExist			= false;

		// it's difficolt to handle the errors
		// Error err = ToolsErrors (__FILE__, __LINE__,
		//	TOOLS_ACCESS_FAILED, 2, pPathName, (long) errno);

		// return err;
	}
	else
		*pbExist			= true;
	*/

	chrono::system_clock::time_point start =
		chrono::system_clock::now();
	chrono::system_clock::time_point end =
		start + chrono::milliseconds(maxMillisecondsToWait);

	bool firstCheck = true;

	do
	{
		if (!firstCheck)
			this_thread::sleep_for(chrono::milliseconds(milliSecondsWaitingBetweenChecks));
		else
			firstCheck = false;

	#ifdef WIN32
		struct _stat						sFileInfo;


		if (_stat (pPathName, &sFileInfo) == -1)
		{
			*pbExist			= false;
			/*
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STAT_FAILED, 2, pPathName, (long) errno);
			err. setUserData (&errno, sizeof (int));

			return err;
			*/
		}
		else
		{
			if (sFileInfo. st_mode & _S_IFDIR)
				*pbExist			= false;
			else if (sFileInfo. st_mode & _S_IFREG)
				*pbExist			= true;
			else
				*pbExist			= false;
		}
	#else
		struct stat							sFileInfo;


		if (lstat (pPathName, &sFileInfo) == -1)
		{
			if (errno == ENOENT)
				*pbExist			= false;
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}
		else
		{
			// lstat successful

			if (S_ISDIR (sFileInfo. st_mode))
				*pbExist			= false;
			else if (S_ISLNK (sFileInfo. st_mode))
			{
				Error_t			errRead;
				char			pRealPathName [512];


				if ((errRead = FileIO:: readLink (pPathName,
					pRealPathName, 512)) != errNoError)
				{

					return errRead;
				}

				if (lstat (pRealPathName, &sFileInfo) == -1)
				{
					if (errno == ENOENT)
						*pbExist			= false;
					else
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_LSTAT_FAILED, 2, pPathName, (long) errno);
						err. setUserData ((void *) (&errno), sizeof (int));

						return err;
					}
				}
				else
				{
					if (S_ISDIR (sFileInfo. st_mode))
						*pbExist			= false;
					else if (S_ISLNK (sFileInfo. st_mode))
						*pbExist			= false;
					else if (S_ISREG (sFileInfo. st_mode))
						*pbExist			= true;
					else
						*pbExist			= false;
				}
			}
			else if (S_ISREG (sFileInfo. st_mode))
				*pbExist			= true;
			else
				*pbExist			= false;
		}
	#endif	
	}
	while(!(*pbExist) && chrono::system_clock::now() < end);


	return errNoError;
}


Boolean_t FileIO:: isFileExisting (const char *pPathName,
		long maxMillisecondsToWait, long milliSecondsWaitingBetweenChecks)

{
	Boolean_t			bExist;


	bExist		= false;

	FileIO:: isFileExisting (pPathName, &bExist,
			maxMillisecondsToWait, milliSecondsWaitingBetweenChecks);


	return bExist;

}

bool FileIO:: fileExisting (string pathName,
		long maxMillisecondsToWait, long milliSecondsWaitingBetweenChecks)

{
	Boolean_t			bExist;


	bExist		= false;

	FileIO:: isFileExisting (pathName.c_str(), &bExist,
			maxMillisecondsToWait, milliSecondsWaitingBetweenChecks);


	return bExist ? true : false;

}

Error FileIO:: copyFile (const char *pSrcPathName,
	const char *pDestPath,
	unsigned long ulBufferSizeToBeUsed)

{

	int							iSrcFileDescriptor;
	int							iDestFileDescriptor;
	unsigned char				*pucLogBuffer;
	long long					llBytesRead;
	long long					llBytesWritten;
	long long					llTotalBytesWritten;
	char						*pDestPathName;
	Boolean_t					bDirectoryExisting;
	Error_t						errIO;
	unsigned long				ulLocalBufferSizeToBeUsed;


	if (pSrcPathName == (const char *) NULL ||
		pDestPath == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (ulBufferSizeToBeUsed == 0)
	{
		unsigned long				ulFileSize;


		if ((errIO = FileIO:: getFileSizeInBytes (pSrcPathName,
			&ulFileSize, false)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_GETFILESIZEINBYTES_FAILED,
				1, pSrcPathName);

			return errIO;
		}

		if (ulFileSize >= 5 * 1000 * 1024)	// 5MB
			ulLocalBufferSizeToBeUsed			= 5 * 1000 * 1024;
		else if (ulFileSize == 0)
			ulLocalBufferSizeToBeUsed			= 8;	// it cannot be 0
		else
			ulLocalBufferSizeToBeUsed			= ulFileSize;
	}
	else
	{
		ulLocalBufferSizeToBeUsed			= ulBufferSizeToBeUsed;
	}

	if ((pucLogBuffer = new unsigned char [ulLocalBufferSizeToBeUsed]) ==
		(unsigned char *) NULL)
   	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}

	if ((errIO = FileIO:: isDirectoryExisting (pDestPath,
		&bDirectoryExisting)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED,
			1, pDestPath);

		delete [] pucLogBuffer;

		return errIO;
	}

	if (bDirectoryExisting)
	{
		// assume pDestPath refers a directory
		const char				*pSourceFileName;


		#ifdef WIN32
			if ((pSourceFileName = strrchr (pSrcPathName, '\\')) ==
				(char *) NULL)
		#else
			if ((pSourceFileName = strrchr (pSrcPathName, '/')) ==
				(char *) NULL)
		#endif
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRONGSOURCEFILE,
				1, pSrcPathName);

			delete [] pucLogBuffer;

			return err;
		}

		#ifdef WIN32
			if (pDestPath [strlen (pDestPath) - 1] == '\\')
				pSourceFileName++;
		#else
			if (pDestPath [strlen (pDestPath) - 1] == '/')
				pSourceFileName++;
		#endif

		if ((pDestPathName = new char [
			strlen (pDestPath) + strlen (pSourceFileName) + 1]) ==
			(char *) NULL)
   		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			delete [] pucLogBuffer;

			return err;
		}

		strcpy (pDestPathName, pDestPath);
		strcat (pDestPathName, pSourceFileName);
	}
	else
	{
		// assume pDestPath refers a file

		pDestPathName			= (char *) pDestPath;
	}

	#ifdef WIN32
		if ((errIO = FileIO:: open (pSrcPathName,
			O_RDONLY | O_BINARY, &iSrcFileDescriptor)) != errNoError)
	#else
		if ((errIO = FileIO:: open (pSrcPathName,
			O_RDONLY, &iSrcFileDescriptor)) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED,
			1, pSrcPathName);

		if (bDirectoryExisting)
		{
			delete [] pDestPathName;
		}

		delete [] pucLogBuffer;

		return errIO;
	}

	#ifdef WIN32
   		if ((errIO = FileIO:: open (pDestPathName,
			O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, _S_IREAD | _S_IWRITE,
			&iDestFileDescriptor)) != errNoError)
	#else
   		if ((errIO = FileIO:: open (pDestPathName,
			O_WRONLY | O_TRUNC | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iDestFileDescriptor)) != errNoError)
	#endif
   	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED,
			1, pDestPathName);

		if (FileIO:: close (iSrcFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
		}

		if (bDirectoryExisting)
		{
			delete [] pDestPathName;
		}

		delete [] pucLogBuffer;

		return errIO;
	}

	do
	{
		if ((errIO = FileIO:: readBytes (iSrcFileDescriptor,
			pucLogBuffer, ulLocalBufferSizeToBeUsed,
			&llBytesRead)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READBYTES_FAILED);

			if (FileIO:: close (iDestFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
			}

			if (FileIO:: close (iSrcFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
			}

			if (FileIO:: remove (pDestPathName) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_REMOVE_FAILED,
					1, pDestPathName);
			}

			if (bDirectoryExisting)
			{
				delete [] pDestPathName;
			}

			delete [] pucLogBuffer;

			return errIO;
		}

		if (llBytesRead == 0)
			break;

		llTotalBytesWritten			= 0;

		while (llTotalBytesWritten < llBytesRead)
		{
			if ((errIO = FileIO:: writeBytes (iDestFileDescriptor,
				pucLogBuffer + llTotalBytesWritten,
				llBytesRead - llTotalBytesWritten,
				&llBytesWritten)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITEBYTES_FAILED);

				if (FileIO:: close (iDestFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
				}

				if (FileIO:: close (iSrcFileDescriptor) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_CLOSE_FAILED);
				}

				if (FileIO:: remove (pDestPathName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_REMOVE_FAILED,
						1, pDestPathName);
				}

				if (bDirectoryExisting)
				{
					delete [] pDestPathName;
				}

				delete [] pucLogBuffer;

				return errIO;
			}

			llTotalBytesWritten			+= llBytesWritten;
		}
	}
	while (llBytesRead == ulLocalBufferSizeToBeUsed);

	if ((errIO = FileIO:: close (iDestFileDescriptor)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		if (FileIO:: close (iSrcFileDescriptor) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
		}

		if (FileIO:: remove (pDestPathName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED,
				1, pDestPathName);
		}

		if (bDirectoryExisting)
		{
			delete [] pDestPathName;
		}

		delete [] pucLogBuffer;

		return errIO;
	}

	if ((errIO = FileIO:: close (iSrcFileDescriptor)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		if (bDirectoryExisting)
		{
			delete [] pDestPathName;
		}

		delete [] pucLogBuffer;

		return errIO;
	}

	if (bDirectoryExisting)
	{
		delete [] pDestPathName;
	}

	delete [] pucLogBuffer;


	return errNoError;
}

void FileIO:: copyFile (string srcPathName,
	string destPath,
	unsigned long ulBufferSizeToBeUsed)
{
    Error errFileIO;
    
    if ((errFileIO = FileIO:: copyFile (srcPathName.c_str(),
	destPath.c_str(), ulBufferSizeToBeUsed)) != errNoError)
    {
        throw runtime_error(string("FileIO::copyFile failed: ")
                + (const char *) errFileIO);
    }
}

void FileIO:: concatFile (string destPathName,
	string srcPathName, bool removeSrcFileAfterConcat,
	unsigned long ulBufferSizeToBeUsed)
{
    char* buffer = nullptr;
    
    try
    {
    	unsigned long				ulLocalBufferSizeToBeUsed;

        if (!FileIO::fileExisting(destPathName))
            throw runtime_error(string("File does not exist")
                + ", destPathName: " + destPathName);
        else if (!FileIO::fileExisting(srcPathName))
            throw runtime_error(string("File does not exist")
                + ", srcPathName: " + srcPathName);
        
        bool inCaseOfLinkHasItToBeRead = false;
        unsigned long srcFileSize = FileIO::getFileSizeInBytes (srcPathName,
            inCaseOfLinkHasItToBeRead);
        
        if (ulBufferSizeToBeUsed == 0)
        {
            if (srcFileSize >= 5 * 1000 * 1024)	// 5MB
                ulLocalBufferSizeToBeUsed			= 5 * 1000 * 1024;
            else if (srcFileSize == 0)
                ulLocalBufferSizeToBeUsed			= 8;	// it cannot be 0
            else
                ulLocalBufferSizeToBeUsed			= srcFileSize;
        }
        else
        {
            ulLocalBufferSizeToBeUsed			= ulBufferSizeToBeUsed;
        }

        buffer = new char [ulLocalBufferSizeToBeUsed];

        ifstream isSrcStream(srcPathName.c_str());
        ofstream osDestStream(destPathName.c_str(), ofstream::binary | ofstream::app);
        
        unsigned long bytesToBeRead;
        unsigned long totalRead = 0;
        unsigned long currentRead;
        while (totalRead < srcFileSize)
        {
            if (srcFileSize - totalRead >= ulLocalBufferSizeToBeUsed)
                bytesToBeRead = ulLocalBufferSizeToBeUsed;
            else
                bytesToBeRead = srcFileSize - totalRead;

            isSrcStream.read(buffer, bytesToBeRead);
            currentRead = isSrcStream.gcount();

            if (currentRead != bytesToBeRead)
            {
                // this should never happen
                throw runtime_error(string("Error reading the source binary")
                    + ", srcFileSize: " + to_string(srcFileSize)
                    + ", totalRead: " + to_string(totalRead)
                    + ", bytesToBeRead: " + to_string(bytesToBeRead)
                    + ", currentRead: " + to_string(currentRead)
                );
            }

            totalRead += currentRead;

            osDestStream.write(buffer, currentRead); 
        }
        isSrcStream.close();
        osDestStream.close();
        
        delete [] buffer;
        
        if (removeSrcFileAfterConcat)
        {
            bool exceptionInCaseOfError = true;
            
            FileIO::remove(srcPathName, exceptionInCaseOfError);
        }
    }
    catch(runtime_error e)
    {
        if (buffer != nullptr)
            delete [] buffer;
        
        throw runtime_error(string("FileIO:: concatFile failed")
            + ", destPathName: " + destPathName
            + ", srcPathName: " + srcPathName
                + ", removeSrcFileAfterConcat: " + to_string(removeSrcFileAfterConcat)
                + ", ulBufferSizeToBeUsed: " + to_string(ulBufferSizeToBeUsed)
                + ", e.what(): " + e.what()
        );
    }
    catch(exception e)
    {
        if (buffer != nullptr)
            delete [] buffer;
        
        throw runtime_error(string("FileIO:: concatFile failed")
            + ", destPathName: " + destPathName
            + ", srcPathName: " + srcPathName
                + ", removeSrcFileAfterConcat: " + to_string(removeSrcFileAfterConcat)
                + ", ulBufferSizeToBeUsed: " + to_string(ulBufferSizeToBeUsed)
                + ", e.what(): " + e.what()
        );
    }
}

Error FileIO:: moveFile (const char *pSrcPathName,
	const char *pDestPathName)

{

	Boolean_t				bIsFileExisting;
	Boolean_t				bIsDestADirectory;
	char					*pCompleteDestPathName;
	Error_t					errIO;


	if ((errIO = FileIO:: isFileExisting (pSrcPathName,
		&bIsFileExisting)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);

		return errIO;
	}

	if (!bIsFileExisting)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((errIO = FileIO:: isDirectoryExisting (pDestPathName,
		&bIsDestADirectory)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

		return errIO;
	}

	if (bIsDestADirectory)
	{
		const char					*pSrcFileName;


		// it is necessary to concatenate the SrcFileName to the DestPathName

		#ifdef WIN32
			if ((pSrcFileName = strrchr (pSrcPathName, '\\')) == (char *) NULL)
				pSrcFileName			= (char *) pSrcPathName;
			else
				pSrcFileName++;
		#else
			if ((pSrcFileName = strrchr (pSrcPathName, '/')) == (char *) NULL)
				pSrcFileName			= (char *) pSrcPathName;
			else
				pSrcFileName++;
		#endif

		if ((pCompleteDestPathName = new char [strlen (pDestPathName) + 1 +
			strlen (pSrcFileName) + 1]) ==
			(char *) NULL)
   		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			return err;
		}

		strcpy (pCompleteDestPathName, pDestPathName);
		#ifdef WIN32
			if (pCompleteDestPathName [strlen (pCompleteDestPathName) - 1] !=
				'\\')
				strcat (pCompleteDestPathName, "\\");
		#else
			if (pCompleteDestPathName [strlen (pCompleteDestPathName) - 1] !=
				'/')
				strcat (pCompleteDestPathName, "/");
		#endif
		strcat (pCompleteDestPathName, pSrcFileName);
	}
	else
		pCompleteDestPathName		= (char *) pDestPathName;


	#ifdef WIN32
		if (!MoveFile (pSrcPathName, pCompleteDestPathName))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_MOVEFILE_FAILED,
				1, (long) GetLastError ());

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}


			return err;
		}
	#else
		char				*pLockPathName;
		int					fdLockFile;


		if ((pLockPathName = new char [strlen (pSrcPathName) + 4 + 1]) ==
			(char *) NULL)
   		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}

			return err;
		}

		sprintf (pLockPathName, "%s.lck", pSrcPathName);

		if ((errIO = FileIO:: lockFile (pLockPathName, 5,
			&fdLockFile)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);

			delete [] (pLockPathName);
			pLockPathName      = (char *) NULL;

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}

			return errIO;
		}

		if ((errIO = FileIO:: copyFile (pSrcPathName,
			pCompleteDestPathName)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_COPYFILE_FAILED);

			if (FileIO:: unLockFile (pLockPathName,
				fdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_UNLOCKFILE_FAILED, 1, pLockPathName);
			}

			delete [] (pLockPathName);
			pLockPathName      = (char *) NULL;

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}

			return errIO;
		}

		if ((errIO = FileIO:: remove (pSrcPathName)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED,
				1, pSrcPathName);

			if (FileIO:: unLockFile (pLockPathName,
				fdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_UNLOCKFILE_FAILED, 1, pLockPathName);
			}

			delete [] (pLockPathName);
			pLockPathName      = (char *) NULL;

			// bisognerebbe rimuovere pDestPathName???

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}

			return errIO;
		}

		if ((errIO = FileIO:: unLockFile (pLockPathName,
			fdLockFile)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_UNLOCKFILE_FAILED, 1, pLockPathName);

			delete [] (pLockPathName);
			pLockPathName      = (char *) NULL;

			if (bIsDestADirectory)
			{
				delete [] pCompleteDestPathName;
				pCompleteDestPathName		= (char *) NULL;
			}

			return errIO;
		}

		delete [] (pLockPathName);
		pLockPathName      = (char *) NULL;
	#endif


	return errNoError;
}

void FileIO:: moveFile (string srcPathName, string destPathName)
{
    Error errFileIO;
    
    if ((errFileIO = moveFile (srcPathName.c_str(), destPathName.c_str())) != errNoError)
    {
        throw runtime_error(string("FileIO::moveFile failed: ")
                + (const char *) errFileIO);
    }
}

#ifdef WIN32
#else
	Error FileIO:: moveLink (const char *pSrcPathName,
		const char *pDestPathName)

	{
		Error_t						errFileIO;
		DirectoryEntryType_t		detDirectoryEntryType;
		char						pRealPathName [512];
		Error_t						errRead;



		if ((errFileIO = FileIO:: getDirectoryEntryType (
			pSrcPathName, &detDirectoryEntryType)) != errNoError)
		{

			return errFileIO;
		}

		if (detDirectoryEntryType != TOOLS_FILEIO_LINKFILE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		if ((errFileIO = FileIO:: getDirectoryEntryType (
			pDestPathName, &detDirectoryEntryType)) != errNoError)
		{
			// no pDestPathName found

			if ((errRead = FileIO:: readLink (pSrcPathName,
				pRealPathName, 512)) != errNoError)
			{

				return errRead;
			}

			if ((errFileIO = FileIO:: createLink (
				pRealPathName, pDestPathName, false, false)) != errNoError)
			{

				return errFileIO;
			}
		}
		else
		{
			// entry found

			if (detDirectoryEntryType == TOOLS_FILEIO_DIRECTORY)
			{
				Buffer_t				bDestPathName;
				const char				*pLinkName;


				if (bDestPathName. init (pDestPathName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);

					return err;
				}

				if (pDestPathName [strlen (pDestPathName) - 1] != '/')
				{
					if (bDestPathName. append ("/") != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_APPEND_FAILED);

						if (bDestPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}
				}

				if ((pLinkName = strrchr (pSrcPathName, '/')) ==
					(const char *) NULL)
				{
					if (bDestPathName. append (pSrcPathName) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_APPEND_FAILED);

						if (bDestPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}
				}
				else
				{
					if (bDestPathName. append (pLinkName + 1) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_APPEND_FAILED);

						if (bDestPathName. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return err;
					}
				}

				if ((errRead = FileIO:: readLink (pSrcPathName,
					pRealPathName, 512)) != errNoError)
				{
					if (bDestPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return errRead;
				}

				if ((errFileIO = FileIO:: createLink (
					pRealPathName, (const char *) bDestPathName,
					false, false)) != errNoError)
				{
					if (bDestPathName. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return errFileIO;
				}

				if (bDestPathName. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);

					return err;
				}
			}
			else // TOOLS_FILEIO_LINKFILE or TOOLS_FILEIO_REGULARFILE
			{
				if ((errRead = FileIO:: readLink (pSrcPathName,
					pRealPathName, 512)) != errNoError)
				{

					return errRead;
				}

				if ((errFileIO = FileIO:: createLink (
					pRealPathName, pDestPathName, true, false)) != errNoError)
				{

					return errFileIO;
				}
			}
		}

		if ((errFileIO = FileIO:: remove (pSrcPathName)) != errNoError)
		{

			return errFileIO;
		}


		return errNoError;
	}
#endif


#ifdef WIN32
	Error FileIO:: createLink (const char *pOriPathName,
		const char *pLinkPathName, Boolean_t bReplaceItIfExist,
		Boolean_t bRecursiveDirectoriesCreation, int mMode)

	{
		HRESULT				hResult;
		IShellLink			*pShellLink;
		IPersistFile		*pPersistFile;
		WORD				wszLinkfile [MAX_PATH];
		int					iWideCharsWritten;


		if (bRecursiveDirectoriesCreation)
		{
			const char				*pLastSlash;


			#ifdef WIN32
				if ((pLastSlash = strrchr (pLinkPathName, '\\')) !=
					(const char *) NULL)
			#else
				if ((pLastSlash = strrchr (pLinkPathName, '/')) !=
					(const char *) NULL)
			#endif
			{
				Buffer_t			bDirectory;
				Boolean_t			bIsExist;
				Error_t				errFileIO;


				if (bDirectory. init (pLinkPathName,
					pLastSlash - pLinkPathName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);

					return err;
				}

				if ((errFileIO = FileIO:: isDirectoryExisting (
					(const char *) bDirectory, &bIsExist)) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

					if (bDirectory. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return errFileIO;
				}

				if (!bIsExist)
				{
					if ((errFileIO = FileIO:: createDirectory (
						(const char *) bDirectory,
						mMode, true, true)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
							1, (const char *) bDirectory);

						if (bDirectory. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				if (bDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);

					return err;
				}
			}
		}

		hResult = CoCreateInstance (
			CLSID_ShellLink,	// pre-defined CLSID of the IShellLink object
			NULL,			// pointer to parent interface if part of aggregate
			CLSCTX_INPROC_SERVER, // caller and called code are in same process
			IID_IShellLink,// pre-defined interface of the IShellLink object
			(LPVOID*) &pShellLink);	// Returns a pointer to the IShellLink obj

		if (!SUCCEEDED(hResult))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_COCREATEINSTANCE_FAILED);

			return err;
		}

		/* Set the fields in the IShellLink object */
		hResult = pShellLink -> SetPath (pOriPathName);
		/*
        hRes = pShellLink->lpVtbl->SetArguments(pShellLink, pszTargetargs);
        if (strlen(pszDescription) > 0)
        {
          hRes = pShellLink->lpVtbl->SetDescription(pShellLink, pszDescription);
        }
        if (iShowmode > 0)
        {
          hRes = pShellLink->lpVtbl->SetShowCmd(pShellLink, iShowmode);
        }
        if (strlen(pszCurdir) > 0)
        {
          hRes = pShellLink->lpVtbl->SetWorkingDirectory(pShellLink, pszCurdir);
        }
        if (strlen(pszIconfile) > 0 && iIconindex >= 0)
        {
          hRes = pShellLink->lpVtbl->SetIconLocation(pShellLink,
			pszIconfile, iIconindex);
        }
		*/

        /* Use the IPersistFile object to save the shell link */
		hResult = pShellLink -> QueryInterface (
			IID_IPersistFile,	// pre-defined interface of the IPersistFile obj
			(LPVOID*) &pPersistFile); // returns a pointer to the IPersistFile object

		if (!SUCCEEDED(hResult))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SHELLLINK_QUERYINTERFACE_FAILED);

			return err;
		}

		iWideCharsWritten		= MultiByteToWideChar(CP_ACP, 0,
			pLinkPathName, -1, wszLinkfile, MAX_PATH);

		hResult = pPersistFile -> Save (wszLinkfile, TRUE);

		pPersistFile -> Release ();

        pShellLink -> Release ();


		return errNoError;
	}
#else
	Error FileIO:: createLink (const char *pOriPathName,
		const char *pLinkPathName, Boolean_t bReplaceItIfExist,
		Boolean_t bRecursiveDirectoriesCreation, int mMode)

	{

		if (bRecursiveDirectoriesCreation)
		{
			const char				*pLastSlash;


			#ifdef WIN32
				if ((pLastSlash = strrchr (pLinkPathName, '\\')) !=
					(const char *) NULL)
			#else
				if ((pLastSlash = strrchr (pLinkPathName, '/')) !=
					(const char *) NULL)
			#endif
			{
				Buffer_t			bDirectory;
				Boolean_t			bIsExist;
				Error_t				errFileIO;


				if (bDirectory. init (pLinkPathName,
					pLastSlash - pLinkPathName) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_INIT_FAILED);

					return err;
				}

				if ((errFileIO = FileIO:: isDirectoryExisting (
					(const char *) bDirectory, &bIsExist)) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED);

					if (bDirectory. finish () != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_BUFFER_FINISH_FAILED);
					}

					return errFileIO;
				}

				if (!bIsExist)
				{
					if ((errFileIO = FileIO:: createDirectory (
						(const char *) bDirectory,
						mMode, true, true)) != errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
							1, (const char *) bDirectory);

						if (bDirectory. finish () != errNoError)
						{
							Error err = ToolsErrors (__FILE__, __LINE__,
								TOOLS_BUFFER_FINISH_FAILED);
						}

						return errFileIO;
					}
				}

				if (bDirectory. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);

					return err;
				}
			}
		}

		if (symlink (pOriPathName, pLinkPathName) != 0)
		{
			if (bReplaceItIfExist && errno == EEXIST)
			{
				Error_t			errRemove;

				if ((errRemove = FileIO:: remove (pLinkPathName)) != errNoError)
				{
					return errRemove;
				}

				if (symlink (pOriPathName, pLinkPathName) != 0)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_SYMLINK_FAILED,
						3, pOriPathName, pLinkPathName, (long) errno);
					err. setUserData ((void *) (&errno), sizeof (int));

					return err;
				}
			}
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SYMLINK_FAILED,
					3, pOriPathName, pLinkPathName, (long) errno);
				err. setUserData ((void *) (&errno), sizeof (int));

				return err;
			}
		}


		return errNoError;
	}
#endif


#ifdef WIN32
	Error FileIO:: open (const char *pPathName, int iFlags, int mMode,
		int *piFileDescriptor)
#else
	Error FileIO:: open (const char *pPathName, int iFlags, mode_t mMode,
		int *piFileDescriptor)
#endif
{

	#ifdef WIN32
		if ((*piFileDescriptor = ::_open (pPathName, iFlags, mMode)) == -1)
	#else
		if ((*piFileDescriptor = ::open (pPathName, iFlags, mMode)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPEN_FAILED, 2, pPathName, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: open (const char *pPathName, int iFlags,
	int *piFileDescriptor)

{

	#ifdef WIN32
		if ((*piFileDescriptor = ::_open (pPathName, iFlags)) == -1)
	#else
		if ((*piFileDescriptor = ::open (pPathName, iFlags)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPEN_FAILED, 2, pPathName, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: close (int iFileDescriptor)

{

	#ifdef WIN32
		if (::_close (iFileDescriptor) == -1)
	#else
		if (::close (iFileDescriptor) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_CLOSE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: touchFile (const char *pPathName)

{

	Error_t					errFileIO;
	int						iFileDescriptor;


	#ifdef WIN32
		if ((errFileIO = FileIO:: open (pPathName,
			O_WRONLY | O_CREAT, _S_IREAD | _S_IWRITE,
			&iFileDescriptor)) != errNoError)
	#else
		if ((errFileIO = FileIO:: open (pPathName,
			O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR |
			S_IRGRP | S_IWGRP |
			S_IROTH,
			&iFileDescriptor)) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED,
			1, pPathName);

		return errFileIO;
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: remove (const char *pPathName)

{

	if (unlink (pPathName) != 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_UNLINK_FAILED,
			2, pPathName, (long) errno);
		err. setUserData ((void *) (&errno), sizeof (int));

		return err;
	}


	return errNoError;
}

void FileIO:: remove (string pathName, bool exceptionInCaseOfError)
{
    Error errFileIO;
    
    if ((errFileIO = FileIO:: remove (pathName.c_str())) != errNoError)
    {
        if (exceptionInCaseOfError)
            throw runtime_error(string("FileIO::remove failed: ")
                + (const char *) errFileIO);
    }
}

#ifdef WIN32
	Error FileIO:: seek (int iFileDescriptor, __int64 llBytes, int iWhence,
		__int64 *pllCurrentPosition)
#else
	Error FileIO:: seek (int iFileDescriptor, long long llBytes, int iWhence,
		long long *pllCurrentPosition)
#endif

{

	#ifdef WIN32
		if ((*pllCurrentPosition = ::_lseeki64 (iFileDescriptor, llBytes,
			iWhence)) == -1)
	#else
		if ((*pllCurrentPosition = ::lseek (iFileDescriptor, (off_t) llBytes,
			iWhence)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_LSEEK_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error FileIO:: readChars (int iFileDescriptor,
		char *pBuffer, __int64 ullCharsNumberToBeRead,
		__int64 *pllBytesRead)
#else
	Error FileIO:: readChars (int iFileDescriptor,
		char *pBuffer, unsigned long long ullCharsNumberToBeRead,
		long long *pllBytesRead)
#endif

{

	#ifdef WIN32
		if ((*pllBytesRead = ::_read (iFileDescriptor, (void *) pBuffer,
			(unsigned int) ullCharsNumberToBeRead)) == -1)
	#else
		if ((*pllBytesRead = ::read (iFileDescriptor, (void *) pBuffer,
			ullCharsNumberToBeRead)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error FileIO:: writeChars (int iFileDescriptor,
		const char *pBuffer, __int64 ullCharsNumber,
		__int64 *pllBytesWritten)
#else
	Error FileIO:: writeChars (int iFileDescriptor,
		const char *pBuffer, unsigned long long ullCharsNumber,
		long long *pllBytesWritten)
#endif

{

	#ifdef WIN32
		if ((*pllBytesWritten = ::_write (iFileDescriptor, (void *) pBuffer,
			(unsigned int) ullCharsNumber)) == -1)
	#else
		if ((*pllBytesWritten = ::write (iFileDescriptor, (void *) pBuffer,
			ullCharsNumber)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error FileIO:: readBytes (int iFileDescriptor,
		unsigned char *pucValue, __int64 ullBytesNumber,
		__int64 *pllBytesRead)
#else
	Error FileIO:: readBytes (int iFileDescriptor,
		unsigned char *pucValue, unsigned long long ullBytesNumber,
		long long *pllBytesRead)
#endif

{

	#ifdef WIN32
		if ((*pllBytesRead = ::_read (iFileDescriptor, (void *) pucValue,
			(unsigned int) ullBytesNumber)) == -1)
	#else
		if ((*pllBytesRead = ::read (iFileDescriptor, (void *) pucValue,
			ullBytesNumber)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error FileIO:: writeBytes (int iFileDescriptor,
		unsigned char *pucValue, __int64 ullBytesNumber,
		__int64 *pllBytesWritten)
#else
	Error FileIO:: writeBytes (int iFileDescriptor,
		unsigned char *pucValue, unsigned long long ullBytesNumber,
		long long *pllBytesWritten)
#endif

{

	if (ullBytesNumber == 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		if ((*pllBytesWritten = ::_write (iFileDescriptor, (void *) pucValue,
			(unsigned int) ullBytesNumber)) == -1)
	#else
		if ((*pllBytesWritten = ::write (iFileDescriptor, (void *) pucValue,
			ullBytesNumber)) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetUnsignedInt8Bit (int iFileDescriptor,
	unsigned long *pulValue)

{

	unsigned char					pucValue [1];


	#ifdef WIN32
		if (::_read (iFileDescriptor, (void *) pucValue, 1) == -1)
	#else
		if (::read (iFileDescriptor, (void *) pucValue, 1) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}

	*pulValue			= (
		pucValue [0]);


	return errNoError;
}


Error FileIO:: writeNetUnsignedInt8Bit (int iFileDescriptor,
	unsigned long ulValue)

{

	unsigned char					pucValue [1];


	pucValue [0]	= (unsigned char) ((ulValue) & 0xFF);

	#ifdef WIN32
		if (::_write (iFileDescriptor, (void *) pucValue, 1) == -1)
	#else
		if (::write (iFileDescriptor, (void *) pucValue, 1) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetUnsignedInt16Bit (int iFileDescriptor,
	unsigned long *pulValue)

{

	unsigned char					pucValue [2];


	#ifdef WIN32
		if (::_read (iFileDescriptor, (void *) pucValue, 2) == -1)
	#else
		if (::read (iFileDescriptor, (void *) pucValue, 2) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}

	*pulValue			= (
		(pucValue [0] << 8) |
		pucValue [1]);


	return errNoError;
}


Error FileIO:: writeNetUnsignedInt16Bit (int iFileDescriptor,
	unsigned long ulValue)

{

	unsigned char					pucValue [2];


	pucValue [0]	= (unsigned char) ((ulValue >> 8) & 0xFF);
	pucValue [1]	= (unsigned char) ((ulValue) & 0xFF);

	#ifdef WIN32
		if (::_write (iFileDescriptor, (void *) pucValue, 2) == -1)
	#else
		if (::write (iFileDescriptor, (void *) pucValue, 2) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetUnsignedInt24Bit (int iFileDescriptor,
	unsigned long *pulValue)

{

	unsigned char					pucValue [3];


	#ifdef WIN32
		if (::_read (iFileDescriptor, (void *) pucValue, 3) == -1)
	#else
		if (::read (iFileDescriptor, (void *) pucValue, 3) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}

	*pulValue			= (
		(pucValue [0] << 16) |
		(pucValue [1] << 8) |
		pucValue [2]);


	return errNoError;
}


Error FileIO:: writeNetUnsignedInt24Bit (int iFileDescriptor,
	unsigned long ulValue)

{

	unsigned char					pucValue [3];


	pucValue [0]	= (unsigned char) ((ulValue >> 16) & 0xFF);
	pucValue [1]	= (unsigned char) ((ulValue >> 8) & 0xFF);
	pucValue [2]	= (unsigned char) ((ulValue) & 0xFF);

	#ifdef WIN32
		if (::_write (iFileDescriptor, (void *) pucValue, 3) == -1)
	#else
		if (::write (iFileDescriptor, (void *) pucValue, 3) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetUnsignedInt32Bit (int iFileDescriptor,
	unsigned long *pulValue)

{

	unsigned char					pucValue [4];


	#ifdef WIN32
		if (::_read (iFileDescriptor, (void *) pucValue, 4) == -1)
	#else
		if (::read (iFileDescriptor, (void *) pucValue, 4) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}

	*pulValue			= (
		(pucValue [0] << 24) |
		(pucValue [1] << 16) |
		(pucValue [2] << 8) |
		pucValue [3]);


	return errNoError;
}


Error FileIO:: writeNetUnsignedInt32Bit (int iFileDescriptor,
	unsigned long ulValue)

{

	unsigned char					pucValue [4];


	pucValue [0]	= (unsigned char) ((ulValue >> 24) & 0xFF);
	pucValue [1]	= (unsigned char) ((ulValue >> 16) & 0xFF);
	pucValue [2]	= (unsigned char) ((ulValue >> 8) & 0xFF);
	pucValue [3]	= (unsigned char) ((ulValue) & 0xFF);

	#ifdef WIN32
		if (::_write (iFileDescriptor, (void *) pucValue, 4) == -1)
	#else
		if (::write (iFileDescriptor, (void *) pucValue, 4) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}

#ifdef WIN32
	Error FileIO:: readNetUnsignedInt64Bit (int iFileDescriptor,
		__int64 *pullValue)
#else
	Error FileIO:: readNetUnsignedInt64Bit (int iFileDescriptor,
		unsigned long long *pullValue)
#endif
{

	unsigned char					pucValue [8];


	#ifdef WIN32
		if (::_read (iFileDescriptor, (void *) pucValue, 8) == -1)
	#else
		if (::read (iFileDescriptor, (void *) pucValue, 8) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_READ_FAILED, 1, (long) errno);

		return err;
	}

	*pullValue			= (
		(pucValue [0] << 56) |
		(pucValue [1] << 48) |
		(pucValue [2] << 40) |
		(pucValue [3] << 32) |
		(pucValue [4] << 24) |
		(pucValue [5] << 16) |
		(pucValue [6] << 8) |
		pucValue [7]);


	return errNoError;
}


#ifdef WIN32
	Error FileIO:: writeNetUnsignedInt64Bit (int iFileDescriptor,
		__int64 ullValue)
#else
	Error FileIO:: writeNetUnsignedInt64Bit (int iFileDescriptor,
		unsigned long long ullValue)
#endif
{

	unsigned char					pucValue [8];
	#ifdef WIN32
		__int64							llLocalValue;
	#else
		unsigned long long				llLocalValue;
	#endif

	llLocalValue			= ullValue;

	pucValue [7]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [6]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [5]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [4]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [3]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [2]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [1]	= (unsigned char) (llLocalValue & 0xFF);
	llLocalValue	>>= 8;
	pucValue [0]	= (unsigned char) (llLocalValue & 0xFF);

	#ifdef WIN32
		if (::_write (iFileDescriptor, (void *) pucValue, 8) == -1)
	#else
		if (::write (iFileDescriptor, (void *) pucValue, 8) == -1)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_WRITE_FAILED, 1, (long) errno);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetFloat16Bit (int iFileDescriptor, float *pfValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (readNetUnsignedInt8Bit (iFileDescriptor, &ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READNETUNSIGNEDINT8BIT_FAILED);

		return err;
	}

	if (readNetUnsignedInt8Bit (iFileDescriptor, &ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READNETUNSIGNEDINT8BIT_FAILED);

		return err;
	}

	*pfValue			= (ulIntegerPart + (((float) ulFloatPart) / 0x100));


	return errNoError;
}


Error FileIO:: writeNetFloat16Bit (int iFileDescriptor, float fValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (fValue >= 0x100)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	ulIntegerPart				= (unsigned long) fValue;
	ulFloatPart					= (unsigned long) ((fValue - ulIntegerPart) *
		0x100);

	if (writeNetUnsignedInt8Bit (iFileDescriptor, ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITENETUNSIGNEDINT8BIT_FAILED);

		return err;
	}

	if (writeNetUnsignedInt8Bit (iFileDescriptor, ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITENETUNSIGNEDINT8BIT_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: readNetFloat32Bit (int iFileDescriptor, float *pfValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (readNetUnsignedInt16Bit (iFileDescriptor, &ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READNETUNSIGNEDINT16BIT_FAILED);

		return err;
	}

	if (readNetUnsignedInt16Bit (iFileDescriptor, &ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READNETUNSIGNEDINT16BIT_FAILED);

		return err;
	}

	*pfValue			= (ulIntegerPart + (((float) ulFloatPart) / 0x10000));


	return errNoError;
}


Error FileIO:: writeNetFloat32Bit (int iFileDescriptor, float fValue)

{

	unsigned long				ulIntegerPart;
	unsigned long				ulFloatPart;


	if (fValue >= 0x10000)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	ulIntegerPart				= (unsigned long) fValue;
	ulFloatPart					= (unsigned long) ((fValue - ulIntegerPart) *
		0x10000);

	if (writeNetUnsignedInt16Bit (iFileDescriptor, ulIntegerPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITENETUNSIGNEDINT16BIT_FAILED);

		return err;
	}

	if (writeNetUnsignedInt16Bit (iFileDescriptor, ulFloatPart) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITENETUNSIGNEDINT16BIT_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: readMP4DescriptorSize (int iFileDescriptor,
	unsigned long *pulSize, unsigned char *pucNumBytes)

{

	unsigned long			ulTmp;
	unsigned char			ucTmp;


	*pulSize			= 0;
	*pucNumBytes		= 0;

	do
	{
		if (readNetUnsignedInt8Bit (iFileDescriptor, &ulTmp) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READNETUNSIGNEDINT8BIT_FAILED);

			return err;
		}
		ucTmp				= (unsigned char) ulTmp;

		*pulSize			= (*pulSize << 7) | (ucTmp & 0x7F);

		(*pucNumBytes)++;
	}
	while ((ucTmp & 0x80) && *pucNumBytes < 4);


	return errNoError;
}


Error FileIO:: writeMP4DescriptorSize (int iFileDescriptor,
	unsigned long ulSize, Boolean_t bCompact, unsigned char ucOriginalNumBytes)

{

	unsigned char				ucNumBytes;
	unsigned char				ucIndex;
	unsigned char				ucTmp;
	unsigned long				ulTmp;


	if (ucOriginalNumBytes == 0)
	{
		if (bCompact)
		{
			if (ulSize <= 0x7F)
			{
				ucNumBytes			= 1;
			}
			else if (ulSize <= 0x3FFF)
			{
				ucNumBytes			= 2;
			}
			else if (ulSize <= 0x1FFFFF)
			{
				ucNumBytes			= 3;
			}
			else
			{
				ucNumBytes			= 4;
			}
		}
		else
		{
			ucNumBytes			= 4;
		}
	}
	else
		ucNumBytes				= ucOriginalNumBytes;

	ucIndex				= ucNumBytes;

	do
	{
		ucIndex--;

		ucTmp				= (unsigned char) ((ulSize >> (ucIndex * 7)) & 0x7F);

		if (ucIndex > 0)
		{
			ucTmp			|= 0x80;
		}

		ulTmp			= ucTmp;

		if (writeNetUnsignedInt8Bit (iFileDescriptor, ulTmp) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITENETUNSIGNEDINT8BIT_FAILED);

			return err;
		}
	}
	while (ucIndex > 0);


	return errNoError;
}


Error FileIO:: lockFile (const char *pLockPathName,
	long lSecondsToWaitIfAlreadyLocked, int *pfdLockFile)

{

	#ifdef WIN32
		// assert (1==0);
	#else
		Error_t			errGeneric;
		struct flock	flFileLock;
		Boolean_t		bConfigIsLocked;
		long			lStartUtcTime;
		long			lFcntlReturn;


		if ((errGeneric = FileIO:: open (pLockPathName, O_WRONLY | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
			pfdLockFile)) != errNoError)
		{

			return errGeneric;
		}

		// l_type: F_RDLCK (read lock), F_WRLCK (write lock),
		//	F_UNLCK (remove lock)
		flFileLock. l_type		= F_WRLCK;
		flFileLock. l_whence	= SEEK_SET;
		flFileLock. l_start		= 0;	// Relative offset in bytes
		flFileLock. l_len		= 0;	// Size; if 0 then until EOF

		// fcntl ()
		// With the F_SETLK parameter, if a read or write lock cannot
		//		be set, fcntl() returns immediately with an error value of -1.
		// With the F_SETLKW parameter, if a read or write lock is blocked
		//		by other locks, the process will sleep until the segment
		//		is free to be locked

		bConfigIsLocked		= false;
		lStartUtcTime		= time (NULL);

		while (!bConfigIsLocked)
		{
			lFcntlReturn = fcntl (*pfdLockFile, F_SETLK, &flFileLock);
			if (lFcntlReturn == -1 && (errno == EACCES || errno == EAGAIN))
			{
				// already locked
				if (lSecondsToWaitIfAlreadyLocked != -1 &&
					time (NULL) - lStartUtcTime > lSecondsToWaitIfAlreadyLocked)
					break;
				else
					continue;
			}
			else if (lFcntlReturn >= 0)
			{
				bConfigIsLocked	= true;
			}
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FCNTL_FAILED, 1, (long) errno);
				FileIO:: close (*pfdLockFile);

				return err;
			}
		}

		if (!bConfigIsLocked)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_FILENOTRELEASED, 1, pLockPathName);
			FileIO:: close (*pfdLockFile);

			return err;
		}
	#endif


	return errNoError;
}


Error FileIO:: unLockFile (const char *pLockPathName, int fdLockFile)

{

	#ifdef WIN32
		// assert (1==0);
	#else
		struct flock	flFileLock;
		Error_t			errGeneric;
		Boolean_t		bConfigIsLocked;
		long			lStartUtcTime;
		long			lFcntlReturn;


		if (fdLockFile != -1)
		{
			flFileLock. l_type		= F_UNLCK;
			flFileLock. l_whence	= SEEK_SET;
			flFileLock. l_start		= 0;	// Relative offset in bytes
			flFileLock. l_len		= 0;	// Size; if 0 then until EOF
			if (fcntl (fdLockFile, F_SETLK, &flFileLock) == -1)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FCNTL_FAILED, 1, (long) errno);
				FileIO:: close (fdLockFile);
				FileIO:: remove (pLockPathName);

				return err;
			}

			if (FileIO:: close (fdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				FileIO:: remove (pLockPathName);

				return err;
			}
		}

		if (FileIO:: remove (pLockPathName) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_REMOVE_FAILED,
				1, pLockPathName);

			return err;
		}
	#endif


	return errNoError;
}


Error FileIO:: appendBuffer (const char *pPathName,
	const char *pBuffer, Boolean_t bAppendNewLine)

{

	int					iFileDescriptor;
	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif


	#ifdef WIN32
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor) != errNoError)
	#else
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iFileDescriptor) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED);

		return err;
	}

	if (FileIO:: writeChars (iFileDescriptor,
		pBuffer, strlen (pBuffer),
		&llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		if (FileIO:: close (iFileDescriptor) != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_FILEIO_CLOSE_FAILED);
		}

		return err;
	}

	if (bAppendNewLine)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			"\n", strlen ("\n"),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_CLOSE_FAILED);
			}

			return err;
		}
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: appendUnsignedLong (const char *pPathName,
	unsigned long ulValue, Boolean_t bAppendNewLine)

{

	int					iFileDescriptor;
	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif
	char				pUnsignedLongBuffer [128 + 1];



	if (sprintf (pUnsignedLongBuffer, "%lu", ulValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	#ifdef WIN32
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor) != errNoError)
	#else
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iFileDescriptor) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED);

		return err;
	}

	if (FileIO:: writeChars (iFileDescriptor,
		pUnsignedLongBuffer, strlen (pUnsignedLongBuffer),
		&llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		if (FileIO:: close (iFileDescriptor) != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_FILEIO_CLOSE_FAILED);
		}

		return err;
	}

	if (bAppendNewLine)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			"\n", strlen ("\n"),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_CLOSE_FAILED);
			}

			return err;
		}
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: appendLongLong (const char *pPathName,
	long long llValue, Boolean_t bAppendNewLine)

{

	int					iFileDescriptor;
	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif
	char				pLongLongBuffer [128 + 1];



	if (sprintf (pLongLongBuffer, "%lld", llValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	#ifdef WIN32
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor) != errNoError)
	#else
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iFileDescriptor) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED);

		return err;
	}

	if (FileIO:: writeChars (iFileDescriptor,
		pLongLongBuffer, strlen (pLongLongBuffer),
		&llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		if (FileIO:: close (iFileDescriptor) != errNoError)
		{
			// Error err = ToolsErrors (__FILE__, __LINE__,
			// 	TOOLS_FILEIO_CLOSE_FAILED);
		}

		return err;
	}

	if (bAppendNewLine)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			"\n", strlen ("\n"),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_CLOSE_FAILED);
			}

			return err;
		}
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


Error FileIO:: appendBuffer (const char *pPathName,
	Boolean_t bAppendNewLine, long lBuffersNumber, ...)

{

	int					iFileDescriptor;
	#ifdef WIN32
		__int64			llBytesWritten;
	#else
		long long		llBytesWritten;
	#endif


	#ifdef WIN32
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			_S_IREAD | _S_IWRITE,
			&iFileDescriptor) != errNoError)
	#else
		if (FileIO:: open (pPathName,
			O_WRONLY | O_APPEND | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iFileDescriptor) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED);

		return err;
	}

	{
		unsigned long		ulArgumentIndex;
		va_list				vaInputList;
		char				*pBuffer;


		va_start (vaInputList, lBuffersNumber);

		for (ulArgumentIndex = 0; ulArgumentIndex < lBuffersNumber;
			ulArgumentIndex++)
		{
			pBuffer          = (char *) va_arg (vaInputList, char *);

			if (pBuffer == (char *) NULL)
				continue;

			if (FileIO:: writeChars (iFileDescriptor,
				pBuffer, strlen (pBuffer),
				&llBytesWritten) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_WRITECHARS_FAILED);

				va_end (vaInputList);

				if (FileIO:: close (iFileDescriptor) != errNoError)
				{
					// Error err = ToolsErrors (__FILE__, __LINE__,
					// 	TOOLS_FILEIO_CLOSE_FAILED);
				}

				return err;
			}
		}

		va_end (vaInputList);
	}

	if (bAppendNewLine)
	{
		if (FileIO:: writeChars (iFileDescriptor,
			"\n", strlen ("\n"),
			&llBytesWritten) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_WRITECHARS_FAILED);

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_CLOSE_FAILED);
			}

			return err;
		}
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}

