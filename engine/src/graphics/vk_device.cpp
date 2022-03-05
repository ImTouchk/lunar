#define VK_USE_PLATFORM_WIN32_KHR

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
    QueueIndices GetPhysicalDeviceQueues(VkPhysicalDevice device)
    {
        static VkPhysicalDevice cached_device = VK_NULL_HANDLE;
        static QueueIndices cached_value      = { 0, 0 };

        if(cached_device == device)
        {
            return cached_value;
        }

        cached_value  = { UINT_MAX, UINT_MAX };
        cached_device = device;

        uint32_t family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, nullptr);

        auto families = std::vector<VkQueueFamilyProperties>(family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &family_count, families.data());

        for(auto i : range(0, family_count - 1))
        {
            VkBool32 can_present = false;
            can_present = vkGetPhysicalDeviceWin32PresentationSupportKHR(device, i);
            if(can_present)
            {
                cached_value.present = i;
            }
            if(families.at(i).queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                cached_value.graphics = i;
            }
            if(cached_value.complete())
            {
                break;
            }
        }

        return cached_value;
    }

    std::vector<const char*> GetRequiredDeviceExtensions()
    {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    std::vector<VkExtensionProperties> GetAvailableDeviceExtensions(VkPhysicalDevice device)
    {
        unsigned count;
        VkResult result;
        result = vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
        if(result != VK_SUCCESS)
        {
            CDebug::Warn
            (
                "Vulkan Renderer | Could not retrieve available device extensions"
                "(vkEnumerateDeviceExtensionProperties did not return VK_SUCCESS)."
            );
            return {};
        }

        auto extensions = std::vector<VkExtensionProperties>(count);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
        return extensions;
    }

    bool DeviceHasRequiredExtensions(VkPhysicalDevice device)
    {
        auto required_extensions = GetRequiredDeviceExtensions();
        auto available_extensions = GetAvailableDeviceExtensions(device);

        for(const auto& available : available_extensions)
        {
            const auto& name = available.extensionName;
            for(const auto& required : required_extensions)
            {
                if(strcmp(name, required) != 0)
                    continue;

                required_extensions.erase
                (
                    std::remove
                            (
                                    required_extensions.begin(),
                                    required_extensions.end(),
                                    required
                            ),
                    required_extensions.end()
                );
            }
        }

        return required_extensions.empty();
    }

    std::vector<const char*> GetAvailableOptionalExtensions(VkPhysicalDevice device)
    {
        auto optional_extensions = std::vector<const char*>
        {
            VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
        };

        auto available_extensions = GetAvailableDeviceExtensions(device);
        auto final_extensions = std::vector<const char*>();

        for(const auto& available : available_extensions)
        {
            const auto& name = available.extensionName;
            for(const auto& optional : optional_extensions)
            {
                if(strcmp(name, optional) != 0)
                    continue;

                final_extensions.emplace_back(optional);
            }
        }

        return final_extensions;
    }

    unsigned GetDeviceScore(VkPhysicalDevice device)
    {
        if(!DeviceHasRequiredExtensions(device))
        {
            return 0;
        }

        auto device_queues = GetPhysicalDeviceQueues(device);
        if(!device_queues.complete())
        {
            return 0;
        }

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties device_memory_properties;

        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceMemoryProperties(device, &device_memory_properties);

        unsigned final_score = 0;

        unsigned local_vram = 0;
        for(auto i : range(0, device_memory_properties.memoryHeapCount))
        {
            VkMemoryHeap heap = device_memory_properties.memoryHeaps[i];
            if(heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)
            {
                local_vram += (static_cast<unsigned>(heap.size) / 1024 / 1024);
            }
        }

        // the number of GBs of VRAM will be added to the score
        // it's not a good way of measuring performance, but it will do
        final_score += (local_vram / 1024);

        if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            final_score += 10;
        }
        else if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            final_score += 1;
        }

        final_score += static_cast<unsigned>(GetAvailableOptionalExtensions(device).size());

        CDebug::Log
        (
            "Vulkan Renderer | Found rendering device \"{}\". API version: {}. Local memory: {}MB. Final score: {}",
            device_properties.deviceName,
            device_properties.apiVersion,
            local_vram,
            final_score
        );

        return final_score;
    }

    std::vector<VkPhysicalDevice> GetAvailableDevices()
    {
        unsigned count;
        vkEnumeratePhysicalDevices(GetInstance(), &count, nullptr);

        auto devices = std::vector<VkPhysicalDevice>(count);
        vkEnumeratePhysicalDevices(GetInstance(), &count, devices.data());
        return devices;
    }

    VkPhysicalDevice GetRenderingDevice()
    {
        static VkPhysicalDevice best_gpu = VK_NULL_HANDLE;

        if(best_gpu != VK_NULL_HANDLE)
        {
            return best_gpu;
        }

        unsigned best_score = 0;

        auto devices = GetAvailableDevices();
        for(const auto& device : devices)
        {
            unsigned score;
            score = GetDeviceScore(device);
            if(score > best_score)
            {
                best_score = score;
                best_gpu = device;
            }
        }

        if(best_gpu == VK_NULL_HANDLE)
        {
            CDebug::Error("Vulkan Renderer | No graphics card is suitable for rendering.");
            throw std::runtime_error("Renderer-Vulkan-GraphicsCardsNotSuitable");
        }

        return best_gpu;
    }

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