#include "../include/epoller.hpp"

Epoller::Epoller(uint32_t maxEvent):_epollFd(epoll_create(512)),_events(maxEvent) {
    assert(_epollFd != -1 && _events.size() > 0);
    LOG_INFO("Epoller init success,size = %ld", _events.size());
}

Epoller::~Epoller()
{
    close(_epollFd);
}

int Epoller::addFd(int fd, uint32_t events) {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    int ret = epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event);
    if (ret != 0) {
        LOG_WARNING("epoll_ctl add fd = %d failed", fd);
    }
    return 0==ret; //Return 1 on success, 0 on failure.
}

int Epoller::modFd(int fd, uint32_t events) {
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    int ret = epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event);
    if (ret != 0) {
        LOG_WARNING("epoll_ctl mod fd = %d failed", fd);
    }
    return 0==ret; //Return 1 on success, 0 on failure.
}

int Epoller::delFd(int fd) {
    epoll_event event;
    event.events = 0;
    event.data.fd = fd;
    int ret = epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &event);
    if (ret != 0) {
        LOG_WARNING("epoll_ctl del fd = %d failed", fd);
    }
    return 0==ret; //Return 1 on success, 0 on failure.
}

int Epoller::wait(int timeoutMs) {
    return epoll_wait(_epollFd, &_events[0], static_cast<int>(_events.size()), timeoutMs);
    //Wait for events using the epoll descriptor, output array, capacity, and timeout.
}

uint32_t Epoller::GetEvents(size_t index)
{
    assert(index < _events.size());
    return _events[index].events;
}

int Epoller::GetFd(size_t index)
{
    assert(index < _events.size());
    return _events[index].data.fd;
}
