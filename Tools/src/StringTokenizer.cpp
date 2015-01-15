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


#include "StringTokenizer.h"


StringTokenizer:: StringTokenizer (void)

{

	_stsStringTokenizerStatus			= TOOLS_STRINGTOKENIZER_BUILDED;
}


StringTokenizer:: StringTokenizer (const StringTokenizer &stStringTokenizer)

{

	*this					= stStringTokenizer;

}


StringTokenizer:: ~StringTokenizer (void)

{

	if (_stsStringTokenizerStatus  == TOOLS_STRINGTOKENIZER_INITIALIZED)
	{
		if (finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}
	}
}


Error StringTokenizer:: init (const char *pBuffer, long lBufferLength,
	const char *pDefaultDelimits)

{

	unsigned long			ulBufferLength;


	if (pDefaultDelimits == (const char *) NULL ||
		(lBufferLength < 0 && lBufferLength != -1) ||
		pBuffer == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (_stsStringTokenizerStatus != TOOLS_STRINGTOKENIZER_BUILDED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _stsStringTokenizerStatus);

		return err;
	}

	if (lBufferLength == -1)
		ulBufferLength			= strlen (pBuffer);
	else
		ulBufferLength			= lBufferLength;

	if ((_pInitialBuffer = new char [ulBufferLength + 1]) == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}
	strncpy (_pInitialBuffer, pBuffer, ulBufferLength);
	_pInitialBuffer [ulBufferLength]		= '\0';

	if ((_pDefaultDelimits = new char [strlen (pDefaultDelimits) + 1]) ==
		(char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		delete [] _pInitialBuffer;

		return err;
	}
	strcpy (_pDefaultDelimits, pDefaultDelimits);

	if ((_pBufferCopy = new char [ulBufferLength + 1]) == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		delete [] _pDefaultDelimits;
		delete [] _pInitialBuffer;

		return err;
	}
	strcpy (_pBufferCopy, _pInitialBuffer);

	_bIsFirstNextToken				= true;

	_stsStringTokenizerStatus		= TOOLS_STRINGTOKENIZER_INITIALIZED;


	return errNoError;
}


Error StringTokenizer:: finish (void)

{

	if (_stsStringTokenizerStatus != TOOLS_STRINGTOKENIZER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _stsStringTokenizerStatus);

		return err;
	}

	delete [] _pBufferCopy;
	delete [] _pDefaultDelimits;
	delete [] _pInitialBuffer;

	_stsStringTokenizerStatus		= TOOLS_STRINGTOKENIZER_BUILDED;


	return errNoError;
}


Error StringTokenizer:: nextToken (const char **pToken,
	const char *pCurrentDelimits)

{

	if (_stsStringTokenizerStatus != TOOLS_STRINGTOKENIZER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _stsStringTokenizerStatus);

		return err;
	}

	if (_bIsFirstNextToken)
	{
		if (pCurrentDelimits == (const char *) NULL)
		{
			#if defined(_REENTRANT) && !defined(WIN32)
				*pToken			= strtok_r (_pBufferCopy, _pDefaultDelimits,
					&_pLastToken);
			#else
				*pToken			= strtok (_pBufferCopy, _pDefaultDelimits);
			#endif
		}
		else
		{
			#if defined(_REENTRANT) && !defined(WIN32)
				*pToken			= strtok_r (_pBufferCopy, pCurrentDelimits,
					&_pLastToken);
			#else
				*pToken			= strtok (_pBufferCopy, pCurrentDelimits);
			#endif
		}

		_bIsFirstNextToken				= false;
	}
	else
	{
		if (pCurrentDelimits == (const char *) NULL)
		{
			#if defined(_REENTRANT) && !defined(WIN32)
				*pToken			= strtok_r ((char *) NULL, _pDefaultDelimits,
					&_pLastToken);
			#else
				*pToken			= strtok ((char *) NULL, _pDefaultDelimits);
			#endif
		}
		else
		{
			#if defined(_REENTRANT) && !defined(WIN32)
				*pToken			= strtok_r ((char *) NULL, pCurrentDelimits,
					&_pLastToken);
			#else
				*pToken			= strtok ((char *) NULL, pCurrentDelimits);
			#endif
		}

		_bIsFirstNextToken				= false;
	}

	if (*pToken == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NOMORETOKEN);

		return err;
	}


	return errNoError;
}


Error StringTokenizer:: countTokens (unsigned long *pulCountTokens)

{

	char					*pLocalBuffer;
	char					*pToken;


	if (_stsStringTokenizerStatus != TOOLS_STRINGTOKENIZER_INITIALIZED)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_OPERATION_NOTALLOWED, 1, _stsStringTokenizerStatus);

		return err;
	}

	if ((pLocalBuffer = new char [strlen (_pInitialBuffer) + 1]) ==
		(char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		return err;
	}
	strcpy (pLocalBuffer, _pInitialBuffer);

	#if defined(_REENTRANT) && !defined(WIN32)
		pToken = strtok_r (pLocalBuffer, _pDefaultDelimits,
			&_pLastToken);
	#else
		pToken = strtok (pLocalBuffer, _pDefaultDelimits);
	#endif

	*pulCountTokens				= 0;

	while (pToken != (char *) NULL)
	{
		*pulCountTokens				+= 1;

		#if defined(_REENTRANT) && !defined(WIN32)
			pToken			= strtok_r ((char *) NULL, _pDefaultDelimits,
				&_pLastToken);
		#else
			pToken			= strtok ((char *) NULL, _pDefaultDelimits);
		#endif
	}

	delete [] pLocalBuffer;


	return errNoError;
}

