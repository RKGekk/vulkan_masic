#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

#include "thread_safe_queue.h"

class join_threads {
public:
    explicit join_threads(std::vector<std::thread>& threads_);
    ~join_threads();

private:
    std::vector<std::thread>& m_threads;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    template<typename FunctionType>
    void Submit(FunctionType fn) {
        m_work_queue.Push(std::function<void()>(fn));
    }

private:
    std::atomic_bool m_done;
    ThreadSafeQueue<std::function<void()>> m_work_queue;
    std::vector<std::thread> m_threads;
    join_threads m_joiner;

    void worker_thread();
};