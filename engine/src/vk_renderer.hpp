#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

class GameWindow;

namespace Vk
{
    struct RendererInternalData
    {

    };

    struct QueueFamilyIndices
    {
        std::optional<unsigned> graphics;
        std::optional<unsigned> present;

        [[nodiscard]] bool is_complete() const
        {
            return graphics.has_value() && present.has_value();
        }
    };

    VkInstance GetInstance();
    VkPhysicalDevice GetRenderingDevice();
    VkSurfaceKHR CreateWindowSurface(GameWindow& window);
    std::vector<const char*> GetRequiredDeviceExtensions();
}
