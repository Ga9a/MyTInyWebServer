#include "../httpconnector/httprequest.hpp"

void HttpRequest::Init()
{
    _status= REQUEST_LINE;
    _tasks = false;
    _cgi = false;
    _code = FILE_REQUEST;
}

void HttpRequest::Read_Buff(Buffer &buff)
{
    assert(buff.Readable_Size() > 0);
    _status = REQUEST_LINE;
    _code = FILE_REQUEST;

    //buff.print();
    while(buff.Readable_Size()>0 && _status != FINISHED)
    {
        char* str_end;
        if(_status != BODY)
        str_end = std::search(buff.Begin_Read_Ptr(), buff.Begin_Write_Ptr() , CRLF, CRLF + 2);
        else
        str_end = buff.Begin_Write_Ptr();
        string line = string(buff.Begin_Read_Ptr(), str_end);
        if(_status != BODY)
        buff.Update_Readp(line.size()+2);
        else
        buff.Release_All_Buff();
        switch(_status)
        {
            case REQUEST_LINE:
                if(!_Read_Request_Line(line))
                {
                    _code = BAD_REQUEST;
                    return;
                }
                //LOG_INFO("_code: %s", GetStatusDescription(_code).c_str());
                break;
            case HEADERS:
                if(!_Read_Header_Line(line))
                {
                    _code = BAD_REQUEST;
                    return;
                }
                //如果剩下的部分是CRLF，说明BODY为空，直接设置
                if(buff.Readable_Size() <= 2){
                    buff.Update_Readp(2);
                    _status = FINISHED;
                }
                //LOG_INFO("_code: %s", GetStatusDescription(_code).c_str());
                break;
            case BODY:
            LOG_INFO("BODY---------------------------");
                _Read_Body_Line(line);
                break;
            default:
                break;
        }
        //string line = "hello";
        // LOG_INFO("request line: %s, remaining readable size = %ld", line.c_str(), buff.Readable_Size());
        // buff.print();
    };
    _Handle_Path();
    LOG_INFO("@@@@@@@@@@@@@@@@")
    LOG_INFO("method: %s, url: %s, version: %s", _method.c_str(), _path.c_str(), _version.c_str());
    if(_method == "POST") LOG_INFO("body: %s", _body.c_str());
    LOG_INFO("@@@@@@@@@@@@@@@@")

}

bool HttpRequest::_Read_Request_Line(const string &line) // 读取请求行
{
    assert(_status == REQUEST_LINE);
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line, subMatch, patten))
    {
        _method = subMatch[1];
        _path = subMatch[2];
        _version = subMatch[3];
        _status = HEADERS;
        LOG_INFO("method: %s, path: %s, version: %s", _method.c_str(), _path.c_str(), _version.c_str());
        return true;
    }
    else
    {
        LOG_FATAL("request line error: %s", line.c_str());
        return false;
    }
    // string::const_iterator pos = std::search(line.begin(), line.end(), " ", " " + 1);
    // _method = string(line.begin(), pos);
    // string::const_iterator pos1 = std::search(pos, line.end(), " ", " " + 1);
    // _path = string(pos+1, pos1);
    // _version = string(pos1+1,line.end()); // 读取请求行


    // LOG_INFO("method: %s, url: %s, version: %s", _method.c_str(), _path.c_str(), _version.c_str());
    
}

bool HttpRequest::_Read_Header_Line(const string &line)
{
    assert(_status == HEADERS);
    std::regex patten("^([^:]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line, subMatch, patten))
    {
        _headers[subMatch[1]] = subMatch[2];
    }
    else
    {
        _status = BODY;
    
    }
    return true;
}

void HttpRequest::_Read_Body_Line(const string &line) // 读取请求体
{
    _body = line;
    _Handle_Post_Body(); // 处理请求体
    _status = FINISHED;
}

void HttpRequest::_Handle_Post_Body()
{
    if(_method == "POST" && _headers.count("Content-Type") && _headers["Content-Type"] == "application/x-www-form-urlencoded")
    //表单请求
    {
        _Handle_Form();  
    }
    
}

void HttpRequest::_Handle_Form()// 处理form表单
{
    /*URL编码的表单数据通常在 HTTP POST 请求的请求体中传递，
    包含多个键值对，每个键值对由 `&` 分隔，键和值之间由 `=` 分隔。
    空格被编码为 `+`，特殊字符如 `&`、`=`、`+` 和 `%` 
    被编码为 `%` 后跟两位十六进制数。*/
    if(_body.empty()) return;
    string key,value;
    int num = 0;
    int n = _body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = _body[i];
        switch (ch) {
        case '=':
            key = _body.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            _body[i] = ' ';
            break;
        case '%':
            num = ConverHex(_body[i + 1]) * 16 + ConverHex(_body[i + 2]);
            _body[i + 2] = num % 10 + '0';
            _body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = _body.substr(j, i - j);
            j = i + 1;
            _form[key] = value;
            LOG_INFO("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(_form.count(key) == 0 && j < i) {
        value = _body.substr(j, i - j);
        _form[key] = value;
    }
}

void HttpRequest::_Handle_Path()
{
    //判断前面报文是否正常解析
    if(_code == BAD_REQUEST)
    {
        _path = SRC_DIR + "400.html";
    }

    //判断是不是系统特殊方法
    if(tasks.find(_path)!=tasks.end())
    {
        //如果是，直接保存标志并返回
        _tasks = true;
        _code = FILE_REQUEST; // 请求文件
        return ;
    }


    //加上主目录前缀
    _path = SRC_DIR + _path;
    //路径有两种情况，一种是目录，一种是文件
    //请求目录

    if(_path[_path.size() - 1] == '/')
    { 
    // 两种情况：
    // 1. 请求的就是Web根目录，即/root/
    // 2. 请求的是某一个子目录，即/root/a/，类似这样的方式
    // 不管是哪种目录，都在后面加上index.html，设置为主页，不管是根目录还是普通目录都返回的是主页
      _path += HOME_PAGE;
      return;
    }
    //请求文件
    
    //如果不是目录，需要检查一下是否有后缀，设置后缀
    size_t findSuffix = _path.rfind('.');
    if(findSuffix == std::string::npos)
    { // 没有后缀，统一按照html处理
      _file_suffix = "html";
    }
    else
    { // 有后缀，设置好后缀
      _file_suffix = _path.substr(findSuffix + 1);
    }

    //然后检查文件的有效性
    struct stat fileStat;
        
    if(stat(_path.c_str(), &fileStat) == -1)
        { // 没有找到这个文件
        LOG_WARNING("%s",move("path " + _path + " NOT_FOUND, err:" + string(strerror(errno))).c_str());
            _path = SRC_DIR+"/404.html";
            _code = NO_RESOURCE; // 找不到文件
            return;
        }
        else
    { // 文件存在，保存一下文件相关属性
      if(S_ISDIR(fileStat.st_mode))// 文件存在但是是一个目录 如：/reference/string ==> string是目录的话就应该访问其中的默认页面
      {
        _path += '/';
        _path += HOME_PAGE;
        stat(_path.c_str(), &fileStat);
      }
      // 文件存在，但是一个可执行文件
      else if((fileStat.st_mode & S_IXUSR) || (fileStat.st_mode & S_IXGRP) || (fileStat.st_mode & S_IXOTH))
      {
        _cgi = true;
      }
    }
    _code = FILE_REQUEST; // 请求文件
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}