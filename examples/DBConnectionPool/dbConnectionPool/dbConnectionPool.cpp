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

#include <iostream>

using namespace std;

#define DB_DEBUG_LOGGER(x) cout << x << endl
#define DB_ERROR_LOGGER(x) cerr << x << endl

#include "MySQLConnection.h"


int main (int iArgc, char *pArgv [])

{

	string dbServer(pArgv[1]);
	string dbUsername(pArgv[2]);
	string dbPassword(pArgv[3]);
	string dbName(pArgv[4]);
	string selectTestingConnection(pArgv[5]);
	int dbPoolSize = atol(pArgv[6]);

	try
	{
		bool reconnect = true;
		string defaultCharacterSet = "utf8";

		shared_ptr<MySQLConnectionFactory>  mySQLConnectionFactory = 
			make_shared<MySQLConnectionFactory>(dbServer, dbUsername, dbPassword, dbName,
			reconnect, defaultCharacterSet, selectTestingConnection);

		shared_ptr<DBConnectionPool<MySQLConnection>> connectionPool =
			make_shared<DBConnectionPool<MySQLConnection>>(
			dbPoolSize, mySQLConnectionFactory);

		shared_ptr<MySQLConnection> conn;

		for (int index = 0; index < 1000; index++)
		{
			try
			{
				conn = connectionPool->borrow();	
				connectionPool->unborrow(conn);
			}
			catch(sql::SQLException se)
			{
				string exceptionMessage(se.what());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;
			}
			catch(runtime_error e)
			{
				string exceptionMessage(e.what());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;
			}
			catch(exception e)
			{
				string exceptionMessage(e.what());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;
			}
		}
	}
	catch(sql::SQLException se)
	{
		string exceptionMessage(se.what());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
			<< endl;
	}
	catch(runtime_error e)
	{
		string exceptionMessage(e.what());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
			<< endl;
	}
	catch(exception e)
	{
		string exceptionMessage(e.what());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
		<< endl;
	}

	return 0;
}

