#include "../httpconnector/httprequest.hpp"

void HttpRequest::Init()
{
    _headers.clear();
    _form.clear();
    _body.clear();
    _method.clear();
    _path.clear();
    _version = "1.1";
    _status= REQUEST_LINE;
    _tasks = false;
    _cgi = false;
    _code = FILE_REQUEST;
}

void HttpRequest::Read_Buff(Buffer &buff) {
    while (buff.Readable_Size() > 0 && _status != FINISHED) {
        if (_status == BODY) {
            _Read_Body_Line(std::string(buff.Begin_Read_Ptr(), buff.Readable_Size()));
            buff.Release_All_Buff();
            break;
        }
        char* end = std::search(buff.Begin_Read_Ptr(), buff.Begin_Write_Ptr(), CRLF, CRLF + 2);
        if (end == buff.Begin_Write_Ptr()) {
            _code = BAD_REQUEST;
            break;
        }
        const std::string line(buff.Begin_Read_Ptr(), end);
        buff.Update_Readp(line.size() + 2);
        if (_status == REQUEST_LINE) {
            if (!_Read_Request_Line(line)) {
                _code = BAD_REQUEST;
                break;
            }
        } else if (line.empty()) {
            _status = buff.Readable_Size() ? BODY : FINISHED;
        } else if (!_Read_Header_Line(line)) {
            _code = BAD_REQUEST;
            break;
        }
    }
    _Handle_Path();
}

bool HttpRequest::_Read_Request_Line(const string &line) // Parse the request line.
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
    // _version = string(pos1+1,line.end()); // Parse the request line.


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
    else return false;
    return true;
}

void HttpRequest::_Read_Body_Line(const string &line) // Parse the request body.
{
    _body = line;
    _Handle_Post_Body(); // Handle the request body.
    _status = FINISHED;
}

void HttpRequest::_Handle_Post_Body()
{
    if(_method == "POST" && _headers.count("Content-Type") && _headers["Content-Type"] == "application/x-www-form-urlencoded")
    //URL-encoded form request.
    {
        _Handle_Form();
    }

}

void HttpRequest::_Handle_Form() {
    auto decode = [this](const std::string& value) {
        std::string decoded;
        for (std::size_t i = 0; i < value.size(); ++i) {
            if (value[i] == '+') decoded += ' ';
            else if (value[i] == '%') {
                if (i + 2 >= value.size()) throw std::invalid_argument("Incomplete URL escape");
                const int high = ConverHex(value[i + 1]);
                const int low = ConverHex(value[i + 2]);
                if (high < 0 || low < 0) throw std::invalid_argument("Invalid URL escape");
                decoded += static_cast<char>(high * 16 + low);
                i += 2;
            } else decoded += value[i];
        }
        return decoded;
    };
    try {
        std::size_t start = 0;
        while (start < _body.size()) {
            const std::size_t end = _body.find('&', start);
            const std::string pair = _body.substr(start, end == std::string::npos ? end : end - start);
            const std::size_t equals = pair.find('=');
            if (equals != std::string::npos)
                _form[decode(pair.substr(0, equals))] = decode(pair.substr(equals + 1));
            if (end == std::string::npos) break;
            start = end + 1;
        }
    } catch (const std::invalid_argument&) {
        _form.clear();
        _code = BAD_REQUEST;
    }
}

void HttpRequest::_Handle_Path()
{
    //Reject malformed requests.
    if(_code == BAD_REQUEST)
    {
        _path = SRC_DIR + "/400.html";
        return;
    }

    //Check built-in routes.
    if(tasks.find(_path)!=tasks.end())
    {
        //Mark the route for application handling.
        _tasks = true;
        _code = FILE_REQUEST; // File request.
        return ;
    }


    if (_path.empty() || _path.front() != '/' || _path.find("..") != std::string::npos ||
        _path.find('%') != std::string::npos || _path.find('\\') != std::string::npos) {
        _path = SRC_DIR + "/400.html";
        _code = BAD_REQUEST;
        return;
    }
    _path = _path.substr(0, _path.find('?'));
    //Prefix the document root.
    _path = SRC_DIR + _path;
    //Resolve directory and file requests.
    //Directory request.

    if(_path[_path.size() - 1] == '/')
    {
    // Directory requests use the default index page.
    // The document root and nested directories use index.html.
      _path += HOME_PAGE;
      return;
    }
    //File request.

    //Determine the file extension.
    size_t findSuffix = _path.rfind('.');
    if(findSuffix == std::string::npos)
    { // Default to HTML when no extension exists.
      _file_suffix = "html";
    }
    else
    { // Store the extension.
      _file_suffix = _path.substr(findSuffix + 1);
    }

    //Check whether the requested file exists.
    struct stat fileStat;

    if(stat(_path.c_str(), &fileStat) == -1)
        { // Return the not-found page.
        LOG_WARNING("%s",std::move("path " + _path + " NOT_FOUND, err:" + string(strerror(errno))).c_str());
            _path = SRC_DIR+"/404.html";
            _code = NO_RESOURCE; // File not found.
            return;
        }
        else
    { // Inspect the file attributes.
      if(S_ISDIR(fileStat.st_mode))// Append the index page for a directory.
      {
        _path += '/';
        _path += HOME_PAGE;
        stat(_path.c_str(), &fileStat);
      }
      // Executable file.
      else if((fileStat.st_mode & S_IXUSR) || (fileStat.st_mode & S_IXGRP) || (fileStat.st_mode & S_IXOTH))
      {
        _cgi = true;
      }
    }
    _code = FILE_REQUEST; // File request.
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    if(ch >= '0' && ch <= '9') return ch - '0';
    return -1;
}
