#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "utils/thread_pool.hpp"
#include "render/renderer.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <stdexcept>

namespace Vk
{
    void ObjectManager::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface)
    {
        pDevice = &device;
        pSurface = &surface;
        pSwapchain = &swapchain;

        create_memory_allocator();
        create_command_pool();

        CDebug::Log("Vulkan Renderer | Object manager created.");
    }

    void ObjectManager::destroy()
    {
        destroy_command_pool();
        destroy_memory_allocator();

        CDebug::Log("Vulkan Renderer | Object manager destroyed.");
    }

    void ObjectManager::create_memory_allocator()
    {
        VmaAllocatorCreateInfo allocator_create_info =
        {
            .physicalDevice   = GetRenderingDevice(),
            .device           = pDevice->handle(),
            .instance         = GetInstance(),
            .vulkanApiVersion = VK_API_VERSION_1_2
        };

        VkResult result;
        result = vmaCreateAllocator(&allocator_create_info, &memoryAllocator);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Object manager creation failed (vmaCreateAllocator didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-MemAllocatorCreationFail");
        }
    }

    void ObjectManager::destroy_memory_allocator()
    {
        vmaDestroyAllocator(memoryAllocator);
    }

    void ObjectManager::create_command_pool()
    {
        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        VkCommandPoolCreateInfo pool_create_info =
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = 0,
            .queueFamilyIndex = queue_indices.graphics.value()
        };

        VkResult result;
        result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &commandPool);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Object manager creation failed (vkCreateCommandPool didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandPoolCreationFail");
        }
    }

    void ObjectManager::destroy_command_pool()
    {
        vkDestroyCommandPool(pDevice->handle(), commandPool, nullptr);
    }

    CMesh ObjectManager::create_object(const MeshCreateInfo& meshCreateInfo)
    {
        return {};
    }
}
