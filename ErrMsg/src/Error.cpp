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


#include "Error.h"
#ifdef __MOSYNC__
#else
	#include <iostream>
#endif


const Error_t	errNoError;
const char		*pNoErrMsgMessage		= "NoErrMsg";
const char		*pNoErrMsgClassName		= "NoErrMsg";
const long		lNoErrMsgIdentifier		= -1;
const char		*pErrorInErrorLibrary	= "Error in ErrMsg library";

const Message_t	msgNoMessage;


Error:: Error (void)

{

	_pebCustomError				= (ErrMsgBase_p) NULL;
	_pucNoErrorUserData			= (unsigned char *) NULL;
	_ulNoErrorUserDataBytes		= 0;

}


Error:: Error (const ErrMsgBase_t &ebCustomError)

{

	if (ebCustomError. duplicate (&_pebCustomError) != 0)
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "ErrMsgBase:: duplicate failed" << std:: endl;
		#endif

		_pebCustomError		= (ErrMsgBase_p) NULL;
	}

}


Error &Error:: operator =(const Error &err)

{

	// delete the current object
	if (_pebCustomError != (ErrMsgBase_p) NULL)
	{
		delete _pebCustomError;
		_pebCustomError		= (ErrMsgBase_p) NULL;
	}
	else
	{
		// if the object contains user data and
		//	the user data was allocated by 'new'
		if (_ulNoErrorUserDataBytes > 0 &&
			_ulNoErrorUserDataBytes > ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
		{
			delete [] _pucNoErrorUserData;
			_pucNoErrorUserData		= (unsigned char *) NULL;
			_ulNoErrorUserDataBytes	= 0;
		}
	}

	// copy the parameter
	if (err. _pebCustomError == (ErrMsgBase_p) NULL)
	{
		_pebCustomError			= (ErrMsgBase_p) NULL;
		if (err. _ulNoErrorUserDataBytes != 0 &&
			err. _pucNoErrorUserData != (unsigned char *) NULL)
			setUserData (err. _pucNoErrorUserData,
				err. _ulNoErrorUserDataBytes);
		else
		{
			_pucNoErrorUserData			= (unsigned char *) NULL;
			_ulNoErrorUserDataBytes		= 0;
		}
	}
	else
	{
		if ((err. _pebCustomError) -> duplicate (&_pebCustomError) != 0)
		{
			_pebCustomError		= (ErrMsgBase_p) NULL;
		}
	}


	return *this;
}


Error:: Error (const Error &err)

{

	_pebCustomError				= (ErrMsgBase_p) NULL;
	_pucNoErrorUserData			= (unsigned char *) NULL;
	_ulNoErrorUserDataBytes		= 0;

	*this		= err;

}


Error:: ~Error (void)

{

	if (_pebCustomError != (ErrMsgBase_p) NULL)
	{
		delete _pebCustomError;
		_pebCustomError		= (ErrMsgBase_p) NULL;
	}
	else
	{
		// if the object contains user data and
		//	the user data was allocated by 'new'
		if (_ulNoErrorUserDataBytes > 0 &&
			_ulNoErrorUserDataBytes > ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
		{
			delete [] _pucNoErrorUserData;
			_pucNoErrorUserData		= (unsigned char *) NULL;
			_ulNoErrorUserDataBytes	= 0;
		}
	}

}


Error:: operator const char *(void) const

{

	const char		*pReturnErrorMessage;


	if (_pebCustomError == (ErrMsgBase_p) NULL)
		pReturnErrorMessage		= pNoErrMsgMessage;
	else
	{
		if ((pReturnErrorMessage = ((const char *) (*_pebCustomError))) ==
			(const char *) NULL)
			pReturnErrorMessage		= pErrorInErrorLibrary;
	}

	return pReturnErrorMessage;
}


const char *Error:: str () const

{

	const char		*pReturnErrorMessage;


	if (_pebCustomError == (ErrMsgBase_p) NULL)
		pReturnErrorMessage		= pNoErrMsgMessage;
	else
	{
		if ((pReturnErrorMessage = ((const char *) (*_pebCustomError))) ==
			(const char *) NULL)
			pReturnErrorMessage		= pErrorInErrorLibrary;
	}

	return pReturnErrorMessage;
}


Error:: operator long (void) const

{

	long			lReturnErrorIdentifier;


	if (getIdentifier (&lReturnErrorIdentifier))
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "Error:: getIdentifier failed" << std:: endl;
		#endif

		return -1;
	}


	return lReturnErrorIdentifier;
}


Error:: operator unsigned long (void) const

{

	long			lReturnErrorIdentifier;


	if (getIdentifier (&lReturnErrorIdentifier))
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "Error:: getIdentifier failed" << std:: endl;
		#endif

		return (unsigned long) (-1);
	}


	return (unsigned long) lReturnErrorIdentifier;
}


Boolean_t Error:: operator ==(const Error &err)

{

	if (_pebCustomError == (ErrMsgBase_p) NULL &&
		err. _pebCustomError == (ErrMsgBase_p) NULL)
		return true;
	else if (_pebCustomError == (ErrMsgBase_p) NULL &&
		err. _pebCustomError != (ErrMsgBase_p) NULL)
		return false;
	else if (_pebCustomError != (ErrMsgBase_p) NULL &&
		err. _pebCustomError == (ErrMsgBase_p) NULL)
		return false;
	else
		return ((*_pebCustomError) == err. _pebCustomError);

}


Boolean_t Error:: operator !=(const Error &err)

{

	if (_pebCustomError == (ErrMsgBase_p) NULL &&
		err. _pebCustomError == (ErrMsgBase_p) NULL)
		return false;
	else if (_pebCustomError == (ErrMsgBase_p) NULL &&
		err. _pebCustomError != (ErrMsgBase_p) NULL)
		return true;
	else if (_pebCustomError != (ErrMsgBase_p) NULL &&
		err. _pebCustomError == (ErrMsgBase_p) NULL)
		return true;
	else
		return ((*_pebCustomError) != err. _pebCustomError);

}


long Error:: getClassName (char *pErrorClassName)

{

	if (_pebCustomError == (ErrMsgBase_p) NULL)
	{
		strcpy (pErrorClassName, pNoErrMsgClassName);

		return 0;
	}
	else
		return _pebCustomError -> getErrMsgClassName (pErrorClassName);

}


long Error:: getIdentifier (long *plErrorIdentifier) const

{

	if (_pebCustomError == (ErrMsgBase_p) NULL)
	{
		*plErrorIdentifier		= lNoErrMsgIdentifier;

		return 0;
	}
	else
		return _pebCustomError -> getErrMsgIdentifier (plErrorIdentifier);

}


long Error:: getErrorKey (char *pErrorClassName, long *plErrorIdentifier)

{

	if (getClassName (pErrorClassName))
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "Error:: getClassName failed" << std:: endl;
		#endif

		return 1;
	}

	if (getIdentifier (plErrorIdentifier))
	{
		#ifdef __MOSYNC__
		#else
			std:: cerr << "Error:: getIdentifier failed" << std:: endl;
		#endif

		return 1;
	}

	return 0;
}


long Error:: setUserData (void *pvUserData, unsigned long ulUserDataBytes)

{

	if (_pebCustomError == (ErrMsgBase_p) NULL)
	{
		unsigned char			*pucLocalNoErrorUserData;


		if (pvUserData == (void *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "activation wrong" << std:: endl;
			#endif

			return 1;
		}

		if (ulUserDataBytes <= ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
		{
			pucLocalNoErrorUserData		= _pucNoErrorPreAllocatedUserData;
		}
		else
		{
			if ((pucLocalNoErrorUserData = new unsigned char [
				ulUserDataBytes]) == (unsigned char *) NULL)
			{
				#ifdef __MOSYNC__
				#else
					std:: cerr << "new failed" << std:: endl;
				#endif

				return 1;
			}
		}

		// if the object contains user data and
		//	the user data was allocated by 'new'
		if (_ulNoErrorUserDataBytes > 0 &&
			_ulNoErrorUserDataBytes > ERRMSGBASE_PREALLOCATEDUSERDATABYTES)
		{
			delete [] _pucNoErrorUserData;
			_pucNoErrorUserData			= (unsigned char *) NULL;
			_ulNoErrorUserDataBytes		= 0;
		}

		_pucNoErrorUserData			= pucLocalNoErrorUserData;
		memcpy (_pucNoErrorUserData, pvUserData, ulUserDataBytes);
		_ulNoErrorUserDataBytes			= ulUserDataBytes;

		return 0;
	}
	else
		return _pebCustomError -> setUserData (pvUserData, ulUserDataBytes);

}


long Error:: getUserData (void *pvUserData, unsigned long *pulUserDataBytes)

{

	if (_pebCustomError == (ErrMsgBase_p) NULL)
	{
		if (pvUserData == (void *) NULL ||
			pulUserDataBytes == (unsigned long *) NULL)
		{
			#ifdef __MOSYNC__
			#else
				std:: cerr << "activation wrong" << std:: endl;
			#endif

			return 1;
		}

		*pulUserDataBytes	= _ulNoErrorUserDataBytes;

		if (_ulNoErrorUserDataBytes != 0)
			memcpy (pvUserData, _pucNoErrorUserData,
				_ulNoErrorUserDataBytes);

		return 0;
	}
	else
		return _pebCustomError -> getUserData (pvUserData, pulUserDataBytes);

}

