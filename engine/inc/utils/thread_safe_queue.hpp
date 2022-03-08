#pragma once
#include <queue>
#include <mutex>

template<typename T>
class CThreadSafeQueue
{
public:
    CThreadSafeQueue() = default;
    CThreadSafeQueue(const CThreadSafeQueue&) = delete;
    CThreadSafeQueue& operator=(const CThreadSafeQueue&) = delete;

    void pop()
    {
        std::unique_lock lock(mutex);
        queue.pop();
    }

    void emplace(const T& object)
    {
        std::unique_lock lock(mutex);
        queue.push(std::move(object));
    }

    void emplace(T&& object)
    {
        std::unique_lock lock(mutex);
        queue.push(std::move(object));
    }

    [[nodiscard]] const T& front() const
    {
        std::unique_lock lock(mutex);
        return queue.front();
    }

    [[nodiscard]] unsigned size() const
    {
        std::unique_lock lock(mutex);
        return static_cast<unsigned>(queue.size());
    }

    [[nodiscard]] bool empty() const
    {
        std::unique_lock lock(mutex);
        return queue.empty();
    }

private:
    std::queue<T> queue = {};
    std::mutex mutex    = {};
};
