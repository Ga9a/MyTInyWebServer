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
    size_t Readable_Size() const; //Return the readable byte count.
    void Update_Writep(size_t len);//Advance the write cursor.
    void Update_Readp(size_t len);//Advance the read cursor.
    void Update_Readp(const char* end);//Advance the read cursor.
    char* Begin_Read_Ptr(); //Return the read cursor.
    char* Begin_Write_Ptr();//Return the write cursor.

    //Clear the buffer.
    void Release_All_Buff();
    void Release_All_Buff(string &str);
    void Release_Buff(size_t len);

    void Add(const string& str);//Append text to the buffer.
    void Add(const char* str, size_t len);//Append text to the buffer.

    private:
    char* _Begin_Ptr(); //Return the start of the storage.
    size_t _Writeable_Size() const; //Return available writable capacity.
    size_t _Prepared_Size() const; //Return the reusable space before the read cursor.
    void _Dilatation(size_t len); //Grow the buffer.
    void _Length_Guard(size_t len); //Ensure sufficient writable capacity.



    private:
    std::vector<char> _buffer;
    std::atomic<std::size_t> _readp;
    std::atomic<std::size_t> _writep;
};
