#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <mutex>

namespace Vk
{
	std::mutex COUNTER_MUTEX = {};
	int RENDERER_COUNT = 0;

	void SignalRendererCreation()
	{
		std::unique_lock lock(COUNTER_MUTEX);
		// fun fact: that's the post increment operator, so it evaluates the condition first then adds 1 later
		if (RENDERER_COUNT++ == 0)
		{
			GetDevice();
		}
	
	
	}

	void SignalRendererDestroy()
	{
		std::unique_lock lock(COUNTER_MUTEX);
		if (RENDERER_COUNT-- == 0)
		{
			vkDestroyDevice(GetDevice().handle, nullptr);
		}
	}
}
