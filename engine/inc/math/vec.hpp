#ifdef USE_OWN_IMPLEMENTATION
#pragma once
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <type_traits>
#include <array>
#include <cmath>

template<typename T, unsigned U>
struct vec
{
private:
    static_assert(U <= 4 && U > 0, "vec<T, U> | U must be in the interval [0, 4]");
    static_assert(std::is_arithmetic_v<T>, "vec<T, U> | T must be a number type");
    const unsigned scalar_values = (U != 4) ? U : 3; // for vec4 there are only 3 scalar values & the W component

public:
    [[nodiscard]] T& values(unsigned pos)
    {
        T* pBuf = reinterpret_cast<T*>(this);
        return pBuf[pos];
    }

    template<typename V, unsigned W>
    explicit vec(const vec<V, W>& other)
    {
        static_assert(W <= U, "vec<T, U> | Cannot create template copy ctor vec<V, W> | W must be between [0, U]");
        for(int i = 0; i < W; i++)
        {
            values(i) = static_cast<T>(other.values(i));
        }
    }

    vec(const vec& other)
    {
        for (int i = 0; i < U; i++)
        {
            values(i) = other.values(i);
        }
    }

    explicit vec(const T& a)
    {
        for(int i = 0; i < scalar_values; i++)
        {
            values(i) = a;
        }
    }

    vec()
    {
        for(int i = 0; i < U; i++)
        {
            values(i) = static_cast<T>(0);
        }
    }

    [[nodiscard]] T length() const
    {
        T square_sum = 0;
        for(int i = 0; i < scalar_values; i++)
        {
            square_sum += (values(i) * values(i));
        }
        return std::sqrt(square_sum);
    }

    [[nodiscard]] vec normalize() const
    {
        return *this / length();
    }

    [[nodiscard]] T dot(const vec& other)
    {
        T sum = 0;
        for(int i = 0; i < scalar_values; i++)
        {
            sum += (values(i) * other.values(i));
        }
        return sum;
    }

    [[nodiscard]] T angle(const vec& other)
    {
        return dot(other) / (length() * other.length());
    }

    [[nodiscard]] vec operator-() const
    {
        vec result;
        for(int i = 0; i < scalar_values; i++)
        {
            result.values(i) = -values(i);
        }

        if(U == 4)
        {
            result.values(3) = values(3); // w component
        }
        return result;
    }

#   define ARITHMETIC_VEC_VEC(op) \
    [[nodiscard]] vec operator op(const vec& other) const    \
    {                                                        \
        vec result;                                          \
        for(int i = 0; i < scalar_values; i++)               \
        { result.values(i) = values(i) op other.values(i); } \
        return result;                                       \
    }

#   define ARITHMETIC_VEC_T(op)                         \
    [[nodiscard]] vec operator op(const T& a) const     \
    {                                                   \
        vec result;                                     \
        for(int i = 0; i < scalar_values; i++)          \
        { result.values(i) = values(i) op a; }          \
        return result;                                  \
    }

#   define APPEND_VEC_VEC(op)                           \
    vec& operator op(const vec& other)                  \
    {                                                   \
        for(int i = 0; i < scalar_values; i++)          \
        { values(i) op##= other.values(i); }            \
        return *this;                                   \
    }

#   define APPEND_VEC_T(op)                             \
    vec& operator op(const T& a)                        \
    {                                                   \
        for(int i = 0; i < scalar_values; i++)          \
        { values(i) op##= a; }                          \
        return *this;                                   \
    }

    ARITHMETIC_VEC_VEC(+) ARITHMETIC_VEC_T(+) APPEND_VEC_VEC(+) APPEND_VEC_T(+)
    ARITHMETIC_VEC_VEC(-) ARITHMETIC_VEC_T(-) APPEND_VEC_VEC(-) APPEND_VEC_T(-)
    ARITHMETIC_VEC_VEC(*) ARITHMETIC_VEC_T(*) APPEND_VEC_VEC(*) APPEND_VEC_T(*)
    ARITHMETIC_VEC_VEC(/) ARITHMETIC_VEC_T(/) APPEND_VEC_VEC(/) APPEND_VEC_T(/)

#   undef ARITHMETIC_VEC_T
#   undef ARITHMETIC_VEC_VEC
#   undef APPEND_VEC_VEC
#   undef APPEND_VEC_T

    [[nodiscard]] std::string to_string() const
    {
        return fmt::format("vec{} {}", U, values());
    }
};

struct vec2f : public vec<float, 2>
{
public:
    float x;
    float y;

    vec2f(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
};

struct vec3f : public vec<float, 3>
{
public:
    float x;
    float y;
    float z;

    vec3f(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

struct vec4f : public vec<float, 4>
{
public:
    float x;
    float y;
    float z;
    float w;
};
#endif