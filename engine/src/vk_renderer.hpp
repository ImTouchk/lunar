#pragma once
#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

class GameWindow;

namespace Vk
{
    struct SurfaceWrapper
    {
    public:
        SurfaceWrapper() = default;
        ~SurfaceWrapper() = default;

        void create(GameWindow& window);
        void destroy();

        [[nodiscard]] VkSurfaceKHR handle() const;

    private:
        VkSurfaceKHR surface = VK_NULL_HANDLE;
    };

    struct LogicalDeviceWrapper
    {
    public:
        LogicalDeviceWrapper() = default;
        ~LogicalDeviceWrapper() = default;

        void create(SurfaceWrapper& surface);
        void destroy();

        [[nodiscard]] VkDevice handle() const;
        [[nodiscard]] VkQueue present_queue() const;
        [[nodiscard]] VkQueue graphics_queue() const;

    private:
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;
    };

    struct RendererInternalData
    {
        SurfaceWrapper surface;
        LogicalDeviceWrapper device;
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

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    VkInstance GetInstance();
    VkPhysicalDevice GetRenderingDevice();
    std::vector<const char*> GetRequiredDeviceExtensions();
    std::vector<const char*> GetDebugLayers();
}
