#include "core/time.hpp"
#include "utils/fast_mutex.hpp"

#ifdef GLFW_WINDOW_BACKEND
#	include <GLFW/glfw3.h>
#endif

namespace CTime
{
	CMutex TIME_MUTEX   = {};
	float  LAST_TIME    = 0.f;
	float  CURRENT_TIME = 0.f;

	void Update()
	{
		CLockGuard lock(TIME_MUTEX);
		LAST_TIME = CURRENT_TIME;
		CURRENT_TIME = static_cast<float>(glfwGetTime());
	}

	float DeltaTime()
	{
		CLockGuard lock(TIME_MUTEX);
		return CURRENT_TIME - LAST_TIME;
	}
}
