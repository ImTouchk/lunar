#pragma once
#include <deque>
#include <queue>
#include <mutex>

template<typename T>
class CThreadSafeQueue
{
public:
    CThreadSafeQueue() = default;
    CThreadSafeQueue(const CThreadSafeQueue&) = delete;
    CThreadSafeQueue& operator=(const CThreadSafeQueue&) = delete;

    using iterator = std::deque<T>::iterator;

    void clear()
    {
        std::unique_lock lock(mutex);
        queue.clear();
    }

    void pop()
    {
        std::unique_lock lock(mutex);
        queue.pop_front();
    }

    void emplace(const T& object)
    {
        std::unique_lock lock(mutex);
        queue.push_front(std::move(object));
    }

    void emplace(T&& object)
    {
        std::unique_lock lock(mutex);
        queue.push_front(std::move(object));
    }

    [[nodiscard]] T& front()
    {
        std::unique_lock lock(mutex);
        return queue.front();
    }

    [[nodiscard]] unsigned size()
    {
        std::unique_lock lock(mutex);
        return static_cast<unsigned>(queue.size());
    }

    [[nodiscard]] bool empty()
    {
        std::unique_lock lock(mutex);
        return queue.empty();
    }

    [[nodiscard]] iterator begin()
    {
        std::unique_lock lock(mutex);
        return queue.begin();
    }

    [[nodiscard]] iterator end()
    {
        std::unique_lock lock(mutex);
        return queue.end();
    }

private:
    std::deque<T> queue = {};
    std::mutex mutex    = {};
};
