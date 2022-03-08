#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "utils/thread_pool.hpp"
#include "utils/thread_safe_queue.hpp"

#include <vector>
#include <future>
#include <thread>
#include <functional>
#include <condition_variable>

namespace CThreadPool
{
    std::atomic<int> THREADS_IN_FLIGHT            = {0};
    std::mutex CONDITION_MUTEX                    = {};
    std::vector<std::jthread> THREADS             = {};
    std::condition_variable_any THREAD_CONDITION  = {};
    CThreadSafeQueue<std::function<void()>> QUEUE = {};

    void Initialize()
    {
        unsigned thread_count = std::thread::hardware_concurrency();
        for(auto i : range(1, thread_count))
        {
            THREADS.emplace_back([](const std::stop_token& stopToken)
            {
                while(true)
                {
                    std::unique_lock lock(CONDITION_MUTEX);
                    THREAD_CONDITION.wait(lock, stopToken, [&stopToken](){ return !QUEUE.empty(); });

                    if(stopToken.stop_requested())
                    {
                        return;
                    }

                    auto function = QUEUE.front();
                    std::invoke(std::move(function));
                    QUEUE.pop();

                    THREADS_IN_FLIGHT--;
                }
            });
        }

        CDebug::Log("Thread Pool | Initialized {} threads.", thread_count);
    }

    void Stop()
    {
        while(THREADS_IN_FLIGHT != 0)
        {
            std::this_thread::yield();
        }

        for(auto& thread : THREADS)
        {
            thread.request_stop();
        }

        THREAD_CONDITION.notify_all();

        CDebug::Log("Thread Pool | Threads stopped.");
    }

    void DoTask(const std::function<void()>& task)
    {
        QUEUE.emplace(task);
        THREADS_IN_FLIGHT++;
        THREAD_CONDITION.notify_one();
    }

    unsigned GetThreadCount()
    {
        return static_cast<unsigned>(THREADS.size());
    }
}