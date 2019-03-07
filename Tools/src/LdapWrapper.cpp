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


void LdapWrapper::init (string ldapURL, string managerUserName, string managerPassword)

{

	_ldapURL = ldapURL;
	_managerUserName = managerUserName;
	_managerPassword = managerPassword;
}

bool LdapWrapper::testCredentials (string userName, string password)
{

	bool testCredentials = false;

	LDAP *ldap;
	LDAPMessage *answer, *entry;
	BerElement *ber;

	int  auth_method    = LDAP_AUTH_SIMPLE;
	int  ldap_version   = LDAP_VERSION3;

	// char *base_dn       = "OU=User,DC=media,DC=int";
	string base_dn		= "DC=media,DC=int";

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


std::cout << "ldap_initialize..." << std::endl;
	/* STEP 1: Get a LDAP connection handle and set any session preferences. */
	/* For ldaps we must call ldap_sslinit(char *host, int port, int secure) */
	int result = ldap_initialize(&ldap, _ldapURL.c_str());
	if (result != LDAP_SUCCESS)
	{
		string errorMessage = "ldap_initialize failed";

		throw runtime_error(errorMessage);
	}
	// printf("Generated LDAP handle.\n");

	/* The LDAP_OPT_PROTOCOL_VERSION session preference specifies the client */
	/* is an LDAPv3 client. */
	result = ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
	if (result != LDAP_OPT_SUCCESS)
	{
		ldap_unbind(ldap);

		string errorMessage = "ldap_set_option failed!";

		throw runtime_error(errorMessage);
	}
	// printf("Set LDAPv3 client version.\n");


	/* STEP 2: Bind to the server. */
	// If no DN or credentials are specified, we bind anonymously to the server */
	// result = ldap_simple_bind_s( ldap, NULL, NULL );
	result = ldap_simple_bind_s(ldap, _managerUserName.c_str(), _managerPassword.c_str() );
	if ( result != LDAP_SUCCESS )
	{
		ldap_unbind(ldap);

		string errorMessage = string("ldap_simple_bind_s: ") + ldap_err2string(result);

		throw runtime_error(errorMessage);
	}


	/* STEP 3: Do the LDAP search. */
	result=ldap_search_s(ldap, base_dn.c_str(), scope, filter.c_str(), attrs, attrsonly, &answer);
	if (result != LDAP_SUCCESS)
	{
		ldap_unbind(ldap);

		string errorMessage = string("ldap_search_s: ") + ldap_err2string(result);

		throw runtime_error(errorMessage);
	}

	/* Return the number of objects found during the search */
	int entriesFound=ldap_count_entries(ldap, answer);
	if (entriesFound == 0)
	{
		ldap_msgfree(answer);
		ldap_unbind(ldap);

		// fprintf(stderr, "LDAP search did not return any data.\n");
		// exit(EXIT_FAILURE);

		return false;
	}

	/* cycle through all objects returned with our search */
	for (entry = ldap_first_entry(ldap, answer); entry != NULL; entry = ldap_next_entry(ldap, entry))
	{
		/* Print the DN string of the object */
		dn = ldap_get_dn(ldap, entry);
		// printf("Found Object: %s\n", dn);


		{
			/* rebind */
			/* STEP 1: Get a LDAP connection handle and set any session preferences. */
			/* For ldaps we must call ldap_sslinit(char *host, int port, int secure) */
			LDAP *ldap2;
			int result = ldap_initialize(&ldap2, _ldapURL.c_str());
			if (result != LDAP_SUCCESS)
			{
				ldap_memfree(dn);
				ldap_msgfree(answer);
				ldap_unbind(ldap);

				string errorMessage = "ldap_initialize failed";

				throw runtime_error(errorMessage);
			}
			// printf("Generated LDAP handle.\n");

			/* The LDAP_OPT_PROTOCOL_VERSION session preference specifies the client */
			/* is an LDAPv3 client. */
			result = ldap_set_option(ldap2, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
			if (result != LDAP_OPT_SUCCESS)
			{
				ldap_unbind(ldap2);
				ldap_memfree(dn);
				ldap_msgfree(answer);
				ldap_unbind(ldap);

				string errorMessage = "ldap_set_option failed!";

				throw runtime_error(errorMessage);
			}
			// printf("Set LDAPv3 client version.\n");

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

				testCredentials = false;
				// string errorMessage = string("ldap_simple_bind_s: ") + ldap_err2string(result);

				// throw runtime_error(errorMessage);
			}
			else
			{
				// printf("CHECK PWD SUCCESS.\n");
				testCredentials = true;
			}

			ldap_unbind(ldap2);
		}

		if (testCredentials)
		{
			// cycle through all returned attributes
			for (attribute = ldap_first_attribute(ldap, entry, &ber); attribute != NULL;
				attribute = ldap_next_attribute(ldap, entry, ber))
			{
				/* Print the attribute name */
				// printf("Found Attribute: %s\n", attribute);
				if ((values = ldap_get_values(ldap, entry, attribute)) != NULL)
				{
					/* cycle through all values returned for this attribute */
					for (int valueIndex = 0; values[valueIndex] != NULL; valueIndex++)
					{
						/* print each value of a attribute here */
						// printf("%s: %s\n", attribute, values[valueIndex] );
					}

					ldap_value_free(values);
				}
			}
		}

		ldap_memfree(dn);
	}

	ldap_msgfree(answer);
	ldap_unbind(ldap);

	return testCredentials;
}

