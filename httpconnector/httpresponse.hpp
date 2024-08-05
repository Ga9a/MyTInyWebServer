#pragma once
#include "httprequest.hpp"
#include "../buffer/buffer.hpp"
#include "httpdata.hpp"
#include "../sqlpool/sql_pool.hpp"

class HttpResponse {
public:
    HttpResponse();

    public:

    void Init(HttpRequest &request); //初始化响应
    void Head_Write_Buff(Buffer &buffer); //将响应头写入缓冲区
    string Get_Path() { return _path; }
    

    private:
    void _Handle_Path(HttpRequest &request);
    bool _Login(HttpRequest &request); //处理登录
    void _Init_Headers_Map(const HttpRequest &request);
    std::string _Generate_HttpDate(); //生成httpdate
    void Set_ContentLength_From_File(const std::string &filePath); //从文件中获取Content-Length

    private:
    bool _File_status;
    string _statue_line;
    string _path; //请求的文件路径
    string _headers;
    HTTP_CODE _code;
    std::unordered_map<std::string, std::string> _headers_map;
};
