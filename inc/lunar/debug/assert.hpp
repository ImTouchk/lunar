#pragma once
#include <debug/log.hpp>
#include <cassert>

constexpr inline void do_assert(bool condition, const char* message)
{
	if (!condition)
	{
		DEBUG_ERROR("Assert condition \"{}\" failed!", message);
		throw;
	}
}

#define DEBUG_ASSERT(condition) \
	do_assert(condition, #condition)
