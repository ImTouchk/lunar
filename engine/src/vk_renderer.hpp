#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
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

    struct SwapchainWrapper
    {
    public:
        SwapchainWrapper() = default;
        ~SwapchainWrapper() = default;

        void create(GameWindow& window, SurfaceWrapper& surface, LogicalDeviceWrapper& device);
        void destroy();

        void resize(GameWindow& window, SurfaceWrapper& surface);

        [[nodiscard]] VkSwapchainKHR handle() const;
        [[nodiscard]] VkRenderPass render_pass() const;
        [[nodiscard]] VkExtent2D surface_extent() const;
        [[nodiscard]] VkFormat surface_format() const;
        [[nodiscard]] const std::vector<VkFramebuffer>& frame_buffers() const;
        [[nodiscard]] const std::vector<VkImageView>& image_views() const;
        [[nodiscard]] int get_width() const;
        [[nodiscard]] int get_height() const;

    private:
        void create_swapchain(GameWindow& window, SurfaceWrapper& surface);
        void create_render_pass();
        void create_image_views();

        LogicalDeviceWrapper* pDevice = nullptr;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFormat surfaceFormat = {};
        VkExtent2D surfaceExtent = {};
        std::vector<VkImage> images = {};
        std::vector<VkImageView> views = {};
        std::vector<VkFramebuffer> frameBuffers = {};
        int width = 0;
        int height = 0;
    };

    struct MemoryAllocatorWrapper
    {
    public:
        MemoryAllocatorWrapper() = default;
        ~MemoryAllocatorWrapper() = default;

        void create(LogicalDeviceWrapper& device);
        void destroy();

        VmaAllocator handle() const;
    private:
        VmaAllocator vmaAllocator;
    };

    struct ShaderPipelineCreateInfo
    {
        const std::vector<char>& vertexCode;
        const std::vector<char>& fragmentCode;
        SwapchainWrapper& swapchain;
        LogicalDeviceWrapper& device;
    };

    struct ShaderPipeline
    {
    public:
        ShaderPipeline() = default;
        ~ShaderPipeline() = default;

        void create(const std::vector<char>& vertexCode, const std::vector<char>& fragmentCode, SwapchainWrapper& swapchain, LogicalDeviceWrapper& device);
        void destroy(LogicalDeviceWrapper& device);

    private:
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    };

    struct RendererInternalData
    {
        SurfaceWrapper surface;
        LogicalDeviceWrapper device;
        SwapchainWrapper swapchain;
        MemoryAllocatorWrapper memoryAllocator;
        ShaderPipeline shader;
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> surfacePresentModes;

        static const SwapchainSupportDetails& query(VkPhysicalDevice device, VkSurfaceKHR surface);
    };

    struct QueueFamilyIndices
    {
        std::optional<unsigned> graphics;
        std::optional<unsigned> present;

        [[nodiscard]] bool is_complete() const
        {
            return graphics.has_value() && present.has_value();
        }

        static const QueueFamilyIndices& query(VkPhysicalDevice device, VkSurfaceKHR surface);
    };

    VkInstance GetInstance();
    VkPhysicalDevice GetRenderingDevice();
    std::vector<const char*> GetRequiredDeviceExtensions();
    std::vector<const char*> GetAvailableOptionalExtensions(VkPhysicalDevice device);
    std::vector<const char*> GetDebugLayers();
}
