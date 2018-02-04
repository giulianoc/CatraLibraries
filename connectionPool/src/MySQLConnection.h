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
 #include "ConnectionPool.h"
#include <string>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>


class MySQLConnection : public Connection {

public:
    shared_ptr<sql::Connection> _sqlConnection;

    ~MySQLConnection() 
    {
        if(_sqlConnection) 
        {
            _DEBUG("MYSQL Destruct");

            _sqlConnection->close();
            _sqlConnection.reset(); 	// Release and destruct
        }
    };
};


class MySQLConnectionFactory : public ConnectionFactory {

private:
    string _dbServer;
    string _dbUsername;
    string _dbPassword;
    string _dbName;

public:
    MySQLConnectionFactory(string dbServer, string dbUsername, string dbPassword, string dbName) 
    {
        _dbServer = dbServer;
        _dbUsername = dbUsername;
        _dbPassword = dbPassword;
        _dbName = dbName;
    };

    // Any exceptions thrown here should be caught elsewhere
    shared_ptr<Connection> create() {

        sql::Driver *driver;
        driver = get_driver_instance();

        // server like "tcp://127.0.0.1:3306"
        shared_ptr<sql::Connection> connectionFromDriver (driver->connect(_dbServer, _dbUsername, _dbPassword));
        connectionFromDriver->setSchema(_dbName);

        shared_ptr<MySQLConnection>     mySqlConnection = make_shared<MySQLConnection>();
        mySqlConnection->_sqlConnection = connectionFromDriver;

        return static_pointer_cast<Connection>(mySqlConnection);
    };

};
