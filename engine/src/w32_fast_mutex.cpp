#include "utils/fast_mutex.hpp"

#include <Windows.h>
#include <mutex>

CMutex::CMutex() : section()
{
	InitializeCriticalSection(&section);
}

CMutex::~CMutex()
{
	DeleteCriticalSection(&section);
}

void CMutex::lock() noexcept
{
	EnterCriticalSection(&section);
}

void CMutex::unlock() noexcept
{
	LeaveCriticalSection(&section);
}

bool CMutex::try_lock() noexcept
{
	return TryEnterCriticalSection(&section);
}
