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


#ifndef Ldap_h
	#define Ldap_h

	#include <string>

	using namespace std;

	typedef class Ldap
	{
		private:
			Ldap (const Ldap &);

			Ldap &operator = (const Ldap &);

		public:
			/**
				Costruttore.
			*/
			Ldap ();

			/**
				Distruttore.
			*/
			~Ldap ();

			void init (string ldapURL, string managerUserName, string managerPassword);

			bool testCredentials (string userName, string password);

	} Ldap_t, *Ldap_p;

#endif

