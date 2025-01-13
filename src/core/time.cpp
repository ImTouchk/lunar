#include <lunar/core/time.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <mutex>

namespace Time
{
	TimeContext_T TIME_CONTEXT = {};

	void Update()
	{
		auto& context = TIME_CONTEXT;
		context.lastTime    = context.currentTime.load();
		context.currentTime = glfwGetTime();
		context.deltaTime   = context.currentTime - context.lastTime;

		context.frames      = context.frames + 1;
		context.fps_sum    += glm::round(1.f / context.deltaTime);
		context.fps_avg     = context.fps_sum / context.frames;
	}

	TimeContext GetGlobalContext()
	{
		return &TIME_CONTEXT;
	}
}
