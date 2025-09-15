#pragma once

#include <queue>
#include <mutex>

template<typename DataType>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() {};
    ThreadSafeQueue(const ThreadSafeQueue& copy) {
        std::lock_guard<std::mutex> lock(copy.m_mutex);
        m_queue = copy.m_queue;
    }

    void Push(DataType value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
    }

    bool TryPop(DataType& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;
        
        value = m_queue.front();
        m_queue.pop();
        
        return true;
    }

    std::shared_ptr<DataType> TryPop() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if(m_queue.empty()) {
            return nullptr;
        }
        std::shared_ptr<DataType> value = std::make_shared<DataType>(m_queue.front());
        m_queue.pop();
        return value;
    }

    void WaitAndPop(DataType& value) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_data_cond.wait(lk, [this](){return !m_queue.empty();});
        value = m_queue.front();
        m_queue.pop();
    }

    std::shared_ptr<DataType> WaitAndPop() {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_data_cond.wait(lk, [this](){return !m_queue.empty();});
        std::shared_ptr<DataType> value = std::make_shared<DataType>(m_queue.front());
        m_queue.pop();
        return value;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

    size_t Size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    std::queue<DataType> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_data_cond;
};