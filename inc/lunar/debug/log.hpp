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
	logf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
}

template<typename... Args>
void error(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	errorf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
}

template<typename... Args>
inline void warn(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	warnf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
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

#ifndef _MSC_VER
#   define LUNAR_FUNCTION __PRETTY_FUNCTION__
#else
#   define LUNAR_FUNCTION __FUNCTION__
#endif

#ifdef NDEBUG
#	define DEBUG_LOG(fmt, ...)
#	define DEBUG_ERROR(fmt, ...)
#   define DEBUG_WARN(fmt, ...)
#else
#	ifdef _MSC_VER
#		define DEBUG_LOG(fmt, ...) log(LUNAR_FUNCTION, fmt, __VA_ARGS__)
#		define DEBUG_WARN(fmt, ...) warn(LUNAR_FUNCTION, fmt, __VA_ARGS__)
#		define DEBUG_ERROR(fmt, ...) error(LUNAR_FUNCTION, fmt, __VA_ARGS__)
#	else
#		define DEBUG_LOG(fmt, ...) log(LUNAR_FUNCTION, fmt __VA_OPT__(,) __VA_ARGS__)
#		define DEBUG_ERROR(fmt, ...) error(LUNAR_FUNCTION, fmt __VA_OPT__(,) __VA_ARGS__)
#		define DEBUG_WARN(fmt, ...) warn(LUNAR_FUNCTION, fmt __VA_OPT__(,) __VA_ARGS__)
#	endif
#endif
