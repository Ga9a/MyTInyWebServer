#pragma once
#include "include.hpp"
#include "../log/log.hpp"
#include "../sqlpool/sql_pool.hpp"
#include "epoller.hpp"
#include "threadpool.hpp"
#include "../httpconnector/httpconnector.hpp"

using std::mutex;
using std::lock_guard;

class WebServer
{
private:
    // 单例实例
    static WebServer* _ws;

    // 受保护的构造函数禁止外界创建
    WebServer(int port,int sql_max_conn,const char* sql_host,const char* sql_user,
    const char* sql_password,const char* sql_database,int sql_port,
    const int max_thread_num = 10,const int max_deque_length = 1024);
    ~WebServer();

    // 私有化拷贝构造和赋值运算符
    WebServer(const WebServer& ws) = delete;
    WebServer& operator=(const WebServer& ws) = delete;

public:
    static WebServer* GetInstance(int port,int pool_max_conn,const char* pool_host,const char* pool_user,const char* pool_password,const char* pool_database,int sql_port)
    {
        static mutex mtx;
        if (_ws == nullptr)
        {
            lock_guard<mutex> lock(mtx);
            if (_ws == nullptr)
            {
                _ws = new WebServer(port,pool_max_conn,pool_host,pool_user,pool_password,pool_database,sql_port);
            }
        }
        return _ws;
    }
private:
    bool _InitListenSocket(); // 初始化监听套接字
    int _SetNoBlock(int &fd); // 设置套接字为非阻塞
    void _HandleNewConnection(); // 处理新连接
    void _AddConnection(int client_fd, sockaddr_in &client_addr); // 添加客户端
    void _CloseConnection(HttpConnector *client_connector); // 关闭连接
    void _Process(HttpConnector *client_connector);// 数据处理完成后的业务逻辑
    void _HandleRead(HttpConnector *client_connector); // 处理读事件
    void _FunRead(HttpConnector *client_connector); // 读事件回调函数
    void _HandleWrite(HttpConnector *client_connector); // 处理读事件
    void _FunWrite(HttpConnector *client_connector); // 读事件回调函数
    

public:
    void Start();// 启动服务器


private:
    int _port;
    std::atomic<bool> _statue; // 服务器状态
    int _epoll_wait_MS; // epoll_wait等待时间
    int _listen_fd; // 监听套接字
    uint32_t _listen_events; // 监听套接字事件
    uint32_t _client_events; // 客户端套接字事件

    static const uint16_t MAX_CLIENTS = 65535; // 最大客户端数


    std::unique_ptr<Epoller> _epoller; // epoll对象
    std::unique_ptr<ThreadPool> _threadpool; // 线程池对象
    std::unordered_map<int, HttpConnector> _clients; // 客户端连接集合

    size_t _pool_max_conn; // 最大连接数
    char* _pool_host; // 数据库ip
    char* _pool_user; // 数据库用户名
    char* _pool_password; // 数据库密码
    char* _pool_database; // 数据库名称
    int _pool_port; // 数据库端口

};


