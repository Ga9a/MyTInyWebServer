#include "../buffer/buffer.hpp"

// Initialize the buffer and its read/write cursors.
Buffer::Buffer(size_t buff_size) : _buffer(buff_size),_readp(0),_writep(0) {
}

// Return the start of the backing buffer.
char* Buffer::_Begin_Ptr()
{
    assert(_buffer.size() > 0);
    return &*_buffer.begin();
};

//Advance the write cursor.
void Buffer::Update_Writep(size_t len)
{
    _writep += len;
}

//Advance the read cursor.
void Buffer::Update_Readp(size_t len)
{
    assert(len <= Readable_Size());
    _readp += len;
}

//Return the read cursor.
char* Buffer::Begin_Read_Ptr()
{
    return _Begin_Ptr() + _readp;
}

//Advance the read cursor.
void Buffer::Update_Readp(const char* end)
{
    assert(Begin_Read_Ptr() <= end);
    Update_Readp(end-Begin_Read_Ptr());
}

// Return available writable capacity.
size_t Buffer::_Writeable_Size() const
{
    return _buffer.size() - _writep;
}

// Return the readable byte count.
size_t Buffer::Readable_Size() const
{
    return _writep-_readp;
}

// Return reusable consumed space.
size_t Buffer::_Prepared_Size() const
{
    return  _readp;
}

//Clear all buffered data.
void Buffer::Release_All_Buff()
{
    bzero(&_buffer[0], _buffer.size());
    _readp = 0;
    _writep = 0;
}

//Clear all buffered data.
void Buffer::Release_All_Buff(string &str)
{
    str=string(Begin_Read_Ptr(),Readable_Size());
    Release_All_Buff();
}

void Buffer::Release_Buff(size_t len)
{
    Update_Readp(len);
}

// Return the write cursor.
char* Buffer::Begin_Write_Ptr()
{
    return _Begin_Ptr() + _writep;
}

// Compact consumed space if sufficient; otherwise grow the storage.
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

// Ensure sufficient writable space before appending.
void Buffer::_Length_Guard(size_t len)
{
    if(_Writeable_Size() < len)
        _Dilatation(len);
    assert(_Writeable_Size() >= len);
}

// Append a string.
void  Buffer::Add(const string& str)
{
    //
    Add(str.c_str(),str.size());
}

// Append a byte array.
void  Buffer::Add(const char* str, size_t len)
{
    assert(str);
    _Length_Guard(len);
    std::copy(str,str+len,Begin_Write_Ptr());//Copy bytes into the buffer.
    _writep += len;
}

// Read bytes from a descriptor.
ssize_t Buffer::Read_FD(int fd,bool& status)
{
    char buff1[65535];
    char buff2[65535];
    iovec iov[3];
    const size_t writeable_size = _Writeable_Size();

    //Use scatter reads to fill available space efficiently.

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
        const size_t overflow = static_cast<size_t>(len) - writeable_size;
        const size_t first = std::min(overflow, sizeof(buff1));
        Add(buff1, first);
        if (overflow > first) Add(buff2, overflow - first);
        status=true;
    }
    return len;

}
