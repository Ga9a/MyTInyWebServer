#pragma once
#include "../include/include.hpp"

using std::deque;
using std::mutex;
using std::condition_variable;
using std::unique_lock;

enum
{
    dq_close,
    dq_open
};

template <class T>
class BlockDeque
{
    public:
    BlockDeque(const size_t& cap):_capability(cap),_status(dq_open){};
    ~BlockDeque(){Close();};
    bool empty();
    bool full();
    void push_back(const T& item);
    bool pop_front(T& item);
    void clear();
    size_t size(); //Return the number of queued items.
    void Close();

    private:
    size_t _capability;
    bool _status; //Queue state.
    deque<T> _dq;
    mutex _mtx;
    condition_variable consumer;
    condition_variable producer;

};

template <class T>
bool BlockDeque<T>::empty()
{
    std::unique_lock<mutex> lock(_mtx);
    return _dq.empty();
};

template <class T>
bool BlockDeque<T>::full()
{
    std::unique_lock<mutex> lock(_mtx);
    return _dq.size() >= _capability;
}

template <class T>
void BlockDeque<T>::push_back(const T& item)
{

    //Lock the queue.
    unique_lock<mutex> lock(_mtx);
    //Wait until the queue has space.
    while( _dq.size() >= _capability)
    {
        producer.wait(lock);
    }
    //Do not call full() while holding the same non-recursive mutex.
    //producer.wait(lock,[this](){return !full();});
    _dq.push_back(item);
    consumer.notify_one(); //Wake one consumer.
}

template <class T>
bool BlockDeque<T>::pop_front(T& item)
{
    //Lock the queue.
    unique_lock<mutex> lock(_mtx);
    //Wait until an item is available.
    while( _dq.empty())
    {
        consumer.wait(lock);
        if(_status==dq_close){
            return false;
        }
    }
    item=_dq.front();
    _dq.pop_front();
    producer.notify_one(); //Wake one producer.
    return true;
}

template <class T>
void BlockDeque<T>::clear()
{
    std::lock_guard<mutex> lock(_mtx);
    _dq.clear();
}

template <class T>
size_t BlockDeque<T>::size()
{
    std::unique_lock<mutex> lock(_mtx);
    return _dq.size();
}

template<class T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _dq.clear();
        _status=dq_close;
    }
    consumer.notify_all();
    producer.notify_all();
};
