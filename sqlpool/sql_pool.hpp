#pragma once
#include "../include/include.hpp"
#include <mysql.h>

using std::condition_variable;
using std::mutex;

class SQLPool
{
    //Singleton.
    private:
        //Disable copying and assignment.
        SQLPool(const SQLPool& sq)= delete;
        SQLPool& operator=(const SQLPool& sq)= delete;

        //Restrict construction and destruction.
        SQLPool();
        ~SQLPool();

    public:
        //Function-local static initialization is thread safe.
        static SQLPool * GetInstance()
        {
            static SQLPool SQLp;
            return &SQLp;
        }

        void Init(const char* host, const char* user, const char* passwd, const char* db, int port, int max_size);
        MYSQL* GetConn(int &id); //Acquire a database connection.
        void ReleaseConn(int& id,MYSQL *mysql); //Release a database connection.

    private:

        //mysql_real_connect(&mysql,host,user,password,db,3306,0,0)
        // string _host;
        // string _user;
        // string _password;
        // string _db;
        // int _port;
        int _max_conn;
        int _free_conn;

        condition_variable _CVpool;//Availability condition.
        mutex _mutex;//Pool mutex.
        std::list<std::pair<int,MYSQL *>> _conn_list; //Database connection pool.
        std::set<int> used_id;//Track borrowed connection IDs to reject foreign handles.

};
