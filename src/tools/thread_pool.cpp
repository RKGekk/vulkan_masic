#include "thread_pool.h"

scope_thread::scope_thread(std::thread t) noexcept : m_thread(std::move(t)) {}

scope_thread::scope_thread(scope_thread&& other) noexcept : m_thread(std::move(other.m_thread)) {}

scope_thread& scope_thread::operator=(scope_thread other) noexcept {
    close_thread();
    m_thread = std::move(other.m_thread);

    return *this;
}

scope_thread& scope_thread::operator=(std::thread t) noexcept {
    close_thread();
    m_thread = std::move(t);

    return *this;
}

scope_thread::~scope_thread() noexcept {
    close_thread();
}

void scope_thread::swap(scope_thread& other) noexcept {
    m_thread.swap(other.m_thread);
}

std::thread::id scope_thread::get_id() const noexcept {
    return m_thread.get_id();
}

bool scope_thread::joinable() const noexcept {
    return m_thread.joinable();
}

void scope_thread::join() {
    m_thread.join();
}

void scope_thread::detach() {
    m_thread.detach();
}

std::thread& scope_thread::as_thread() noexcept {
    return m_thread;
}

const std::thread& scope_thread::as_thread() const noexcept {
    return m_thread;
}

void scope_thread::close_thread() noexcept {
    if(m_thread.joinable()) {
        m_thread.join();
    }
}

join_threads::join_threads(std::vector<std::thread>& threads_) : m_threads(threads_) {}
    
join_threads::~join_threads() {
    for(unsigned long i = 0; i < m_threads.size(); ++i) {
        if(m_threads[i].joinable()) {
            m_threads[i].join();
        }
    }
}

ThreadPool::ThreadPool() : m_done(false), m_joiner(m_threads) {
    const unsigned thread_count = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() / 2u : 1u;
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
        FunctionWrapper task;
        if(m_work_queue.TryPop(task)) {
            task();
        }
        else {
            std::this_thread::yield();
        }
    }
}
