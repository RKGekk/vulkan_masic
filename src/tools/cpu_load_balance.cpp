#include "cpu_load_balance.h"

CPULoadBalanceParams::CPULoadBalanceParams(size_t amt_work, unsigned long min_per_thread) : min_per_thread(min_per_thread)
                                                                                          , max_threads((amt_work + min_per_thread - 1u) / min_per_thread)
                                                                                          , hw_threads(std::thread::hardware_concurrency())
                                                                                          , num_threads(std::min(hw_threads != 0u ? hw_threads : 2u, max_threads))
                                                                                          , block_size(amt_work / num_threads) {}