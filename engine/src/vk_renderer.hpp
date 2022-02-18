#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <shared_mutex>
#include <optional>
#include <vector>
#include <memory>
#include <array>
#include <any>

#include "render/renderer.hpp"

class GameWindow;

namespace Vk
{
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    struct MeshHandleInternalData
    {
        bool needsUpdating;
        VkBuffer vertexBuffer;
        VkDeviceMemory vbMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory ibMemory;
        unsigned shader;
    };

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
        void create_framebuffers();

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

    struct CommandQueueWrapper
    {
    public:
        CommandQueueWrapper() = default;
        ~CommandQueueWrapper() = default;

        void create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface, VkPipeline pipeline);
        void destroy();

        [[nodiscard]] std::vector<VkCommandBuffer>& buffers();

    private:
        SurfaceWrapper* pSurface = nullptr;
        SwapchainWrapper* pSwapchain = nullptr;
        LogicalDeviceWrapper* pDevice = nullptr;

        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers = {};
    };

    class SyncObjectsWrapper
    {
    public:
        SyncObjectsWrapper() = default;
        ~SyncObjectsWrapper() = default;

        void create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain);
        void destroy();

        [[nodiscard]] VkFence& image_in_flight(unsigned i);
        [[nodiscard]] VkFence& in_flight_fence(unsigned i);
        [[nodiscard]] VkSemaphore& image_available(unsigned i);
        [[nodiscard]] VkSemaphore& rendering_finished(unsigned i);

    private:
        LogicalDeviceWrapper* pDevice = nullptr;

        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores = {};
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderingFinishedSemaphore = {};
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences = {};
        std::vector<VkFence> imagesInFlight = {};
    };

    enum class ShaderType
    {
        eUnknown = 0,
        eGraphics,
        eCompute,
    };

    struct ShaderData
    {
        ShaderType type = ShaderType::eUnknown;
        VkPipelineLayout layout = VK_NULL_HANDLE;
        VkPipeline handle = VK_NULL_HANDLE;
    };

    struct ShaderManager
    {
    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        void create(LogicalDeviceWrapper& device);
        void destroy();

        void create_graphics(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);
        //void create_compute();

    private:
        LogicalDeviceWrapper* pDevice = nullptr;
        std::vector<ShaderData> shaders;
    };

    struct ObjectManager
    {
    public:
        ObjectManager() = default;
        ~ObjectManager() = default;

        void create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface);
        void destroy();

        CMesh create_object(const MeshCreateInfo& meshCreateInfo);

    private:
        void create_memory_allocator();
        void destroy_memory_allocator();

        void create_command_pool();
        void destroy_command_pool();

        void update_command_buffer_block(unsigned block);

    private:
        LogicalDeviceWrapper* pDevice = nullptr;
        SurfaceWrapper* pSurface = nullptr;
        SwapchainWrapper* pSwapchain = nullptr;

        VmaAllocator memoryAllocator = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkCommandBuffer mainCmdBuffer = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> secondaryCmdBuffers = {};
    };

    struct RendererInternalData
    {
        SurfaceWrapper surface;
        LogicalDeviceWrapper device;
        SwapchainWrapper swapchain;
        CommandQueueWrapper commandQueue;
        SyncObjectsWrapper syncObjects;
        ObjectManager objectManager;
        std::vector<ShaderData> shaders;
        bool hasOptionalDynamicRendering;
        size_t currentFrame;
    };

    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> surfacePresentModes;

        static const SwapchainSupportDetails query(VkPhysicalDevice device, VkSurfaceKHR surface);
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
