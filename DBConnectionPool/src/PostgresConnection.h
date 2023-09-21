/* Copyright 2013 Active911 Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http: *www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "DBConnectionPool.h"
#include <string>
#include <pqxx/pqxx>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>


class PostgresConnection : public DBConnection {

public:
	shared_ptr<pqxx::connection> _sqlConnection;

	PostgresConnection(): DBConnection() 
	{
	}

	PostgresConnection(string selectTestingConnection, int connectionId):
		DBConnection(selectTestingConnection, connectionId) 
	{
	}

	~PostgresConnection() 
	{
		DB_DEBUG_LOGGER(__FILEREF__ + "sql connection destruct"
			", _connectionId: " + to_string(_connectionId)
		);
	};

	virtual bool connectionValid()
	{
		bool connectionValid = true;


		if (_sqlConnection == nullptr)
		{
			DB_ERROR_LOGGER(__FILEREF__ + "sql connection is null"
				+ ", _connectionId: " + to_string(_connectionId)
			);
			connectionValid = false;
		}
		else
		{
			if (_selectTestingConnection != "")
			{
				try
				{
					pqxx::work transation{*_sqlConnection};

					// int count = txn.query_value<int>(txn.quote(_selectTestingConnection));
					pqxx::row r = transation.exec1(transation.quote(_selectTestingConnection));

					transation.commit();
				}
				catch(pqxx::sql_error const &e)
				{
					DB_ERROR_LOGGER(__FILEREF__ + "sql connection exception"
						+ ", _connectionId: " + to_string(_connectionId)
						+ ", e.query(): " + e.query()
						+ ", e.what(): " + e.what()
					);

					connectionValid = false;
				}
				catch(exception e)
				{
					DB_ERROR_LOGGER(__FILEREF__ + "sql connection exception"
						+ ", _connectionId: " + to_string(_connectionId)
						+ ", e.what(): " + e.what()
					);

					connectionValid = false;
				}
			}
		}

		return connectionValid;
	}
};


class PostgresConnectionFactory : public DBConnectionFactory {

private:
	string _dbServer;
	string _dbUsername;
	string _dbPassword;
	string _dbName;
	// bool _reconnect;
	// string _defaultCharacterSet;
	string _selectTestingConnection;

public:
	PostgresConnectionFactory(string dbServer, string dbUsername, string dbPassword, string dbName,
		/* bool reconnect, string defaultCharacterSet, */ string selectTestingConnection) 
	{
		_dbServer = dbServer;
		_dbUsername = dbUsername;
		_dbPassword = dbPassword;
		_dbName = dbName;
		// _reconnect = reconnect;
		// _defaultCharacterSet = defaultCharacterSet;
		_selectTestingConnection = selectTestingConnection;
	};

	// Any exceptions thrown here should be caught elsewhere
	shared_ptr<DBConnection> create(int connectionId)
	{
		try
		{
			// reconnect? character set?
			string connectionDetails = "dbname=" + _dbName
				+ " user=" + _dbUsername 
				+ " password=" + _dbPassword
				+ " hostaddr=" + _dbServer
				+ " port=5432"
			;
			shared_ptr<pqxx::connection> connection = make_shared<pqxx::connection> (connectionDetails);

			DB_DEBUG_LOGGER(__FILEREF__ + "sql connection creating..."
				+ ", _dbServer: " + _dbServer
				+ ", _dbUsername: " + _dbUsername
				+ ", _dbPassword: " + _dbPassword
				+ ", _dbName: " + _dbName
				// + ", _reconnect: " + to_string(_reconnect)
				// + ", _defaultCharacterSet: " + _defaultCharacterSet
			);

			shared_ptr<PostgresConnection>     postgresConnection = make_shared<PostgresConnection>(
				_selectTestingConnection, connectionId);
			postgresConnection->_sqlConnection = connection;

			bool connectionValid = postgresConnection->connectionValid();
			if (!connectionValid)
			{
				string errorMessage = string("just created sql connection is not valid")
					+ ", _connectionId: " + to_string(postgresConnection->getConnectionId())
					+ ", _dbServer: " + _dbServer
					+ ", _dbUsername: " + _dbUsername
					+ ", _dbName: " + _dbName
					;
				DB_ERROR_LOGGER(__FILEREF__ + errorMessage);

				return nullptr;
			}
			else
			{
				DB_DEBUG_LOGGER(__FILEREF__ + "just created sql connection"
					+ ", _connectionId: " + to_string(postgresConnection->getConnectionId())
					+ ", _dbServer: " + _dbServer
					+ ", _dbUsername: " + _dbUsername
					+ ", _dbName: " + _dbName
				);
			}

			return static_pointer_cast<DBConnection>(postgresConnection);
		}
		catch(runtime_error e)
		{        
			DB_ERROR_LOGGER(__FILEREF__ + "sql connection creation failed"
				+ ", e.what(): " + e.what()
					);

			throw e;
		}
		catch(exception e)
		{        
			DB_ERROR_LOGGER(__FILEREF__ + "sql connection creation failed"
				+ ", e.what(): " + e.what()
					);

			throw e;
		}
    };

};
