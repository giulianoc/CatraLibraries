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


#ifndef LdapWrapper_h
	#define LdapWrapper_h

	#include <tuple>
	#include <string>

	using namespace std;

	typedef class LdapWrapper
	{
		private:
			LdapWrapper (const LdapWrapper &);

			LdapWrapper &operator = (const LdapWrapper &);

			string _ldapURL;
			string _managerUserName;
			string _managerPassword;

		public:
			/**
				Costruttore.
			*/
			LdapWrapper ();

			/**
				Distruttore.
			*/
			~LdapWrapper ();

			void init (string ldapURL, string managerUserName, string managerPassword);

			pair<bool,string> testCredentials (
					string userName, string password, string searchBaseDn);

	} LdapWrapper_t, *LdapWrapper_p;

#endif

