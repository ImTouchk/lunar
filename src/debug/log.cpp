#include <debug/log.hpp>
#include <iostream>
#include <format>
#include <chrono>
#include <cstdio>
#include <cstdarg>

void logf(std::string_view fn_name, std::string_view format, std::format_args&& args)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::format("[{}] ", now);

	auto str = std::vformat(format, args);
	std::cout << timestamp << std::format("\033[36m{}\033[39m ", fn_name) << str << "\n";
}

void warnf(std::string_view fn_name, std::string_view format, std::format_args&& args)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::format("[{}] ", now);

	auto str = std::vformat(format, args);
	std::cout << timestamp << std::format("\033[36m{} \033[33mWARNING\033[39m ", fn_name) << str << "\n";
}

void errorf(std::string_view fn_name, std::string_view format, std::format_args&& args)
{
	auto now = std::chrono::system_clock::now();
	auto timestamp = std::format("[{}] ", now);

	auto str = std::vformat(format, args);
	std::cout << timestamp << std::format("\033[36m{} \033[31mERROR\033[39m ", fn_name) << str << "\n";
}
