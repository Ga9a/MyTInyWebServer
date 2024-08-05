#include "../httpconnector/httpdata.hpp"

std::unordered_set<std::string> tasks = {"/login"};
const char* CRLF = "\r\n";
const std::string SRC_DIR = "../wwwroot";
const std::string HOME_PAGE = "index.html";

std::unordered_map<HTTP_CODE, std::string> statusDescriptions = {
    {NO_REQUEST, "400 Bad Request"},
    {BAD_REQUEST, "400 Bad Request"},
    {NO_RESOURCE, "404 Not Found"},
    {FORBIDDEN_REQUEST, "403 Forbidden"},
    {FILE_REQUEST, "200 OK"},
    {INTERNAL_ERROR, "500 Internal Server Error"},
    {CLOSED_CONNECTION, "200 OK"}
};

std::string GetStatusDescription(HTTP_CODE code) {
    return statusDescriptions[code];
}