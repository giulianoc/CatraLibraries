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


#ifndef FileIO_h
#define FileIO_h

#include <memory>
#include <chrono>
#include <string>
#include "ToolsErrors.h"
#include "Buffer.h"
#include <sys/types.h>
#ifdef WIN32
#else
        #include <dirent.h>
#endif
#include <fcntl.h>
#ifdef __QTCOMPILER__
        #include <linux/stat.h>
#endif

using namespace std;

struct DirectoryNotExisting : std::exception 
{ 
    char const* what() const throw() 
    {
        return "Directory not existing";
    }; 
};

struct FileNotExisting : std::exception 
{ 
    char const* what() const throw() 
    {
        return "File not existing";
    }; 
};

struct DirectoryListFinished : std::exception 
{ 
    char const* what() const throw() 
    {
        return "Directory list is finished";
    }; 
};

/**
        The FileIO class is a collection of static methods just
        to hide the differences to access a File/Directory between
        different operative system.
*/
typedef class FileIO

{

public:
    typedef struct Directory
    {
        char					*_pPathName;
        #ifdef WIN32
                long				_lIdentifier;
        #else
                DIR					*_pdDir;
        #endif

        Directory()
        {
            _pPathName = (char *) NULL;
            
            #ifdef WIN32
                    _lIdentifier    = -1;
            #else
                    _pdDir          = (DIR*) NULL;
            #endif
        }
        ~Directory()
        {
            if (_pPathName != (char *) NULL)
                FileIO::closeDirectory(this);
        }
    } Directory_t, *Directory_p;

    typedef enum DirectoryEntryType
    {
            TOOLS_FILEIO_REGULARFILE,
            TOOLS_FILEIO_DIRECTORY,
            TOOLS_FILEIO_LINKFILE,
            TOOLS_FILEIO_UNKNOWN
    } DirectoryEntryType_t, *DirectoryEntryType_p;

private:
    FileIO (const FileIO &);

    FileIO &operator = (const FileIO &);

public:
    /**
            Costruttore.
    */
    FileIO ();

    /**
            Distruttore.
    */
    ~FileIO ();

    static Error getFileNameFromPathFileName (const char *pPathFileName,
            Boolean_t bExtension, Buffer_p pbFileName,
            const char *pDirectorySeparator = "/");

    static Error replaceUnsafeFileNameChars (Buffer_p pbFileName);

    static Error openDirectory (const char *pDirectoryPathName,
            Directory_p pdDirectory);

    static shared_ptr<FileIO::Directory> openDirectory (string directoryPathName);

    static Error readDirectory (Directory_p pdDirectory,
            Buffer_p pbDirectoryEntry,
            DirectoryEntryType_p pdetDirectoryEntryType);

    static string readDirectory (shared_ptr<Directory> directory,
        DirectoryEntryType_p pdetDirectoryEntryType);

    static Error closeDirectory (Directory_p pdDirectory);

    static void closeDirectory (shared_ptr<Directory> directory);

    static Error getWorkingDirectory (char *pWorkingDirectory,
            unsigned long ulBufferLength);

    static Error isDirectoryExisting (const char *pDirectoryPathName,
            Boolean_p pbExist);

    static bool directoryExisting (string directoryPathName);

    static Error getDirectoryEntryType (const char *pPathName,
            DirectoryEntryType_p pdetDirectoryEntryType);

    static FileIO::DirectoryEntryType_t getDirectoryEntryType (string pathName);

/*
    #ifdef WIN32
            static Error getFileSystemInfo (const char *pPathName,
                    __int64 *pulUsedInKB,
                    __int64 *pulAvailableInKB,
                    long *plPercentUsed);
    #else
*/
            static Error getFileSystemInfo (const char *pPathName,
                    unsigned long long *pulUsedInKB,
                    unsigned long long *pulAvailableInKB,
                    long *plPercentUsed);

            static void getFileSystemInfo (string pathName,
                    unsigned long long *pulUsedInKB,
                    unsigned long long *pulAvailableInKB,
                    long *plPercentUsed);
//			#endif

    /*
     *	Synonymous of the getDirectoryUsage API
     */
    static Error getDirectorySizeInBytes (
            const char *pDirectoryPathName,
            unsigned long long *pullDirectorySizeInBytes);

    static unsigned long long getDirectorySizeInBytes (string directoryPathName);

    /*
     * Return the bytes used by the directory considering the size of
     * any recorsive subdirectory
     */
    static Error getDirectoryUsage (const char *pDirectoryPathName,
            unsigned long long *pullDirectoryUsageInBytes);

    static unsigned long long getDirectoryUsage (string directoryPathName);

    static Error moveDirectory (const char *pSrcPathName,
            const char *pDestPathName, int mDirectoryMode);

    static void moveDirectory (string srcPathName,
	string destPathName, int mDirectoryMode);

    static Error copyDirectory (const char *pSrcPathName,
            const char *pDestPathName, int mDirectoryMode);

    static void copyDirectory (string srcPathName,
            string destPathName, int mDirectoryMode);

#ifdef WIN32
    #else
            /**
                    Change the permission of a file.
                    To have details on the mMode parameters
                    see 'man 2 chmod'.
                    mMode could be:
                            S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
            */
            static Error changePermission (const char *pPathName,
                    mode_t mMode);
    #endif

    #ifdef WIN32
            /*
             * it assumes that the CoInitialize function has already
             * been called
             * May be you could call it in the main or in the init
             * of the class Service
             */
            static Error readLink (const char *pLinkPathName,
                    char *pRealPathName,
                    unsigned long ulRealPathNameLength);
    #else
            /**
                    This method return a NULL terminated string containing
                    the absolute real path name of the entry pointed by the
                    link
            */
            static Error readLink (const char *pLinkPathName,
                    char *pRealPathName,
                    unsigned long ulRealPathNameLength);
    #endif

    /*
     * bRecursive: create all the directories of the pPathName
     * 	if some of the directories in the path is missing
     */
    static Error createDirectory (const char *pPathName,
            int mMode, Boolean_t bNoErrorIfExists = true,
            Boolean_t bRecursive = false);

    static void createDirectory (string pathName,
            int mMode, bool bNoErrorIfExists = true,
            bool bRecursive = false);

    static Error removeDirectory (const char *pPathName,
            Boolean_t bRemoveRecursively);

    static void removeDirectory (string pathName, bool removeRecursively);

#ifdef USEGZIPLIB
            static Error gzip (const char *pUnCompressedPathName,
                    Boolean_t bSourceFileToBeRemoved,
                    long lSizeOfEachBlockToGzip = 1000);
    #endif

    /**
            It returns the time of the last modification of the file
    */
    static Error getFileTime (const char *pPathName,
            time_t *ptLastModificationTime);

    static chrono::system_clock::time_point getFileTime (string pathName);

    /**
     * It returns the size of the file. In case the path name is a link,
     * this method read the link and return the size of the real file
     */
    static Error getFileSizeInBytes (const char *pPathName,
            unsigned long *pulFileSize,
            Boolean_t bInCaseOfLinkHasItToBeRead);

    static unsigned long getFileSizeInBytes (string pathName,
	bool inCaseOfLinkHasItToBeRead);

    static Error isFileExisting (const char *pPathName,
            Boolean_p pbExist);

    static Boolean_t isFileExisting (const char *pPathName);
    
    static bool fileExisting (string pathName);

    /*
     *
     * pDestPath: it could refer a file or a directory
     * ulBufferSizeToBeUsed: if it is 0, it is calculated
     * 	dinamically the buffer to be used
     *
     */
    static Error copyFile (const char *pSrcPathName,
            const char *pDestPath,
            unsigned long ulBufferSizeToBeUsed = 0);

    static void copyFile (string srcPathName,
            string destPath,
            unsigned long ulBufferSizeToBeUsed = 0);

    void concatFile (string destPathName,
	string srcPathName, bool removeSrcFileAfterConcat,
	unsigned long ulBufferSizeToBeUsed);

    /**
            * pSrcPathName has to refer a file
            * pDestPathName could refer a file or a directory
    */
    static Error moveFile (const char *pSrcPathName,
            const char *pDestPathName);

    static void moveFile (string srcPathName, string destPathName);

    #ifdef WIN32
    #else
            /*
             * pSrcPathName is the Path Name of the link
             * pDestPathName could be a the name of the link or a directory
             */
            static Error moveLink (const char *pSrcPathName,
                    const char *pDestPathName);
    #endif

    #ifdef WIN32
            /*
             * it assumes that the CoInitialize function has already
             * been called
             * May be you could call it in the main or in the init
             * of the class Service
             */
            static Error createLink (const char *pOriPathName,
                    const char *pLinkPathName, Boolean_t bReplaceItIfExist,
                    Boolean_t bRecursiveDirectoriesCreation = false,
                    int mMode = 0);
    #else
            /**
                    * pOriPathName is the original Path Name
                    * pLinkPathName is the Path Name of the link
            */
            static Error createLink (const char *pOriPathName,
                    const char *pLinkPathName, Boolean_t bReplaceItIfExist,
                    Boolean_t bRecursiveDirectoriesCreation = false,
                    int mMode = S_IRUSR | S_IWUSR | S_IXUSR |
                            S_IRGRP | S_IXGRP |
                            S_IROTH | S_IXOTH);
    #endif

    /**
            To have details on the iFlags and mMode parameters
            see 'man 2 open' for unix system and
            help on _open on Windows system
            iFlags for unix or windows env. could be:
                    O_WRONLY | O_TRUNC | O_CREAT
            mMode for unix env. could be:
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
            mMode for windows env. could be:
                    _S_IREAD | _S_IWRITE
    */
    #ifdef WIN32
            static Error open (const char *pPathName, int iFlags,
                    int mMode, int *piFileDescriptor);
    #else
            static Error open (const char *pPathName, int iFlags,
                    mode_t mMode, int *piFileDescriptor);
    #endif

    /**
            To have details on the iFlags and mMode parameters
            see 'man 2 open' for unix system and
            help on _open on Windows system
            iFlags for unix or windows env. could be:
                    O_WRONLY | O_TRUNC | O_CREAT
    */
    static Error open (const char *pPathName, int iFlags,
            int *piFileDescriptor);

    static Error close (int iFileDescriptor);

    static Error touchFile (const char *pPathName);

    /**
            Remove a file or a link (only on linux?)
    */
    static Error remove (const char *pPathName);

    static void remove (string pathName, bool exceptionInCaseOfError = true);

    #ifdef WIN32
            static Error seek (int iFileDescriptor, __int64 llBytes,
                    int iWhence, __int64 *pllCurrentPosition);
    #else
            static Error seek (int iFileDescriptor, long long llBytes,
                    int iWhence, long long *pllCurrentPosition);
    #endif

    #ifdef WIN32
            static Error readChars (int iFileDescriptor,
                    char *pBuffer, __int64 ullCharsNumberToBeRead,
                    __int64 *pllBytesRead);
    #else
            static Error readChars (int iFileDescriptor,
                    char *pBuffer, unsigned long long ullCharsNumberToBeRead,
                    long long *pllBytesRead);
    #endif

    #ifdef WIN32
            static Error writeChars (int iFileDescriptor,
                    const char *pBuffer, __int64 ullCharsNumber,
                    __int64 *pllBytesWritten);
    #else
            static Error writeChars (int iFileDescriptor,
                    const char *pBuffer, unsigned long long ullCharsNumber,
                    long long *pllBytesWritten);
    #endif

    #ifdef WIN32
            static Error readBytes (int iFileDescriptor,
                    unsigned char *pucValue, __int64 ullBytesNumber,
                    __int64 *pllBytesRead);
    #else
            static Error readBytes (int iFileDescriptor,
                    unsigned char *pucValue, unsigned long long ullBytesNumber,
                    long long *pllBytesRead);
    #endif

    #ifdef WIN32
            static Error writeBytes (int iFileDescriptor,
                    unsigned char *pucValue, __int64 ullBytesNumber,
                    __int64 *pllBytesWritten);
    #else
            static Error writeBytes (int iFileDescriptor,
                    unsigned char *pucValue, unsigned long long ullBytesNumber,
                    long long *pllBytesWritten);
    #endif

    static Error readNetUnsignedInt8Bit (int iFileDescriptor,
            unsigned long *pulValue);

    static Error writeNetUnsignedInt8Bit (int iFileDescriptor,
            unsigned long ulValue);

    static Error readNetUnsignedInt16Bit (int iFileDescriptor,
            unsigned long *pulValue);

    static Error writeNetUnsignedInt16Bit (int iFileDescriptor,
            unsigned long ulValue);

    static Error readNetUnsignedInt24Bit (int iFileDescriptor,
            unsigned long *pulValue);

    static Error writeNetUnsignedInt24Bit (int iFileDescriptor,
            unsigned long ulValue);

    static Error readNetUnsignedInt32Bit (int iFileDescriptor,
            unsigned long *pulValue);

    static Error writeNetUnsignedInt32Bit (int iFileDescriptor,
            unsigned long ulValue);

    #ifdef WIN32
            static Error readNetUnsignedInt64Bit (int iFileDescriptor,
                    __int64 *pullValue);
    #else
            static Error readNetUnsignedInt64Bit (int iFileDescriptor,
                    unsigned long long *pullValue);
    #endif

    #ifdef WIN32
            static Error writeNetUnsignedInt64Bit (int iFileDescriptor,
                    __int64 ullValue);
    #else
            static Error writeNetUnsignedInt64Bit (int iFileDescriptor,
                    unsigned long long ullValue);
    #endif

    static Error readNetFloat16Bit (int iFileDescriptor,
            float *pfValue);

    static Error writeNetFloat16Bit (int iFileDescriptor,
            float fValue);

    static Error readNetFloat32Bit (int iFileDescriptor,
            float *pfValue);

    static Error writeNetFloat32Bit (int iFileDescriptor,
            float fValue);

    static Error readMP4DescriptorSize (int iFileDescriptor,
            unsigned long *pulSize, unsigned char *pucNumBytes);

    static Error writeMP4DescriptorSize (int iFileDescriptor,
            unsigned long ulSize, Boolean_t bCompact,
            unsigned char ucOriginalNumBytes);

    /**
            Lock the file specified by the pLockPathName parameter
            and return its file descriptor.
            In case the file is already locked, this method wait for
            lSecondsToWaitIfAlreadyLocked seconds to see if the file
            is unlocked.
            This method returns the TOOLS_FILEIO_FILENOTRELEASED in case:
            1. lSecondsToWaitIfAlreadyLocked is major than 0 and
                    the file is already locked after that the time is passed
            2. lSecondsToWaitIfAlreadyLocked is -1 and
                    the file is already locked
    */
    static Error lockFile (const char *pLockPathName,
            long lSecondsToWaitIfAlreadyLocked, int *pfdLockFile);

    /**
            Unlock the lock file specified by the pLockPathName parameter
            and fdLockFile file descriptor. In case of error, this method
            reset (close file descriptor and remove the lock file)
            the lock file.
    */
    static Error unLockFile (const char *pLockPathName, int fdLockFile);

    /**
            Append a buffer terminated with '\0' to the file
            specified by pPathName
    */
    static Error appendBuffer (const char *pPathName,
            const char *pBuffer, Boolean_t bAppendNewLine);

    /**
            Append an unsigned long to the file specified by pPathName
    */
    static Error appendUnsignedLong (const char *pPathName,
            unsigned long ulValue, Boolean_t bAppendNewLine);

    /**
            Append an long long to the file specified by pPathName
    */
    static Error appendLongLong (const char *pPathName,
            long long llValue, Boolean_t bAppendNewLine);

    /**
            Append a variable number of buffers terminated with '\0'
            to the file specified by pPathName
            All the variable parameters must be 'char *'
    */
    static Error appendBuffer (const char *pPathName,
            Boolean_t bAppendNewLine, long lBuffersNumber, ...);

} FileIO_t, *FileIO_p;

#endif

