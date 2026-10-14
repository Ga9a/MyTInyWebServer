#pragma once
#include "httprequest.hpp"
#include "../buffer/buffer.hpp"
#include "httpdata.hpp"
#include "../sqlpool/sql_pool.hpp"

class HttpResponse {
public:
    HttpResponse();

    public:

    void Init(HttpRequest &request); //Initialize the response.
    void Head_Write_Buff(Buffer &buffer); //Append HTTP response headers.
    string Get_Path() { return _path; }


    private:
    void _Handle_Path(HttpRequest &request);
    bool _Login(HttpRequest &request); //Authenticate a login request.
    void _Init_Headers_Map(const HttpRequest &request);
    std::string _Generate_HttpDate(); //Format an HTTP date.
    void Set_ContentLength_From_File(const std::string &filePath); //Set Content-Length from the file size.

    private:
    bool _File_status;
    string _statue_line;
    string _path; //Requested file path.
    string _headers;
    HTTP_CODE _code;
    std::unordered_map<std::string, std::string> _headers_map;
};
