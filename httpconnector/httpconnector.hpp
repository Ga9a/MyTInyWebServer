#pragma once
#include "../include/include.hpp"
#include "../buffer/buffer.hpp"
#include "httprequest.hpp"
#include "httpresponse.hpp"

class HttpConnector
{
    public:
        HttpConnector();
        ~HttpConnector();

        void Init(int sock_fd, const sockaddr_in &addr);
        ssize_t Read(bool& status);//Returns the I/O status through the reference parameter.
        ssize_t Write(bool& status);//Returns the I/O status through the reference parameter.
        size_t Writeable_Size(){return _iov[0].iov_len+_iov[1].iov_len;}

        void Close();
        bool Process();//Process the HTTP request.
        void ClearWriteBuffer(){_write_buffer.Release_All_Buff();}

        static std::atomic<uint16_t> user_cnt;//Number of active connections.

        //Connection accessors.
        int GetFd() const{return _fd;};
        sockaddr_in GetAddr() const{return _addr;};
        const char* GetIp() const{return inet_ntoa(_addr.sin_addr);};
        int GetPort() const{return ntohs(_addr.sin_port);};
        std::shared_ptr<char> ReadFileToSharedCharArray(const std::string& filePath, std::size_t& fileSize);



    private:
        int _fd;
        sockaddr_in _addr;
        bool _status;//Connection state.

        Buffer _read_buffer;//Read buffer.
        Buffer _write_buffer;//Write buffer.
        iovec _iov[2];//Scatter/gather output buffers.
        std::shared_ptr<char> _file_content;
        size_t _iov_size;//Number of output buffers.

        HttpRequest _request;//Request.
        HttpResponse _reponse;//Response.


};
