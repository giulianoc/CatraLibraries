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


int main (int iArgc, char *pArgv [])

{

	string dbServer("mms");
	string dbUsername("mms");
	string dbPassword("mms");
	string ("mms");
        string selectTestingConnection("select count(*) from MM_TestTable");
	int dbPoolSize = 5;

	logger
	
	shared_ptr<MySQLConnectionFactory>  mySQLConnectionFactory = 
		make_shared<MySQLConnectionFactory>(dbServer, dbUsername, dbPassword, dbName,
		selectTestingConnection, logger);

	_connectionPool = make_shared<DBConnectionPool<MySQLConnection>>(
		dbPoolSize, mySQLConnectionFactory, logger);

	shared_ptr<MySQLConnection> conn;

	for (int index = 0; index < 10; index++)
	{
		try
		{
		}
		catch(sql::SQLException se)
		{
		string exceptionMessage(se.what());
        
		logger->error(__FILEREF__ + "SQL exception"
		+ ", exceptionMessage: " + exceptionMessage
		);

		}
	}

	return 0;
}

