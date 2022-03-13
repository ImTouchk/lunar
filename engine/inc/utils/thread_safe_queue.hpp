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

    [[nodiscard]] T get_and_pop_front()
    {
        std::unique_lock lock(mutex);
        T value = std::move(queue.front());
        queue.pop_front();
        return value;
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

    // Should be used only when you need to do multiple operations in sync
    void unsafe_lock()
    {
        mutex.lock();
    }

    void unsafe_unlock()
    {
        mutex.unlock();
    }

    std::deque<T>& unsafe_handle()
    {
        return queue;
    }

    class CThreadSafeIterable
    {
    public:
        using iterator = std::deque<T>::iterator;

        CThreadSafeIterable(CThreadSafeQueue& queue) : queue_wrapper(queue) { queue_wrapper.mutex.lock(); }
        ~CThreadSafeIterable() { queue_wrapper.mutex.unlock(); }

        [[nodiscard]] iterator begin() { return queue_wrapper.queue.begin(); }
        [[nodiscard]] iterator end() { return queue_wrapper.queue.end(); }

    private:
        CThreadSafeQueue<T>& queue_wrapper;
    };

    [[nodiscard]] CThreadSafeIterable safe_iterator()
    {
        return CThreadSafeIterable(*this);
    }

private:
    std::deque<T> queue = {};
    std::mutex mutex    = {};
};
