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

#define LDAP_DEPRECATED 1

#include "LdapWrapper.h"
#include <stdexcept>
#include <assert.h>

#include <iostream>


extern "C" {
// #include <lber.h>
#include <ldap.h>
}


LdapWrapper:: LdapWrapper (void)

{

}


LdapWrapper:: ~LdapWrapper (void)

{

}



LdapWrapper:: LdapWrapper (const LdapWrapper &)

{

	assert (1==0);

	// to do

}


LdapWrapper &LdapWrapper:: operator = (const LdapWrapper &)

{

	assert (1==0);

	// to do

	return *this;

}


void LdapWrapper::init (string ldapURL, string certificatePathName,
		string managerUserName, string managerPassword)

{

	_ldapURL = ldapURL;							// "ldaps://media.int:636" or "ldap://media.int:389"
	_ldapHostName = "";
	_ldapPort = -1;
	_certificatePathName = certificatePathName;	// used only in case of ldaps://...
	_managerUserName = managerUserName;
	_managerPassword = managerPassword;

	string ldapOverSSLPrefix ("ldaps://");
	if (_ldapURL.size() >= ldapOverSSLPrefix.size()
		&& 0 == _ldapURL.compare(0, ldapOverSSLPrefix.size(), ldapOverSSLPrefix))
		_overSSL = true;
	else
		_overSSL = false;
}

void LdapWrapper::init (string ldapHostName, int ldapPort, bool overSSL,
	string certificatePathName, string managerUserName, string managerPassword)

{

	_ldapURL = "";
	_ldapHostName = ldapHostName;				// mscs-lgcur-0001.media.int
	_ldapPort = ldapPort;						// 389 (NO SSL), 636 (SSL)
	_overSSL = overSSL;
	_certificatePathName = certificatePathName;	// used only in case of ldaps://...
	_managerUserName = managerUserName;
	_managerPassword = managerPassword;
}

pair<bool,string> LdapWrapper::testCredentials (
	string userName, string password, string searchBaseDn)
{

	bool testCredentialsSuccessful = false;
	string email;

	LDAP *ldap;
	LDAPMessage *answer, *entry;
	BerElement *ber;

	int  auth_method    = LDAP_AUTH_SIMPLE;
	int  ldap_version   = LDAP_VERSION3;

	// char *base_dn       = "OU=User,DC=media,DC=int";
	// string base_dn		= "DC=media,DC=int";

	// The search scope must be either LDAP_SCOPE_SUBTREE or LDAP_SCOPE_ONELEVEL
	int  scope          = LDAP_SCOPE_SUBTREE;

	// The search filter, "(objectClass=*)" returns everything. Windows can return
	// 1000 objects in one search. Otherwise, "Size limit exceeded" is returned.
	string filter		= string("(&(objectClass=user)(sAMAccountName=") + userName + "))";

	// The attribute list to be returned, use {NULL} for getting all attributes
	char *attrs[]       = {"memberOf", "userPrincipalName", NULL};

	// Specify if only attribute types (1) or both type and value (0) are returned
	int  attrsonly      = 0;

	// dn holds the DN name string of the object(s) returned by the search
	char *dn            = "";

	// attribute holds the name of the object(s) attributes returned
	char *attribute     = "";

	// values is  array to hold the attribute values of the object(s) attributes
	char **values;


	/* STEP 1: Get a LDAP connection handle and set any session preferences. */
	int result;
	{
		if (_ldapURL != "")
		{
			result = ldap_initialize(&ldap, _ldapURL.c_str());
			if (result != LDAP_SUCCESS)
			{
				string errorMessage = "ldap_initialize failed";

				throw runtime_error(errorMessage);
			}
		}
		else
		{
			ldap = ldap_init(_ldapHostName.c_str(), _ldapPort);
			if (ldap == NULL)
			{
				string errorMessage = "ldap_init failed";

				throw runtime_error(errorMessage);
			}
		}
	}
	// printf("Generated LDAP handle.\n");

	/* The LDAP_OPT_PROTOCOL_VERSION session preference specifies the client */
	/* is an LDAPv3 client. */
	result = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
	if (result != LDAP_OPT_SUCCESS)
	{
		ldap_unbind_ext_s(ldap, NULL, NULL);

		string errorMessage = "ldap_set_option LDAP_OPT_PROTOCOL_VERSION failed!";

		throw runtime_error(errorMessage);
	}
	// printf("Set LDAPv3 client version.\n");

	// timeout is important, for example, in case ldap is not listening,
	// the next ldap api (ldap_simple_bind_s) will wait the default timeout
	// that is about 6 minutes
	struct timeval timeOut = {30, 0};   /* 30 second connection timeout */
	result = ldap_set_option (ldap, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);
	if (result != LDAP_OPT_SUCCESS)
	{
		ldap_unbind_ext_s(ldap, NULL, NULL);

		string errorMessage = "ldap_set_option LDAP_OPT_NETWORK_TIMEOUT, failed!";

		throw runtime_error(errorMessage);
	}

    if (_overSSL)
	{
		int opt = LDAP_OPT_X_TLS_NEVER;
		// if (ldap_tls > 1)
		// opt = LDAP_OPT_X_TLS_DEMAND;
		result = ldap_set_option(ldap, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
		if (result != LDAP_OPT_SUCCESS)
		{
			ldap_unbind_ext_s(ldap, NULL, NULL);

			string errorMessage = string("ldap_set_option LDAP_OPT_X_TLS_REQUIRE_CERT failed")
				+ ", error: " + ldap_err2string(result);

			throw runtime_error(errorMessage);
		}
	}

    if (_overSSL)
	{
		result = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE, (void *)_certificatePathName.c_str());
		if (result != LDAP_OPT_SUCCESS)
		{
			ldap_unbind_ext_s(ldap, NULL, NULL);

			string errorMessage = string("ldap_set_option LDAP_OPT_X_TLS_CACERTFILE failed")
				+ ", error: " + ldap_err2string(result);

			throw runtime_error(errorMessage);
		}
	}

	// StartTLS is the name of the standard LDAP operation for initiating TLS/SSL.
	// TLS/SSL is initiated upon successful completion of this LDAP operation. No alternative port is necessary.
	// It is sometimes referred to as the TLS upgrade operation, as it upgrades a normal LDAP connection
	// to one protected by TLS/SSL.

	// ldaps:// and LDAPS refers to "LDAP over TLS/SSL" or "LDAP Secured". TLS/SSL is initated upon connection
	// to an alternative port (normally 636). Though the LDAPS port (636) is registered for this use,
	// the particulars of the TLS/SSL initiation mechanism are not standardized.

	// Summary:
	// 1) ldap:// + StartTLS should be directed to a normal LDAP port (normally 389), not the ldaps:// port.
	// 2) ldaps:// should be directed to an LDAPS port (normally 636), not the LDAP port.

	/*
	if (!_ldapOverSSL)
	{
		// starts SSL connection
		result = ldap_start_tls_s(ldap, NULL, NULL);
		switch(result)
		{
			case LDAP_SUCCESS:

			break;
			case LDAP_CONNECT_ERROR:
			{
				char *errmsg;

				ldap_get_option(ldap, LDAP_OPT_DIAGNOSTIC_MESSAGE, (void*)&errmsg);

				string errorMessage = string("ldap_start_tls_s failed")
					+ ", errmsg: " + errmsg;

				ldap_memfree(errmsg);

				ldap_unbind_ext_s(ldap, NULL, NULL);

				throw runtime_error(errorMessage);
			}
			default:
			{
				ldap_unbind_ext_s(ldap, NULL, NULL);

				string errorMessage = string("ldap_start_tls_s failed")
					+ ", error: " + ldap_err2string(result);

				throw runtime_error(errorMessage);
			}
		};
	}
	*/

	/* STEP 2: Bind to the server. */
	// If no DN or credentials are specified, we bind anonymously to the server */
	// result = ldap_simple_bind_s( ldap, NULL, NULL );
	result = ldap_simple_bind_s(ldap, _managerUserName.c_str(), _managerPassword.c_str() );
	if ( result != LDAP_SUCCESS )
	{
		ldap_unbind_ext_s(ldap, NULL, NULL);

		string errorMessage = string("ldap_simple_bind_s error: ") + ldap_err2string(result)
			+ ", _ldapURL: " + _ldapURL
			+ ", _ldapHostName: " + _ldapHostName
			+ ", _ldapPort: " + to_string(_ldapPort)
			+ ", _overSSL: " + to_string(_overSSL)
			;

		throw runtime_error(errorMessage);
	}

	/* STEP 3: Do the LDAP search. */
	result=ldap_search_s(ldap, searchBaseDn.c_str(), scope, filter.c_str(), attrs, attrsonly,
			&answer);
	if (result != LDAP_SUCCESS)
	{
		ldap_unbind_ext_s(ldap, NULL, NULL);

		string errorMessage = string("ldap_search_s: ") + ldap_err2string(result)
			+ ", _ldapURL: " + _ldapURL
			+ ", _ldapHostName: " + _ldapHostName
			+ ", _ldapPort: " + to_string(_ldapPort)
			+ ", _overSSL: " + to_string(_overSSL)
			;

		throw runtime_error(errorMessage);
	}

	/* Return the number of objects found during the search */
	int entriesFound=ldap_count_entries(ldap, answer);
	if (entriesFound == 0)
	{
		ldap_msgfree(answer);
		ldap_unbind_ext_s(ldap, NULL, NULL);

		return make_pair(testCredentialsSuccessful, email);
	}
	else if (entriesFound != 1)
	{
		// more than one entries returned by the search

		ldap_msgfree(answer);
		ldap_unbind_ext_s(ldap, NULL, NULL);

		return make_pair(testCredentialsSuccessful, email);
	}

	/* cycle through all objects returned with our search */
	for (entry = ldap_first_entry(ldap, answer);
			entry != NULL;
			entry = ldap_next_entry(ldap, entry))
	{
		/* Print the DN string of the object */
		dn = ldap_get_dn(ldap, entry);
		// printf("Found Object: %s\n", dn);


		{
			/* rebind */
			/* STEP 1: Get a LDAP connection handle and set any session preferences. */
			LDAP *ldap2;
			{
				if (_ldapURL != "")
				{
					result = ldap_initialize(&ldap2, _ldapURL.c_str());
					if (result != LDAP_SUCCESS)
					{
						ldap_memfree(dn);
						ldap_msgfree(answer);
						ldap_unbind_ext_s(ldap, NULL, NULL);

						string errorMessage = "ldap_initialize failed";

						throw runtime_error(errorMessage);
					}
				}
				else
				{
					ldap2 = ldap_init(_ldapHostName.c_str(), _ldapPort);
					if (ldap2 == NULL)
					{
						ldap_memfree(dn);
						ldap_msgfree(answer);
						ldap_unbind_ext_s(ldap, NULL, NULL);

						string errorMessage = "ldap_init failed";

						throw runtime_error(errorMessage);
					}
				}
				/*
				result = ldap_initialize(&ldap2, _ldapURL.c_str());
				if (result != LDAP_SUCCESS)
				{
					ldap_memfree(dn);
					ldap_msgfree(answer);
					ldap_unbind_ext_s(ldap, NULL, NULL);

					string errorMessage = "ldap_initialize failed";

					throw runtime_error(errorMessage);
				}
				*/
			}
			// printf("Generated LDAP handle.\n");

			/* The LDAP_OPT_PROTOCOL_VERSION session preference specifies the client */
			/* is an LDAPv3 client. */
			result = ldap_set_option(ldap2, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
			if (result != LDAP_OPT_SUCCESS)
			{
				ldap_unbind_ext_s(ldap2, NULL, NULL);
				ldap_memfree(dn);
				ldap_msgfree(answer);
				ldap_unbind_ext_s(ldap, NULL, NULL);

				string errorMessage = "ldap_set_option failed!";

				throw runtime_error(errorMessage);
			}
			// printf("Set LDAPv3 client version.\n");

			// timeout is important, for example, in case ldap is not listening,
			// the next ldap api (ldap_simple_bind_s) will wait the default timeout
			// that is about 6 minutes
			struct timeval timeOut = {30, 0};   /* 30 second connection timeout */
			result = ldap_set_option (ldap2, LDAP_OPT_NETWORK_TIMEOUT, &timeOut);
			if (result != LDAP_OPT_SUCCESS)
			{
				ldap_unbind_ext_s(ldap2, NULL, NULL);
				ldap_memfree(dn);
				ldap_msgfree(answer);
				ldap_unbind_ext_s(ldap, NULL, NULL);

				string errorMessage = "ldap_set_option LDAP_OPT_NETWORK_TIMEOUT, failed!";

				throw runtime_error(errorMessage);
			}

			if (_overSSL)
			{
				int opt = LDAP_OPT_X_TLS_NEVER;
				// if (ldap_tls > 1)
				// 	opt = LDAP_OPT_X_TLS_DEMAND;
				result = ldap_set_option(ldap2, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
				if (result != LDAP_OPT_SUCCESS)
				{
					ldap_unbind_ext_s(ldap2, NULL, NULL);
					ldap_memfree(dn);
					ldap_msgfree(answer);
					ldap_unbind_ext_s(ldap, NULL, NULL);

					string errorMessage = string("ldap_set_option LDAP_OPT_X_TLS_REQUIRE_CERT failed")
						+ ", error: " + ldap_err2string(result);

					throw runtime_error(errorMessage);
				}
			}

			if (_overSSL)
			{
				result = ldap_set_option(NULL, LDAP_OPT_X_TLS_CACERTFILE,
					(void *)_certificatePathName.c_str());
				if (result != LDAP_OPT_SUCCESS)
				{
					ldap_unbind_ext_s(ldap2, NULL, NULL);
					ldap_memfree(dn);
					ldap_msgfree(answer);
					ldap_unbind_ext_s(ldap, NULL, NULL);

					string errorMessage = string("ldap_set_option LDAP_OPT_X_TLS_CACERTFILE failed")
						+ ", error: " + ldap_err2string(result);

					throw runtime_error(errorMessage);
				}
			}

			/*
			if (!_ldapOverSSL)
			{
				// starts SSL connection
				result = ldap_start_tls_s(ldap2, NULL, NULL);
				switch(result)
				{
					case LDAP_SUCCESS:

					break;
					case LDAP_CONNECT_ERROR:
					{
						char *errmsg;

						ldap_get_option(ldap2, LDAP_OPT_DIAGNOSTIC_MESSAGE, (void*)&errmsg);

						string errorMessage = string("ldap_start_tls_s failed")
							+ ", error: " + errmsg;

						ldap_memfree(errmsg);

						ldap_unbind_ext_s(ldap2, NULL, NULL);
						ldap_memfree(dn);
						ldap_msgfree(answer);
						ldap_unbind_ext_s(ldap, NULL, NULL);

						throw runtime_error(errorMessage);
					}
					default:
					{
						ldap_unbind_ext_s(ldap2, NULL, NULL);
						ldap_memfree(dn);
						ldap_msgfree(answer);
						ldap_unbind_ext_s(ldap, NULL, NULL);

						string errorMessage = string("ldap_start_tls_s failed")
							+ ", error: " + ldap_err2string(result);

						throw runtime_error(errorMessage);
					}
				};
			}
			*/

			/* STEP 2: Bind to the server. */
			// If no DN or credentials are specified, we bind anonymously to the server */
			// result = ldap_simple_bind_s( ldap, NULL, NULL );
			result=ldap_simple_bind_s(ldap2, dn, password.c_str());
			if (result != LDAP_SUCCESS)
			{
				/*
				ldap_unbind(ldap2);
				ldap_memfree(dn);
				ldap_msgfree(answer);
				ldap_unbind(ldap);
				*/

				testCredentialsSuccessful = false;
				// string errorMessage = string("ldap_simple_bind_s: ") + ldap_err2string(result);

				// throw runtime_error(errorMessage);
			}
			else
			{
				// printf("CHECK PWD SUCCESS.\n");
				testCredentialsSuccessful = true;
			}

			ldap_unbind_ext_s(ldap2, NULL, NULL);
		}

		if (testCredentialsSuccessful)
		{
			char** emailValues = ldap_get_values(ldap, entry, "userPrincipalName");
			if (ldap_count_values(emailValues) == 1)
				email = emailValues[0];
			ldap_value_free(emailValues);

			// cycle through all returned attributes
			/*
			for (attribute = ldap_first_attribute(ldap, entry, &ber); attribute != NULL;
				attribute = ldap_next_attribute(ldap, entry, ber))
			{
				// Print the attribute name
				// printf("Found Attribute: %s\n", attribute);
				if ((values = ldap_get_values(ldap, entry, attribute)) != NULL)
				{
					// cycle through all values returned for this attribute
					for (int valueIndex = 0; values[valueIndex] != NULL; valueIndex++)
					{
						// print each value of a attribute here
						// printf("%s: %s\n", attribute, values[valueIndex] );
std::cout << attribute << ": " << values[valueIndex] << std::endl;
					}

					ldap_value_free(values);
				}
			}
			*/
		}

		ldap_memfree(dn);
	}

	ldap_msgfree(answer);
	ldap_unbind_ext_s(ldap, NULL, NULL);

	return make_pair(testCredentialsSuccessful, email);
}

