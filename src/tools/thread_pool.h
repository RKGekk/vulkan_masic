#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

#include "thread_safe_queue.h"

class join_threads {
public:
    explicit join_threads(std::vector<std::thread>& threads_);
    ~join_threads();

private:
    std::vector<std::thread>& m_threads;
};

class scope_thread {
public:
    scope_thread() noexcept = default;
    template<typename Callable, typename ... Args>
    explicit scope_thread(Callable&& func, Args&& ... args) : m_thread(std::forward<Callable>(func), std::forward<Args>(args)...) {}
    explicit scope_thread(std::thread t) noexcept;
    scope_thread(scope_thread&& other) noexcept;
    scope_thread& operator=(scope_thread other) noexcept;
    scope_thread& operator=(std::thread t) noexcept;
    ~scope_thread() noexcept;

    void swap(scope_thread& other) noexcept;
    std::thread::id get_id() const noexcept;
    bool joinable() const noexcept;
    void join();
    void detach();
    std::thread& as_thread() noexcept;
    const std::thread& as_thread() const noexcept;

private:
    void close_thread() noexcept;

    std::thread m_thread;
};

class FunctionWrapper {
public:

    template<typename F>
    FunctionWrapper(F&& f_) : m_impl(new ImplType<F>(std::move(f_))) {}

    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper&& other) : m_impl(std::move(other.m_impl)) {}

    FunctionWrapper(const FunctionWrapper& other) = delete;
    FunctionWrapper(FunctionWrapper& other) = delete;

    FunctionWrapper& operator=(FunctionWrapper&& other) {
        m_impl = std::move(other.m_impl);
        return *this;
    }
    FunctionWrapper& operator=(const FunctionWrapper& other) = delete;

    void operator()() {
        m_impl->Call();
    }

private:
    struct ImplBase {
        virtual void Call() = 0;
        virtual ~ImplBase() {}
    };

    std::unique_ptr<ImplBase> m_impl;

    template<typename F>
    struct ImplType : ImplBase {
        F f;
        ImplType(F&& f_) : f(std::move(f_)) {}
        void call() {
            f();
        }
    };
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    template<typename FunctionType>
    void Submit(FunctionType fn) {
        m_work_queue.Push(std::function<void()>(fn));
    }

    template<typename FunctionType>
    using submit_result_type = std::invoke_result<FunctionType()>::type;

    template<typename FunctionType>
    std::future<submit_result_type<FunctionType>> SubmitAsync(FunctionType f) {
        submit_result_type<FunctionType> result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        m_work_queue.Push(std::move(task));
        return res;
    }

private:
    std::atomic_bool m_done;
    ThreadSafeQueue<FunctionWrapper> m_work_queue;
    std::vector<std::thread> m_threads;
    join_threads m_joiner;

    void worker_thread();
};