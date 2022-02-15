#pragma once
#include <functional>

namespace CThreadPool
{
    void Initialize();
    void Stop();

    void DoTask(const std::function<void()>& task);
}
