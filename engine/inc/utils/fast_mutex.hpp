#pragma once
#include <mutex>

#ifdef WIN32
#	include <Windows.h>
#endif

class CMutex
{
public:
	CMutex();
	~CMutex();

	CMutex(const CMutex&) = delete;
	CMutex& operator=(const CMutex&) = delete;

	void lock() noexcept;
	void unlock() noexcept;
	[[nodiscard]] bool try_lock() noexcept;
private:
#ifdef WIN32
	CRITICAL_SECTION section;
#else
	std::mutex lock;
#endif
};

using CLockGuard = std::lock_guard<CMutex>;
