#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <set>

namespace Vk
{
    void LogicalDeviceWrapper::create(SurfaceWrapper& surface)
    {
        assert(device == VK_NULL_HANDLE);

        const auto physical_device = GetRenderingDevice();
        const auto queue_families = GetQueueFamilies(physical_device, surface.handle());
        const auto device_extensions = GetRequiredDeviceExtensions();
        const auto device_layers = GetDebugLayers();

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<unsigned> unique_families =
        {
            static_cast<unsigned>(queue_families.graphics.value()),
            static_cast<unsigned>(queue_families.present.value())
        };

        const float queue_priority = 1.f;
        for(unsigned i : range(0, unique_families.size()))
        {
            VkDeviceQueueCreateInfo queue_create_info =
            {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = i,
                .queueCount       = 1,
                .pQueuePriorities = &queue_priority
            };

            queue_create_infos.push_back(queue_create_info);
        }

        VkPhysicalDeviceFeatures device_features = {};
        VkDeviceCreateInfo device_create_info =
        {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = nullptr,
            .queueCreateInfoCount    = static_cast<unsigned>(queue_create_infos.size()),
            .pQueueCreateInfos       = queue_create_infos.data(),
            .enabledLayerCount       = static_cast<unsigned>(device_layers.size()),
            .ppEnabledLayerNames     = device_layers.data(),
            .enabledExtensionCount   = static_cast<unsigned>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
            .pEnabledFeatures        = &device_features
        };

        VkResult result;
        result = vkCreateDevice(physical_device, &device_create_info, nullptr, &device);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Failed to create a logical device (vkCreateDevice did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-LogicalDevice-CreationFail");
        }

        vkGetDeviceQueue(device, queue_families.present.value(), 0, &presentQueue);
        vkGetDeviceQueue(device, queue_families.graphics.value(), 0, &graphicsQueue);
        CDebug::Log("Vulkan Renderer | Logical device created.");
    }

    void LogicalDeviceWrapper::destroy()
    {
        assert(device != VK_NULL_HANDLE);
        vkDestroyDevice(device, nullptr);
        graphicsQueue = VK_NULL_HANDLE;
        presentQueue = VK_NULL_HANDLE;
        device = VK_NULL_HANDLE;

        CDebug::Log("Vulkan Renderer | Logical device destroyed.");
    }

    VkDevice LogicalDeviceWrapper::handle() const
    {
        assert(device != VK_NULL_HANDLE);
        return device;
    }

    VkQueue LogicalDeviceWrapper::graphics_queue() const
    {
        assert(graphicsQueue != VK_NULL_HANDLE);
        return graphicsQueue;
    }

    VkQueue LogicalDeviceWrapper::present_queue() const
    {
        assert(presentQueue != VK_NULL_HANDLE);
        return presentQueue;
    }
}