#include "thread_pool.h"

join_threads::join_threads(std::vector<std::thread>& threads_) : m_threads(threads_) {}
    
join_threads::~join_threads() {
    for(unsigned long i = 0; i < m_threads.size(); ++i) {
        if(m_threads[i].joinable()) {
            m_threads[i].join();
        }
    }
}

ThreadPool::ThreadPool() : m_done(false), m_joiner(m_threads) {
    const unsigned thread_count = std::thread::hardware_concurrency();
    try {
        for(unsigned i = 0; i < thread_count; ++i) {
            m_threads.push_back(std::thread(&ThreadPool::worker_thread, this));
        }
    }
    catch(...) {
        m_done = true;
        throw;
    }
}

ThreadPool::~ThreadPool() {
    m_done = true;
}

void ThreadPool::worker_thread() {
    while(!m_done) {
        std::function<void()> task;
        if(m_work_queue.TryPop(task)) {
            task();
        }
        else {
            std::this_thread::yield();
        }
    }
}
