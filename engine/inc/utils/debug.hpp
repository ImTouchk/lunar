#pragma once
#include <string_view>
#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>
#define MAX_LINE_CHARACTERS 80

namespace CDebug
{
    namespace
    {
        template<typename... T>
        void Print(const char* title, fmt::color color, std::string_view formatString, T&&... args)
        {
#       ifdef PRINT_DEBUG_OUTPUT
            auto result = fmt::vformat(formatString, fmt::make_format_args(args...));

            fmt::print(fmt::fg(color), "[{}] ", title);
            for(int i = 0; i < result.size(); i += MAX_LINE_CHARACTERS)
            {
                if(i != 0)
                {
                    fmt::print(" | ");
                }

                fmt::print(fmt::fg(color), "{:.{}}\n", result.data() + i, MAX_LINE_CHARACTERS);
            }
#       endif
        }
    }

    template<typename S, typename... Args>
    void Log(const S& formatString, Args&&... args)
    {
        Print("INFO", fmt::color::white, formatString, args...);
    }

    template<typename S, typename... Args>
    void Error(const S& formatString, Args&&... args)
    {
        Print("ERROR", fmt::color::red, formatString, args...);
    }

    template<typename S, typename... Args>
    void Warn(const S& formatString, Args&&... args)
    {
        Print("WARNING", fmt::color::gold, formatString, args...);
    }
}

