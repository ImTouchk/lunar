#pragma once
#include <functional>
#include <memory>
#include <future>

namespace CThreadPool
{
    void Initialize();
    void Stop();

    void DoTask(const std::function<void()>& task);
    
    template<
        typename Fn, 
        typename... Args,
        typename FnRes = std::invoke_result_t<Fn&&, Args&&...>
    >
    std::future<FnRes> DoTaskAsync(const Fn& function, Args&&... args)
    {
        auto promise = std::make_shared<std::promise<FnRes>>();
        auto future = promise->get_future();

        auto task = 
        [
            func     = std::move(function), 
            ...largs = std::move(args), 
            prom     = promise
        ]()
        {
            prom->set_value(func(largs...));
        };

        DoTask(std::move(task));

        return future;
    }

    unsigned GetThreadCount();
}
