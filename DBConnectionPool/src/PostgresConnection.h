
#include "DBConnectionPool.h"
#include <string>
#include <pqxx/pqxx>
#include <pqxx/nontransaction>

// #define DBCONNECTIONPOOL_LOG

using namespace pqxx;

class PostgresConnection : public DBConnection {

public:
	shared_ptr<connection> _sqlConnection;

/*
	PostgresConnection(): DBConnection() 
	{
	}
*/

	PostgresConnection(string selectTestingConnection, int connectionId):
		DBConnection(selectTestingConnection, connectionId) 
	{
	}

	~PostgresConnection() 
	{
		#ifdef DBCONNECTIONPOOL_LOG                                                                                  
		SPDLOG_DEBUG("sql connection destruct"
			", _connectionId: {}", _connectionId
		);
		#endif
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
			if (_selectTestingConnection != "")
			{
				try
				{
					#ifdef DBCONNECTIONPOOL_LOG                                                                                  
					SPDLOG_DEBUG("sql connection test"
						", _connectionId: {}"
						", _selectTestingConnection: {}", _connectionId, _selectTestingConnection
					);
					#endif
					nontransaction trans{*_sqlConnection};

					trans.exec(_selectTestingConnection);

					// This doesn't really do anything
					trans.commit();
				}
				catch(sql_error const &e)
				{
					#ifdef DBCONNECTIONPOOL_LOG                                                                                  
					SPDLOG_ERROR("sql connection exception"
						", _connectionId: {}"
						", e.query(): {}"
						", e.what(): {}", _connectionId, e.query(), e.what()
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
			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			string connectionDetails = fmt::format("dbname={} user={} password={} hostaddr={}"
				" port=5432",
				_dbName, _dbUsername, _dbPassword, _dbServer
			);
			#else
			string connectionDetails = "dbname=" + _dbName + " user=" + _dbUsername
				+ " password=" + _dbPassword + " hostaddr=" + _dbServer + " port=5432";
			#endif
			shared_ptr<connection> conn = make_shared<connection> (connectionDetails);

			#ifdef DBCONNECTIONPOOL_LOG                                                                                  
			SPDLOG_DEBUG("sql connection creating..."
				", _dbServer: {}"
				", _dbUsername: {}"
				", _dbPassword: {}"
				", _dbName: {}", _dbServer, _dbUsername, _dbPassword, _dbName
				// + ", _reconnect: " + to_string(_reconnect)
				// + ", _defaultCharacterSet: " + _defaultCharacterSet
			);
			#endif

			shared_ptr<PostgresConnection>     postgresConnection = make_shared<PostgresConnection>(
				_selectTestingConnection, connectionId);
			postgresConnection->_sqlConnection = conn;

			bool connectionValid = postgresConnection->connectionValid();
			if (!connectionValid)
			{
				#ifdef DBCONNECTIONPOOL_LOG                                                                                  
				string errorMessage = fmt::format("just created sql connection is not valid"
					", _connectionId: {}"
					", _dbServer: {}"
					", _dbUsername: {}"
					", _dbName: {}",
					postgresConnection->getConnectionId(), _dbServer, _dbUsername, _dbName
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
					postgresConnection->getConnectionId(), _dbServer, _dbUsername, _dbName
				);
				#endif
			}

			return static_pointer_cast<DBConnection>(postgresConnection);
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

template<class T>
class PostgresDBConnectionPool : public DBConnectionPool<T> {

public:
	PostgresDBConnectionPool(size_t poolSize, shared_ptr<DBConnectionFactory> factory):
		DBConnectionPool<T>(poolSize, factory) 
	{
	}

	pair<nontransaction, shared_ptr<T>> borrowNontransactionConnection()
	{
		shared_ptr<T> conn = nullptr;
		try
		{
			shared_ptr<T> conn = this->borrow();
			return make_pair(nontransaction{*(conn->_sqlConnection)}, conn);
		}
		catch(sql_error const &e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("SQL exception"
				", query: {}"
				", exceptionMessage: {}"
				", conn: {}",
				e.query(), e.what(), (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
			);
			#endif

			if (conn != nullptr)
			{
				this->unborrow(conn);
				conn = nullptr;
			}

			throw e;
		}
		catch(runtime_error& e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("runtime exception"
				", exceptionMessage: {}"
				", conn: {}",
				e.what(), (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
			);
			#endif

			if (conn != nullptr)
			{
				this->unborrow(conn);
				conn = nullptr;
			}

			throw e;
		}
		catch(exception& e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("exception"
				", exceptionMessage: {}"
				", conn: {}",
				e.what(), (conn != nullptr ? to_string(conn->getConnectionId()) : "-1")
			);
			#endif

			if (conn != nullptr)
			{
				this->unborrow(conn);
				conn = nullptr;
			}

			throw e;
		}
	}

	void unborrowNontransactionConnection(
		pair<nontransaction, shared_ptr<T>>& transDetails, bool commit)
	{
		bool unborrowDone = false;
		try
		{
			if (commit)
				transDetails.first.commit();
			else
				transDetails.first.abort();

			this->unborrow(transDetails.second);
			unborrowDone = true;
		}
		catch(sql_error const &e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("SQL exception"
				", query: {}"
				", exceptionMessage: {}"
				", conn: {}",
				e.query(), e.what(), transDetails.second->getConnectionId()
			);
			#endif

			this->unborrow(transDetails.second);

			throw e;
		}
		catch(runtime_error& e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("runtime exception"
				", exceptionMessage: {}"
				", conn: {}",
				e.what(), transDetails.second->getConnectionId()
			);
			#endif

			this->unborrow(transDetails.second);

			throw e;
		}
		catch(exception& e)
		{
			#ifdef DBCONNECTIONPOOL_LOG
			SPDLOG_ERROR("exception"
				", exceptionMessage: {}"
				", conn: {}",
				e.what(), transDetails.second->getConnectionId()
			);
			#endif

			this->unborrow(transDetails.second);

			throw e;
		}
	}
};

