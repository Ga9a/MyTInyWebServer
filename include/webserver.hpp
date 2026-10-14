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
    // Singleton instance.
    static WebServer* _ws;

    // Restrict construction to the singleton accessor.
    WebServer(int port,int sql_max_conn,const char* sql_host,const char* sql_user,
    const char* sql_password,const char* sql_database,int sql_port,
    const int max_thread_num = 10,const int max_deque_length = 1024);
    ~WebServer();

    // Disable copying.
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
    bool _InitListenSocket(); // Initialize the listening socket.
    int _SetNoBlock(int &fd); // Enable non-blocking I/O.
    void _HandleNewConnection(); // Accept new connections.
    void _AddConnection(int client_fd, sockaddr_in &client_addr); // Register a client connection.
    void _CloseConnection(HttpConnector *client_connector); // Close the connection.
    void _Process(HttpConnector *client_connector);// Update epoll interest after request processing.
    void _HandleRead(HttpConnector *client_connector); // Handle readable events.
    void _FunRead(HttpConnector *client_connector); // Worker callback.
    void _HandleWrite(HttpConnector *client_connector); // Handle readable events.
    void _FunWrite(HttpConnector *client_connector); // Worker callback.


public:
    void Start();// Start the server.


private:
    int _port;
    std::atomic<bool> _statue; // Server state.
    int _epoll_wait_MS; // epoll wait timeout in milliseconds.
    int _listen_fd; // Listening socket.
    uint32_t _listen_events; // Listening socket events.
    uint32_t _client_events; // Client socket events.

    static const uint16_t MAX_CLIENTS = 65535; // Maximum active clients.


    std::unique_ptr<Epoller> _epoller; // Epoll instance.
    std::unique_ptr<ThreadPool> _threadpool; // Worker pool.
    std::unordered_map<int, HttpConnector> _clients; // Client connections.


};


