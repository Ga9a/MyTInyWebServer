#include "webserver.hpp"

#define LISTEN_CLOSE(ret,msg) \
    do { \
        if ((ret) < 0) { \
            close(_listen_fd); \
            LOG_FATAL(msg); \
            return false; \
        } \
    } while (0);

// 类外初始化静态成员
WebServer* WebServer::_ws = nullptr;

WebServer::WebServer(int port,int pool_max_conn,const char* pool_host,const char* pool_user,
                    const char* pool_password,const char* pool_database,int pool_port,
                    const int max_thread_num,const int max_deque_length)
{
    _port = port;
    _statue.store(true); // 服务器默认开启
    _epoll_wait_MS = -1; // epoll_wait 毫秒数
    _listen_events =  EPOLLRDHUP | EPOLLET; // 监听事件,检测对端关闭事件和设置边缘触发模式
    _client_events =  EPOLLONESHOT | EPOLLRDHUP | EPOLLET; // 监听事件,检测对端关闭事件和设置边缘触发模式,只读取一次事件后重新注册

    HttpConnector::user_cnt=0;

    auto log = Log::GetInstance();
    log->Init(max_deque_length); // 初始化日志

    LOG_INFO("Log Init Success.");

    auto sqlpool = SQLPool::GetInstance();
    sqlpool->Init(pool_host,pool_user,pool_password,pool_database,pool_port,pool_max_conn);// 初始化数据库连接池

    unique_ptr<Epoller> NewEpolloer = std::make_unique<Epoller>();
    _epoller = move(NewEpolloer); // epoll对象

    unique_ptr<ThreadPool> NewThreadPool = std::make_unique<ThreadPool>(max_thread_num);
    _threadpool = move(NewThreadPool); // 线程池对象


    if(!_InitListenSocket())
    {
        LOG_FATAL("InitListenSocket error!");
        _statue = false;
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
    struct sockaddr_in listen_addr;
    if( _port > 65535 || _port < 1024) {
        LOG_FATAL("Port: %d error!",  _port);
        return false;
    }
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_addr.sin_port = htons(_port);

    //设置优雅关闭连接
    struct linger Linger;
    Linger.l_onoff = 1;
    Linger.l_linger = 2;

    //初始化监听socket
    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    LISTEN_CLOSE(ret,"socket init error!")

    //设置优雅断开连接
    ret = setsockopt(_listen_fd, SOL_SOCKET, SO_LINGER, &Linger, sizeof(Linger));
    LISTEN_CLOSE(ret,"linger set error!")

    int optval = 1;
    //设置端口复用
    ret = setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    LISTEN_CLOSE(ret,"set port reuse error!")

    //绑定
    ret = bind(_listen_fd, (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    LISTEN_CLOSE(ret,"bind error!")

    //监听
    ret = listen(_listen_fd, 5);
    LISTEN_CLOSE(ret,"listen error!")

    //将监听socket加入epoll
    ret = _epoller->addFd(_listen_fd, _listen_events | EPOLLIN);
    LISTEN_CLOSE(ret,"epoll add listen fd error!")

    //设置非阻塞
    _SetNoBlock(_listen_fd);
    //LOG_INFO("_listen_fd noblock set success!, _listen_fd = %d, NOBLOCK FLAG = %d" , _listen_fd,(fcntl(_listen_fd,F_GETFL) & O_NONBLOCK));
    LOG_INFO("listen_fd = %d, port = %d, init success.", _listen_fd, _port);

    return true;
}

void WebServer::_HandleNewConnection() // 处理新连接
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    //由于是边缘触发所以要设置死循环来监听
    while(true)
    {
        int client_fd = accept(_listen_fd,(struct sockaddr*)&client_addr, &client_addr_len);
        if(client_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 所有连接已处理
            return;
        } else {
            LOG_FATAL("accept error: %s",strerror(errno));
            return;
        }
        } 
        else if(HttpConnector::user_cnt>=MAX_CLIENTS)
        {
            LOG_WARNING("clients are full!");
            return;
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

void WebServer::_CloseConnection(HttpConnector *client_connector) // 关闭连接
{
    assert(client_connector);
    _epoller->delFd(client_connector->GetFd());
    LOG_INFO("============Client[%d] out!============", client_connector->GetFd());
    client_connector->Close();
}

void  WebServer::_Process(HttpConnector *client_connector)//处理业务逻辑
{
    //由于设置了EPOll的EPOLLONESHOT,所以需要重新注册。
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
    //connector把数据读入它的缓冲区中，然后看状态
    auto ret = client_connector->Read(status);
    if(ret == 0 && !status)
    {
        LOG_WARNING("read error, close connection");
        _CloseConnection(client_connector);
        return;
    }
    //执行到这里说明读数据成功了，那么就把数据交给业务逻辑处理
    _Process(client_connector);
    LOG_INFO("_FunRead ends.")
}

void WebServer::_HandleRead(HttpConnector *client_connector) // 处理读请求
{
    assert(client_connector);
    //LOG_INFO("handle read process.")
    _threadpool->AddTask(std::bind(&WebServer::_FunRead, this,client_connector));
    //_threadpool->AddTask([](){LOG_INFO("my mission fullfiled.");});
}

void  WebServer::_FunWrite(HttpConnector *client_connector)//处理写任务
{
    assert(client_connector);
    bool status = true;
    long ret = -1;
    ret = client_connector->Write(status);
    if(!client_connector->Writeable_Size()) // 传输完成
    {
        _Process(client_connector);
    }
    else if(ret < 0)
    {
        //在写入操作中，如果不能立即写入所有数据且操作设置为非阻塞模式，则返回 EWOULDBLOCK。
        if(errno == EAGAIN || errno == EWOULDBLOCK)
        {
            _epoller->modFd(client_connector->GetFd(), _client_events | EPOLLOUT);
        }
        else
        {
            LOG_WARNING("write error, close connection");
        }
    }
    //单次http连接是否需要中断
    //_CloseConnection(client_connector);
}

void WebServer::_HandleWrite(HttpConnector *client_connector)//处理写请求
{
    assert(client_connector);
    _threadpool->AddTask(std::bind(&WebServer::_FunWrite, this,client_connector));
}   

void WebServer::Start()//主线程
{
    if(_statue.load())
        LOG_INFO("===============WebServer Start===============");
    //持续监听
    
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
                _HandleNewConnection(); // 处理新连接
            }
            else if(events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR))
            {
                assert(_clients.count(fd) > 0);
                _CloseConnection(&_clients[fd]); // 连接关闭
            }
            else if(events & EPOLLIN)
            {
                //读请求
                assert(_clients.count(fd) > 0);
                _HandleRead(&_clients[fd]);
            }
            else if(events & EPOLLOUT)
            {
                //写请求
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