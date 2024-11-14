#ifndef PostgresConnection_h
#define PostgresConnection_h

#include "DBConnectionPool.h"
#include <pqxx/nontransaction>
#include <pqxx/pqxx>
#include <string>

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

  PostgresConnection(string selectTestingConnection, int connectionId)
      : DBConnection(selectTestingConnection, connectionId) {}

  ~PostgresConnection() {
#ifdef DBCONNECTIONPOOL_LOG
    SPDLOG_DEBUG("sql connection destruct"
                 ", _connectionId: {}",
                 _connectionId);
#endif
  };

  virtual bool connectionValid() {
    bool connectionValid = true;

    if (_sqlConnection == nullptr) {
#ifdef DBCONNECTIONPOOL_LOG
      SPDLOG_ERROR("sql connection is null"
                   ", _connectionId: {}",
                   _connectionId);
#endif
      connectionValid = false;
    } else {
      if (_selectTestingConnection != "") {
        try {
#ifdef DBCONNECTIONPOOL_LOG
          SPDLOG_DEBUG("sql connection test"
                       ", _connectionId: {}"
                       ", _selectTestingConnection: {}",
                       _connectionId, _selectTestingConnection);
#endif
          nontransaction trans{*_sqlConnection};

          trans.exec(_selectTestingConnection);

          // This doesn't really do anything
          trans.commit();
        } catch (sql_error const &e) {
#ifdef DBCONNECTIONPOOL_LOG
          SPDLOG_ERROR("sql connection exception"
                       ", _connectionId: {}"
                       ", e.query(): {}"
                       ", e.what(): {}",
                       _connectionId, e.query(), e.what());
#endif

          connectionValid = false;
        } catch (exception &e) {
#ifdef DBCONNECTIONPOOL_LOG
          SPDLOG_ERROR("sql connection exception"
                       ", _connectionId: {}"
                       ", _selectTestingConnection: {}"
                       ", e.what(): {}",
                       _connectionId, _selectTestingConnection, e.what());
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
  int _dbPort;
  string _dbPassword;
  string _dbName;
  // bool _reconnect;
  // string _defaultCharacterSet;
  string _selectTestingConnection;

public:
  PostgresConnectionFactory(string dbServer, string dbUsername, int dbPort,
                            string dbPassword, string dbName,
                            /* bool reconnect, string defaultCharacterSet, */
                            string selectTestingConnection) {
    _dbServer = dbServer;
    _dbUsername = dbUsername;
    _dbPort = dbPort;
    _dbPassword = dbPassword;
    _dbName = dbName;
    // _reconnect = reconnect;
    // _defaultCharacterSet = defaultCharacterSet;
    _selectTestingConnection = selectTestingConnection;
  };

  // Any exceptions thrown here should be caught elsewhere
  shared_ptr<DBConnection> create(int connectionId) {
    try {
// reconnect? character set?
#ifdef DBCONNECTIONPOOL_LOG
      // string connectionDetails = fmt::format("dbname={} user={} password={}
      // hostaddr={}" 	" port=5432", 	_dbName, _dbUsername, _dbPassword,
      // _dbServer
      // );
      string connectionDetails =
          fmt::format("postgresql://{}:{}@{}:5432/{}", _dbUsername, _dbPassword,
                      _dbServer, _dbName);
#else
      // string connectionDetails = "dbname=" + _dbName + " user=" + _dbUsername
      // 	+ " password=" + _dbPassword + " hostaddr=" + _dbServer + "
      // port=5432";
      string connectionDetails = "postgresql://" + _dbUsername + ":" +
                                 _dbPassword + "@" + _dbServer + ":" +
                                 to_string(_dbPort) + "/" + _dbName;
#endif
#ifdef DBCONNECTIONPOOL_LOG
      SPDLOG_DEBUG("sql connection creating..."
                   ", _dbServer: {}"
                   ", _dbUsername: {}"
                   ", _dbPassword: {}"
                   ", _dbName: {}"
                   ", connectionDetails: {}",
                   _dbServer, _dbUsername, _dbPassword, _dbName,
                   connectionDetails);
#endif
      shared_ptr<connection> conn = make_shared<connection>(connectionDetails);

      shared_ptr<PostgresConnection> postgresConnection =
          make_shared<PostgresConnection>(_selectTestingConnection,
                                          connectionId);
      postgresConnection->_sqlConnection = conn;

      bool connectionValid = postgresConnection->connectionValid();
      if (!connectionValid) {
#ifdef DBCONNECTIONPOOL_LOG
        string errorMessage =
            fmt::format("just created sql connection is not valid"
                        ", _connectionId: {}"
                        ", _dbServer: {}"
                        ", _dbUsername: {}"
                        ", _dbName: {}",
                        postgresConnection->getConnectionId(), _dbServer,
                        _dbUsername, _dbName);
        SPDLOG_ERROR(errorMessage);
#endif

        return nullptr;
      } else {
#ifdef DBCONNECTIONPOOL_LOG
        SPDLOG_DEBUG("just created sql connection"
                     ", _connectionId: {}"
                     ", _dbServer: {}"
                     ", _dbUsername: {}"
                     ", _dbName: {}",
                     postgresConnection->getConnectionId(), _dbServer,
                     _dbUsername, _dbName);
#endif
      }

      return static_pointer_cast<DBConnection>(postgresConnection);
    } catch (runtime_error &e) {
#ifdef DBCONNECTIONPOOL_LOG
      SPDLOG_ERROR("sql connection creation failed"
                   ", e.what(): {}",
                   e.what());
#endif

      throw e;
    } catch (exception &e) {
#ifdef DBCONNECTIONPOOL_LOG
      SPDLOG_ERROR("sql connection creation failed"
                   ", e.what(): {}",
                   e.what());
#endif

      throw e;
    }
  };
};

#endif
