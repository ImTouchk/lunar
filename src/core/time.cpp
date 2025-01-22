#include <lunar/core/time.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <mutex>

namespace lunar::Time
{
	TimeContext_T TIME_CONTEXT = {};

	void Update()
	{
		auto& context = TIME_CONTEXT;
		context.lastTime    = context.currentTime.load();
		context.currentTime = glfwGetTime();
		context.deltaTime   = context.currentTime - context.lastTime;

		context.frames      = context.frames + 1;

		if (context.currentTime - context.timer >= 1.f)
		{
			context.fps    = context.frames / (context.currentTime - context.timer);
			context.timer  = context.currentTime.load();
			context.frames = 0;
		}
	}

	TimeContext GetGlobalContext()
	{
		return &TIME_CONTEXT;
	}

	double DeltaTime()
	{
		return TIME_CONTEXT.deltaTime;
	}
}
