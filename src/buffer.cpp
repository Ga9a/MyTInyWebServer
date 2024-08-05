#include "../buffer/buffer.hpp"

// 构造函数，初始化_buffer，_readp，_writep
Buffer::Buffer(size_t buff_size) : _buffer(buff_size),_readp(0),_writep(0) {
}

// 返回指向_buffer.begin()的指针
char* Buffer::_Begin_Ptr()
{
    assert(_buffer.size() > 0);
    return &*_buffer.begin();
}; 

//更新写入指针
void Buffer::Update_Writep(size_t len)
{
    _writep += len;
}

//更新读指针
void Buffer::Update_Readp(size_t len)
{
    assert(len <= Readable_Size());
    _readp += len;
}

//返回指向可读部分的指针
char* Buffer::Begin_Read_Ptr()
{
    return _Begin_Ptr() + _readp;
}

//更新读指针
void Buffer::Update_Readp(const char* end)
{
    assert(Begin_Read_Ptr() <= end);
    Update_Readp(end-Begin_Read_Ptr());
}

// 返回可写入的大小
size_t Buffer::_Writeable_Size() const
{
    return _buffer.size() - _writep;
}

// 返回可读的大小
size_t Buffer::Readable_Size() const
{
    return _writep-_readp;
}

// 返回已准备好的大小
size_t Buffer::_Prepared_Size() const
{
    return  _readp;
}

//释放缓冲区中所有数据
void Buffer::Release_All_Buff()
{
    bzero(&_buffer[0], _buffer.size());
    _readp = 0;
    _writep = 0;
}

//释放缓冲区中所有数据
void Buffer::Release_All_Buff(string &str)
{
    str=string(Begin_Read_Ptr(),Readable_Size());
    Release_All_Buff();
}

void Buffer::Release_Buff(size_t len)
{
    Update_Readp(len);
}

// 返回指向可写入部分的指针
char* Buffer::Begin_Write_Ptr()
{
    return _Begin_Ptr() + _writep;
}

// 扩容函数，如果可写入的大小加上已准备好的大小大于等于要求长度，则不需要扩容，只需要调整大小；否则需要扩容
void Buffer::_Dilatation(size_t len)
{
    if(_Writeable_Size()+_Prepared_Size() >= len)
    {
        auto readable_size = Readable_Size();
        std::copy(_Begin_Ptr()+_readp,_Begin_Ptr()+_writep,_Begin_Ptr());
        _readp = 0;
        _writep = readable_size;
        assert(readable_size == Readable_Size());        
    }
    else
    {
        _buffer.resize(_writep+len);
    }
}

// 长度保护函数，如果可写入的大小小于要求长度，则调用扩容函数
void Buffer::_Length_Guard(size_t len)
{
    if(_Writeable_Size() < len)
        _Dilatation(len);
    assert(_Writeable_Size() >= len);
}

// 添加字符串到缓冲区
void  Buffer::Add(const string& str)
{
    //
    Add(str.c_str(),str.size());
}

// 添加字符数组到缓冲区
void  Buffer::Add(const char* str, size_t len)
{
    assert(str);
    _Length_Guard(len);
    std::copy(str,str+len,Begin_Write_Ptr());//将str的内容拷贝到缓冲区中
    _writep += len;
}

// 从文件描述符中读取数据到缓冲区
ssize_t Buffer::Read_FD(int fd,bool& status)
{
    char buff1[65535];
    char buff2[65535];
    iovec iov[3];
    const size_t writeable_size = _Writeable_Size();
    
    //分散读取提高效率

    iov[0].iov_base = _Begin_Ptr() + _writep;
    iov[0].iov_len = writeable_size;
    iov[1].iov_base = buff1;
    iov[1].iov_len = sizeof(buff1);
    iov[2].iov_base = buff2;
    iov[2].iov_len = sizeof(buff2);

    const ssize_t len = readv(fd,iov,3);

    if(len < 0){
        status = false;
    }else if(static_cast<size_t>(len) <= writeable_size)
    {
        _writep += len;
        status=true;
    }else
    {
        _writep = _buffer.size();
        Add(buff1,len-writeable_size);
        Add(buff2,len-writeable_size);
        status=true;
    }
    return len;
    
}