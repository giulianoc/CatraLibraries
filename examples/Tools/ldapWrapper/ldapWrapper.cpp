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

#include "LdapWrapper.h"
#include <iostream>

int main (int iArgc, char *pArgv [])

{

	string ldapURL("ldap://media.int:389");
	string managerUserName("svc-rsi-adread09");
	string managerPassword("MP-data-processor");
	string userName("catramgi");
	string baseDn("DC=media,DC=int");

	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <password>"
			<< std:: endl;

		return 1;
	}

	string password = pArgv[1];

	LdapWrapper ldapWrapper;

	ldapWrapper.init(ldapURL, managerUserName, managerPassword);
	pair<bool, string> testCredentialsSuccessfulAndEmail = ldapWrapper.testCredentials(
			userName, password, baseDn);
	bool testCredentialsSuccessful;
	string email;
	tie(testCredentialsSuccessful, email) = testCredentialsSuccessfulAndEmail;

	std::cout << "testCredentialsSuccessful: " << testCredentialsSuccessful << std::endl;
	std::cout << "email: " << email << std::endl;


	return 0;
}

