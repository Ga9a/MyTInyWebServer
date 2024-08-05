#pragma once
#include "../include/include.hpp"

class Buffer
{
    public:
    Buffer(size_t buff_size = 1024);
    ~Buffer() = default;

    ssize_t Read_FD(int fd,bool& status);

    //test
    void print()
    {
    //std::string str(_buffer.begin(), _buffer.end()-1);
    std::string str(Begin_Read_Ptr(),Readable_Size());
    if (!str.empty() && str.back() == '\n') {
        str.pop_back();
    }
    LOG_INFO("recieved data: %s", str.c_str());
    }

    public:
    size_t Readable_Size() const; //返回可读的大小
    void Update_Writep(size_t len);//更新写入指针
    void Update_Readp(size_t len);//更新读取指针
    void Update_Readp(const char* end);//更新读取指针
    char* Begin_Read_Ptr(); //返回指向可读部分的指针
    char* Begin_Write_Ptr();//返回指向可写入部分的指针

    //释放缓冲区
    void Release_All_Buff();
    void Release_All_Buff(string &str);
    void Release_Buff(size_t len);

    void Add(const string& str);//向缓冲区添加字符串
    void Add(const char* str, size_t len);//向缓冲区添加字符串

    private:
    char* _Begin_Ptr(); //返回指向缓冲区起始位置的指针
    size_t _Writeable_Size() const; //返回可写入的大小
    size_t _Prepared_Size() const; //返回已经被读写过的空间，即预备好可以用作写入的部分
    void _Dilatation(size_t len); //扩展缓冲区大小
    void _Length_Guard(size_t len); //检查缓冲区长度是否足够,并调整
    
    

    private:
    std::vector<char> _buffer;
    std::atomic<std::size_t> _readp;
    std::atomic<std::size_t> _writep;
};