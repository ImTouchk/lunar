#include "core/time.hpp"
#include "utils/fast_mutex.hpp"

#ifdef GLFW_WINDOW_BACKEND
#	include <GLFW/glfw3.h>
#endif

namespace CTime
{
	CMutex TIME_MUTEX        = {};
	float  LAST_TIME         = 0.f;
	float  LAST_SECOND       = 0.f;
	float  CURRENT_TIME      = 0.f;
	size_t FRAME_COUNT       = 0;
	size_t FRAMES_PER_SECOND = 1;

	void Update()
	{
		CLockGuard lock(TIME_MUTEX);
		LAST_TIME = CURRENT_TIME;
		CURRENT_TIME = static_cast<float>(glfwGetTime());

		FRAME_COUNT++;
		if(CURRENT_TIME - LAST_SECOND >= 1.f)
		{
			FRAMES_PER_SECOND = (FRAMES_PER_SECOND + FRAME_COUNT) / 2;
			LAST_SECOND = CURRENT_TIME;
			FRAME_COUNT = 0;
		}
	}

	float DeltaTime()
	{
		CLockGuard lock(TIME_MUTEX);
		return CURRENT_TIME - LAST_TIME;
	}

	int FramesPerSecond()
	{
		CLockGuard lock(TIME_MUTEX);
		return static_cast<int>(FRAMES_PER_SECOND);
	}
}
