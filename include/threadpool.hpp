#pragma once
#include "include.hpp"
#include <cassert>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads = 10) : _pool(std::make_shared<Pool>()) {
        assert(numThreads > 0);
        for (size_t i = 0; i < numThreads; ++i) {
            std::thread([pool = _pool, i]() {
                std::unique_lock<std::mutex> lock(pool->mtx);
                while (true) {
                    if (!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        lock.unlock();
                        LOG_INFO("thread[%ld] start execute task.", i);
                        task();
                        lock.lock();
                    } else if (!pool->status) {
                        break;
                    } else {
                        pool->cv.wait(lock);
                    }
                }
            }).detach();
        }
        LOG_INFO("ThreadPool Init Success.");
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if (_pool) {
            std::unique_lock<std::mutex> lock(_pool->mtx);
            _pool->status = false;
            _pool->cv.notify_all();
        }
    }

    template<class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(_pool->mtx);
            LOG_INFO("add task success.");
            _pool->tasks.emplace(std::forward<F>(task));
        }
        _pool->cv.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cv;
        bool status = true;
        std::queue<std::function<void()>> tasks;
    };

    std::shared_ptr<Pool> _pool;
};
