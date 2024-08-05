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
        /// 文件名
        std::string fileName = "../wwwroot/index.html";
        std::ifstream file(fileName);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + fileName);
        }

        std::stringstream fileContent;
        fileContent << file.rdbuf();
        std::string content = fileContent.str();

        // 构建 HTTP 响应
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(content.length()) + "\r\n\r\n" + content;
        buffer.Add(response);
#endif
        buffer.Add(move(_statue_line+"\r\n"+_headers+"\r\n"));
    }

void HttpResponse::_Handle_Path(HttpRequest &request)
{
    //查看是否是特殊请求
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

void HttpResponse::_Init_Headers_Map(const HttpRequest &request)
{
     switch(_code)
        {
            case FILE_REQUEST:
                _headers_map["Content-Type"] = "text/html";
                //TODO 支持更多文件种类
                break;
            case BAD_REQUEST:
                _headers_map["Content-Type"] = "text/html";
                break;
            case NO_RESOURCE:
                _headers_map["Content-Type"] = "text/html";
            default:
                break;
        }

        //TODO 支持更多文件种类
        Set_ContentLength_From_File(_path);
        _headers_map["Date"] = _Generate_HttpDate();
}

std::string HttpResponse::_Generate_HttpDate() {
// 获取当前时间
std::time_t t = std::time(nullptr);
std::tm tm = *std::gmtime(&t);
// 格式化时间为 HTTP 日期字符串
std::ostringstream oss;
oss << "Date: "<< std::put_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
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
    int id;
    bool result = true;
    MYSQL* mysql = SQLPool::GetInstance()->GetConn(id); //获取连接
    if (!mysql) {
        LOG_ERROR("Failed to get MySQL connection");
        return false;
    }

    std::string username = request.Select_Form("username");
    std::string password = request.Select_Form("pwd");

    // Prepare statement to prevent SQL injection
    std::string query = "SELECT * FROM user WHERE username = ?";
    MYSQL_STMT* stmt = mysql_stmt_init(mysql);
    if (!stmt) {
        LOG_ERROR("Failed to initialize MySQL statement");
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    if (mysql_stmt_prepare(stmt, query.c_str(), query.size()) != 0) {
        LOG_ERROR("MySQL statement prepare failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(username.c_str());
    bind[0].buffer_length = username.size();

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        LOG_ERROR("MySQL statement bind failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        LOG_ERROR("MySQL statement execute failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    MYSQL_RES* res = mysql_stmt_result_metadata(stmt);
    if (!res) {
        LOG_ERROR("MySQL statement result metadata failed: %s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    if (mysql_stmt_store_result(stmt) != 0) {
        LOG_ERROR("MySQL statement store result failed: %s", mysql_stmt_error(stmt));
        mysql_free_result(res);
        mysql_stmt_close(stmt);
        SQLPool::GetInstance()->ReleaseConn(id, mysql);
        return false;
    }

    if (mysql_stmt_num_rows(stmt) == 0) {
        LOG_WARNING("Username is not registered!");
        result = false;
    } else {
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row && row[1] && password != row[1]) {
            LOG_WARNING("Password is wrong!");
            result = false;
        }
    }

    mysql_free_result(res);
    mysql_stmt_close(stmt);
    SQLPool::GetInstance()->ReleaseConn(id, mysql);
    return result;
}