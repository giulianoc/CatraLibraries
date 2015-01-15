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


#ifndef FileReader_h
	#define FileReader_h

	#include "ToolsErrors.h"

	#define TOOLS_FILEREADER_MINIMUMCACHESIZE		256

	#define TOOLS_FILEREADER_DEFAULTCACHESIZE		(1024 * 20)


	typedef class FileReader {

		protected:
			typedef enum FileReaderStatus {
				TOOLS_FILEREADER_BUILDED,
				TOOLS_FILEREADER_INITIALIZED
			} FileReaderStatus_t, *FileReaderStatus_p;

		private:
			FileReaderStatus_t		_frFileReaderStatus;
			int						_iFileDescriptor;
			char					*_pFilePath;
			unsigned char			*_pucCache;
			unsigned long			_ulCacheSize;
			unsigned long long		_ullCurrentFileOffset;
			unsigned long long		_ullCacheFileOffset;
			unsigned long long		_ullFileSize;


			Error refreshCacheIfNecessary (
				unsigned long long ullBytesToRead);

			Error readMoreThanCache (
				unsigned char *pucBuffer,
				unsigned long long ullBytesToRead);

		protected:
			FileReader (const FileReader &);

			FileReader &operator = (const FileReader &);

		public:
			/**
				Costruttore.
			*/
			FileReader ();

			/**
				Distruttore.
			*/
			~FileReader ();

			Error init (const char *pFilePath,
				unsigned long ulMaxCacheSize =
					TOOLS_FILEREADER_DEFAULTCACHESIZE);

			Error finish (void);

			Error write (const char *pFilePath);

			/**
				return the size of the file
			*/
			operator unsigned long long (void) const;

			/**
				return the path of the file
			*/
			operator const char * (void) const;

			Error seek (long long llBytes, int iWhence,
				long long *pllCurrentPosition);

			Error seekBySearch (const unsigned char *pucBufferToSearch,
				unsigned long ulBufferToSearchLength,
				unsigned long ulBufferSizeToBeUsed =
					TOOLS_FILEREADER_DEFAULTCACHESIZE);

			Error readChars (char *pBuffer,
				unsigned long long ullCharsNumber,
				Boolean_t bErrorIfAllBytesNotAvailable,
				unsigned long long *pullCharsRead);

			Error readLine (char *pBuffer,
				unsigned long long ullBufferLength,
				unsigned long long *pullCharsRead);

			Error readBytes (unsigned char *pucValue,
				unsigned long long ullBytesNumber,
				Boolean_t bErrorIfAllBytesNotAvailable,
				unsigned long long *pullBytesRead);

			Error readNetUnsignedInt8Bit (unsigned long *pulValue);

			Error readNetUnsignedInt16Bit (unsigned long *pulValue);

			Error readNetUnsignedInt24Bit (unsigned long *pulValue);

			Error readNetUnsignedInt32Bit (unsigned long *pulValue);

			Error readNetUnsignedInt64Bit (unsigned long long *pullValue);

			Error readNetFloat16Bit (float *pfValue);

			Error readNetFloat32Bit (float *pfValue);

			Error readMP4DescriptorSize (unsigned long *pulSize,
				unsigned char *pucNumBytes);

	} FileReader_t, *FileReader_p;

#endif

