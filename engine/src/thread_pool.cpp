#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "utils/thread_pool.hpp"

#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <condition_variable>

std::vector<std::thread> WORKERS;
std::queue<std::function<void()>> WORK_QUEUE = {};
std::condition_variable CONDITION = {};
std::mutex WORK_MUTEX = {};
bool TERMINATE_POOL = false;

namespace CThreadPool
{
    void WorkerThread()
    {
        while(true)
        {
            std::function<void()> task = nullptr;

            {
                std::unique_lock<std::mutex> lock(WORK_MUTEX);
                CONDITION.wait(lock, []()
                {
                    return !WORK_QUEUE.empty() || TERMINATE_POOL;
                });

                if(TERMINATE_POOL)
                    return;

                task = WORK_QUEUE.front();
                WORK_QUEUE.pop();
            }

            task();
        }
    }

    void Initialize()
    {
        unsigned thread_count = 0;
        thread_count = std::thread::hardware_concurrency();
        for(auto i : range(0, thread_count - 1))
        {
            WORKERS.emplace_back(WorkerThread);
        }

        CDebug::Log("Thread Pool | Initialized {} threads.", thread_count);
    }

    void Stop()
    {
        TERMINATE_POOL = true;
        CONDITION.notify_all();

        for(auto& thread : WORKERS)
        {
            thread.join();
        }

        WORKERS.clear();

        CDebug::Log("Thread Pool | Threads stopped.");
    }

    void DoTask(const std::function<void()>& task)
    {
        {
            std::unique_lock<std::mutex> lock(WORK_MUTEX);
            WORK_QUEUE.push(task);
        }

        CONDITION.notify_one();
    }
}