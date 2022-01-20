#pragma once
#include "include/fmt/core.h"
#include "include/fmt/color.h"
#define MAX_LINE_CHARACTERS 80

namespace CDebug
{
    namespace
    {
        template<typename... T>
        void Print(const char* title, fmt::color color, fmt::format_string<T...> format_string, T&&... args)
        {
#       ifdef PRINT_DEBUG_OUTPUT
            auto result = fmt::vformat(format_string, fmt::make_format_args(args...));

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

    template<typename... T>
    void Log(fmt::format_string<T...> format_string, T&&... args)
    {
        Print("INFO", fmt::color::white, format_string, args...);
    }

    template<typename... T>
    void Error(fmt::format_string<T...> format_string, T&&... args)
    {
        Print("ERROR", fmt::color::red, format_string, args...);
    }

    template<typename... T>
    void Warn(fmt::format_string<T...> format_string, T&&... args)
    {
        Print("WARNING", fmt::color::gold, format_string, args...);
    }
}

