#pragma once
#include <lunar/api.hpp>
#include <lunar/debug/log.hpp>
#include <cassert>

constexpr inline void do_assert(bool condition, const char* message)
{
	if (!condition)
	{
		DEBUG_ERROR("Assert condition \"{}\" failed!", message);
		throw;
	}
}


#ifdef LUNAR_DEBUG_BUILD
#	define DEBUG_ASSERT(condition) do_assert(condition, #condition)
#	define DEBUG_ONLY_EXPR(expr) expr
#else
#	define DEBUG_ASERT(condition)
#	define DEBUG_ONLY_EXPR(expr)
#endif

#define DEBUG_INIT_CHECK() DEBUG_ASSERT(initialized == true)
#define DEBUG_NOT_IMPLEMENTED() throw
