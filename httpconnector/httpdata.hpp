// httpdata.hpp
#ifndef HTTPDATA_HPP
#define HTTPDATA_HPP

#include <unordered_set>
#include <unordered_map>
#include <string>

extern std::unordered_set<std::string> tasks;
extern const char* CRLF ;
extern const std::string SRC_DIR ;
extern const std::string HOME_PAGE ;

// 定义 HTTP_CODE 枚举
enum HTTP_CODE {
    NO_REQUEST,
    BAD_REQUEST,
    NO_RESOURCE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};

// 状态码到状态描述的映射
extern std::unordered_map<HTTP_CODE, std::string> statusDescriptions;

// 获取状态描述
std::string GetStatusDescription(HTTP_CODE code);

#endif // HTTPDATA_HPP