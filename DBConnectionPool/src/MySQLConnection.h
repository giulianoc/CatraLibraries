#include "DBConnectionPool.h"
#include <string>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>

// #define DBCONNECTIONPOOL_LOG

class MySQLConnection : public DBConnection {

public:
    shared_ptr<sql::Connection> _sqlConnection;

/*
    MySQLConnection(): DBConnection() 
    {
    }
*/

    MySQLConnection(string selectTestingConnection, int connectionId):
		DBConnection(selectTestingConnection, connectionId) 
    {
	}

    ~MySQLConnection() 
    {
        if(_sqlConnection) 
        {
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_DEBUG("sql connection destruct"
				", _connectionId: {}", _connectionId
			);
			#endif

            _sqlConnection->close();
            _sqlConnection.reset(); 	// Release and destruct
        }
    };

	virtual bool connectionValid()
	{
		bool connectionValid = true;

		if (_sqlConnection == nullptr)
		{
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_ERROR("sql connection is null"
				", _connectionId: {}", _connectionId
			);
			#endif
			connectionValid = false;
		}
		else
		{
			if (_selectTestingConnection != "" && _sqlConnection != nullptr)
			{
				try
				{
					shared_ptr<sql::PreparedStatement> preparedStatement (
						_sqlConnection->prepareStatement(_selectTestingConnection));
					shared_ptr<sql::ResultSet> resultSet (preparedStatement->executeQuery());
					if (resultSet->next())
					{
						int count     = resultSet->getInt(1);
					}
					else
					{
						connectionValid = false;
					}
				}
				catch(sql::SQLException& se)
				{
					#ifdef DBCONNECTIONPOOL_LOG                                                                                  
					SPDLOG_ERROR("sql connection exception"
						", _connectionId: {}"
						", se.what(): {}",
						_connectionId, se.what()
					);
					#endif

					connectionValid = false;
				}
				catch(exception& e)
				{
					#ifdef DBCONNECTIONPOOL_LOG                                                                                  
					SPDLOG_ERROR("sql connection exception"
						", _connectionId: {}"
						", e.what(): {}", _connectionId, e.what()
					);
					#endif

					connectionValid = false;
				}
			}
		}

		return connectionValid;
	}
};


class MySQLConnectionFactory : public DBConnectionFactory {

private:
    string _dbServer;
    string _dbUsername;
    string _dbPassword;
    string _dbName;
	bool _reconnect;
	string _defaultCharacterSet;
	string _selectTestingConnection;

public:
    MySQLConnectionFactory(string dbServer, string dbUsername, string dbPassword, string dbName,
		bool reconnect, string defaultCharacterSet, string selectTestingConnection) 
    {
        _dbServer = dbServer;
        _dbUsername = dbUsername;
        _dbPassword = dbPassword;
        _dbName = dbName;
		_reconnect = reconnect;
		_defaultCharacterSet = defaultCharacterSet;
		_selectTestingConnection = selectTestingConnection;
    };

    // Any exceptions thrown here should be caught elsewhere
    shared_ptr<DBConnection> create(int connectionId)
	{
		try
		{
			sql::Driver *driver;
			driver = get_driver_instance();

			sql::ConnectOptionsMap connection_properties;

			connection_properties["hostName"] = _dbServer;
			connection_properties["userName"] = _dbUsername;
			connection_properties["password"] = _dbPassword;
			connection_properties["schema"] = _dbName;
			connection_properties["OPT_RECONNECT"] = _reconnect;
			connection_properties["OPT_CHARSET_NAME"] = _defaultCharacterSet;

			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_DEBUG("sql connection creating..."
				", _dbServer: {}"
				", _dbUsername: {}"
				", _dbPassword: {}"
				", _dbName: {}"
				", _reconnect: {}"
				", _defaultCharacterSet: {}",
				_dbServer, _dbUsername, _dbPassword, _dbName, _reconnect, _defaultCharacterSet
			);
			#endif

			// server like "tcp://127.0.0.1:3306"
			shared_ptr<sql::Connection> connectionFromDriver (driver->connect(connection_properties));

			// shared_ptr<sql::Connection> connectionFromDriver (driver->connect(_dbServer, _dbUsername, _dbPassword));
			// bool reconnect_state = true;
			// connectionFromDriver->setClientOption("OPT_RECONNECT", &reconnect_state);    
			// connectionFromDriver->setSchema(_dbName);
	
			shared_ptr<MySQLConnection>     mySqlConnection = make_shared<MySQLConnection>(
				_selectTestingConnection, connectionId);
			mySqlConnection->_sqlConnection = connectionFromDriver;

			bool connectionValid = mySqlConnection->connectionValid();
			if (!connectionValid)
			{
				#ifdef DBCONNECTIONPOOL_LOG                                                                                  
				string errorMessage = fmt::format("just created sql connection is not valid"
					", _connectionId: {}"
					", _dbServer: {}"
					", _dbUsername: {}"
					", _dbName: {}",
					mySqlConnection->getConnectionId(), _dbServer, _dbUsername, _dbName
					);
				SPDLOG_ERROR(errorMessage);
				#endif

				return nullptr;
			}
			else
			{
				#ifdef DBCONNECTIONPOOL_LOG                                                                                  
				SPDLOG_DEBUG("just created sql connection"
					", _connectionId: {}"
					", _dbServer: {}"
					", _dbUsername: {}"
					", _dbName: {}",
					mySqlConnection->getConnectionId(), _dbServer, _dbUsername, _dbName
				);
				#endif
			}

			return static_pointer_cast<DBConnection>(mySqlConnection);
		}
		catch(sql::SQLException& se)
		{
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_ERROR("sql connection creation failed"
				", se.what(): {}", se.what()
					);
			#endif

			throw runtime_error(se.what());
		}
		catch(runtime_error& e)
		{        
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_ERROR("sql connection creation failed"
				", e.what(): {}", e.what()
					);
			#endif

			throw e;
		}
		catch(exception& e)
		{        
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_ERROR("sql connection creation failed"
				", e.what(): {}", e.what()
					);
			#endif

			throw e;
		}
    };

};
