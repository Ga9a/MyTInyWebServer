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

        void Init(); // Initialize request state.
        HTTP_CODE Get_Code() { return _code; };
        const string Get_Method() const{ return _method; } ;
        const string Get_Path() const{ return _path; };
        bool Is_CGI() { return _cgi; };
        bool Is_Tasks() { return _tasks; };
        string Get_Version() { return _version; };
        string Select_Form(string key) { return _form[key]; };
        string Select_Header(string key) { return _headers[key]; };
        void Read_Buff(Buffer &buff); // Parse an HTTP request from the buffer.

    private:
        bool _Read_Request_Line(const string &line); // Parse the request line.
        void _Handle_Path();
        bool _Read_Header_Line(const string &line); // Parse a header line.
        void _Read_Body_Line(const string &line); // Parse a header line.
        void _Handle_Post_Body(); // Handle a POST body.
        void _Handle_Form();// Decode form fields.

        int ConverHex(char ch);// Decode a hexadecimal digit.

    private:
        HTTP_PART _status;
        HTTP_CODE _code;
        string _method,_path,_version;
        string _body;
        std::unordered_map<string,string> _headers;
        std::unordered_map<string,string> _form;
        string _file_suffix;

        bool _cgi;
        bool _tasks;//Built-in application route.


};
