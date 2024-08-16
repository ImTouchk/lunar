#pragma once
#include <format>
#include <source_location>

void logf(std::string_view fn_name, std::string_view format, std::format_args&& args);
void errorf(std::string_view fn_name, std::string_view format, std::format_args&& args);
void warnf(std::string_view fn_name, std::string_view format, std::format_args&& args);

template<typename... Args>
inline void log(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	logf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
}

template<typename... Args>
inline void error(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	errorf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
}

template<typename... Args>
inline void warn(const std::string_view& fn_name, const std::string_view& format, Args&&... args)
{
	warnf(fn_name, format, std::make_format_args(std::forward<Args>(args)...));
}

#ifdef NDEBUG
#	define DEBUG_LOG(fmt, ...)
#	define DEBUG_ERROR(fmt, ...)
#else
#	define DEBUG_LOG(fmt, ...) log(__FUNCTION__, fmt, __VA_ARGS__)
#	define DEBUG_ERROR(fmt, ...) error(__FUNCTION__, fmt, __VA_ARGS__)
#	define DEBUG_WARN(fmt, ...) warn(__FUNCTION__, fmt, __VA_ARGS__)
#endif
