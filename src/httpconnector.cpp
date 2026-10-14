#include "../httpconnector/httpconnector.hpp"

//Define static members.
std::atomic<uint16_t> HttpConnector::user_cnt(0);

HttpConnector::HttpConnector()
{
    _status = false;
    _fd = -1;
    _addr = {};
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
    ++user_cnt; // Increment active connections.
    _status = true; // Mark the connection active.
    LOG_INFO("Client[%d](%s:%d) visit, Usercnt = %d.",GetFd(),GetIp(),GetPort(),static_cast<int>(user_cnt))
    Buffer buffer1(10);

};

void HttpConnector::Close()//Close the connection.
{
    if(_status)
    {
        _status=false;
        --user_cnt;
        _iov_size = 0;
        memset(_iov, 0, sizeof(_iov));
        close(_fd);
        _file_content.reset();
        LOG_INFO("Client[%d](%s:%d) left, Usercnt = %d.",GetFd(),GetIp(),GetPort(),static_cast<int>(user_cnt))
    }
}

ssize_t HttpConnector::Read(bool& status) {
    ssize_t total = 0;
    while (true) {
        const ssize_t count = _read_buffer.Read_FD(_fd, status);
        if (count > 0) {
            total += count;
            if (_read_buffer.Readable_Size() > 1024 * 1024) {
                status = false;
                return -1;
            }
            continue;
        }
        if (count < 0 && errno == EINTR) continue;
        status = total > 0 || (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK));
        return total > 0 ? total : count;
    }
}

ssize_t HttpConnector::Write(bool& status) // Write response bytes.
{
    ssize_t len = -1;
    while(true)
    {
        len = writev(_fd,_iov,_iov_size);
        if (len < 0) {
            if (errno == EINTR) continue;
            status = errno == EAGAIN || errno == EWOULDBLOCK;
            return len;
        }
        if(!len){
            status = false;
            return len;
        }

        if(!Writeable_Size()) break;
        //When writev consumes the full header buffer,
        //advance the file buffer by the remaining bytes written.
        else if(len > static_cast<ssize_t>(_iov[0].iov_len))
        {
            _iov[1].iov_base = static_cast<uint8_t*>(_iov[1].iov_base) + (len - _iov[0].iov_len);
            _iov[1].iov_len -= (len - _iov[0].iov_len);
            if(_iov[0].iov_len) {
                _write_buffer.Release_All_Buff();
                _iov[0].iov_len = 0;
            }
        }
        //Otherwise advance only the header buffer.
        else{
            _iov[0].iov_base = (uint8_t*) _iov[0].iov_base + len;
            _iov[0].iov_len -= len;
            _write_buffer.Release_Buff(len);
        }
        if(Writeable_Size()==0) break;
    }
    return len;
}

bool HttpConnector::Process() // Process the HTTP request.
{

    if(_read_buffer.Readable_Size() == 0) {
        return false;//No request data is available.
    }else{

        // test for http GET
        // _read_buffer.print();
        // string str;
        // _read_buffer.Release_All_Buff(str);
        // _write_buffer.Add(str);

        // char* head= "head:";

        // _iov[0].iov_base = head;
        // _iov[0].iov_len = strlen(head);
        // _iov[1].iov_base = _write_buffer.Begin_Read_Ptr();
        // _iov[1].iov_len = _write_         buffer.Readable_Size();

        // _iov_size=2;
        // test end
        const std::string pending(_read_buffer.Begin_Read_Ptr(), _read_buffer.Readable_Size());
        const auto headers_end = pending.find("\r\n\r\n");
        if (headers_end == std::string::npos) return false;
        std::smatch match;
        const std::regex content_length("(?:^|\r\n)Content-Length: *([0-9]+)\r\n", std::regex::icase);
        std::size_t body_length = 0;
        const std::string headers = pending.substr(0, headers_end + 2);
        if (std::regex_search(headers, match, content_length)) {
            body_length = std::stoul(match[1].str());
            if (body_length > 1024 * 1024) throw std::invalid_argument("Request body is too large");
        }
        if (pending.size() < headers_end + 4 + body_length) return false;
        _request.Init();
        _request.Read_Buff(_read_buffer);//Parse the request.
        _read_buffer.Release_All_Buff();//Clear consumed request bytes.

        _reponse.Init(_request);
        _reponse.Head_Write_Buff(_write_buffer);//Build response headers.
        _write_buffer.print();


        std::size_t fileSize = 0;
        _file_content = ReadFileToSharedCharArray(_reponse.Get_Path(), fileSize);

        // std::cout.write(fileContent.get(), fileSize);
        // std::cout << std::endl;



        _iov[0].iov_base = _write_buffer.Begin_Read_Ptr();
        _iov[0].iov_len = _write_buffer.Readable_Size();
        _iov[1].iov_base = _file_content.get();
        _iov[1].iov_len = fileSize;
        _iov_size=2;

        return true;//A response is ready to write.
    }
}

std::shared_ptr<char> HttpConnector::ReadFileToSharedCharArray(const std::string& filePath, std::size_t& fileSize) {
    std::ifstream file(filePath, std::ios::in | std::ios::binary | std::ios::ate);
    LOG_INFO("filePath = %s",filePath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    // Read the file size.
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::shared_ptr<char> buffer(new char[fileSize], std::default_delete<char[]>());
    file.read(buffer.get(), fileSize);
    // Close the input file.
    file.close();

    return buffer;
}
