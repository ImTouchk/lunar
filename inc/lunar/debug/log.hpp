#pragma once
#include <lunar/api.hpp>
#include <source_location>
#include <format>
#include <cctype>

LUNAR_API void logf(std::string_view fn_name, std::string_view format, std::format_args&& args);
LUNAR_API void errorf(std::string_view fn_name, std::string_view format, std::format_args&& args);
LUNAR_API void warnf(std::string_view fn_name, std::string_view format, std::format_args&& args);

template<typename... Args>
void log(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	logf(fn_name, format, std::make_format_args(args...));
}

template<typename... Args>
void error(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	errorf(fn_name, format, std::make_format_args(args...));
}

template<typename... Args>
inline void warn(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	warnf(fn_name, format, std::make_format_args(args...));
}

inline std::string printable(const char c)
{
	if (isprint(c))
		return { c };
	else
	{
		switch (c)
		{
		case '\n': return "\\n";
		default: return "[?]";
		}
	}
}

inline std::string printable(const std::string& str)
{
	auto printableVersion = std::string("");
	for (auto it = str.begin(); it != str.end(); it++)
		printableVersion += printable(*it);

	return printableVersion;
}

#if LUNAR_DEBUG_BUILD == 1
#	define DEBUG_LOG(fmt, ...) log(LUNAR_FN_NAME, fmt __VA_OPT__(,) __VA_ARGS__)
#	define DEBUG_ERROR(fmt, ...) error(LUNAR_FN_NAME, fmt __VA_OPT__(,) __VA_ARGS__)
#	define DEBUG_WARN(fmt, ...) warn(LUNAR_FN_NAME, fmt __VA_OPT__(,) __VA_ARGS__)
#	ifdef LUNAR_VERBOSE
#		define DEBUG_INFO(fmt, ...) log(LUNAR_FN_NAME, fmt __VA_OPT__(,) __VA_ARGS__)
#	endif
#else
#	define DEBUG_LOG(fmt, ...)
#	define DEBUG_ERROR(fmt, ...)
#   define DEBUG_WARN(fmt, ...)
#	define DEBUG_INFO(fmt, ...)
#endif
