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


#ifndef Buffer_h
	#define Buffer_h


	#include "ToolsErrors.h"
	#ifdef WIN32
		#include <windows.h>
	#endif
	#include <iostream>

	#ifndef Boolean_t
		typedef long	Boolean_t;
	#endif

	#ifndef Boolean_p
		typedef long	*Boolean_p;
	#endif

	#ifndef false
		#define false	0L
	#endif

	#ifndef true
		#define true	1L
	#endif


	#define TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW			2


	typedef class Buffer

	{
		protected:
			typedef enum BufferStatus {
				TOOLS_BUFFER_BUILDED,
				TOOLS_BUFFER_INITIALIZED
			} BufferStatus_t, *BufferStatus_p;

		public:
			typedef enum StripType {
				STRIPTYPE_LEADING,
				STRIPTYPE_TRAILING,
				STRIPTYPE_BOTH
			} StripType_t, *StripType_p;

		private:
			BufferStatus_t		_bsBufferStatus;
			Boolean_t			_bMemoryAllocated;

			long				_lBufferBlockNumber;
			long				_lBufferBlockNumberOnOverflow;
			#ifdef WIN32
				__int64					_ullCharsNumber;
			#else
				unsigned long long		_ullCharsNumber;
			#endif

			#ifdef WIN32
				Error allocMemoryBlockForBufferIfNecessary (
					__int64 ullBufferLengthToInsert);
			#else
				Error allocMemoryBlockForBufferIfNecessary (
					unsigned long long ullBufferLengthToInsert);
			#endif

		protected:
			char				*_pBuffer;

			friend std:: ostream &operator << (std:: ostream &osOutputStream,
				Buffer &bBuffer);


		public:
			Buffer (void);

			Buffer (const Buffer &);

			/**
				Distrugge il corrente oggetto. Chiama il metodo finish
				nel caso in cui non sia stato chiamato.
			*/
			~Buffer (void);

			Buffer &operator = (const Buffer &bBuffer);

			Buffer &operator = (const char *pBuffer);

			Buffer &operator = (long lValue);

			Buffer &operator = (float fValue);

			Buffer &operator = (double dValue);
			#ifdef WIN32
				Buffer &operator = (__int64 llValue);
			#else
				Buffer &operator = (long long llValue);

				Buffer &operator = (unsigned long long ullValue);
			#endif

			#ifdef WIN32
				Error init (const char *pBuffer = "",
					__int64 llBufferLength = -1,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);
			#else
				Error init (const char *pBuffer = "",
					long long llBufferLength = -1,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);
			#endif

			#ifdef WIN32
				Error init (WCHAR *pwValue,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

				/**
					This static method allocates memory for the WCHAR string
					and converts a chars string into a WCHAR string.

					N.B.: If this method returns errNoError,
						the caller must deallocate the memory
						pointed by pwValue
				*/
				static Error conversionFromCharToWCHAR (
					const char *pBuffer,
					WCHAR **pwValue,
					unsigned long *pulWCHARBufferLength);
			#elif __QTCOMPILER__
				// it seems wchar is not used yet by Android
			#else
				static Error conversionFromCharToWCHAR (
					const char *pBuffer,
					wchar_t *pwBuffer,
					unsigned long ulWCHARBufferLength);

				static Error conversionFromWCHARToChar (
					const wchar_t *pwBuffer,
					char *pBuffer,
					unsigned long ulBufferLength);
			#endif

			Error init (long lValue,
				long lBufferBlockNumberOnOverflow =
				TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

			Error init (unsigned long ulValue,
				long lBufferBlockNumberOnOverflow =
				TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

			Error init (float fValue,
				long lBufferBlockNumberOnOverflow =
				TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

			Error init (double dValue,
				long lBufferBlockNumberOnOverflow =
				TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

			#ifdef WIN32
				Error init (__int64 llValue,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);
			#else
				Error init (long long llValue,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);

				Error init (unsigned long long ullValue,
					long lBufferBlockNumberOnOverflow =
					TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW);
			#endif

			/**
				Questo metodo e' anche invocato automaticamente dal
				distruttore del corrente oggetto.
				In quest'ultimo caso pero' non e' possibile gestire
				il valore di ritorno.

				The bReuseMemoryFromNextInit parameter indicates if the memory
				must be deallocate in the finish method (false) or if must be
				deallocated in the Buffer destructor and could be reused
				by a following init call.
				In case the parameter is set to true, we have an optimization,
				in fact when we have a scenario like
					'init - finish - init'
				instead to
					1. allocate (first init)
					2. deallocate (finish)
					3. allocate (second init)
				with the bReuseMemoryFromNextInit parameter set to true
				we will allocate the memory at the first 'init' and
				deallocate it when the Buffer will be destroyed
				by the destructor. The second init will use the same memory
				allocated by the first init.

				Note: The memory could be reused only if the
					lBufferBlockNumberOnOverflow parameter (in init method)
					does not change between the two init.
			*/
			Error finish (Boolean_t bReuseMemoryFromNextInit = true);

			Boolean_t isEmpty (void);

			Error startWith (const char *pBuffer, Boolean_p pbStartWith);

			Error isEqualWith (const char *pBuffer,
				Boolean_p pbIsEqualWith) const;

			Error isEqualWith (const Buffer &bBuffer,
				Boolean_p pbIsEqualWith) const;

			Error append (const Buffer &bBuffer);

			Error append (const Buffer *pbBuffer);

			Error append (const char *pBuffer, long lBufferLength = -1);

			Error append (char cValue);

			Error append (long lValue);

			Error append (unsigned long ulValue);

			Error append (short sValue);

			/*
			 * pSprintfFormat could be something like: "%.3f"
			 */
			Error append (float fValue,
				const char *pSprintfFormat = (const char *) NULL);

			/*
			 * pSprintfFormat could be something like: "%.3f"
			 */
			Error append (double dValue,
				const char *pSprintfFormat = (const char *) NULL);

/*
			#ifdef WIN32
				Error append (__int64 llValue);
			#else
*/
				Error append (long long llValue);

				Error append (unsigned long long ullValue);
//			#endif

			Error insertAt (long lStartIndex, const Buffer &bBuffer);

			Error insertAt (long lStartIndex, const char *pBuffer);

			Error substitute (const char *pInitial, const char *pFinal);

			Error substitute (const char *pInitial, unsigned long ulFinal);

			#ifdef WIN32
				Error substitute (const char *pInitial, __int64 ullFinal);
			#else
				Error substitute (const char *pInitial,
					unsigned long long ullFinal);
			#endif

			Error removeCTRLCharacters (void);

			Error removeChar (unsigned long ulIndex);

			Error truncateStartingFrom (StripType_t stStripType,
				char cCharToSearch);

			Error truncateIfBigger (unsigned long ulMaxLength);

			Error setChar (unsigned long ulIndex, char cNewChar);

			Error getBufferLength (unsigned long *pulBufferLength) const;

			Error getChar (char *pcChar, unsigned long ulIndex) const;

			Error getBuffer (char *pBuffer) const;

			Error setBuffer (const Buffer &bBuffer);

			Error setBuffer (const Buffer *pbBuffer);

			Error setBuffer (const char *pBuffer, long lBufferLength = -1);

			Error setBuffer (long lValue);

			Error setBuffer (unsigned long ulValue);

			Error setBuffer (int iValue);

			Error setBuffer (float fValue,
				const char *pSprintfFormat = (const char *) NULL);

			Error setBuffer (double dValue,
				const char *pSprintfFormat = (const char *) NULL);

			#ifdef WIN32
				Error setBuffer (__int64 llValue);
			#else
				Error setBuffer (long long llValue);

				Error setBuffer (unsigned long long ullValue);
			#endif

			Error readBufferFromFile (const char *pPathName,
				long lFileStartOffset = -1, long lFileEndOffset = -1);

			/*
			 * In case of WIN32, bIsBinaryFile is used because
			 * otherwise, it is placed \r\n at the end of any text line
			 */
			#ifdef WIN32
				Error writeBufferOnFile (const char *pPathName,
					Boolean_t bAppend = false,
					Boolean_t bExecutionPermission = false,
					Boolean_t bIsBinaryFile = false);
			#else
				Error writeBufferOnFile (const char *pPathName,
					Boolean_t bAppend = false,
					Boolean_t bExecutionPermission = false);
			#endif

			Error strip (StripType_t stStripType = STRIPTYPE_LEADING,
				const char *pStringToStrip = " ");

			Error strip (StripType_t stStripType = STRIPTYPE_LEADING,
				unsigned long ulCharactersNumberToStrip = 1);

			const char *str (void) const;

			unsigned long length (void) const;

			operator const char * (void) const;

			operator unsigned long (void) const;

			operator long (void) const;

			Boolean_t operator < (const Buffer &bBuffer);

			Boolean_t operator <= (const Buffer &bBuffer);

			Boolean_t operator > (const Buffer &bBuffer);

			Boolean_t operator >= (const Buffer &bBuffer);

			Buffer &operator += (const Buffer &bBuffer);

			Buffer &operator += (const char *pBuffer);

			Buffer &operator += (long lValue);

			Buffer &operator += (float fValue);

			Buffer &operator += (double dValue);

			#ifdef WIN32
				Buffer &operator += (__int64 llValue);
			#else
				Buffer &operator += (long long llValue);

				Buffer &operator += (unsigned long long ullValue);
			#endif

			Buffer operator + (const Buffer &bBuffer);

			Buffer operator + (const char *pBuffer);

			Boolean_t operator == (const Buffer &bBuffer) const;

			Boolean_t operator == (const char *pBuffer) const;

			char operator[] (long lIndex) const;

	} Buffer_t, *Buffer_p;


#endif

