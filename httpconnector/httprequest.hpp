#pragma once
#include "../include/include.hpp"
#include "../buffer/buffer.hpp"
#include "httpdata.hpp"

class HttpRequest {
    public:

        enum HTTP_PART {
            REQUEST_LINE,
            HEADERS,
            BODY,
            FINISHED      
        };

        HttpRequest() {
            // std::cout << "HttpRequest" << std::endl;
        }
        ~HttpRequest() {
            // std::cout << "~HttpRequest" << std::endl;
        }

        void Init(); // 初始化
        HTTP_CODE Get_Code() { return _code; };
        const string Get_Method() const{ return _method; } ;
        const string Get_Path() const{ return _path; };
        bool Is_CGI() { return _cgi; };
        bool Is_Tasks() { return _tasks; };
        string Get_Version() { return _version; };
        string Select_Form(string key) { return _form[key]; };
        string Select_Header(string key) { return _headers[key]; };
        void Read_Buff(Buffer &buff); // 将缓冲区内容转换为http请求

    private:
        bool _Read_Request_Line(const string &line); // 读取请求行
        void _Handle_Path();
        bool _Read_Header_Line(const string &line); // 读取请求头
        void _Read_Body_Line(const string &line); // 读取请求头
        void _Handle_Post_Body(); // 处理post请求体
        void _Handle_Form();// 处理form表单

        int ConverHex(char ch);// 将16进制字符转换为数字

    private:
        HTTP_PART _status;
        HTTP_CODE _code;
        string _method,_path,_version;
        string _body;
        std::unordered_map<string,string> _headers;
        std::unordered_map<string,string> _form;
        string _file_suffix;

        bool _cgi;
        bool _tasks;//是否是系统提供方法


};