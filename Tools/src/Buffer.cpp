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


#include "Buffer.h"
#include "FileIO.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef WIN32
	#include <io.h>
#else
	#include <unistd.h>
#endif

Buffer:: Buffer (void)

{

	_bsBufferStatus			= TOOLS_BUFFER_BUILDED;

	_bMemoryAllocated		= false;
}


Buffer:: Buffer (const Buffer &bBuffer)

{

	_bsBufferStatus			= TOOLS_BUFFER_BUILDED;

	*this					= bBuffer;

}


Buffer:: ~Buffer (void)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}

	if (_bMemoryAllocated)
	{
		delete [] _pBuffer;
		_pBuffer				= (char *) NULL;

		_lBufferBlockNumber		= 0;

		_bMemoryAllocated		= false;
	}

}


Buffer &Buffer:: operator = (const Buffer &bBuffer)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}

	if (bBuffer. _bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		if (init (bBuffer. _pBuffer, bBuffer. _ullCharsNumber,
			bBuffer. _lBufferBlockNumberOnOverflow) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}


	return *this;
}


Buffer &Buffer:: operator = (long lValue)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (init (lValue, _lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
	}


	return *this;
}


Buffer &Buffer:: operator = (float fValue)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (init (fValue, _lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
	}


	return *this;
}


Buffer &Buffer:: operator = (double dValue)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (init (dValue, _lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
	}


	return *this;
}

#ifdef WIN32
	Buffer &Buffer:: operator = (__int64 llValue)
#else
	Buffer &Buffer:: operator = (long long llValue)
#endif
{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (init (llValue, _lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
	}


	return *this;
}


#ifndef WIN32
Buffer &Buffer:: operator = (unsigned long long ullValue)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (init (ullValue, _lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
	}


	return *this;
}
#endif


Buffer &Buffer:: operator = (const char *pBuffer)

{

	if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		// delete the current object
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}
	}
	else
		_lBufferBlockNumberOnOverflow						=
			TOOLS_DEFAULTBUFFERBLOCKNUMBERONOVERFLOW;

	if (pBuffer != (const char *) NULL)
	{
		if (init (pBuffer, strlen (pBuffer), _lBufferBlockNumberOnOverflow) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}
	else
	{
		if (init ((const char *) NULL, 0, _lBufferBlockNumberOnOverflow) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}


	return *this;
}


#ifdef WIN32
	Error Buffer:: init (const char *pBuffer, __int64 llBufferLength,
		long lBufferBlockNumberOnOverflow)
#else
	Error Buffer:: init (const char *pBuffer, long long llBufferLength,
		long lBufferBlockNumberOnOverflow)
#endif

{

	if (pBuffer == (const char *) NULL ||
		(llBufferLength < 0 && llBufferLength != -1) ||
		lBufferBlockNumberOnOverflow <= 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, (long) _bsBufferStatus);

		return err;
	}

	// In a scenario 'init - finish - init' the finish will not deallocate
	// the memory in order to be reused by the next init
	if (!_bMemoryAllocated)
	{
		_lBufferBlockNumber						= 0;
		_lBufferBlockNumberOnOverflow			= lBufferBlockNumberOnOverflow;

		_pBuffer								= (char *) NULL;
	}
	else
	{
		// memory already allocated
		if (_lBufferBlockNumberOnOverflow != lBufferBlockNumberOnOverflow)
		{
			delete [] _pBuffer;
			_pBuffer				= (char *) NULL;

			_lBufferBlockNumber		= 0;
			_lBufferBlockNumberOnOverflow		= lBufferBlockNumberOnOverflow;
		}
		else
		{
			// empty the buffer from data

			if (_lBufferBlockNumber * _lBufferBlockNumberOnOverflow > 0)
				_pBuffer [0]			= '\0';
			else
			{
				// since when the memory is already allocated
				// we have at least one byte, the flow should'n never enter here
				delete [] _pBuffer;
				_pBuffer				= (char *) NULL;

				_lBufferBlockNumber		= 0;
				_lBufferBlockNumberOnOverflow	= lBufferBlockNumberOnOverflow;
			}
		}
	}

	if (llBufferLength == -1)
	{
		unsigned long			ulBufferLength;


		ulBufferLength				= strlen (pBuffer);

		if (allocMemoryBlockForBufferIfNecessary (ulBufferLength) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		strcpy (_pBuffer, pBuffer);
		_ullCharsNumber				= ulBufferLength;
	}
	else
	{
		if (allocMemoryBlockForBufferIfNecessary (llBufferLength) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		if (llBufferLength != 0)
		{
			strncpy (_pBuffer, pBuffer, (size_t) llBufferLength);
			_pBuffer [llBufferLength]		= '\0';
		}

		_ullCharsNumber				= llBufferLength;
	}


	_bsBufferStatus		= TOOLS_BUFFER_INITIALIZED;


	return errNoError;
}


#ifdef WIN32
	Error Buffer:: init (WCHAR *pwValue,
		long lBufferBlockNumberOnOverflow)

	{

		char				*pBuffer;
		unsigned long		ulCharsLength;


		ulCharsLength = WideCharToMultiByte (CP_ACP, 0,
			pwValue, -1, (char *) NULL, 0, NULL, NULL);

		if ((pBuffer = new char [ulCharsLength]) ==
			(char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			return err;
		}

		if (WideCharToMultiByte (CP_ACP, 0, pwValue, -1,
			pBuffer, ulCharsLength, NULL, NULL) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_WIDECHARTOMULTIBYTE_FAILED,
				1, (long) GetLastError ());

			delete [] pBuffer;

			return err;
		}

		if (init (pBuffer, strlen (pBuffer),
			lBufferBlockNumberOnOverflow) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			delete [] pBuffer;

			return err;
		}

		delete [] pBuffer;


		return errNoError;
	}

	Error Buffer:: conversionFromCharToWCHAR (
		const char *pBuffer,
		WCHAR **pwValue,
		unsigned long *pulWCHARBufferLength)

	{

		*pulWCHARBufferLength = MultiByteToWideChar (CP_ACP, 0,
			pBuffer, -1, (WCHAR *) NULL, 0);

		if ((*pwValue = new WCHAR [*pulWCHARBufferLength]) ==
			(WCHAR *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			return err;
		}

		if (MultiByteToWideChar (CP_ACP, 0, pBuffer, -1,
			*pwValue, *pulWCHARBufferLength) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_MULTIBYTETOWIDECHAR_FAILED);

			delete [] (*pwValue);

			return err;
		}


		return errNoError;
	}
#elif __QTCOMPILER__
#else
	Error Buffer:: conversionFromCharToWCHAR (
		const char *pBuffer,
		wchar_t *pwBuffer,
		unsigned long ulWCHARBufferLength)

	{
		unsigned long			ulLength;
		size_t					ulReturn;


		if (pBuffer == (const char *) NULL ||
			pwBuffer == (wchar_t *) NULL ||
			ulWCHARBufferLength == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		ulLength			= strlen (pBuffer);

		if (ulLength >= ulWCHARBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		if ((ulReturn = mbstowcs(pwBuffer, pBuffer, ulLength)) != ulLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_MBSTOWCS_FAILED);

			return err;
		}


		return errNoError;
	}

	Error Buffer:: conversionFromWCHARToChar (
		const wchar_t *pwBuffer,
		char *pBuffer,
		unsigned long ulBufferLength)

	{
		unsigned long			ulWCHARLength;
		size_t					ulReturn;
		mbstate_t				ps;


		if (pBuffer == (char *) NULL ||
			pwBuffer == (const wchar_t *) NULL ||
			ulBufferLength == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		ulWCHARLength			= wcslen (pwBuffer);

		if (ulWCHARLength >= ulBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_ACTIVATION_WRONG);

			return err;
		}

		if ((ulReturn = wcsrtombs(pBuffer, &pwBuffer, ulWCHARLength,
			&ps)) != ulWCHARLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_WCSRTOMBS_FAILED);

			return err;
		}


		return errNoError;
	}
#endif


Error Buffer:: init (long lValue,
	long lBufferBlockNumberOnOverflow)

{

	char			pLongBuffer [128 + 1];


	if (sprintf (pLongBuffer, "%ld", lValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pLongBuffer, strlen (pLongBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: init (unsigned long ulValue,
	long lBufferBlockNumberOnOverflow)

{

	char			pUnsignedLongBuffer [128 + 1];


	if (sprintf (pUnsignedLongBuffer, "%lu", ulValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pUnsignedLongBuffer, strlen (pUnsignedLongBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: init (float fValue,
	long lBufferBlockNumberOnOverflow)

{

	char			pFloatBuffer [128 + 1];


	if (sprintf (pFloatBuffer, "%lf", fValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pFloatBuffer, strlen (pFloatBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: init (double dValue,
	long lBufferBlockNumberOnOverflow)

{

	char			pDoubleBuffer [128 + 1];


	if (sprintf (pDoubleBuffer, "%lf", dValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pDoubleBuffer, strlen (pDoubleBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error Buffer:: init (__int64 llValue,
		long lBufferBlockNumberOnOverflow)
#else
	Error Buffer:: init (long long llValue,
		long lBufferBlockNumberOnOverflow)
#endif

{

	char			pLongLongBuffer [128 + 1];


	if (sprintf (pLongLongBuffer, "%lld", llValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pLongLongBuffer, strlen (pLongLongBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}

#ifndef WIN32
Error Buffer:: init (unsigned long long ullValue,
	long lBufferBlockNumberOnOverflow)

{

	char			pUnsignedLongLongBuffer [128 + 1];


	if (sprintf (pUnsignedLongLongBuffer, "%llu", ullValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (init (pUnsignedLongLongBuffer, strlen (pUnsignedLongLongBuffer),
		lBufferBlockNumberOnOverflow) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}


	return errNoError;
}
#endif


Error Buffer:: finish (Boolean_t bReuseMemoryFromNextInit)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
		*/

		// nothing to be done

		return errNoError;
	}

	if (!bReuseMemoryFromNextInit && _lBufferBlockNumber != 0)
	{
		delete [] _pBuffer;
		_pBuffer				= (char *) NULL;

		_lBufferBlockNumber		= 0;

		_bMemoryAllocated		= false;
	}

	_bsBufferStatus		= TOOLS_BUFFER_BUILDED;


	return errNoError;
}


Boolean_t Buffer:: isEmpty (void)

{
	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return true;
	}

	if (!strcmp (_pBuffer, ""))
		return true;
	else
		return false;
}


Error Buffer:: startWith (const char *pBuffer, Boolean_p pbStartWith)

{

	unsigned long				ulBufferLength;


	if (pBuffer == (const char *) NULL ||
		pbStartWith == (Boolean_p) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (getBufferLength (&ulBufferLength) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_GETBUFFERLENGTH_FAILED);

		return err;
	}

	if (ulBufferLength < strlen (pBuffer))
		*pbStartWith			= false;
	else
	{
		if (!strncmp (_pBuffer, pBuffer, strlen (pBuffer)))
			*pbStartWith			= true;
		else
			*pbStartWith			= false;
	}


	return errNoError;
}


Error Buffer:: isEqualWith (const char *pBuffer, Boolean_p pbIsEqualWith) const

{

	if (pBuffer == (const char *) NULL ||
		pbIsEqualWith == (Boolean_p) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	// se sono entrambi puntatori nulli o comunque puntano alla
	// stessa locazione di memoria
	if (pBuffer == (const char *) (*this))
		*pbIsEqualWith				= true;
	// se solo uno dei due e' un puntatore nullo
	else if (pBuffer == (const char *) NULL ||
		(const char *) (*this) == (const char *) NULL)
		*pbIsEqualWith				= false;
	// se entrambi i puntatori sono non nulli
	else if (!strcmp ((const char *) (*this), pBuffer))
		*pbIsEqualWith				= true;
	else
		*pbIsEqualWith				= false;


	return errNoError;
}


Error Buffer:: isEqualWith (const Buffer &bBuffer,
	Boolean_p pbIsEqualWith) const

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED ||
		bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (!strcmp ((const char *) (*this), (const char *) bBuffer))
		*pbIsEqualWith					= true;
	else
		*pbIsEqualWith					= false;


	return errNoError;
}


Error Buffer:: append (const Buffer &bBuffer)

{

	if (bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (append ((const char *) bBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (const Buffer *pbBuffer)

{

	if (pbBuffer == (const Buffer *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (pbBuffer -> _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (append ((const char *) (*pbBuffer)) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (const char *pBuffer, long lBufferLength)

{

	if (pBuffer == (const char *) NULL ||
		(lBufferLength < 0 && lBufferLength != -1))
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
		*/

		if (init ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}
	}

	if (lBufferLength == -1)
	{
		unsigned long				ulBufferLength;


		ulBufferLength				= strlen (pBuffer);

		if (allocMemoryBlockForBufferIfNecessary (ulBufferLength) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		memcpy (_pBuffer + _ullCharsNumber, pBuffer, ulBufferLength);
		_ullCharsNumber							+= ulBufferLength;
		_pBuffer [_ullCharsNumber]				= '\0';
	}
	else
	{
		if (allocMemoryBlockForBufferIfNecessary (lBufferLength) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		memcpy (_pBuffer + _ullCharsNumber, pBuffer, lBufferLength);
		_ullCharsNumber							+= lBufferLength;
		_pBuffer [_ullCharsNumber]				= '\0';
	}


	return errNoError;
}


Error Buffer:: append (char cValue)

{

	char			pChar [2];


	pChar [0]			= cValue;
	pChar [1]			= '\0';

	if (append (pChar) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (long lValue)

{

	char			pLongBuffer [128 + 1];


	if (sprintf (pLongBuffer, "%ld", lValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (append (pLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (unsigned long ulValue)

{

	char			pUnsignedLongBuffer [128 + 1];


	if (sprintf (pUnsignedLongBuffer, "%lu", ulValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (append (pUnsignedLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (short sValue)

{

	char			pShortBuffer [128 + 1];


	if (sprintf (pShortBuffer, "%d", sValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (append (pShortBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (float fValue, const char *pSprintfFormat)

{

	char			pFloatBuffer [128 + 1];


	if (pSprintfFormat == (const char *) NULL)
	{
		if (sprintf (pFloatBuffer, "%lf", fValue) < 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SPRINTF_FAILED);

			return err;
		}
	}
	else
	{
		if (sprintf (pFloatBuffer, pSprintfFormat, fValue) < 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SPRINTF_FAILED);

			return err;
		}
	}

	if (append (pFloatBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: append (double dValue, const char *pSprintfFormat)

{

	char			pDoubleBuffer [128 + 1];


	if (pSprintfFormat == (const char *) NULL)
	{
		if (sprintf (pDoubleBuffer, "%lf", dValue) < 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SPRINTF_FAILED);

			return err;
		}
	}
	else
	{
		if (sprintf (pDoubleBuffer, pSprintfFormat, dValue) < 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SPRINTF_FAILED);

			return err;
		}
	}

	if (append (pDoubleBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


/*
#ifdef WIN32
	Error Buffer:: append (__int64 llValue)
#else
*/
	Error Buffer:: append (long long llValue)
// #endif

{

	char			pLongLongBuffer [128 + 1];


	if (sprintf (pLongLongBuffer, "%lld", llValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (append (pLongLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


// #ifndef WIN32
Error Buffer:: append (unsigned long long ullValue)

{

	char			pUnsignedLongLongBuffer [128 + 1];


	if (sprintf (pUnsignedLongLongBuffer, "%llu", ullValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (append (pUnsignedLongLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}
// #endif


Error Buffer:: insertAt (long lStartIndex, const Buffer &bBuffer)

{

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED ||
		bBuffer. _bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	return insertAt (lStartIndex, (const char *) bBuffer);
}


Error Buffer:: insertAt (long lStartIndex, const char *pBuffer)

{

	if (lStartIndex < 0 || pBuffer == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (lStartIndex > (long) (*this))
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (lStartIndex == (long) (*this))
	{
		if (append (pBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			return err;
		}
	}
	else
	{
		char				*pRemainingBuffer;


		if ((pRemainingBuffer = new char [
			strlen (_pBuffer + lStartIndex) + 1]) == (char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			return err;
		}
		strcpy (pRemainingBuffer, _pBuffer + lStartIndex);

		_pBuffer [lStartIndex]				= '\0';
		_ullCharsNumber						= lStartIndex;

		if (append (pBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			delete [] pRemainingBuffer;

			return err;
		}

		if (append (pRemainingBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_APPEND_FAILED);

			delete [] pRemainingBuffer;

			return err;
		}

		delete [] pRemainingBuffer;
	}


	return errNoError;
}


Error Buffer:: substitute (const char *pInitial, const char *pFinal)

{

	char			*pDestinationToSubstitutePrev;
	char			*pDestinationToSubstituteNext;
	long			lSubstitutionNumber;
	long			lDifferentMemoryAfterSubstitution;
	long			lFinalLength;
	long			lInitialLength;
	long			lTmpLength;
	char			*pStringToCopy;


	if (pInitial == (const char *) NULL ||
		pFinal == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_lBufferBlockNumber == 0)
		return errNoError;

	lFinalLength						= strlen (pFinal);
	lInitialLength						= strlen (pInitial);

	pDestinationToSubstitutePrev		= _pBuffer;
	pDestinationToSubstituteNext		= _pBuffer;
	lSubstitutionNumber					= 0;

	while ((pDestinationToSubstituteNext = strstr (pDestinationToSubstituteNext,
		pInitial)) != (char *) NULL)
	{
		lSubstitutionNumber++;

		// update the previous and the next pointer to the header
		pDestinationToSubstitutePrev		= pDestinationToSubstituteNext +
			lInitialLength;
		pDestinationToSubstituteNext		= pDestinationToSubstitutePrev;
	}

	if (lSubstitutionNumber == 0)
		return errNoError;

	lDifferentMemoryAfterSubstitution	= (lFinalLength - lInitialLength) *
		lSubstitutionNumber;

	if (lDifferentMemoryAfterSubstitution > 0)
	{
		if (allocMemoryBlockForBufferIfNecessary (
			lDifferentMemoryAfterSubstitution) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}
	}

	pDestinationToSubstitutePrev		= _pBuffer;
	pDestinationToSubstituteNext		= _pBuffer;

	while ((pDestinationToSubstituteNext = strstr (pDestinationToSubstituteNext,
		pInitial)) != (char *) NULL)
	{
		if (lDifferentMemoryAfterSubstitution == 0)
		{
			strncpy (pDestinationToSubstituteNext, pFinal,
				(unsigned int) lFinalLength);
		}
		else
		{
			// strdup alloc the memory with the malloc function
			pStringToCopy	= strdup (
				pDestinationToSubstituteNext + lInitialLength);

			lTmpLength	= strlen (pStringToCopy);
			strncpy (pDestinationToSubstituteNext + lFinalLength, pStringToCopy,
				(unsigned int) lTmpLength);
			*(pDestinationToSubstituteNext + lFinalLength + lTmpLength) = '\0';
			strncpy (pDestinationToSubstituteNext, pFinal, 
				(unsigned int) lFinalLength);

			free (pStringToCopy);
		}

		// update the previous and the next pointer to the header
		pDestinationToSubstitutePrev		= pDestinationToSubstituteNext +
			strlen (pFinal);
		pDestinationToSubstituteNext		= pDestinationToSubstitutePrev;
	}

	_ullCharsNumber						+= lDifferentMemoryAfterSubstitution;


	return errNoError;
}


Error Buffer:: substitute (const char *pInitial, unsigned long ulFinal)

{

	char			pUnsignedLongBuffer [128 + 1];


	if (pInitial == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}


	if (sprintf (pUnsignedLongBuffer, "%lu", ulFinal) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (substitute (pInitial, pUnsignedLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SUBSTITUTE_FAILED);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error Buffer:: substitute (const char *pInitial,
		__int64 ullFinal)
#else
	Error Buffer:: substitute (const char *pInitial,
		unsigned long long ullFinal)
#endif

{

	char			pUnsignedLongLongBuffer [128 + 1];


	if (pInitial == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}


	if (sprintf (pUnsignedLongLongBuffer, "%llu", ullFinal) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (substitute (pInitial, pUnsignedLongLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SUBSTITUTE_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: removeCTRLCharacters (void)

{

	unsigned long				ulIndex;
	unsigned long				ulCopyIndex;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	for (ulIndex = 0; ulIndex < _ullCharsNumber; ulIndex++)
	{
		if (iscntrl (_pBuffer [ulIndex]))
		{
			for (ulCopyIndex = ulIndex; ulCopyIndex < _ullCharsNumber;
				ulCopyIndex++)
			{
				_pBuffer [ulCopyIndex]		= _pBuffer [ulCopyIndex + 1];
			}

			_ullCharsNumber--;
		}
		else if (_pBuffer [ulIndex] == 160 ||	// \xa0
			_pBuffer [ulIndex] == -96)	// may be because 256 - 96 = 160
		{
			// that should not be placed here because 160 (\xa0) is not
			// a CTRL char. Anyway, it was added here to manage this char
			// as well since, all the CTRL and this char too, cause an error
			// during the parsing of an XML by the 'xmlParseMemory'
			// function (xml2 library)
			//
			// About the 160 (\xa0) char I found on Internet:
			// "Infamous non-breaking Unicode space \xa0
			// Press CTRL+space / AltGr space on Linux to accidentally
			// create it.
			// You can't see it. But it breaks everything."
			//
			// To fix it, we will replace it with <space>

			_pBuffer [ulIndex]			= ' ';
		}
	}


	return errNoError;
}


Error Buffer:: removeChar (unsigned long ulIndex)

{

	unsigned long				ulCopyIndex;


	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (ulIndex >= length ())
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	for (ulCopyIndex = ulIndex; ulCopyIndex < _ullCharsNumber;
		ulCopyIndex++)
	{
		_pBuffer [ulCopyIndex]		= _pBuffer [ulCopyIndex + 1];
	}

	_ullCharsNumber--;


	return errNoError;
}


Error Buffer:: truncateStartingFrom (StripType_t stStripType,
	char cCharToSearch)

{

	const char					*pCharPosition;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (stStripType == STRIPTYPE_BOTH)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (stStripType == STRIPTYPE_LEADING)
	{
		if ((pCharPosition = strchr (_pBuffer, cCharToSearch)) ==
			(const char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_CHARNOTFOUND,
				2, cCharToSearch, _pBuffer);

			return err;
		}
	}
	else if (stStripType == STRIPTYPE_TRAILING)
	{
		if ((pCharPosition = strrchr (_pBuffer, cCharToSearch)) ==
			(const char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_CHARNOTFOUND,
				2, cCharToSearch, _pBuffer);

			return err;
		}
	}

	return setChar (pCharPosition - _pBuffer, '\0');
}


Error Buffer:: truncateIfBigger (unsigned long ulMaxLength)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_ullCharsNumber > ulMaxLength)
		return setChar (ulMaxLength, '\0');
	else
		return errNoError;
}


Error Buffer:: setChar (unsigned long ulIndex, char cNewChar)

{

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (ulIndex >= length ())
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	_pBuffer [ulIndex]				= cNewChar;

	if (cNewChar == '\0')
		_ullCharsNumber			= ulIndex;



	return errNoError;
}


Error Buffer:: getBufferLength (unsigned long *pulBufferLength) const

{

	if (pulBufferLength == (unsigned long *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_lBufferBlockNumber == 0)
		*pulBufferLength		= 0;
	else
		*pulBufferLength		= (unsigned long) _ullCharsNumber;


	return errNoError;
}


Error Buffer:: getChar (char *pcChar, unsigned long ulIndex) const

{

	unsigned long			ulBufferLength;


	if (pcChar == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (getBufferLength (&ulBufferLength) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_GETBUFFERLENGTH_FAILED);

		return err;
	}

	if (ulIndex >= ulBufferLength)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INDEXOUTOFRANGE);

		return err;
	}

	*pcChar				= _pBuffer [ulIndex];


	return errNoError;
}


Error Buffer:: getBuffer (char *pBuffer) const

{

	if (pBuffer == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_lBufferBlockNumber == 0)
		strcpy (pBuffer, "");
	else
		strcpy (pBuffer, _pBuffer);


	return errNoError;
}


Error Buffer:: setBuffer (const Buffer &bBuffer)

{

	if (bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		if (init ((const char *) bBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}
	}
	else
	{
		if (setBuffer ((const char *) bBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error Buffer:: setBuffer (const Buffer *pbBuffer)

{

	if (pbBuffer == (const Buffer *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (pbBuffer -> _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		if (init ((const char *) (*pbBuffer)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}
	}
	else
	{
		if (setBuffer ((const char *) (*pbBuffer)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);

			return err;
		}
	}


	return errNoError;
}


Error Buffer:: setBuffer (const char *pBuffer, long lBufferLength)

{

	if (pBuffer == (const char *) NULL ||
		(lBufferLength < 0 && lBufferLength != -1))
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		if (init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}
	}

	if (_lBufferBlockNumber != 0)
		strcpy (_pBuffer, "");

	if (lBufferLength == -1)
	{
		if (allocMemoryBlockForBufferIfNecessary (strlen (pBuffer)) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		strcpy (_pBuffer, pBuffer);

		_ullCharsNumber					= strlen (pBuffer);
	}
	else
	{
		if (allocMemoryBlockForBufferIfNecessary (lBufferLength) !=
			errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

			return err;
		}

		strncpy (_pBuffer, pBuffer, lBufferLength);
		_pBuffer [lBufferLength]						= '\0';

		_ullCharsNumber					= lBufferLength;
	}


	return errNoError;
}


Error Buffer:: setBuffer (long lValue)

{

	char			pLongBuffer [128 + 1];


	if (sprintf (pLongBuffer, "%ld", lValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (setBuffer (pLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: setBuffer (unsigned long ulValue)

{

	char			pLongBuffer [128 + 1];


	if (sprintf (pLongBuffer, "%lu", ulValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (setBuffer (pLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: setBuffer (int iValue)

{

	char			pIntegerBuffer [128 + 1];


	if (sprintf (pIntegerBuffer, "%d", iValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (setBuffer (pIntegerBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: setBuffer (float fValue, const char *pSprintfFormat)

{

	if (setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	if (append (fValue, pSprintfFormat) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: setBuffer (double dValue, const char *pSprintfFormat)

{

	if (setBuffer ("") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}

	if (append (dValue, pSprintfFormat) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error Buffer:: setBuffer (__int64 llValue)
#else
	Error Buffer:: setBuffer (long long llValue)
#endif

{

	char			pLongLongBuffer [128 + 1];


	if (sprintf (pLongLongBuffer, "%lld", llValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (setBuffer (pLongLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}


#ifndef WIN32
Error Buffer:: setBuffer (unsigned long long ullValue)
{

	char			pUnsignedLongLongBuffer [128 + 1];


	if (sprintf (pUnsignedLongLongBuffer, "%llu", ullValue) < 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SPRINTF_FAILED);

		return err;
	}

	if (setBuffer (pUnsignedLongLongBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);

		return err;
	}


	return errNoError;
}
#endif


Error Buffer:: readBufferFromFile (const char *pPathName,
	long lFileStartOffset, long lFileEndOffset)

{

	int								iFileDescriptor;
	long							lStartOffset;
	#ifdef WIN32
		__int64						ullEndOffset;
		__int64						llFileLength;
		__int64						llCurrentPosition;
		__int64						llByteRead;
	#else
		unsigned long long			ullEndOffset;
		long long					llFileLength;
		long long					llCurrentPosition;
		long long					llByteRead;
	#endif


	if (pPathName == (const char *) NULL ||
		lFileStartOffset < -1 ||
		lFileEndOffset < -1 ||
		lFileEndOffset < lFileStartOffset)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
		*/

		if (init ("") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);

			return err;
		}
	}

	if (FileIO:: open (pPathName, O_RDONLY, &iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED, 1, pPathName);

		return err;
	}

	if (FileIO:: seek (iFileDescriptor, (off_t) 0, SEEK_END,
		&llFileLength) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_SEEK_FAILED);

		FileIO:: close (iFileDescriptor);

		return err;
	}

	if (lFileStartOffset == -1)
		lStartOffset		= 0;
	else
		lStartOffset		= lFileStartOffset;

	if (lFileEndOffset == -1)
		ullEndOffset		= llFileLength;
	else
	{
		if (lFileEndOffset > llFileLength)
			ullEndOffset		= llFileLength;
		else
			ullEndOffset		= lFileEndOffset;
	}

	if (_lBufferBlockNumber != 0)
		strcpy (_pBuffer, "");

	if (allocMemoryBlockForBufferIfNecessary (ullEndOffset - lStartOffset) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED);

		FileIO:: close (iFileDescriptor);

		return err;
	}

	if (FileIO:: seek (iFileDescriptor, (off_t) lStartOffset, SEEK_SET,
		&llCurrentPosition) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_SEEK_FAILED);

		FileIO:: close (iFileDescriptor);

		return err;
	}

	if (FileIO:: readChars (iFileDescriptor, _pBuffer,
		ullEndOffset - lStartOffset,
		&llByteRead) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_READCHARS_FAILED);

		FileIO:: close (iFileDescriptor);

		return err;
	}

	if (llByteRead < ullEndOffset - lStartOffset)
	{
		_pBuffer [llByteRead]						= '\0';
		_ullCharsNumber								= llByteRead;
	}
	else
	{
		_pBuffer [ullEndOffset - lStartOffset]		= '\0';
		_ullCharsNumber								= ullEndOffset - lStartOffset;
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


#ifdef WIN32
	Error Buffer:: writeBufferOnFile (const char *pPathName,
		Boolean_t bAppend, Boolean_t bExecutionPermission,
		Boolean_t bIsBinaryFile)
#else
	Error Buffer:: writeBufferOnFile (const char *pPathName,
		Boolean_t bAppend, Boolean_t bExecutionPermission)
#endif

{

	int					iFileDescriptor;
	#ifdef WIN32
		__int64				llBytesWritten;
		int					mMode;
	#else
		long long			llBytesWritten;
		mode_t				mMode;
	#endif
	int					iFlags;


	if (pPathName == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	// iFlags
	{
		if (bAppend)
			iFlags			= O_WRONLY | O_APPEND | O_CREAT;
		else
			iFlags			= O_WRONLY | O_TRUNC | O_CREAT;

		#ifdef WIN32
			if (bIsBinaryFile)
				iFlags			|= O_BINARY;
		#endif
	}

	#ifdef WIN32
		if (bExecutionPermission)
			mMode			= _S_IREAD | _S_IWRITE;
		else
			mMode			= _S_IREAD | _S_IWRITE;
	#else
		if (bExecutionPermission)
			mMode			= S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH;
		else
			mMode			= S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	#endif

	if (FileIO:: open (pPathName, iFlags, mMode, &iFileDescriptor) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_OPEN_FAILED, 1, pPathName);

		return err;
	}

	// + 1 for the '\0'
	if (FileIO:: writeChars (iFileDescriptor,
		(char *) ((const char *) (*this)),
		((unsigned long) (*this)),
		&llBytesWritten) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_WRITECHARS_FAILED);

		FileIO:: close (iFileDescriptor);

		return err;
	}

	if (FileIO:: close (iFileDescriptor) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CLOSE_FAILED);

		return err;
	}


	return errNoError;
}


Error Buffer:: strip (StripType_t stStripType,
	const char *pStringToStrip)

{

	unsigned long	ulBufferLength;
	long			lBufferIndex;
	unsigned long	ulStringToStripLength;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	ulBufferLength			= (unsigned long) (*this);
	ulStringToStripLength	= strlen (pStringToStrip);

	if (stStripType == STRIPTYPE_LEADING || stStripType == STRIPTYPE_BOTH)
	{
		for (lBufferIndex = 0; lBufferIndex < ulBufferLength; )
		{
			if (ulBufferLength - lBufferIndex < ulStringToStripLength ||
				strncmp (&(_pBuffer [lBufferIndex]), pStringToStrip,
				ulStringToStripLength))
				break;

			lBufferIndex			+= ulStringToStripLength;
		}

		if (lBufferIndex != 0)
		{
			long			lFirstCharOfNewBufferIndex;


			lFirstCharOfNewBufferIndex		= lBufferIndex;

			for (; lBufferIndex < ulBufferLength; lBufferIndex++)
			{
				_pBuffer [lBufferIndex - lFirstCharOfNewBufferIndex]	=
					_pBuffer [lBufferIndex];
			}

			_pBuffer [lBufferIndex - lFirstCharOfNewBufferIndex]		= '\0';
			_ullCharsNumber		= lBufferIndex - lFirstCharOfNewBufferIndex;
		}
	}

	ulBufferLength			= (unsigned long) (*this);

	if (stStripType == STRIPTYPE_TRAILING || stStripType == STRIPTYPE_BOTH)
	{
		for (lBufferIndex = ulBufferLength - ulStringToStripLength;
			lBufferIndex >= 0; )
		{
			if (strncmp (&(_pBuffer [lBufferIndex]), pStringToStrip,
				ulStringToStripLength))
				break;

			lBufferIndex			-= ulStringToStripLength;
		}

		if (lBufferIndex != ulBufferLength - ulStringToStripLength)
		{
			_pBuffer [lBufferIndex + ulStringToStripLength]		= '\0';
			_ullCharsNumber			= lBufferIndex + ulStringToStripLength;
		}
	}


	return errNoError;
}


Error Buffer:: strip (StripType_t stStripType,
	unsigned long ulCharactersNumberToStrip)

{

	long			lBufferIndex;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return err;
	}

	if (
		(stStripType == STRIPTYPE_BOTH &&
		ulCharactersNumberToStrip * 2 > _ullCharsNumber) ||
		((stStripType == STRIPTYPE_LEADING ||
		stStripType == STRIPTYPE_TRAILING) &&
		ulCharactersNumberToStrip > _ullCharsNumber)
		)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (stStripType == STRIPTYPE_LEADING || stStripType == STRIPTYPE_BOTH)
	{
		for (lBufferIndex = ulCharactersNumberToStrip;
			lBufferIndex < _ullCharsNumber;
			lBufferIndex++)
		{
			_pBuffer [lBufferIndex - ulCharactersNumberToStrip]	=
				_pBuffer [lBufferIndex];
		}
		_ullCharsNumber		= _ullCharsNumber - ulCharactersNumberToStrip;
		_pBuffer [_ullCharsNumber]			= '\0';
	}

	if (stStripType == STRIPTYPE_TRAILING || stStripType == STRIPTYPE_BOTH)
	{
		_ullCharsNumber		= _ullCharsNumber - ulCharactersNumberToStrip;
		_pBuffer [_ullCharsNumber]			= '\0';
	}


	return errNoError;
}


const char *Buffer:: str (void) const

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return ((const char *) NULL);
		*/

		return "";
	}


	return _pBuffer;
}


Buffer:: operator const char * (void) const

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		/*
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return ((const char *) NULL);
		*/

		return "";
	}


	return _pBuffer;
}


Buffer:: operator unsigned long (void) const

{

	unsigned long			ulBufferLength;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return ((unsigned long) -1);
	}

	if (getBufferLength (&ulBufferLength) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_GETBUFFERLENGTH_FAILED);

		return ((unsigned long) -1);
	}


	return ulBufferLength;
}


unsigned long Buffer:: length (void) const

{
	return (unsigned long) (*this);
}


Buffer:: operator long (void) const

{

	unsigned long			ulBufferLength;


	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return ((unsigned long) -1);
	}

	if (getBufferLength (&ulBufferLength) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_GETBUFFERLENGTH_FAILED);

		return ((unsigned long) -1);
	}


	return (long) ulBufferLength;
}


Boolean_t Buffer:: operator < (const Buffer &bBuffer)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED ||
		bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return false;		// ??????
	}

	return (strcmp ((const char *) (*this), (const char *) bBuffer) < 0 ?
		true : false);
}


Boolean_t Buffer:: operator <= (const Buffer &bBuffer)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED ||
		bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return false;		// ??????
	}

	return (strcmp ((const char *) (*this), (const char *) bBuffer) <= 0 ?
		true : false);
}


Boolean_t Buffer:: operator > (const Buffer &bBuffer)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED ||
		bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return false;		// ??????
	}

	return (strcmp ((const char *) (*this), (const char *) bBuffer) > 0 ?
		true : false);
}


Boolean_t Buffer:: operator >= (const Buffer &bBuffer)

{

	if (_bsBufferStatus != TOOLS_BUFFER_INITIALIZED ||
		bBuffer. _bsBufferStatus != TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _bsBufferStatus);

		return false;		// ??????
	}

	return (strcmp ((const char *) (*this), (const char *) bBuffer) >= 0 ?
		true : false);
}


Buffer &Buffer:: operator += (const Buffer &bBuffer)

{

	if (append ((const char *) bBuffer, (long) bBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}


Buffer &Buffer:: operator += (const char *pBuffer)

{

	if (append (pBuffer) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}


Buffer &Buffer:: operator += (long lValue)

{

	if (append (lValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}


Buffer &Buffer:: operator += (float fValue)

{

	if (append (fValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}


Buffer &Buffer:: operator += (double dValue)

{

	if (append (dValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}


#ifdef WIN32
	Buffer &Buffer:: operator += (__int64 llValue)
#else
	Buffer &Buffer:: operator += (long long llValue)
#endif
{

	if (append (llValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}

#ifndef WIN32
Buffer &Buffer:: operator += (unsigned long long ullValue)

{

	if (append (ullValue) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);

		return *this;
	}


	return *this;
}
#endif


Buffer Buffer:: operator + (const Buffer &bBuffer)

{

	Buffer_t				bSumBuffer;


	if (_bsBufferStatus == TOOLS_BUFFER_BUILDED &&
		bBuffer. _bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		if (bSumBuffer. init () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}
	else if (_bsBufferStatus == TOOLS_BUFFER_INITIALIZED &&
		bBuffer. _bsBufferStatus == TOOLS_BUFFER_BUILDED)
	{
		if (bSumBuffer. init ((const char *) (*this)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}
	else if (_bsBufferStatus == TOOLS_BUFFER_BUILDED &&
		bBuffer. _bsBufferStatus == TOOLS_BUFFER_INITIALIZED)
	{
		if (bSumBuffer. init ((const char *) bBuffer) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
	}
	else
	{
		if (bSumBuffer. init ((const char *) (*this)) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_INIT_FAILED);
		}
		else
		{
			if (bSumBuffer. append ((const char *) (bBuffer)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_APPEND_FAILED);
			}
		}
	}


	return bSumBuffer;
}


Buffer Buffer:: operator + (const char *pBuffer)

{

	Buffer_t				bSumBuffer;


	if (pBuffer == (const char *) NULL)
		bSumBuffer				= *this;
	else
	{
		if (_bsBufferStatus == TOOLS_BUFFER_BUILDED)
		{
			if (bSumBuffer. init (pBuffer) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
			}
		}
		else
		{
			if (bSumBuffer. init ((const char *) (*this)) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_BUFFER_INIT_FAILED);
			}
			else
			{
				if (bSumBuffer. append (pBuffer) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_APPEND_FAILED);
				}
			}
		}
	}


	return bSumBuffer;
}


Boolean_t Buffer:: operator == (const Buffer &bBuffer) const

{

	Boolean_t				bIsEqualWith;


	if (isEqualWith (bBuffer, &bIsEqualWith) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_ISEQUALWITH_FAILED);

		return false;		// ????
	}


	return bIsEqualWith;
}


Boolean_t Buffer:: operator == (const char *pBuffer) const

{

	Boolean_t				bIsEqualWith;


	if (isEqualWith (pBuffer, &bIsEqualWith) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_ISEQUALWITH_FAILED);

		return false;	// ??????
	}


	return bIsEqualWith;
}


char Buffer:: operator [] (long lIndex) const

{

	char				cChar;


	if (getChar (&cChar, lIndex) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_GETCHAR_FAILED);

		return ' ';
	}

	return cChar;
}


std:: ostream &operator << (std:: ostream &osOutputStream, Buffer &bBuffer)

{

	if (bBuffer. _bsBufferStatus != Buffer:: TOOLS_BUFFER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, bBuffer. _bsBufferStatus);

		return osOutputStream;
	}

	if (bBuffer. _lBufferBlockNumber == 0)
		osOutputStream << "";
	else
		osOutputStream << bBuffer. _pBuffer;


	return osOutputStream;
}


#ifdef WIN32
	Error Buffer:: allocMemoryBlockForBufferIfNecessary (
		__int64 ullBufferLengthToInsert)
#else
	Error Buffer:: allocMemoryBlockForBufferIfNecessary (
		unsigned long long ullBufferLengthToInsert)
#endif

{

	long		lBufferLength;


	if (_lBufferBlockNumber != 0)
		lBufferLength		= strlen (_pBuffer);
	else
		lBufferLength		= 0;

	if (_lBufferBlockNumber == 0 ||
		lBufferLength + ullBufferLengthToInsert + 1 >=
		_lBufferBlockNumber * _lBufferBlockNumberOnOverflow)
	{
		char			*pLocalStringBuffer;
		long			lLocalBufferBlockNumber;


		lLocalBufferBlockNumber				= _lBufferBlockNumber;

		do
		{
			lLocalBufferBlockNumber		+= 1;
		}
		while (lBufferLength + ullBufferLengthToInsert >=
			lLocalBufferBlockNumber * _lBufferBlockNumberOnOverflow);

		if ((pLocalStringBuffer = new char [
			lLocalBufferBlockNumber * _lBufferBlockNumberOnOverflow]) ==
			(char *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_NEW_FAILED);

			return err;
		}

		if (_lBufferBlockNumber != 0)
		{
			strcpy (pLocalStringBuffer, _pBuffer);
			delete [] _pBuffer;
		}
		else
			strcpy (pLocalStringBuffer, "");

		_pBuffer				= pLocalStringBuffer;
		_lBufferBlockNumber		= lLocalBufferBlockNumber;

		_bMemoryAllocated		= true;
	}


	return errNoError;
}

