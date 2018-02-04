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
#ifndef _DEBUG
	#define _DEBUG(x)
#endif



#include <deque>
#include <set>
#include <memory>
#include <mutex>
#include <exception>
#include <string>

using namespace std;

struct ConnectionUnavailable : std::exception 
{ 
    char const* what() const throw() 
    {
        return "Unable to allocate connection";
    }; 
};

class Connection {

public:
    Connection(){};
    virtual ~Connection(){};

};

class ConnectionFactory {

public:
    virtual shared_ptr<Connection> create()=0;
};

struct ConnectionPoolStats 
{
    size_t _poolSize;
    size_t _borrowedSize;
};

template<class T>
class ConnectionPool {

protected:
    shared_ptr<ConnectionFactory>       _factory;
    size_t                              _poolSize;
    deque<shared_ptr<Connection> >      _connectionPool;
    set<shared_ptr<Connection> >        _connectionBorrowed;
    mutex _connectionPoolMutex;

public:

    ConnectionPool(size_t poolSize, shared_ptr<ConnectionFactory> factory)
    {
        _poolSize=poolSize;
        _factory=factory;

        // Fill the pool
        while(_connectionPool.size() < _poolSize)
        {
            _connectionPool.push_back(_factory->create());
        }
    };

    ~ConnectionPool() 
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
            // Are there any crashed connections listed as "borrowed"?
            for(set<shared_ptr<Connection> >::iterator it = _connectionBorrowed.begin(); it != _connectionBorrowed.end(); ++it)
            {
                // generally use_count is 2, one because of borrowed set and one because it is used for sql statements.
                // If it is 1, it means this connection has been abandoned
                if((*it).use_count() == 1)   
                {
                    // Destroy it and create a new connection
                    try 
                    {
                        // If we are able to create a new connection, return it
                        _DEBUG("Creating new connection to replace discarded connection");

                        shared_ptr<Connection> sqlConnection=_factory->create();

                        _connectionBorrowed.erase(it);
                        _connectionBorrowed.insert(sqlConnection);

                        return static_pointer_cast<T>(sqlConnection);
                    } 
                    catch(std::exception& e) 
                    {
                        // Error creating a replacement connection
                        throw ConnectionUnavailable();
                    }
                }
            }

            // Nothing available
            throw ConnectionUnavailable();
        }

        // Take one off the front
        shared_ptr<Connection>sqlConnection = _connectionPool.front();
        _connectionPool.pop_front();

        // Add it to the borrowed list
        _connectionBorrowed.insert(sqlConnection);

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
        lock_guard<mutex> locker(_connectionPoolMutex);

        // Push onto the pool
        _connectionPool.push_back(static_pointer_cast<Connection>(sqlConnection));

        // Unborrow
        _connectionBorrowed.erase(sqlConnection);
    };

    ConnectionPoolStats get_stats() 
    {
        lock_guard<mutex> locker(_connectionPoolMutex);

        // Get stats
        ConnectionPoolStats stats;
        stats._poolSize=_connectionPool.size();
        stats._borrowedSize=_connectionBorrowed.size();			

        return stats;
    };
};
