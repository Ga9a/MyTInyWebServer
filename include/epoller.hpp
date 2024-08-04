#pragma once
#include "include.hpp"

class Epoller {
    public:
    Epoller(uint32_t maxEvent = 512); // 构造函数
    ~Epoller(); // 析构函数
    int addFd(int fd, uint32_t events); // 添加文件描述符
    int modFd(int fd, uint32_t events); // 修改文件描述符
    int delFd(int fd); // 删除文件描述符
    int wait(int timeout = -1); // 成功返回就绪的文件描述符数量，失败返回-1,默认一直等待
    uint32_t GetEvents(size_t index); // 获取事件
    int GetFd(size_t index); // 获取文件描述符


    private:
    int _epollFd; // epoll文件描述符
    std::vector<struct epoll_event> _events; // 事件数组
};