#include "webserver.hpp"

#define LISTEN_CLOSE(ret,msg) \
    do { \
        if ((ret) < 0) { \
            close(_listen_fd); \
            LOG_FATAL(msg); \
            return false; \
        } \
    } while (0);

// Define the static singleton.
WebServer* WebServer::_ws = nullptr;

WebServer::WebServer(int port,int pool_max_conn,const char* pool_host,const char* pool_user,
                    const char* pool_password,const char* pool_database,int pool_port,
                    const int max_thread_num,const int max_deque_length)
{
    _port = port;
    _statue.store(true); // Start with the server enabled.
    _epoll_wait_MS = -1; // Wait indefinitely for events.
    _listen_events =  EPOLLRDHUP | EPOLLET; // Detect peer closure and use edge-triggered events.
    _client_events =  EPOLLONESHOT | EPOLLRDHUP | EPOLLET; // Use one-shot edge-triggered client events and rearm after processing.

    HttpConnector::user_cnt=0;

    auto log = Log::GetInstance();
    log->Init(max_deque_length); // Initialize logging.

    LOG_INFO("Log Init Success.");

    auto sqlpool = SQLPool::GetInstance();
    sqlpool->Init(pool_host,pool_user,pool_password,pool_database,pool_port,pool_max_conn);// Initialize the database pool.

    unique_ptr<Epoller> NewEpolloer = std::make_unique<Epoller>();
    _epoller = std::move(NewEpolloer); // Epoll instance.

    unique_ptr<ThreadPool> NewThreadPool = std::make_unique<ThreadPool>(max_thread_num);
    _threadpool = std::move(NewThreadPool); // Worker pool.


    if(!_InitListenSocket())
    {
        LOG_FATAL("InitListenSocket error!");
        _statue = false;
        throw std::runtime_error("Failed to initialize the listening socket");
    }
    // int id;
    // MYSQL* m;
    // LOG_INFO("_sqlpool free conn:%d",_sqlpool->GetFreeConn());
    // m=_sqlpool->GetConn(id);
    // LOG_INFO("_sqlpool free conn:%d",_sqlpool->GetFreeConn());
    // _sqlpool->ReleaseConn(id,m);

}

bool WebServer::_InitListenSocket()
{
    int ret=0;
    struct sockaddr_in listen_addr{};
    if( _port > 65535 || _port < 1024) {
        LOG_FATAL("Port: %d error!",  _port);
        return false;
    }
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(_port);

    //Configure socket linger.
    struct linger Linger;
    Linger.l_onoff = 1;
    Linger.l_linger = 2;

    //Create the listening socket.
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    LISTEN_CLOSE(_listen_fd,"socket init error!")

    //Apply socket linger.
    ret = setsockopt(_listen_fd, SOL_SOCKET, SO_LINGER, &Linger, sizeof(Linger));
    LISTEN_CLOSE(ret,"linger set error!")

    int optval = 1;
    //Allow address reuse.
    ret = setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    LISTEN_CLOSE(ret,"set port reuse error!")

    //Bind the listening address.
    ret = ::bind(_listen_fd, (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    LISTEN_CLOSE(ret,"bind error!")

    //Listen for clients.
    ret = listen(_listen_fd, 5);
    LISTEN_CLOSE(ret,"listen error!")

    //Register the listener with epoll.
    if (!_epoller->addFd(_listen_fd, _listen_events | EPOLLIN)) {
        close(_listen_fd);
        LOG_FATAL("epoll add listen fd error!");
        return false;
    }

    //Enable non-blocking I/O.
    _SetNoBlock(_listen_fd);
    //LOG_INFO("_listen_fd noblock set success!, _listen_fd = %d, NOBLOCK FLAG = %d" , _listen_fd,(fcntl(_listen_fd,F_GETFL) & O_NONBLOCK));
    LOG_INFO("listen_fd = %d, port = %d, init success.", _listen_fd, _port);

    return true;
}

void WebServer::_HandleNewConnection() // Accept new connections.
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    //Drain pending connections for edge-triggered notification.
    while(true)
    {
        int client_fd = accept(_listen_fd,(struct sockaddr*)&client_addr, &client_addr_len);
        if(client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // All pending connections have been accepted.
            return;
        } else {
            LOG_FATAL("accept error: %s",strerror(errno));
            return;
        }
        }
        else if(HttpConnector::user_cnt>=MAX_CLIENTS)
        {
            LOG_WARNING("clients are full!");
            close(client_fd);
            continue;
        }
        _AddConnection(client_fd,client_addr);
    }
}

void WebServer::_AddConnection(int client_fd, sockaddr_in &client_addr)
{
    assert(client_fd>=0);
    _clients[client_fd].Init(client_fd,client_addr);
    _epoller->addFd(client_fd, _client_events | EPOLLIN);
    _SetNoBlock(client_fd);
    LOG_INFO("============Client[%d] in!============", _clients[client_fd].GetFd());
}

void WebServer::_CloseConnection(HttpConnector *client_connector) // Close the connection.
{
    assert(client_connector);
    _epoller->delFd(client_connector->GetFd());
    LOG_INFO("============Client[%d] out!============", client_connector->GetFd());
    client_connector->Close();
}

void  WebServer::_Process(HttpConnector *client_connector)//Process the request.
{
    //Rearm EPOLLONESHOT after processing.
    if(client_connector->Process()){
        _epoller->modFd(client_connector->GetFd(), _client_events | EPOLLOUT);
    }else{
        _epoller->modFd(client_connector->GetFd(), _client_events | EPOLLIN);
    }
}

void  WebServer::_FunRead(HttpConnector *client_connector)
{
    LOG_INFO("_FunRead process start.")
    assert(client_connector);
    bool status = false;
    //Read into the connection buffer and check the result.
    auto ret = client_connector->Read(status);
    if(ret <= 0 && !status)
    {
        LOG_WARNING("read error, close connection");
        _CloseConnection(client_connector);
        return;
    }
    //Process the successfully received request data.
    try { _Process(client_connector); }
    catch (const std::exception& error) {
        LOG_WARNING("Request failed: %s", error.what());
        _CloseConnection(client_connector);
    }
    LOG_INFO("_FunRead ends.")
}

void WebServer::_HandleRead(HttpConnector *client_connector) // Schedule a read worker.
{
    assert(client_connector);
    //LOG_INFO("handle read process.")
    _threadpool->AddTask(std::bind(&WebServer::_FunRead, this,client_connector));
    //_threadpool->AddTask([](){LOG_INFO("my mission fullfiled.");});
}

void  WebServer::_FunWrite(HttpConnector *client_connector)//Write pending response bytes.
{
    assert(client_connector);
    bool status = true;
    long ret = -1;
    ret = client_connector->Write(status);
    if(!client_connector->Writeable_Size()) // Response transmission completed.
    {
        _Process(client_connector);
    }
    else if(ret < 0)
    {
        //Rearm writable notification when the socket would block.
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            _epoller->modFd(client_connector->GetFd(), _client_events | EPOLLOUT);
        }
        else
        {
            LOG_WARNING("write error, close connection");
            _CloseConnection(client_connector);
        }
    }
    //Connection close handling.
    //_CloseConnection(client_connector);
}

void WebServer::_HandleWrite(HttpConnector *client_connector)//Schedule a write worker.
{
    assert(client_connector);
    _threadpool->AddTask(std::bind(&WebServer::_FunWrite, this,client_connector));
}

void WebServer::Start()//Main event loop.
{
    if(_statue.load())
        LOG_INFO("===============WebServer Start===============");
    //Wait for events.

    while(_statue.load())
    {
        //LOG_INFO("Listening ......")
        int event_cnt = _epoller->wait(_epoll_wait_MS);
        for(int i=0;i<event_cnt;i++)
        {
            auto fd=_epoller->GetFd(i);
            auto events=_epoller->GetEvents(i);
            if(fd == _listen_fd)
            {
                _HandleNewConnection(); // Accept new connections.
            }
            else if(events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
            {
                assert(_clients.count(fd) > 0);
                _CloseConnection(&_clients[fd]); // Close the client.
            }
            else if(events & EPOLLIN)
            {
                //Readable event.
                assert(_clients.count(fd) > 0);
                _HandleRead(&_clients[fd]);
            }
            else if(events & EPOLLOUT)
            {
                //Writable event.
                assert(_clients.count(fd) > 0);
                _HandleWrite(&_clients[fd]);
            }
        }
    }
}

int WebServer::_SetNoBlock(int &fd)
{
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

WebServer::~WebServer()
{
};
