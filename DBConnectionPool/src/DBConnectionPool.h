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


/* ConnectionPool manages a connection pool of some kind.  Worker threads can ask for a connection, and must return it when done.
 * Each connection is guaranteed to be healthy, happy, and free of disease.
 *
 * Connection and ConnectionFactory are virtual classes that should be overridden to their actual type.
 *
 * NOTE: To avoid using templates AND inheritance at the same time in the ConnectionFactory, ConnectionFactory::create must create a derved type 
 * but return the base class. 	
 */


// Define your custom logging function by overriding this #define
#ifndef DB_DEBUG_LOGGER
	#define DB_DEBUG_LOGGER(x)
#endif
#ifndef DB_ERROR_LOGGER
	#define DB_ERROR_LOGGER(x)
#endif



#include <deque>
#include <set>
#include <memory>
#include <mutex>
#include <exception>
#include <string>
#include <libgen.h>

#ifndef __FILEREF__
    #ifdef __APPLE__
        #define __FILEREF__ string("[") + string(__FILE__).substr(string(__FILE__).find_last_of("/") + 1) + ":" + to_string(__LINE__) + "] "
    #else
        #define __FILEREF__ string("[") + basename((char *) __FILE__) + ":" + to_string(__LINE__) + "] "
    #endif
#endif

using namespace std;

struct ConnectionUnavailable : exception 
{ 
    char const* what() const throw() 
    {
        return "Unable to allocate connection";
    }; 
};

class DBConnection {

protected:
	string _selectTestingConnection;
	int _connectionId;

public:
    DBConnection()
	{
		_selectTestingConnection	= "";
		_connectionId				= -1;
	};
    DBConnection(string selectTestingConnection, int connectionId)
	{
		_selectTestingConnection	= selectTestingConnection;
		_connectionId				= connectionId;
	};
    virtual ~DBConnection(){};

	int getConnectionId()
	{
		return _connectionId;
	}

    virtual bool connectionValid()
    {
		return true;
    };
};

class DBConnectionFactory {

public:
    virtual shared_ptr<DBConnection> create(int connectionId)=0;
};

struct DBConnectionPoolStats 
{
    size_t _poolSize;
    size_t _borrowedSize;
};

template<class T>
class DBConnectionPool {

protected:
    shared_ptr<DBConnectionFactory>       _factory;
    size_t                              _poolSize;
    deque<shared_ptr<DBConnection> >      _connectionPool;
    set<shared_ptr<DBConnection> >        _connectionBorrowed;
    mutex _connectionPoolMutex;

public:

    DBConnectionPool(size_t poolSize, shared_ptr<DBConnectionFactory> factory)
    {
        _poolSize=poolSize;
        _factory=factory;

		int lastConnectionId = 0;

        // Fill the pool
        while(_connectionPool.size() < _poolSize)
        {
			shared_ptr<DBConnection> sqlConnection = _factory->create(lastConnectionId++);
			if (sqlConnection != nullptr)
				_connectionPool.push_back(sqlConnection);
        }
    };

    ~DBConnectionPool() 
    {
    };

    /**
     * Borrow
     *
     * Borrow a connection for temporary use
     *
     * When done, either (a) call unborrow() to return it, or (b) (if it's bad) just let it go out of scope.  This will cause it to automatically be replaced.
     * @retval a shared_ptr to the connection object
     */
    shared_ptr<T> borrow()
    {
        lock_guard<mutex> locker(_connectionPoolMutex);

		// Check for a free connection
        if(_connectionPool.size()==0)
        {
			DB_DEBUG_LOGGER(__FILEREF__ + "_connectionPool.size is 0, look to recover a borrowed one");

            // Are there any crashed connections listed as "borrowed"?
            for(set<shared_ptr<DBConnection> >::iterator it = _connectionBorrowed.begin(); it != _connectionBorrowed.end(); ++it)
            {
                // generally use_count is 2, one because of borrowed set and one because it is used for sql statements.
                // If it is 1, it means this connection has been abandoned
                if((*it).use_count() == 1)   
                {
                    // Destroy it and create a new connection
                    try 
                    {
                        // If we are able to create a new connection, return it
						DB_DEBUG_LOGGER(__FILEREF__ + "Creating new connection to replace discarded connection");

						int connectionId = (*it)->getConnectionId();

                        shared_ptr<DBConnection> sqlConnection=_factory->create(connectionId);
						if (sqlConnection == nullptr)
						{
							DB_ERROR_LOGGER(__FILEREF__ + "sqlConnection is null");

							throw std::exception();
						}

                        _connectionBorrowed.erase(it);
                        _connectionBorrowed.insert(sqlConnection);

                        return static_pointer_cast<T>(sqlConnection);
                    } 
                    catch(std::exception& e) 
                    {
						DB_ERROR_LOGGER(__FILEREF__ + "exception");

                        // Error creating a replacement connection
                        throw ConnectionUnavailable();
                    }
                }
            }

			DB_ERROR_LOGGER(__FILEREF__ + "No connection available");

            // Nothing available
            throw ConnectionUnavailable();
        }

        // Take one off the front
        shared_ptr<DBConnection> sqlConnection = _connectionPool.front();
		/*
		if (sqlConnection->getConnectionId() == 0)
		{
			_connectionPool.pop_front();
			_connectionPool.push_back(sqlConnection);

			sqlConnection = _connectionPool.front();
		}
		*/

		// shared_ptr<T> customSqlConnection = static_pointer_cast<T>(sqlConnection);
		bool connectionValid = sqlConnection->connectionValid();
		if (!connectionValid)
		{
			DB_ERROR_LOGGER(__FILEREF__ + "sqlConnection is null or is not valid"
					", connectionValid: " + to_string(connectionValid)
					);

			int connectionId = sqlConnection->getConnectionId();

			// we will create a new connection. The previous connection will be deleted by the shared_ptr
			sqlConnection=_factory->create(connectionId);
			if (sqlConnection == nullptr)
			{
				DB_ERROR_LOGGER(__FILEREF__ + "sqlConnection is null");

				throw std::exception();
			}
		}

        _connectionPool.pop_front();
       	// Add it to the borrowed list
       	_connectionBorrowed.insert(sqlConnection);

		DB_DEBUG_LOGGER(__FILEREF__ + "borrow"
				+ ", connectionId: " + to_string(sqlConnection->getConnectionId())
				);

        return static_pointer_cast<T>(sqlConnection);
    };

    /**
     * Unborrow a connection
     *
     * Only call this if you are returning a working connection.  If the connection was bad, just let it go out of scope (so the connection manager can replace it).
     * @param the connection
     */
    void unborrow(shared_ptr<T> sqlConnection) 
    {
		if (sqlConnection == nullptr)
		{
			DB_ERROR_LOGGER(__FILEREF__ + "sqlConnection is null");

			throw std::exception();
		}

        lock_guard<mutex> locker(_connectionPoolMutex);

        // Push onto the pool
        _connectionPool.push_back(static_pointer_cast<DBConnection>(sqlConnection));

        // Unborrow
        _connectionBorrowed.erase(sqlConnection);

		DB_DEBUG_LOGGER(__FILEREF__ + "unborrow"
				+ ", connectionId: " + to_string(sqlConnection->getConnectionId())
				);
    };

    DBConnectionPoolStats get_stats() 
    {
        lock_guard<mutex> locker(_connectionPoolMutex);

        // Get stats
        DBConnectionPoolStats stats;
        stats._poolSize=_connectionPool.size();
        stats._borrowedSize=_connectionBorrowed.size();			

        return stats;
    };
};
