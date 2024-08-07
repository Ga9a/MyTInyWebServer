#include "../sqlpool/sql_pool.hpp"

unsigned int timeout = 5; // 设置连接超时时间为5秒

SQLPool::SQLPool()
{};

SQLPool::~SQLPool()
{
    mysql_library_end();
    for(auto i:_conn_list)
    {
        mysql_close(i.second);
    }
};

MYSQL* SQLPool::GetConn(int &id)
{
    std::unique_lock<std::mutex> lock(_mutex);
    while(0==_free_conn)
    {
        _CVpool.wait(lock);
    }
    auto item= _conn_list.front();
    MYSQL *mysql = item.second;
    id=item.first;
    used_id.insert(id);
    _conn_list.pop_front();
    _free_conn--;
    LOG_INFO("get conn id = %d.",id)
    return mysql;    
}

void SQLPool::ReleaseConn(int& id,MYSQL *mysql)
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(used_id.find(id)==used_id.end())
    {
        LOG_FATAL("unknown conn id = %d.",id)
        exit(1);
    }
    else
    {
        _conn_list.emplace_back(id,mysql);
        LOG_INFO("release conn id = %d.",id)
        _free_conn++;
        used_id.erase(id);
        mysql=nullptr;
        id=-1;
        _CVpool.notify_one();
    }
}

void SQLPool::Init(const char* host, const char* user, const char* passwd, const char* db, int port, int max_size) {
    _max_conn = max_size;
    _free_conn = max_size;

    if (mysql_library_init(0, 0, 0) != 0) {
        LOG_FATAL("mysql library init fail.");
        throw std::runtime_error("mysql library init fail");
    }

    std::unique_lock<std::mutex> lock(_mutex);

    for (int i = 0; i < max_size; ++i) {
        MYSQL* mysql = mysql_init(nullptr);
        if (mysql == nullptr) {
            LOG_FATAL("mysql init error.");
            throw std::runtime_error("mysql init error");
        }


        mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
        if (!mysql_real_connect(mysql, host, user, passwd, db, port, nullptr, 0)) {
            LOG_FATAL("mysql connect error: %s", mysql_error(mysql));
            mysql_close(mysql); // 关闭连接以释放资源
            throw std::runtime_error("mysql connect error");
        }

        _conn_list.emplace_back(i, mysql);
    }

    LOG_INFO("SQLPool Init Success.");
}

