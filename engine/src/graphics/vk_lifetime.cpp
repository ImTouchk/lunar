#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
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
            GetMemoryAllocator();
            CreateCommandSubmitter();
		}
	}

	void SignalRendererDestroy()
	{
		std::unique_lock lock(COUNTER_MUTEX);
		if (--RENDERER_COUNT == 0)
		{
            DestroyCommandSubmitter();
            vmaDestroyAllocator(GetMemoryAllocator());
			vkDestroyDevice(GetDevice().handle, nullptr);
		}
	}

    const VmaAllocator& GetMemoryAllocator()
    {
        static VmaAllocator ALLOCATOR = VK_NULL_HANDLE;

        if(ALLOCATOR == VK_NULL_HANDLE)
        {
            VmaAllocatorCreateInfo allocator_create_info =
            {
                .physicalDevice   = GetRenderingDevice(),
                .device           = GetDevice().handle,
                .instance         = GetInstance(),
                .vulkanApiVersion = VK_API_VERSION_1_2
            };

            VkResult result;
            result = vmaCreateAllocator(&allocator_create_info, &ALLOCATOR);
            if(result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Memory allocator creation fail.");
                throw std::runtime_error("Renderer-Vulkan-MemoryAllocator-CreationFail");
            }

            CDebug::Log("Vulkan Renderer | Memory allocator created.");
        }

        return ALLOCATOR;
    }
}
