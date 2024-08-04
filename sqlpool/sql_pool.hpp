#pragma once
#include "../include/include.hpp"
#include "../outheader/mysql.h"

using std::condition_variable;
using std::mutex;

class SQLPool
{
    //单例模式
    private:
        //禁用拷贝和“=”复制
        SQLPool(const SQLPool& sq)= delete;
        SQLPool& operator=(const SQLPool& sq)= delete;

        //私有化构造析构函数
        SQLPool();
        ~SQLPool();
    
    public:
        //获取单例对象,懒汉模式局部不需要加锁
        static SQLPool * GetInstance()
        {
            static SQLPool SQLp;
            return &SQLp;
        }

        void Init(const char* host, const char* user, const char* passwd, const char* db, int port, int max_size);
        int GetFreeConn(){return _free_conn;};
        MYSQL* GetConn(int &id); //获取连接
        void ReleaseConn(int& id,MYSQL *mysql); //释放连接

    private:

        //mysql_real_connect(&mysql,host,user,password,db,3306,0,0)
        // string _host;
        // string _user;
        // string _password;
        // string _db;
        // int _port;
        int _max_conn;
        int _free_conn;

        condition_variable _CVpool;//条件变量
        mutex _mutex;//互斥锁
        std::list<std::pair<int,MYSQL *>> _conn_list; //连接池
        std::set<int> used_id;//被使用的mysql序号，防止放入自定义的MYSQL 

};