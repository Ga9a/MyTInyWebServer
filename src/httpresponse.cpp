#include "../httpconnector/httpresponse.hpp"

HttpResponse::HttpResponse()
    {};

void HttpResponse::Init(HttpRequest &request)
{
    _File_status = false;
    _headers="";
    _statue_line="";
    _path = SRC_DIR;
    _headers_map.clear();
    _code = request.Get_Code();
    _statue_line = "HTTP/" + request.Get_Version() + " " + GetStatusDescription(_code) ;
    LOG_INFO("%s",_statue_line.c_str());
    _Handle_Path(request);
    LOG_INFO("%s",_path.c_str());
    _Init_Headers_Map(request);
    for(auto &it : _headers_map)
    {
        _headers += it.first + ": " + it.second + "\r\n";
    }
    _headers += "\r\n";
}

void HttpResponse::Head_Write_Buff(Buffer &buffer) {
    buffer.Release_All_Buff();

#ifdef DEBUG
        /// Debug file path.
        std::string fileName = "../wwwroot/index.html";
        std::ifstream file(fileName);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + fileName);
        }

        std::stringstream fileContent;
        fileContent << file.rdbuf();
        std::string content = fileContent.str();

        // Build a debug HTTP response.
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(content.length()) + "\r\n\r\n" + content;
        buffer.Add(response);
#endif
        buffer.Add(std::move(_statue_line+"\r\n"+_headers));
    }

void HttpResponse::_Handle_Path(HttpRequest &request)
{
    //Handle application routes.
    if(request.Is_Tasks())
    {
        if(tasks.find("/login")!=tasks.end())
        {
            if(_Login(request))
                _path += "/penetration.html";
            else
                _path += "/mysqlerror.html";
        }
    }
    else
    _path = request.Get_Path();
}

void HttpResponse::_Init_Headers_Map(const HttpRequest &)
{
     switch(_code)
        {
            case FILE_REQUEST:
                _headers_map["Content-Type"] = "text/html";
                //TODO: Add more MIME types.
                break;
            case BAD_REQUEST:
                _headers_map["Content-Type"] = "text/html";
                break;
            case NO_RESOURCE:
                _headers_map["Content-Type"] = "text/html";
            default:
                break;
        }

        //TODO: Add more MIME types.
        Set_ContentLength_From_File(_path);
        _headers_map["Date"] = _Generate_HttpDate();
}

std::string HttpResponse::_Generate_HttpDate() {
// Get the current time.
std::time_t t = std::time(nullptr);
std::tm tm{};
gmtime_r(&t, &tm);
// Format the HTTP date.
std::ostringstream oss;
oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
return oss.str();
}

void HttpResponse::Set_ContentLength_From_File(const std::string &filePath)
{
        std::filesystem::path path(filePath);
        LOG_INFO("%s",path.c_str());
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
        auto fileSize = std::filesystem::file_size(path);
        _headers_map[std::string("Content-Length")] = std::to_string(fileSize);
        } else {
        throw std::runtime_error("File does not exist or is not a regular file");
        }

}

bool HttpResponse::_Login(HttpRequest &request) {
    const std::string username = request.Select_Form("username");
    const std::string password = request.Select_Form("pwd");
    if (username.empty() || username.size() > 50 || password.empty() || password.size() > 50)
        return false;
    int id = -1;
    MYSQL* mysql = SQLPool::GetInstance()->GetConn(id);
    if (!mysql) return false;
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    bool authenticated = false;
    if (stmt) {
        const char* query = "SELECT passwd FROM user WHERE username = ? LIMIT 1";
        MYSQL_BIND parameter{};
        parameter.buffer_type = MYSQL_TYPE_STRING;
        parameter.buffer = const_cast<char*>(username.data());
        parameter.buffer_length = username.size();
        char stored_password[256]{};
        unsigned long length = 0;
        MYSQL_BIND result{};
        result.buffer_type = MYSQL_TYPE_STRING;
        result.buffer = stored_password;
        result.buffer_length = sizeof(stored_password);
        result.length = &length;
        if (mysql_stmt_prepare(stmt, query, std::strlen(query)) == 0 &&
            mysql_stmt_bind_param(stmt, &parameter) == 0 &&
            mysql_stmt_execute(stmt) == 0 &&
            mysql_stmt_bind_result(stmt, &result) == 0 &&
            mysql_stmt_store_result(stmt) == 0 &&
            mysql_stmt_fetch(stmt) == 0 && length <= sizeof(stored_password)) {
            authenticated = password == std::string(stored_password, length);
        }
        mysql_stmt_close(stmt);
    }
    SQLPool::GetInstance()->ReleaseConn(id, mysql);
    return authenticated;
}
