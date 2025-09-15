#pragma once

#include <algorithm>
#include <thread>

class CPULoadBalanceParams {
public:
    CPULoadBalanceParams() = delete;
    CPULoadBalanceParams(const CPULoadBalanceParams& other) = delete;
    CPULoadBalanceParams(CPULoadBalanceParams&& other) = delete;
    const CPULoadBalanceParams& operator=(const CPULoadBalanceParams& other) = delete;
    const CPULoadBalanceParams& operator=(CPULoadBalanceParams&& other) = delete;

    CPULoadBalanceParams(size_t amt_work, unsigned long min_per_thread = 25u);

    size_t const min_per_thread;
    size_t const max_threads;
    size_t const hw_threads;
    size_t const num_threads;
    size_t const block_size;
};