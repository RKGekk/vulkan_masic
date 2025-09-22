#pragma once

#include <queue>
#include <mutex>
#include <utility>

template<typename DataType>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue& other) {
        std::lock_guard<std::mutex> lock(other.m_mutex);
        m_queue = other.m_queue;
    }
    ThreadSafeQueue(ThreadSafeQueue&& other) {
        std::lock_guard<std::mutex> lock(other.m_mutex);
        std::swap(m_queue, other.m_queue);
    };

    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) {
        if (this == &other) return *this;
        std::lock_guard<std::mutex> lock(other.m_mutex);
        m_queue = other.m_queue;
    };

    ThreadSafeQueue& operator=(ThreadSafeQueue&& other) {
        if (this == &other) return *this;
        std::lock_guard<std::mutex> lock(other.m_mutex);
        std::swap(m_queue, other.m_queue);
    };

    void Push(DataType value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        m_data_cond.notify_one();
    }

    bool TryPop(DataType& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;
        
        value = std::move(m_queue.front());
        m_queue.pop();
        
        return true;
    }

    std::shared_ptr<DataType> TryPop() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if(m_queue.empty()) {
            return nullptr;
        }
        std::shared_ptr<DataType> value = std::make_shared<DataType>(std::move(m_queue.front()));
        m_queue.pop();
        return value;
    }

    void WaitAndPop(DataType& value) {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            if(!m_queue.empty()) {
                value = std::move(m_queue.front());
                m_queue.pop();
                return;
            }
        }
        std::unique_lock<std::mutex> lk(m_mutex);
        m_data_cond.wait(lk, [this](){return !m_queue.empty();});
        value = std::move(m_queue.front());
        m_queue.pop();
    }

    std::shared_ptr<DataType> WaitAndPop() {
        {
            std::lock_guard<std::mutex> lk(m_mutex);
            if(!m_queue.empty()) {
                std::shared_ptr<DataType> value = std::make_shared<DataType>(std::move(m_queue.front()));
                m_queue.pop();
                return value;
            }
        }
        std::unique_lock<std::mutex> lk(m_mutex);
        m_data_cond.wait(lk, [this](){return !m_queue.empty();});
        std::shared_ptr<DataType> value = std::make_shared<DataType>(std::move(m_queue.front()));
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