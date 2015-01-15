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
#include "FileIO.h"
#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#ifdef ANDROID_DEBUG
    #include <QDebug>
#endif
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

#if defined(__GNUC__) || defined(WIN32) || defined(__QTCOMPILER__)
	int yyparse ();
#else
	extern "C" {
		int yyparse ();
	}
#endif

extern Config_p				pcfgOutputConfig;
extern Error_t				gerrParserError;
extern Boolean_t			gbIsCaseSensitive;
extern long					glCfgSectionsToAllocOnOverflow;
extern long					glCfgItemsToAllocOnOverflow;
extern long					glBufferToAllocOnOverflow;

#ifdef WIN32
#elif __QTCOMPILER__
#else
	extern int errno;
#endif

char						*gpBufferToParse;
#ifdef WIN32
	__int64					gllBufferToParseLength;
#else
	long long				gllBufferToParseLength;
#endif
long						glBufferToParseIndex;


ConfigurationFile:: ConfigurationFile (void): Config ()

{

	_stCfgFileStatus			= CFGFILE_BUILDED;

}


ConfigurationFile:: ~ConfigurationFile (void)

{

	if (_stCfgFileStatus  != CFGFILE_BUILDED)
		finish ();
}


ConfigurationFile:: ConfigurationFile (const ConfigurationFile &)

{

	assert (1==0);

	// to do

}


ConfigurationFile &ConfigurationFile:: operator = (const ConfigurationFile &)

{

	assert (1==0);

	// to do

	return *this;

}


ConfigurationFile:: CfgFileStatus_t ConfigurationFile:: getCfgStatus ()

{

	return _stCfgFileStatus;
}


Error ConfigurationFile:: init (const char *pPathName,
	const char *pConfigName, Boolean_t bIsCaseSensitive,
	long lCfgSectionsToAllocOnOverflow,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow,
	Boolean_t bLockFile)

{

	#ifdef WIN32
	#else
		int				fdLockFile;
		Boolean_t		bConfigIsLocked;
		long			lStartUtcTime;
		long			lFcntlReturn;
		char			pLockPathName [CFGFILE_MAXPATHNAMELENGTH];
		Error_t			errFileIO;
	#endif


	if (_stCfgFileStatus != CFGFILE_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgFileStatus);

		return err;
	}

	if (pPathName == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	if ((_pPathName = new char [strlen (pPathName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		return err;
	}

	strcpy (_pPathName, pPathName);

	#ifdef WIN32
		{
			int					iFileDescriptor;
			__int64				llCurrentPosition;
			__int64				llBytesRead;
			Error_t				errOpen;


			if ((errOpen = FileIO:: open (_pPathName, O_RDONLY,
				&iFileDescriptor)) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_OPEN_FAILED, 1, _pPathName);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return errOpen;
			}

			if (FileIO:: seek (iFileDescriptor, (off_t) 0,
				SEEK_END, &gllBufferToParseLength) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_SEEK_FAILED);
				FileIO:: close (iFileDescriptor);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if ((gpBufferToParse = new char [
				(unsigned int) (gllBufferToParseLength + 1)]) ==
				(char *) NULL)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_NEW_FAILED);
				FileIO:: close (iFileDescriptor);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if (FileIO:: seek (iFileDescriptor, (off_t) 0, SEEK_SET,
				&llCurrentPosition) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_SEEK_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if (FileIO:: readChars (iFileDescriptor, gpBufferToParse,
				gllBufferToParseLength, &llBytesRead) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_READCHARS_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			// sure llBytesRead <= gllBufferToParseLength
			if (llBytesRead < gllBufferToParseLength)
				gllBufferToParseLength					= llBytesRead;

			gpBufferToParse [gllBufferToParseLength]		= '\0';

			#ifdef YACC_DEBUG
                #ifdef ANDROID_DEBUG
                    qDebug() << "Configuration file in input:" << endl << gpBufferToParse;
                #else
                    cout << "Configuration file in input:" << endl
                        << gpBufferToParse << endl;
                #endif
			#endif

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}
		}
	/*
		HANDLE		hFile;
		DWORD		dwNumberOfBytesRead;


		if ((hFile = CreateFile(_pPathName, GENERIC_READ,
			0,           // share mode. 0 means no share
			NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			NULL)) == INVALID_HANDLE_VALUE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_OPEN_FAILED, 1, _pPathName);
			delete [] (_pPathName);
			_pPathName		= (char *) NULL;

			return err;
		}

		if ((gllBufferToParseLength = GetFileSize (hFile, NULL)) == 0xFFFFFFFF)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READCHARS_FAILED);
			CloseHandle (hFile);
			delete [] (_pPathName);
			_pPathName		= (char *) NULL;

			return err;
		}

		if ((gpBufferToParse = new char [gllBufferToParseLength + 1]) ==
			(char *) NULL)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_NEW_FAILED);
			CloseHandle (hFile);
			delete [] (_pPathName);
			_pPathName		= (char *) NULL;

			return err;
		}

		if (ReadFile (hFile, gpBufferToParse, gllBufferToParseLength,
			&dwNumberOfBytesRead, NULL) == FALSE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_READCHARS_FAILED);
			CloseHandle (hFile);
			delete [] (_pPathName);
			_pPathName		= (char *) NULL;

			return err;
		}
 
		gpBufferToParse [gllBufferToParseLength]	= '\0';

		if (CloseHandle (hFile) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_CLOSE_FAILED);
			delete [] (_pPathName);
			_pPathName		= (char *) NULL;

			return err;
		}
		*/
	#else
		if (bLockFile)
		{
			sprintf (pLockPathName, "%s.lck", _pPathName);

			if (FileIO:: lockFile (pLockPathName, 30,
				&fdLockFile) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}
		}

		{
			int					iFileDescriptor;
			long long			llCurrentPosition;
			long long			llBytesRead;
			Error_t				errOpen;


			if ((errOpen = FileIO:: open (_pPathName, O_RDONLY,
				&iFileDescriptor)) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_FILEIO_OPEN_FAILED, 1, _pPathName);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return errOpen;
			}

			if (FileIO:: seek (iFileDescriptor, (off_t) 0,
				SEEK_END, &gllBufferToParseLength) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_SEEK_FAILED);

				FileIO:: close (iFileDescriptor);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if ((gpBufferToParse = new char [gllBufferToParseLength + 1]) ==
				(char *) NULL)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
					CFG_NEW_FAILED);
				FileIO:: close (iFileDescriptor);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if (FileIO:: seek (iFileDescriptor, (off_t) 0, SEEK_SET,
				&llCurrentPosition) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_SEEK_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			if (FileIO:: readChars (iFileDescriptor, gpBufferToParse,
				gllBufferToParseLength, &llBytesRead) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_READCHARS_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}

			// sure llBytesRead <= gllBufferToParseLength
			if (llBytesRead < gllBufferToParseLength)
				gllBufferToParseLength					= llBytesRead;

			gpBufferToParse [gllBufferToParseLength]	= '\0';

			if (FileIO:: close (iFileDescriptor) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				delete [] gpBufferToParse;
				gpBufferToParse		= (char *) NULL;
				FileIO:: close (iFileDescriptor);

				if (bLockFile)
				{
					if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
						errNoError)
					{
						Error err = ToolsErrors (__FILE__, __LINE__,
							TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
					}
				}

				delete [] (_pPathName);
				_pPathName		= (char *) NULL;

				return err;
			}
		}
	#endif

    #ifdef YACC_DEBUG
        #ifdef ANDROID_DEBUG
            qDebug() << "Configuration file in input (length: " << gllBufferToParseLength
                << "):" << endl << gpBufferToParse;
        #else
            cout << "Configuration file in input:" << endl
                << gpBufferToParse << endl;
        #endif
    #endif

    gerrParserError			= errNoError;
	glBufferToParseIndex	= 0;
	gbIsCaseSensitive				= bIsCaseSensitive;
	glCfgSectionsToAllocOnOverflow	= lCfgSectionsToAllocOnOverflow;
	glCfgItemsToAllocOnOverflow		= lCfgItemsToAllocOnOverflow;
	glBufferToAllocOnOverflow		= lBufferToAllocOnOverflow;

	if (yyparse () == 1)
	{
		delete [] gpBufferToParse;
		gpBufferToParse		= (char *) NULL;
		#ifdef WIN32
		#else
			if (bLockFile)
			{
				if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
					errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
				}
			}
		#endif
		delete [] (_pPathName);
		_pPathName				= (char *) NULL;

		return gerrParserError;
	}

//	pcfgOutputConfig (Config_p) is allocated and initialized
//		from yyparse and is the pointer to configuration

	delete [] gpBufferToParse;
	gpBufferToParse		= (char *) NULL;

	#ifdef WIN32
	#else
		if (bLockFile)
		{
			if ((errFileIO = FileIO:: unLockFile (pLockPathName, fdLockFile)) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_UNLOCKFILE_FAILED, 1, pLockPathName);

				pcfgOutputConfig -> finish ();
				delete pcfgOutputConfig;
				pcfgOutputConfig		= (Config_p) NULL;
				delete [] (_pPathName);
				_pPathName				= (char *) NULL;

				return errFileIO;
			}
		}
	#endif

	if (Config:: copy (this, pcfgOutputConfig,
		lCfgSectionsToAllocOnOverflow, lCfgItemsToAllocOnOverflow,
		lBufferToAllocOnOverflow) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_COPY_FAILED);
		pcfgOutputConfig -> finish ();
		delete pcfgOutputConfig;
		pcfgOutputConfig		= (Config_p) NULL;
		delete [] (_pPathName);
		_pPathName				= (char *) NULL;

		return err;
	}

	if (pcfgOutputConfig -> finish () != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_FINISH_FAILED);
		Config:: finish ();
		delete pcfgOutputConfig;
		pcfgOutputConfig		= (Config_p) NULL;
		delete [] (_pPathName);
		_pPathName					= (char *) NULL;

		return err;
	}

	delete pcfgOutputConfig;
	pcfgOutputConfig		= (Config_p) NULL;

	if (modifyConfigName (pConfigName) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_MODIFYCONFIGNAME_FAILED);
		Config:: finish ();
		delete [] (_pPathName);
		_pPathName					= (char *) NULL;

		return err;
	}

	_stCfgFileStatus		= CFGFILE_INITIALIZED;


	return errNoError;
}


Error ConfigurationFile:: init (const char *pPathName,
	Config_p pcfgConfig, const char *pConfigName,
	Boolean_t bIsCaseSensitive, long lCfgSectionsToAllocOnOverflow,
	long lCfgItemsToAllocOnOverflow, long lBufferToAllocOnOverflow)

{

	if (_stCfgFileStatus != CFGFILE_BUILDED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgFileStatus);

		return err;
	}

	if (pPathName == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_ACTIVATION_WRONG);

		return err;
	}

	if ((_pPathName = new char [strlen (pPathName) + 1]) == (char *) NULL)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_NEW_FAILED);

		return err;
	}

	strcpy (_pPathName, pPathName);

	if (pcfgConfig != (Config_p) NULL)
	{
		if (Config:: copy (this, pcfgConfig,
			lCfgSectionsToAllocOnOverflow, lCfgItemsToAllocOnOverflow,
			lBufferToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_COPY_FAILED);

			delete [] (_pPathName);
			_pPathName				= (char *) NULL;

			return err;
		}
	}
	else
	{
		if (Config::init (pConfigName,
			bIsCaseSensitive, lCfgSectionsToAllocOnOverflow) != errNoError)
		{
			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_INIT_FAILED);

			delete [] (_pPathName);
			_pPathName				= (char *) NULL;

			return err;
		}
	}

	_stCfgFileStatus		= CFGFILE_INITIALIZED;


	return errNoError;
}


Error ConfigurationFile:: finish (void)

{

	if (_stCfgFileStatus  != CFGFILE_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgFileStatus);

		return err;
	}

	Config:: finish ();

	delete [] (_pPathName);
	_pPathName				= (char *) NULL;

	_stCfgFileStatus		= CFGFILE_BUILDED;


	return errNoError;
}


ConfigurationFile:: operator const char * (void) const

{

	if (_stCfgFileStatus  != CFGFILE_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgFileStatus);

		return "";
	}


	return _pPathName;
}


Error ConfigurationFile:: save (const char *pNewPathName)

{

	int					iFileDescriptor;
	#ifdef WIN32
	#else
		int				fdLockFile;
		struct flock	flFileLock;
		Boolean_t		bConfigIsLocked;
		long			lStartUtcTime;
		long			lFcntlReturn;
		char			pLockPathName [CFGFILE_MAXPATHNAMELENGTH];
	#endif
	char			*pPathName;
	Error_t			errRemove;


	if (_stCfgFileStatus  != CFGFILE_INITIALIZED)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_OPERATIONNOTALLOWED,
			1, (long) _stCfgFileStatus);

		return err;
	}

	if (pNewPathName == (char *) NULL)
		pPathName		= _pPathName;
	else
		pPathName		= (char *) pNewPathName;

	#ifdef WIN32
	#else
		sprintf (pLockPathName, "%s.lck", _pPathName);

		if (FileIO:: lockFile (pLockPathName, 30,
			&fdLockFile) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);

			return err;
		}
	#endif

	#ifdef WIN32
		if (FileIO:: open (pPathName,
			O_WRONLY | O_TRUNC | O_CREAT,
			_S_IREAD | _S_IWRITE, &iFileDescriptor) != errNoError)
	#else
		if (FileIO:: open (pPathName,
			O_WRONLY | O_TRUNC | O_CREAT,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
			&iFileDescriptor) != errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED, 1, pPathName);

		#ifdef WIN32
		#else
			if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
			}
		#endif

		return err;
	}

	if (this -> write (iFileDescriptor) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_WRITE_FAILED);

		FileIO:: close (iFileDescriptor);
		#ifdef WIN32
		#else
			if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
			}
		#endif

		return err;
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_CLOSE_FAILED);

		#ifdef WIN32
		#else
			if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
				errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);
			}
		#endif

		return err;
	}

	#ifdef WIN32
	#else
		if (FileIO:: unLockFile (pLockPathName, fdLockFile) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEIO_LOCKFILE_FAILED, 1, pLockPathName);

			return err;
		}
	#endif


	return errNoError;
}


Error ConfigurationFile:: addItemAndSave (const char *pSectionName,
	const char *pItemName, const char *pValue,
	long lCfgItemsToAllocOnOverflow,
	long lBufferToAllocOnOverflow)

{
	Error_t			errCfg;


	if ((errCfg = appendItemValue (pSectionName, pItemName, pValue,
		lCfgItemsToAllocOnOverflow, lBufferToAllocOnOverflow)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_APPENDITEMVALUE_FAILED);

		return errCfg;
	}

	if ((errCfg = save ()) != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_SAVE_FAILED);

		return errCfg;
	}


	return errNoError;
}

