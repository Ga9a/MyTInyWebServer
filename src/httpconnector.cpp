#include "../httpconnector/httpconnector.hpp"

//静态成员初始化
std::atomic<uint16_t> HttpConnector::user_cnt(0);

HttpConnector::HttpConnector()
{
    _status = false;
    _fd = -1;
    _addr = {0};
    memset(_iov, 0, sizeof(_iov));
    _iov_size = 0; 
}

HttpConnector::~HttpConnector()
{
    Close();
}

void HttpConnector::Init(int sock_fd, const sockaddr_in &addr)
{
    _fd = sock_fd;
    _addr = addr;
    ++user_cnt; // 用户数+1
    _status = true; // 开始运行
    LOG_INFO("Client[%d](%s:%d) visit, Usercnt = %d.",GetFd(),GetIp(),GetPort(),static_cast<int>(user_cnt))
    Buffer buffer1(10);

};

void HttpConnector::Close()//关闭
{
    if(_status)
    {
        _status=false;
        --user_cnt;
        _iov_size = 0;
        memset(_iov, 0, sizeof(_iov));
        close(_fd);
        LOG_INFO("Client[%d](%s:%d) left, Usercnt = %d.",GetFd(),GetIp(),GetPort(),static_cast<int>(user_cnt))
    }
}

ssize_t HttpConnector::Read(bool& status) // 读取数据
{
    ssize_t len = -1;
    while(1)
    {
        len = _read_buffer.Read_FD(_fd,status);
        if(len <= 0) break;
    }
    return len;
}

ssize_t HttpConnector::Write(bool& status) // 写入数据
{
    ssize_t len = -1;
    while(true)
    {
        len = writev(_fd,_iov,_iov_size);
        if(!len){
            status = false;
            return len;
        }

        if(!Writeable_Size()) break;
        //如果writev写入的字节数大于iov_[0]的长度，
        //说明iov_[0]中的数据已经全部写入，需要处理iov_[1]中的数据
        else if(len > static_cast<ssize_t>(_iov[0].iov_len))
        {
            _iov[1].iov_base = static_cast<uint8_t*>(_iov[1].iov_base) + (len - _iov[0].iov_len);
            _iov[1].iov_len -= (len - _iov[0].iov_len);
            if(_iov[0].iov_len) {
                _write_buffer.Release_All_Buff();
                _iov[0].iov_len = 0;
            }
        }
        //不然就是连Iov[0]都没读完，调整iov_[0]的长度
        else{
            _iov[0].iov_base = (uint8_t*) _iov[0].iov_base + len;
            _iov[0].iov_len -= len;
            _write_buffer.Release_Buff(len);
        }
        if(Writeable_Size()==0) break;
    }
    return len;
}

bool HttpConnector::Process() // 业务逻辑处理
{
    
    if(_read_buffer.Readable_Size() == 0) {
        return false;//没有可读数据，说明需要写
    }else{

        //test for http GET
        // _read_buffer.print();
        // string str;
        // _read_buffer.Release_All_Buff(str);
        // _write_buffer.Add(str);

        // char* head= "head:";

        // _iov[0].iov_base = head;
        // _iov[0].iov_len = strlen(head);
        // _iov[1].iov_base = _write_buffer.Begin_Read_Ptr();
        // _iov[1].iov_len = _write_buffer.Readable_Size();

        // _iov_size=2;
        //test end



        return true;//不然就是需要读
    } 
}