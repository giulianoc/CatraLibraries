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


#include "Encrypt.h"
#include <assert.h>
#include <algorithm>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/bio.h>


Encrypt:: Encrypt (void)

{

}


Encrypt:: ~Encrypt (void)

{

}


Encrypt:: Encrypt (const Encrypt &)

{

	assert (1==0);

	// to do
}


Encrypt &Encrypt:: operator = (const Encrypt &)

{

	assert (1==0);

	// to do

	return *this;
}


#ifdef WIN32

	#include <windows.h>
	#include <stdio.h>

	const char *pServiceProvider		= "Microsoft Base Cryptographic Provider v1.0";
	const char *pKeyContainer			= "Metallica";
	const char *pDataForHash			= "a24kj2q";


	long Encrypt:: getDecryptedBufferLength (const char *pCryptedBuffer)

	{
		long				lCryptedDataLength;


		if (pCryptedBuffer == (const char *) NULL)
			return -1;

		lCryptedDataLength		= strlen (pCryptedBuffer);

		if (lCryptedDataLength % 4 != 0)
			return -2;

		return (lCryptedDataLength / 4);
	}

	long Encrypt:: getCryptedBufferLength (const char *pBufferToEncrypt)

	{

		HCRYPTPROV			hCryptProv;
		HCRYPTHASH			hHash;
		HCRYPTKEY			hKey;
		DWORD				lBufferToEncryptLength = 0;
		DWORD				lCryptedBufferLength = 0;


		if (pBufferToEncrypt == (const char *) NULL)
			return -1;

		// Get handle to CSP, create container if doesn't already exist
		if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
			PROV_RSA_FULL, CRYPT_NEWKEYSET))
		{
			if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
				PROV_RSA_FULL, 0))
			{
				return -2;
			}
		}

		// Create a hash object.
		if (!CryptCreateHash (hCryptProv, CALG_MD5, 0, 0, &hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -3;
		}

		// Hash the password.
		if (!CryptHashData (hHash, (BYTE *) pDataForHash,
			strlen (pDataForHash), 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -4;
		}

		// Derive a session key from the hash object.
		if (!CryptDeriveKey(hCryptProv, CALG_RC4, hHash, 0, &hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -5;
		}

		lBufferToEncryptLength		= strlen (pBufferToEncrypt);
		lCryptedBufferLength		= lBufferToEncryptLength;

		if (!CryptEncrypt(hKey, 0, TRUE, CRYPT_OAEP, NULL,
			&lCryptedBufferLength, 0))
		{
			CryptDestroyKey(hKey);
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -6;
		}

		if (!CryptDestroyKey(hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -7;
		}

		if (!CryptDestroyHash(hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -8;
		}

		if (!CryptReleaseContext(hCryptProv, 0))
		{
			return -9;
		}

		if (lCryptedBufferLength < lBufferToEncryptLength)
			return (lBufferToEncryptLength * 4);
		else
			return (lCryptedBufferLength * 4);
	}

	long Encrypt:: encrypt (const char *pBufferToEncrypt, char *pCryptedBuffer,
		unsigned long ulCryptedBufferLength)

	{

		HCRYPTPROV			hCryptProv;
		HCRYPTHASH			hHash;
		HCRYPTKEY			hKey;
		DWORD				lCryptedDataLength;


		lCryptedDataLength		= strlen (pBufferToEncrypt);

		if (pBufferToEncrypt == (const char *) NULL ||
			pCryptedBuffer == (char *) NULL ||
			ulCryptedBufferLength < lCryptedDataLength)
			return -1;

		// Get handle to CSP, create container if doesn't already exist
		if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
			PROV_RSA_FULL, CRYPT_NEWKEYSET))
		{
			if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
				PROV_RSA_FULL, 0))
			{
				return -2;
			}
		}

		// Create a hash object.
		if (!CryptCreateHash (hCryptProv, CALG_MD5, 0, 0, &hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -3;
		}

		// Hash the password.
		if (!CryptHashData (hHash, (BYTE *) pDataForHash,
			strlen (pDataForHash), 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -4;
		}

		// Derive a session key from the hash object.
		if (!CryptDeriveKey(hCryptProv, CALG_RC4, hHash, 0, &hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -5;
		}

		{
			char			*pLocalCryptedBuffer;
			char			*pPointerToCryptedBuffer;
			DWORD			lCryptedIndex;


			if ((pLocalCryptedBuffer = new char [ulCryptedBufferLength]) == (char *) NULL)
			{
				CryptDestroyKey(hKey);
				CryptDestroyHash(hHash);
				CryptReleaseContext(hCryptProv, 0);

				return -6;
			}

			strcpy (pLocalCryptedBuffer, pBufferToEncrypt);

			if (!CryptEncrypt(hKey, 0, TRUE, CRYPT_OAEP, (BYTE *) pLocalCryptedBuffer,
				&lCryptedDataLength, ulCryptedBufferLength))
			{
				delete [] pLocalCryptedBuffer;
				CryptDestroyKey(hKey);
				CryptDestroyHash(hHash);
				CryptReleaseContext(hCryptProv, 0);

				return -7;
			}

			pPointerToCryptedBuffer			= pCryptedBuffer;
			for (lCryptedIndex = 0; lCryptedIndex < lCryptedDataLength; lCryptedIndex++)
			{
				sprintf (pPointerToCryptedBuffer, "%4ld",
					(long) (pLocalCryptedBuffer [lCryptedIndex]));
				pPointerToCryptedBuffer		+= 4;
			}

			*pPointerToCryptedBuffer		= '\0';

			delete [] pLocalCryptedBuffer;
		}

		if (!CryptDestroyKey(hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -8;
		}

		if (!CryptDestroyHash(hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -9;
		}

		if (!CryptReleaseContext(hCryptProv, 0))
		{
			return -10;
		}


		return 0;
	}

	long Encrypt:: decrypt (const char *pCryptedBuffer, char *pDecryptedBuffer,
		unsigned long uDecryptedBufferLength)

	{
		DWORD				lCryptedDataLength;
		HCRYPTPROV			hCryptProv;
		HCRYPTHASH			hHash;
		HCRYPTKEY			hKey;
		DWORD				lDecryptedDataLength;


		if (pCryptedBuffer == (const char *) NULL ||
			pDecryptedBuffer == (char *) NULL)
			return -1;

		if (!strcmp (pCryptedBuffer, ""))
		{
			if (uDecryptedBufferLength == 0)
				return -2;

			pDecryptedBuffer [0]		= '\0';


			return 0;
		}

		lCryptedDataLength			= strlen (pCryptedBuffer);
		lDecryptedDataLength		= lCryptedDataLength / 4;

		if (lCryptedDataLength % 4 != 0 || lDecryptedDataLength > uDecryptedBufferLength)
			return -3;

		// initialize pDecryptedBuffer
		{
			char			pCryptedChar [5];
			DWORD			lCryptedIndex;


			for (lCryptedIndex = 0; lCryptedIndex < lDecryptedDataLength; lCryptedIndex++)
			{
				strncpy (pCryptedChar, pCryptedBuffer + (lCryptedIndex * 4), 4);
				pCryptedChar [4]			= '\0';

				*(pDecryptedBuffer + lCryptedIndex)		= (char) atol (pCryptedChar);
			}

			*(pDecryptedBuffer + lCryptedIndex)		= '\0';
		}

		// Get handle to CSP, create container if doesn't already exist
		if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
			PROV_RSA_FULL, CRYPT_NEWKEYSET))
		{
			if (!CryptAcquireContext(&hCryptProv, pKeyContainer, pServiceProvider,
				PROV_RSA_FULL, 0))
			{
				return -4;
			}
		}

		// Create a hash object.
		if (!CryptCreateHash (hCryptProv, CALG_MD5, 0, 0, &hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -5;
		}

		// Hash the password.
		if (!CryptHashData (hHash, (BYTE *) pDataForHash,
			strlen (pDataForHash), 0))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -6;
		}

		// Derive a session key from the hash object.
		if (!CryptDeriveKey(hCryptProv, CALG_RC4, hHash, 0, &hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -7;
		}

		if (!CryptDecrypt(hKey, 0, TRUE, CRYPT_OAEP, (BYTE *) pDecryptedBuffer,
			&lDecryptedDataLength))
		{
			CryptDestroyKey(hKey);
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -8;
		}

		if (!CryptDestroyKey(hKey))
		{
			CryptDestroyHash(hHash);
			CryptReleaseContext(hCryptProv, 0);

			return -9;
		}

		if (!CryptDestroyHash(hHash))
		{
			CryptReleaseContext(hCryptProv, 0);

			return -10;
		}

		if (!CryptReleaseContext(hCryptProv, 0))
		{
			return -11;
		}


		return 0;
	}

#elif WIN32_OLD

	char sPassword[] = "lkajsdflkjsaldfj";

	#include <iostream>


	clsCryptography:: clsCryptography ()

	{
		// Start with a small bit of memory
		lBufLen = 1;
		sBuffer = new char[lBufLen+1];
		sBufferCopy = new char[lBufLen+1];
		// Clear out memory
		memset( sBuffer, 0, lBufLen+1 );
		memset( sBufferCopy, 0, lBufLen+1 );

		// Get handle to CSP, create container if doesn't already exist
		if (!CryptAcquireContext(&hCryptProv, KEY_CONTAINER, SERVICE_PROVIDER, PROV_RSA_FULL, CRYPT_NEWKEYSET))
		{
			if (!CryptAcquireContext(&hCryptProv, KEY_CONTAINER, SERVICE_PROVIDER, PROV_RSA_FULL, 0))
			{
				vHandleError ("Error during CryptAcquireContext for a new key container.\nA container with this name probably already exists.");
			}
		}
	}


	clsCryptography:: ~clsCryptography ()

	{
		// Free memory
		delete[] sBuffer;
		delete[] sBufferCopy;

		// Release provider handle.
		if (hCryptProv)
			CryptReleaseContext (hCryptProv, 0);
	}


	void clsCryptography:: vResizeBuffer (DWORD lNewLength)

	{
		// Free old memory
		delete[] sBuffer;
		// Remember new length
		lBufLen = lNewLength;
		// Allocate new memory
		sBuffer = new char[lNewLength+1];
		// Clear out memory
		memset( sBuffer, 0, lBufLen+1 );
	}


	bool clsCryptography:: bEncryptData (char *sData, char *sPassword)

	{
		long lEncryptionCount = 0;
		char *sTempPassword;
	    
		// It is possible that the normal encryption will give you a string
		// containing cr or lf characters which make it difficult to write to files
		// Do a loop changing the password and keep encrypting until the result is ok
		// To be able to decrypt we need to also store the number of loops in the result
	    
		// Allow space for password & 8 numbers
		sTempPassword = new char[strlen( sPassword ) + 9];
		sprintf( sTempPassword, "%s%ld", sPassword, lEncryptionCount );

		// Try first encryption
		if (!bEncryptDecrypt(sData, sTempPassword, true))
		{
			delete[] sTempPassword;
			return false;
		}
	    
		// Loop if this contained a bad character
		while ( ( strrchr( sBuffer,  9 ) != NULL ) ||
				( strrchr( sBuffer, 10 ) != NULL ) ||
				( strrchr( sBuffer, 13 ) != NULL ) )
		{         
			// Try the next password
			lEncryptionCount++;
			sprintf( sTempPassword, "%s%ld", sPassword, lEncryptionCount );
			if (!bEncryptDecrypt(sData, sTempPassword, true))
			{
				delete[] sTempPassword;
				return false;
			}
	        
			// Don't go on for ever, 1 billion attempts should be plenty
			if (lEncryptionCount == 99999999)
			{
				vHandleError ("This data cannot be successfully encrypted.");
				delete[] sTempPassword;
				return false;
			}
		}
	    
		// Build encrypted string, starting with number of encryption iterations
		vAddNumber(lEncryptionCount);
		
		delete[] sTempPassword;
		return true;
	}


	bool clsCryptography:: bDecryptData (char *sData, char *sPassword)

	{

		long lEncryptionCount = 0;
		char *sTempPassword;
		bool bRetVal = false;
	    
		// Allow space for password & 8 numbers
		sTempPassword = new char[strlen( sPassword ) + 9];

		// When encrypting we may have gone through a number of iterations
		// How many did we go through?
		lEncryptionCount = lDecryptNumber(sData);
	    
		// Use the right password
		sprintf( sTempPassword, "%s%ld", sPassword, lEncryptionCount );

		bRetVal = bEncryptDecrypt(sData+8, sTempPassword, false);
		delete[] sTempPassword;
		return bRetVal;
	}


	char *clsCryptography:: sGetBuffer ()

	{
		// We want to return a copy not the real buffer
		// This will stop the user messing it up
		// Free old memory used by copy
		delete[] sBufferCopy;
		// Allocate new memory
		sBufferCopy = new char[lBufLen+1];
		// Copy the real buffer
		strcpy( sBufferCopy, sBuffer );
		// Return the copy
		return sBufferCopy;
	}

	long clsCryptography:: lGetBufferLength()

	{
		return lBufLen;
	}


	// 99% of the encryption / decryption code is the same
	// so keep in this common function
	bool clsCryptography:: bEncryptDecrypt (char *sData, char *sPassword,
		bool bEncrypt)

	{
		HCRYPTKEY hKey = 0;
		HCRYPTHASH hHash = 0;
		DWORD lLength = 0;

		// This handle should have been found in the constructor
		if (!hCryptProv) 
		{
			vHandleError ("No handle to cryptography key container.");
 			return false;
		}

		// Create a hash object.
		if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash))
		{
			vHandleError ("Error during CryptCreateHash.");
			return false;
		}

		// Hash the password.
		if (!CryptHashData (hHash, (CONST BYTE *) sPassword,
			strlen(sPassword), 0))
		{
			vHandleError ("Error during CryptHashData.");
			return false;
		}

		// Derive a session key from the hash object.
		if (!CryptDeriveKey(hCryptProv, CALG_RC4, hHash, 0, &hKey))
		{
			vHandleError ("Error during CryptDeriveKey.");
			return false;
		}

		// Do the work

		// Start by resizing the buffer to match the data length
		// This should be fine in most cases, but if this is too small
		// it will get resized in a minute
		vResizeBuffer( strlen( sData ) );
		lLength = lBufLen;
		// Copy in the Data
		strcpy( sBuffer, sData );

		// This is where the only difference happens depending on whether
		// we are encrypting or decrypting
		if (bEncrypt)
		{
			// Encrypt data.
			if (!CryptEncrypt(hKey, 0, 1, 0, (BYTE *)sBuffer,
				&lLength, lBufLen))
			{
				// Check if buffer was too small
				if (lLength > lBufLen)
				{
					// Make buffer big enough
					vResizeBuffer(lLength);
					// Try again
					strcpy( sBuffer, sData );
					if (!CryptEncrypt(hKey, 0, 1, 0, (BYTE *)sBuffer,
						&lLength, lBufLen))
					{
						// Buffer was big enough but still failed
						vHandleError ("Error during CryptEncrypt.");
						return false;
					}
				}
				else
				{
					// Buffer was big enough but still failed
					vHandleError ("Error during CryptEncrypt.");
					return false;
				}
			}
		}
		else
		{
			// Decrypt data.
			if (!CryptDecrypt(hKey, 0, 1, 0, (BYTE *)sBuffer, &lLength))
			{
				// Check if buffer was too small
				if (lLength > lBufLen)
				{
					// Make buffer big enough
					vResizeBuffer(lLength);
					// Try again
					strcpy( sBuffer, sData );
					if (!CryptDecrypt(hKey, 0, 1, 0, (BYTE *)sBuffer, &lLength))
					{
						// Buffer was big enough but still failed
						vHandleError ("Error during CryptDecrypt.");
						return false;
					}
				}
				else
				{
					// Buffer was big enough but still failed
					vHandleError ("Error during CryptDecrypt.");
					return false;
				}
			}
		}

		// Destroy session key.
		if (hKey)
			CryptDestroyKey (hKey);

		// Destroy hash object.
		if (hHash)
			CryptDestroyHash (hHash);

		// If we made it this far then all was ok
		return true;
	}


	void clsCryptography:: vHandleError (char *sError)

	{
		// Log error if required
		std:: cerr << sError << std:: endl;
		return;
	}


	void clsCryptography:: vAddNumber (long lNumber)

	{
		char *sTemp;
		char sNumber[9];
		char sNumberEncryptString[] = NUMBER_ENCRYPT_STRING;

		sTemp = new char[lBufLen+1];

		// Copy the existing buffer
		strcpy( sTemp, sBuffer );

		// Resize the buffer to allow room for encrypted number
		vResizeBuffer(lBufLen+8);

		// Turn number into string
		sprintf( sNumber, "%08ld", lNumber );

		// Start encrypting number
		for (int i=0; i<8; i++)
		{
			sBuffer[i] = (sNumberEncryptString[i] + (sNumber[i] - '0'));
		}

		// Copy back original buffer after number
		strcpy( sBuffer+8, sTemp );

		delete[] sTemp;

		return;
	}

	long clsCryptography:: lDecryptNumber (char *sEncrypted)

	{
		long lResult = 0;
		char sNumberEncryptString[] = NUMBER_ENCRYPT_STRING;
	    
		for (int i=0; i<8; i++)
		{
			lResult = (10 * lResult) +
				(sEncrypted[i] - sNumberEncryptString[i]);
		}

		return lResult;
	}


	long Encrypt:: encrypt (const char *pBufferToEncrypt, char *pCryptedBuffer)

	{

		clsCryptography				MyCrypt;
		char						*pLocalCryptedBuffer;
		long						lLocalCryptedBufferLength;
		long						lIndex;
		char						*pPointerToCryptedBuffer;


		MyCrypt. bEncryptData ((char *) pBufferToEncrypt, sPassword);

		lLocalCryptedBufferLength		= MyCrypt. lGetBufferLength ();

		if ((pLocalCryptedBuffer = new char [lLocalCryptedBufferLength + 1]) ==
			(char *) NULL)
			return 1;

		strcpy (pLocalCryptedBuffer, MyCrypt. sGetBuffer ());

		pPointerToCryptedBuffer			= pCryptedBuffer;

		for (lIndex = 0; lIndex < lLocalCryptedBufferLength; lIndex++)
		{
			sprintf (pPointerToCryptedBuffer, "%4ld",
				(long) (pLocalCryptedBuffer [lIndex]));
			pPointerToCryptedBuffer		+= 4;
		}

		delete [] pLocalCryptedBuffer;


		return 0;
	}


	long Encrypt:: decrypt (const char *pCryptedBuffer, char *pDecryptedBuffer)

	{

		clsCryptography				MyCrypt;
		char						*pLocalCryptedBuffer;
		long						lCryptedBufferLength;
		long						lIndex;
		char						pCryptedChar [5];


		lCryptedBufferLength			= strlen (pCryptedBuffer);

		if (lCryptedBufferLength % 4 != 0)
			return 1;

		if ((pLocalCryptedBuffer = new char [(lCryptedBufferLength / 4) + 1]) ==
			(char *) NULL)
			return 1;

		for (lIndex = 0; lIndex < lCryptedBufferLength / 4; lIndex++)
		{
			strncpy (pCryptedChar, pCryptedBuffer + (lIndex * 4), 4);
			pCryptedChar [4]			= '\0';

			*(pLocalCryptedBuffer + lIndex)		= (char) atol (pCryptedChar);
		}
		*(pLocalCryptedBuffer + lIndex)		= '\0';

		MyCrypt. bDecryptData (pLocalCryptedBuffer, sPassword);

		strcpy (pDecryptedBuffer, MyCrypt. sGetBuffer());

		delete [] pLocalCryptedBuffer;


		return 0;
	}


	long Encrypt:: getDecryptedBufferLength (const char *pCryptedBuffer)

	{

		clsCryptography				MyCrypt;
		char						*pLocalCryptedBuffer;
		long						lCryptedBufferLength;
		long						lIndex;
		char						pCryptedChar [5];


		lCryptedBufferLength			= strlen (pCryptedBuffer);

		if (lCryptedBufferLength % 4 != 0)
			return 1;

		if ((pLocalCryptedBuffer = new char [(lCryptedBufferLength / 4) + 1]) ==
			(char *) NULL)
			return 1;

		for (lIndex = 0; lIndex < lCryptedBufferLength / 4; lIndex++)
		{
			strncpy (pCryptedChar, pCryptedBuffer + (lIndex * 4), 4);
			pCryptedChar [4]			= '\0';

			*(pLocalCryptedBuffer + lIndex)		= (char) atol (pCryptedChar);
		}
		*(pLocalCryptedBuffer + lIndex)		= '\0';

		MyCrypt. bDecryptData (pLocalCryptedBuffer, sPassword);

		delete [] pLocalCryptedBuffer;


		return MyCrypt. lGetBufferLength ();
	}


	long Encrypt:: getCryptedBufferLength (const char *pBufferToEncrypt)

	{

		clsCryptography MyCrypt;


		MyCrypt.bEncryptData ((char *) pBufferToEncrypt, sPassword);


		return (MyCrypt. lGetBufferLength () * 4);
	}
#elif __QTCOMPILER__
	#include <QByteArray>
	#include <QtDebug>
	#include <QtGlobal>
	#include <QDateTime>
	#include <QCryptographicHash>
    #include <QDataStream>
 
	SimpleCrypt::SimpleCrypt():
    	m_key(0),
    	m_compressionMode(CompressionAuto),
    	m_protectionMode(ProtectionChecksum),
    	m_lastError(ErrorNoError)
	{
    	qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
	}
 
	SimpleCrypt::SimpleCrypt(quint64 key):
    	m_key(key),
    	m_compressionMode(CompressionAuto),
    	m_protectionMode(ProtectionChecksum),
    	m_lastError(ErrorNoError)
	{
    	qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
    	splitKey();
	}
 
	void SimpleCrypt::setKey(quint64 key)
	{
    	m_key = key;
    	splitKey();
	}
 
	void SimpleCrypt::splitKey()
	{
    	m_keyParts.clear();
    	m_keyParts.resize(8);
    	for (int i=0;i<8;i++) {
        	quint64 part = m_key;
        	for (int j=i; j>0; j--)
            	part = part >> 8;
        	part = part & 0xff;
        	m_keyParts[i] = static_cast<char>(part);
    	}
	}
 
	QByteArray SimpleCrypt::encryptToByteArray(const QString& plaintext)
	{
    	QByteArray plaintextArray = plaintext.toUtf8();
    	return encryptToByteArray(plaintextArray);
	}
 
	QByteArray SimpleCrypt::encryptToByteArray(QByteArray plaintext)
	{
    	if (m_keyParts.isEmpty()) {
        	qWarning() << "No key set.";
        	m_lastError = ErrorNoKeySet;
        	return QByteArray();
    	}
 
 
    	QByteArray ba = plaintext;
 
    	CryptoFlags flags = CryptoFlagNone;
    	if (m_compressionMode == CompressionAlways) {
        	ba = qCompress(ba, 9); //maximum compression
        	flags |= CryptoFlagCompression;
    	} else if (m_compressionMode == CompressionAuto) {
        	QByteArray compressed = qCompress(ba, 9);
        	if (compressed.count() < ba.count()) {
            	ba = compressed;
            	flags |= CryptoFlagCompression;
        	}
    	}
 
    	QByteArray integrityProtection;
    	if (m_protectionMode == ProtectionChecksum) {
        	flags |= CryptoFlagChecksum;
        	QDataStream s(&integrityProtection, QIODevice::WriteOnly);
        	s << qChecksum(ba.constData(), ba.length());
    	} else if (m_protectionMode == ProtectionHash) {
        	flags |= CryptoFlagHash;
        	QCryptographicHash hash(QCryptographicHash::Sha1);
        	hash.addData(ba);
 
        	integrityProtection += hash.result();
    	}
 
		//prepend a random char to the string
    	char randomChar = char(qrand() & 0xFF);
    	ba = randomChar + integrityProtection + ba;
 
    	int pos(0);
    	char lastChar(0);
 
    	int cnt = ba.count();
 
    	while (pos < cnt) {
        	ba[pos] = ba.at(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
        	lastChar = ba.at(pos);
        	++pos;
    	}
 
    	QByteArray resultArray;
    	resultArray.append(char(0x03));  //version for future updates to algorithm
    	resultArray.append(char(flags)); //encryption flags
    	resultArray.append(ba);
 
    	m_lastError = ErrorNoError;
    	return resultArray;
	}
 
	QString SimpleCrypt::encryptToString(const QString& plaintext)
	{
    	QByteArray plaintextArray = plaintext.toUtf8();
    	QByteArray cypher = encryptToByteArray(plaintextArray);
        QString cypherString = QString::fromLatin1((const char *) cypher.toBase64());
    	return cypherString;
	}
 
	QString SimpleCrypt::encryptToString(QByteArray plaintext)
	{
    	QByteArray cypher = encryptToByteArray(plaintext);
        QString cypherString = QString::fromLatin1(cypher.toBase64());
    	return cypherString;
	}
 
	QString SimpleCrypt::decryptToString(const QString &cyphertext)
	{
        QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    	QByteArray plaintextArray = decryptToByteArray(cyphertextArray);
    	QString plaintext = QString::fromUtf8(plaintextArray, plaintextArray.size());
 
    	return plaintext;
	}
 
	QString SimpleCrypt::decryptToString(QByteArray cypher)
	{
    	QByteArray ba = decryptToByteArray(cypher);
    	QString plaintext = QString::fromUtf8(ba, ba.size());
 
    	return plaintext;
	}
 
	QByteArray SimpleCrypt::decryptToByteArray(const QString& cyphertext)
	{
        QByteArray cyphertextArray = QByteArray::fromBase64(cyphertext.toLatin1());
    	QByteArray ba = decryptToByteArray(cyphertextArray);
 
    	return ba;
	}
 
	QByteArray SimpleCrypt::decryptToByteArray(QByteArray cypher)
	{
    	if (m_keyParts.isEmpty()) {
        	qWarning() << "No key set.";
        	m_lastError = ErrorNoKeySet;
        	return QByteArray();
    	}
 
    	QByteArray ba = cypher;
 
    	char version = ba.at(0);
 
    	if (version !=3) {  //we only work with version 3
        	m_lastError = ErrorUnknownVersion;
        	qWarning() << "Invalid version or not a cyphertext.";
        	return QByteArray();
    	}
 
    	CryptoFlags flags = CryptoFlags(ba.at(1));
 
    	ba = ba.mid(2);
    	int pos(0);
    	int cnt(ba.count());
    	char lastChar = 0;
 
    	while (pos < cnt) {
        	char currentChar = ba[pos];
        	ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
        	lastChar = currentChar;
        	++pos;
    	}
 
    	ba = ba.mid(1); //chop off the random number at the start
 
    	bool integrityOk(true);
    	if (flags.testFlag(CryptoFlagChecksum)) {
        	if (ba.length() < 2) {
            	m_lastError = ErrorIntegrityFailed;
            	return QByteArray();
        	}
        	quint16 storedChecksum;
        	{
            	QDataStream s(&ba, QIODevice::ReadOnly);
            	s >> storedChecksum;
        	}
        	ba = ba.mid(2);
        	quint16 checksum = qChecksum(ba.constData(), ba.length());
        	integrityOk = (checksum == storedChecksum);
    	} else if (flags.testFlag(CryptoFlagHash)) {
        		if (ba.length() < 20) {
            	m_lastError = ErrorIntegrityFailed;
            	return QByteArray();
        	}
        	QByteArray storedHash = ba.left(20);
        	ba = ba.mid(20);
        	QCryptographicHash hash(QCryptographicHash::Sha1);
        	hash.addData(ba);
        	integrityOk = (hash.result() == storedHash);
    	}
 
    	if (!integrityOk) {
        	m_lastError = ErrorIntegrityFailed;
        	return QByteArray();
    	}
 
    	if (flags.testFlag(CryptoFlagCompression))
        	ba = qUncompress(ba);
 
    	m_lastError = ErrorNoError;
    	return ba;
	}


	long Encrypt:: getDecryptedBufferLength (const char *pCryptedBuffer)

	{
        QString     sCryptedBuffer;


		SimpleCrypt crypto(Q_UINT64_C(0x0ca2d4aa4c9bf023)); //some random number 

        sCryptedBuffer          = pCryptedBuffer;
        sCryptedBuffer.replace("%61", "=");

		QString sDecrypt = crypto.decryptToString(sCryptedBuffer);


		return (long) sDecrypt.length();
	}


	long Encrypt:: getCryptedBufferLength (const char *pBufferToEncrypt)

	{
		SimpleCrypt crypto(Q_UINT64_C(0x0ca2d4aa4c9bf023)); //some random number 

		QString sCrypt = crypto.encryptToString(QString(pBufferToEncrypt));

        sCrypt.replace("=", "%61");


		return (long) sCrypt.length();
	}


	long Encrypt:: encrypt (const char *pBufferToEncrypt, char *pCryptedBuffer,
		unsigned long ulCryptedBufferLength)

	{
		SimpleCrypt crypto(Q_UINT64_C(0x0ca2d4aa4c9bf023)); //some random number 

		QString sCrypt = crypto.encryptToString(QString(pBufferToEncrypt));


        // '=' is not accepted as item value in the ConfigurationFile library, so it is just replaced
        sCrypt.replace("=", "%61");

		if (sCrypt.length() >= ulCryptedBufferLength)
		{
			return -1;
		}

		strcpy (pCryptedBuffer, (const char *) (sCrypt.toLatin1()));


		return 0;
	}


	long Encrypt:: decrypt (const char *pCryptedBuffer, char *pDecryptedBuffer,
		unsigned long uDecryptedBufferLength)

	{
        QString         sCryptedBuffer;


		SimpleCrypt crypto(Q_UINT64_C(0x0ca2d4aa4c9bf023)); //some random number 

        sCryptedBuffer          = pCryptedBuffer;
        sCryptedBuffer.replace("%61", "=");

        QString sDecrypt = crypto.decryptToString(sCryptedBuffer);

		if (sDecrypt.length() >= uDecryptedBufferLength)
		{
			return -1;
		}

		strcpy (pDecryptedBuffer, (const char *) (sDecrypt.toLatin1()));


		return 0;
	}

#else

	#include <iostream>
	#include <string.h>
	#include <stdexcept>

	#include <openssl/conf.h>
	#include <openssl/evp.h>
	#include <openssl/err.h>



	long Encrypt:: getDecryptedBufferLength (const char *pCryptedBuffer)
	{
		return strlen(pCryptedBuffer) / 2;
	}

	long Encrypt:: getCryptedBufferLength (const char *pBufferToEncrypt)
	{
		return strlen(pBufferToEncrypt) * 2;
	}

	long Encrypt:: encrypt (const char *pBufferToEncrypt, char *pCryptedBuffer,
		unsigned long ulCryptedBufferLength)
	{
		unsigned long ulBufferToEncryptLength = strlen(pBufferToEncrypt);

		if (ulCryptedBufferLength <= Encrypt::getCryptedBufferLength(pBufferToEncrypt))
			return 1;

		int indexForToEncrypt;
		int indexForCrypted;
		for(indexForToEncrypt = 0, indexForCrypted = 0;
				indexForToEncrypt < ulBufferToEncryptLength;
				indexForToEncrypt++, indexForCrypted += 2)
		{
			if (       (int) (pBufferToEncrypt[indexForToEncrypt] + 3) >= 32
					&& (int) (pBufferToEncrypt[indexForToEncrypt] + 3) <= 126
					// chars creating problems in URL...
					&& (char) (pBufferToEncrypt[indexForToEncrypt] + 3) != '<'
					&& (char) (pBufferToEncrypt[indexForToEncrypt] + 3) != '>'
					&& (char) (pBufferToEncrypt[indexForToEncrypt] + 3) != '='
					// chars creating problems for the token...
					&& (char) (pBufferToEncrypt[indexForToEncrypt] + 3) != ';'
				)
			{
				// printable char
        		pCryptedBuffer[indexForCrypted] = '1';
        		pCryptedBuffer[indexForCrypted + 1] = pBufferToEncrypt[indexForToEncrypt] + 3; // the key for encryption is 3 that is added to ASCII value
			}
			else
			{
				// no printable char
        		pCryptedBuffer[indexForCrypted] = '0';
        		pCryptedBuffer[indexForCrypted + 1] = pBufferToEncrypt[indexForToEncrypt];
			}
		}

		pCryptedBuffer[indexForCrypted] = '\0';


		return 0;
	}

	long Encrypt:: decrypt (const char *pCryptedBuffer, char *pDecryptedBuffer,
		unsigned long uDecryptedBufferLength)
	{
		unsigned long uCryptedBufferLength = strlen(pCryptedBuffer);

		if (uDecryptedBufferLength <= Encrypt::getDecryptedBufferLength(pCryptedBuffer))
			return 1;

		int indexForCrypted;
		int indexForDecrypted;
		for(indexForCrypted = 0, indexForDecrypted = 0;
			indexForCrypted < uCryptedBufferLength;
			indexForCrypted += 2, indexForDecrypted++)
		{
			if (pCryptedBuffer[indexForCrypted] == '1')
				pDecryptedBuffer[indexForDecrypted] = pCryptedBuffer[indexForCrypted + 1] - 3; // the key for encryption is 3 that is subtracted to ASCII value
			else
				pDecryptedBuffer[indexForDecrypted] = pCryptedBuffer[indexForCrypted + 1];

			// std::cout << "pDecryptedBuffer[indexForCrypted]: " + pDecryptedBuffer[indexForCrypted] << std::endl;
			// std::cout << ": " + pDecryptedBuffer[indexForCrypted] << std::endl;
		}
		pDecryptedBuffer[indexForDecrypted] = '\0';

		return 0;
	}


    string Encrypt::encrypt(string stringToBeEncrypted)
    {

        long lBufferCryptedLength = Encrypt::getCryptedBufferLength (stringToBeEncrypted.c_str());

        char *pBufferCrypted;
        if ((pBufferCrypted = new char [lBufferCryptedLength + 1]) == nullptr)
        {
            throw runtime_error(string("new failed"));
        }

        strcpy (pBufferCrypted, "");
        if (Encrypt::encrypt (stringToBeEncrypted.c_str(), pBufferCrypted,
            lBufferCryptedLength + 1) != 0)
        {
            delete [] pBufferCrypted;

            throw runtime_error(string("Encrypt::encrypt failed"));
        }

        string cryptedBuffer (pBufferCrypted);

        delete [] pBufferCrypted;


        return cryptedBuffer;
    }

    string Encrypt::decrypt(string stringToBeDecrypted)
    {

        char			*pDecryptedBuffer;
        long			lDecryptedBufferLength;


        lDecryptedBufferLength = Encrypt::getDecryptedBufferLength(stringToBeDecrypted.c_str());

        if ((pDecryptedBuffer = new char [lDecryptedBufferLength + 1]) == nullptr)
        {
            throw runtime_error(string("new failed"));
        }

        strcpy (pDecryptedBuffer, "");
        if (Encrypt:: decrypt (stringToBeDecrypted.c_str(), pDecryptedBuffer,
                lDecryptedBufferLength + 1) != 0)
        {
            delete [] pDecryptedBuffer;

            throw runtime_error(string("Encrypt::encrypt failed"));
        }

        string decryptedBuffer(pDecryptedBuffer);

        delete [] pDecryptedBuffer;

        return decryptedBuffer;
    }

/*
int main (void)
{
	// Set up the key and iv. Do I need to say to not hard code these in a
	// real application? :-)

    // A 256 bit key
    unsigned char *key = (unsigned char *)"01234567890123456789012345678901";

    // A 128 bit IV
    unsigned char *iv = (unsigned char *)"0123456789012345";

    // Message to be encrypted
    unsigned char *plaintext =
        (unsigned char *)"The quick brown fox jumps over the lazy dog";

    // Buffer for ciphertext. Ensure the buffer is long enough for the
    // ciphertext which may be longer than the plaintext, depending on the
    // algorithm and mode.
    unsigned char ciphertext[128];

    // Buffer for the decrypted text
    unsigned char decryptedtext[128];

    int decryptedtext_len, ciphertext_len;

    // Encrypt the plaintext
    ciphertext_len = encrypt (plaintext, strlen ((char *)plaintext), key, iv,
                              ciphertext);

    // Do something useful with the ciphertext here
    printf("Ciphertext is:\n");
    BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

    // Decrypt the ciphertext
    decryptedtext_len = decrypt(ciphertext, ciphertext_len, key, iv,
                                decryptedtext);

    // Add a NULL terminator. We are expecting printable text
    decryptedtext[decryptedtext_len] = '\0';

	// Show the decrypted text
    printf("Decrypted text is:\n");
    printf("%s\n", decryptedtext);


    return 0;
}
*/


	string Encrypt::opensslEncrypt(string plaintext)
	{
		// A 256 bit key
		// unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
		unsigned char *key =   (unsigned char *) "r12Dd5678h012_45.7890*534D6l/9R1";

		// A 128 bit IV
		// unsigned char *iv = (unsigned char *)"0123456789012345";
		unsigned char *iv =   (unsigned char *) "0/2xd5678GA12*_5";

		return Encrypt::opensslEncrypt(key, iv, plaintext);
	}

	string Encrypt::opensslEncrypt(
		unsigned char *key, unsigned char *iv,
		string plaintext
	)
	{
		// Buffer for ciphertext. Ensure the buffer is long enough for the
		// ciphertext which may be longer than the plaintext, depending on the
		// algorithm and mode.
		unsigned char ciphertext[10240];
		int ciphertext_len;
		{
			EVP_CIPHER_CTX *ctx;

			int len;

			int plaintext_len = plaintext.size();
			unsigned char *ucPlaintext = (unsigned char*) malloc(plaintext_len);
			memcpy(ucPlaintext, plaintext.c_str(), plaintext_len);

			/* Create and initialise the context */
			if(!(ctx = EVP_CIPHER_CTX_new()))
			{
				free(ucPlaintext);

				throw runtime_error(string("EVP_CIPHER_CTX_new failed"));
			}

			/*
			* Initialise the encryption operation. IMPORTANT - ensure you use a key
			* and IV size appropriate for your cipher
			* In this example we are using 256 bit AES (i.e. a 256 bit key). The
			* IV size for *most* modes is the same as the block size. For AES this
			* is 128 bits
			*/
			if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
			{
				free(ucPlaintext);
				EVP_CIPHER_CTX_free(ctx);

				throw runtime_error(string("EVP_EncryptInit_ex failed"));
			}

			/*
			* Provide the message to be encrypted, and obtain the encrypted output.
			* EVP_EncryptUpdate can be called multiple times if necessary
			*/
			if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, ucPlaintext, plaintext_len))
			{
				free(ucPlaintext);
				EVP_CIPHER_CTX_free(ctx);

				throw runtime_error(string("EVP_EncryptUpdate failed"));
			}
			ciphertext_len = len;

			/*
			* Finalise the encryption. Further ciphertext bytes may be written at
			* this stage.
			*/
			if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
			{
				free(ucPlaintext);
				EVP_CIPHER_CTX_free(ctx);

				throw runtime_error(string("EVP_EncryptFinal_ex failed"));
			}
			ciphertext_len += len;

			free(ucPlaintext);

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);
		}

		// std::cout << "ciphertext_len: " << ciphertext_len << std::endl;
		// for(int index =  0; index < ciphertext_len; index++)
		// 	std::cout << "'" << ciphertext[index] << "'" << (int) (ciphertext[index]) << std::endl;
		string base64Encoded = convertFromBinaryToBase64(ciphertext, ciphertext_len);
		// std::cout << "base64Encoded.size: " << base64Encoded.size() << std::endl;
		// for(int index =  0; index < base64Encoded.size(); index++)
		// 	std::cout << "'" << base64Encoded[index] << "'" << (int) (base64Encoded[index]) << std::endl;

		// base64, see https://it.wikipedia.org/wiki/Base64 contains two characters
		// that could create issues: + and /
		// Also the '=' character is used as padding
		// For this reasons we wil just replace them
		::replace(base64Encoded.begin(), base64Encoded.end(), '+', '-');
		::replace(base64Encoded.begin(), base64Encoded.end(), '/', '~');
		::replace(base64Encoded.begin(), base64Encoded.end(), '=', '_');


		return base64Encoded;
	}

	string Encrypt::opensslDecrypt(string base64Encoded)
	{
		// A 256 bit key
		// unsigned char *key = (unsigned char *)"01234567890123456789012345678901";
		unsigned char *key =   (unsigned char *) "r12Dd5678h012_45.7890*534D6l/9R1";

		// A 128 bit IV
		// unsigned char *iv = (unsigned char *)"0123456789012345";
		unsigned char *iv =   (unsigned char *) "0/2xd5678GA12*_5";

		return Encrypt::opensslDecrypt(key, iv, base64Encoded);
	}

	string Encrypt::opensslDecrypt(
		unsigned char *key, unsigned char *iv,
		string base64Encoded
	)
	{
		// std::cout << "base64Encoded.size: " << base64Encoded.size() << std::endl;

		string localBase64Encoded = base64Encoded;
		::replace(localBase64Encoded.begin(), localBase64Encoded.end(), '-', '+');
		::replace(localBase64Encoded.begin(), localBase64Encoded.end(), '~', '/');
		::replace(localBase64Encoded.begin(), localBase64Encoded.end(), '_', '=');

		unsigned char* ciphertext;
		size_t ciphertext_len;
		if (convertFromBase64ToBinary(localBase64Encoded.c_str(),
			&ciphertext, &ciphertext_len) != 0)
		{
			throw runtime_error(string("base64Decode failed"));
		}
		// std::cout << "ciphertext_len: " << ciphertext_len << std::endl;

		unsigned char ucPlaintext[10240];
		int plaintext_len;
		{
			EVP_CIPHER_CTX *ctx;

			int len;

			/* Create and initialise the context */
			if(!(ctx = EVP_CIPHER_CTX_new()))
			{
				free(ciphertext);

				throw runtime_error(string("EVP_CIPHER_CTX_new failed"));
			}

			/*
			* Initialise the decryption operation. IMPORTANT - ensure you use a key
			* and IV size appropriate for your cipher
			* In this example we are using 256 bit AES (i.e. a 256 bit key). The
			* IV size for *most* modes is the same as the block size. For AES this
			* is 128 bits
			*/
			if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
			{
				EVP_CIPHER_CTX_free(ctx);
				free(ciphertext);

				throw runtime_error(string("EVP_DecryptInit_ex failed"));
			}

			/*
			* Provide the message to be decrypted, and obtain the plaintext output.
			* EVP_DecryptUpdate can be called multiple times if necessary.
			*/
			if(1 != EVP_DecryptUpdate(ctx, ucPlaintext, &len, ciphertext, ciphertext_len))
			{
				EVP_CIPHER_CTX_free(ctx);
				free(ciphertext);

				throw runtime_error(string("EVP_DecryptUpdate failed"));
			}
			plaintext_len = len;

			/*
			* Finalise the decryption. Further plaintext bytes may be written at
			* this stage.
			*/
			if(1 != EVP_DecryptFinal_ex(ctx, ucPlaintext + len, &len))
			{
				EVP_CIPHER_CTX_free(ctx);
				free(ciphertext);

				throw runtime_error(string("EVP_DecryptFinal_ex failed"));
			}
			plaintext_len += len;

			/* Clean up */
			EVP_CIPHER_CTX_free(ctx);
		}

		free(ciphertext);

		string plainText((const char *) ucPlaintext, plaintext_len);

		return plainText;
	}

	string Encrypt::convertFromBinaryToBase64(const unsigned char* buffer, size_t length)
	{
		// Encodes a binary safe base 64 string

		BIO *bio, *b64;
		BUF_MEM *bufferPtr;

		// _logger->info(__FILEREF__ + "BIO_new...");
		b64 = BIO_new(BIO_f_base64());
		bio = BIO_new(BIO_s_mem());

		// _logger->info(__FILEREF__ + "BIO_push...");
		bio = BIO_push(b64, bio);

		// _logger->info(__FILEREF__ + "BIO_write...");
		BIO_write(bio, buffer, length);
		BIO_flush(bio);
		BIO_get_mem_ptr(bio, &bufferPtr);

		// _logger->info(__FILEREF__ + "BIO_set_close...");
		BIO_set_close(bio, BIO_NOCLOSE);
		// _logger->info(__FILEREF__ + "BIO_free_all...");
		BIO_free_all(bio);
			// _logger->info(__FILEREF__ + "BIO_free...");
		// BIO_free(b64);   // useless because of BIO_free_all

		// _logger->info(__FILEREF__ + "base64Text set...");
		string base64Encoded; // = string(bufferPtr->data);
		// 2022-08-09: a differenza di ubuntu 20.04, con ubuntu 22.04,
		// 	l'ultimo carattere di bufferPtr->data è un \n (10)
		// 	Questo carattere non serve e crea problemi, quindi controllo e
		// 	se presente, lo elimino
		if(strlen(bufferPtr->data) > 0
			&& bufferPtr->data[strlen(bufferPtr->data) - 1] == '\n')
			base64Encoded = string(bufferPtr->data, strlen(bufferPtr->data) - 1);
		else
			base64Encoded = string(bufferPtr->data);

		BUF_MEM_free(bufferPtr);

		// _logger->info(__FILEREF__ + "signature: " + signature);

		return base64Encoded;
	}

	size_t Encrypt::calcBinaryLength(const char* b64input)
	{
		// Calculates the length of a base64 decoded string

		size_t len = strlen(b64input);
		size_t padding = 0;

		if (b64input[len-1] == '=' && b64input[len-2] == '=') //last two chars are =
			padding = 2;
		else if (b64input[len-1] == '=') //last char is =
			padding = 1;

		return (len * 3) / 4 - padding;
	}

	int Encrypt::convertFromBase64ToBinary(const char* b64message,
		unsigned char** buffer, size_t* length)
	{ //Decodes a base64 encoded string
		BIO *bio, *b64;

		int decodeLen = calcBinaryLength(b64message);
		*buffer = (unsigned char*) malloc(decodeLen + 1);
		(*buffer)[decodeLen] = '\0';

		bio = BIO_new_mem_buf(b64message, -1);
		b64 = BIO_new(BIO_f_base64());
		bio = BIO_push(b64, bio);

		BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); //Do not use newlines to flush buffer
		*length = BIO_read(bio, *buffer, strlen(b64message));
		if (*length != decodeLen)
		{
			free(*buffer);

			throw runtime_error(string("EVP_DecryptFinal_ex failed"));
		}
		
		BIO_free_all(bio);

		return (0); //success
	}
#endif

