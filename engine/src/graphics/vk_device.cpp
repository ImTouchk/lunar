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
    LogicalDeviceWrapperV2 LOGICAL_DEVICE = {};
    bool LOGICAL_DEVICE_CREATED = false;

    void CreateLogicalDevice()
    {
        const auto physical_device      = GetRenderingDevice();
        const auto queue_indices        = GetPhysicalDeviceQueues(physical_device);
        const auto device_layers        = GetDebugLayers();
        const auto optional_device_exts = GetAvailableOptionalExtensions(physical_device);

        auto device_extensions = GetRequiredDeviceExtensions();
        device_extensions.insert
        (
            std::end(device_extensions),
            std::begin(optional_device_exts),
            std::end  (optional_device_exts)
        );

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
        std::set<unsigned> unique_families =
        {
            static_cast<unsigned>(queue_indices.present),
            static_cast<unsigned>(queue_indices.graphics)
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
        result = vkCreateDevice(physical_device, &device_create_info, nullptr, &LOGICAL_DEVICE.handle);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Logical device creation fail.");
            throw std::runtime_error("Renderer-Vulkan-LogicalDevice-CreationFail");
        }

        vkGetDeviceQueue(LOGICAL_DEVICE.handle, queue_indices.present, 0, &LOGICAL_DEVICE.present);
        vkGetDeviceQueue(LOGICAL_DEVICE.handle, queue_indices.graphics, 0, &LOGICAL_DEVICE.graphics);
        CDebug::Log("Vulkan Renderer | Logical device created.");

        LOGICAL_DEVICE_CREATED = true;
    }

    const LogicalDeviceWrapperV2& GetDevice()
    {
        if(!LOGICAL_DEVICE_CREATED)
        {
            CreateLogicalDevice();
        }

        return LOGICAL_DEVICE;
    }

    QueueIndices GetQueueIndices()
    {
        return GetPhysicalDeviceQueues(GetRenderingDevice());
    }
}