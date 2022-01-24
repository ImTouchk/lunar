#pragma once

template<typename T>
struct range
{
private:
    T last;
    T iter;

public:
    range(T start, T end) : iter(start), last(end) {}
    range(const range& other) : iter(other.iter), last(other.last) {}
    template<typename U> range(T start, U end) : iter(start), last(static_cast<T>(end)) {}

    const range& begin() const { return *this; }
    const range& end() const { return *this; }

    bool operator!=(const range&) const { return iter <= last; }
    void operator++() { ++iter; }

    T operator*() const { return iter; }
};