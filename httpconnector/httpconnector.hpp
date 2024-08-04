#pragma once
#include "../include/include.hpp"
#include "../buffer/buffer.hpp"

class HttpConnector
{
    public:
        HttpConnector();
        ~HttpConnector();

        void Init(int sock_fd, const sockaddr_in &addr);
        ssize_t Read(bool& status);//参数为返回的状态码
        ssize_t Write(bool& status);//参数为返回的状态码
        int Writeable_Size(){return _iov[0].iov_len+_iov[1].iov_len;}

        void Close();
        bool Process();//业务逻辑处理
        static std::atomic<uint16_t> user_cnt;//当前连接器上的用户数量

        //为静态函数提供接口
        int GetFd() const{return _fd;};
        sockaddr_in GetAddr() const{return _addr;};
        const char* GetIp() const{return inet_ntoa(_addr.sin_addr);};
        int GetPort() const{return ntohs(_addr.sin_port);};

        

    private:
        int _fd;
        sockaddr_in _addr;
        bool _status;//连接器状态

        Buffer _read_buffer;//读缓冲区
        Buffer _write_buffer;//写缓冲区
        iovec _iov[2];//写缓冲区辅助
        size_t _iov_size;//写缓冲区辅助大小


};