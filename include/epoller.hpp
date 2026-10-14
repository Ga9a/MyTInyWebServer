#pragma once
#include "include.hpp"

class Epoller {
    public:
    Epoller(uint32_t maxEvent = 512); // Constructor.
    ~Epoller(); // Destructor.
    int addFd(int fd, uint32_t events); // Register a file descriptor.
    int modFd(int fd, uint32_t events); // Update a file descriptor.
    int delFd(int fd); // Remove a file descriptor.
    int wait(int timeout = -1); // Return the ready count, or -1 on failure; wait indefinitely by default.
    uint32_t GetEvents(size_t index); // Read the event mask.
    int GetFd(size_t index); // Read the file descriptor.


    private:
    int _epollFd; // Epoll descriptor.
    std::vector<struct epoll_event> _events; // Ready events.
};
