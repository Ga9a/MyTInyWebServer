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
    size_t size(); //返回队列中元素的个数
    void Close();

    private:
    size_t _capability;
    bool _status; //队列状态
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
    
    //上线程锁
    unique_lock<mutex> lock(_mtx);
    //先判断队列是否是满的，再等待条件变量
    while( _dq.size() >= _capability)
    {
        producer.wait(lock);
    }
    //这么写是不对的！因为full在也要上锁，但这个锁被push_back()占用了
    //producer.wait(lock,[this](){return !full();});
    _dq.push_back(item);
    consumer.notify_one(); //唤醒一个消费者
}

template <class T>
bool BlockDeque<T>::pop_front(T& item)
{
    //上线程锁
    unique_lock<mutex> lock(_mtx);
    //先判断队列是否是空的，再等待条件变量
    while( _dq.empty())
    {
        consumer.wait(lock);
        if(_status==dq_close){
            return false;
        }
    }
    item=_dq.front();
    _dq.pop_front();
    producer.notify_one(); //唤醒一个生产者
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