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

#define DB_BORROW_DEBUG_LOGGER(x) cout << x << endl
#define DB_BORROW_ERROR_LOGGER(x) cerr << x << endl
#define DB_DEBUG_LOGGER(x) cout << x << endl
#define DB_ERROR_LOGGER(x) cerr << x << endl

#include "PostgresConnection.h"


int main (int iArgc, char *pArgv [])

{

	if (iArgc != 8)
	{
		cout << "wrong args number: " << iArgc
			<< ", usage: "
			<< pArgv[0]
			<< " \"tcp://rsis-cp-wedep1.media.int:3306\" u_vedatest \"~Kpq6Mll]Y\" vedatest \"select count(*) from MMS_TestConnection\" 1 100" << endl;

		return 1;
	}

	string dbServer(pArgv[1]);
	string dbUsername(pArgv[2]);
	string dbPassword(pArgv[3]);
	string dbName(pArgv[4]);
	string selectTestingConnection(pArgv[5]);
	int dbPoolSize = atol(pArgv[6]);
	int iterations = atol(pArgv[7]);

	cout << pArgv[0]
		<< ", dbServer: " + dbServer
		<< ", dbUsername: " + dbUsername
		<< ", dbPassword: " + dbPassword
		<< ", dbName: " + dbName
		<< ", selectTestingConnection: " + selectTestingConnection
		<< ", dbPoolSize: " + to_string(dbPoolSize)
		<< ", iterations: " + to_string(iterations)
		<< endl;

	shared_ptr<PostgresConnectionFactory>  postgrSQLConnectionFactory;
	shared_ptr<DBConnectionPool<PostgresConnection>> connectionPool;
	shared_ptr<PostgresConnection> conn = nullptr;
	try
	{
		bool reconnect = true;
		string defaultCharacterSet = "utf8";

		cout << "postgresConnectionFactory..." << endl;
		postgrSQLConnectionFactory = 
			make_shared<PostgresConnectionFactory>(dbServer, dbUsername, dbPassword, dbName,
			/* reconnect, defaultCharacterSet,*/ selectTestingConnection);

		cout << "connectionPool..." << endl;
		connectionPool =
			make_shared<DBConnectionPool<PostgresConnection>>(
			dbPoolSize, postgrSQLConnectionFactory);

		for (int index = 0; index < iterations; index++)
		{
			cout << "index: " << index << endl;

			try
			{
				conn = connectionPool->borrow();	
				connectionPool->unborrow(conn);
				conn = nullptr;
			}
			catch(pqxx::sql_error const &se)
			{
				string exceptionMessage(se.what() + string(", query: ") + se.query());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;

				if (conn != nullptr)
				{
					connectionPool->unborrow(conn);
					conn = nullptr;
				}
			}
			catch(runtime_error e)
			{
				string exceptionMessage(e.what());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;

				if (conn != nullptr)
				{
					connectionPool->unborrow(conn);
					conn = nullptr;
				}
			}
			catch(exception e)
			{
				string exceptionMessage(e.what());
 
				cerr <<__FILEREF__ + "SQL borrow/unborrow exception"
					+ ", exceptionMessage: " + exceptionMessage
					<< endl;

				if (conn != nullptr)
				{
					connectionPool->unborrow(conn);
					conn = nullptr;
				}
			}
		}
	}
	catch(pqxx::sql_error const &se)
	{
		string exceptionMessage(se.what() + string(", query: ") + se.query());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
			+ ", conn: " + (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
			<< endl;

		if (conn != nullptr)
        {
            connectionPool->unborrow(conn);
            conn = nullptr;
        }
	}
	catch(runtime_error e)
	{
		string exceptionMessage(e.what());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
			+ ", conn: " + (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
			<< endl;

		if (conn != nullptr)
        {
            connectionPool->unborrow(conn);
            conn = nullptr;
        }
	}
	catch(exception e)
	{
		string exceptionMessage(e.what());

		cerr <<__FILEREF__ + "SQL exception"
			+ ", exceptionMessage: " + exceptionMessage
			+ ", conn: " + (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
		<< endl;

		if (conn != nullptr)
        {
            connectionPool->unborrow(conn);
            conn = nullptr;
        }
	}

	return 0;
}

