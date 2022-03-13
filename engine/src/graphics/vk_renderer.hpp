#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <shared_mutex>
#include <functional>
#include <optional>
#include <future>
#include <vector>
#include <memory>
#include <array>
#include <any>

#include "render/renderer.hpp"
#include "vk_buffer.hpp"
#include "vk_shader.hpp"
#include "vk_object.hpp"
#include "vk_texture.hpp"
#include "vk_draw_call.hpp"

class GameWindow;

namespace Vk
{
    struct SwapchainSupportDetails
    {
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> surfacePresentModes;

        static const SwapchainSupportDetails query(VkPhysicalDevice device, VkSurfaceKHR surface);
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

    struct SwapchainWrapper
    {
    public:
        SwapchainWrapper() = default;
        ~SwapchainWrapper() = default;

        void create(GameWindow& window, SurfaceWrapper& surface);
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
        [[nodiscard]] const VkViewport& get_viewport() const;
        [[nodiscard]] const VkRect2D& get_scissor() const;

    private:
        void create_swapchain(GameWindow& window, SurfaceWrapper& surface);
        void create_render_pass();
        void create_image_views();
        void create_framebuffers();
        void create_depth_buffer();

        struct
        {
            VmaAllocation allocation = VK_NULL_HANDLE;
            VkImage image            = VK_NULL_HANDLE;
            VkImageView view         = VK_NULL_HANDLE;
            VkFormat format          = {};
        } depthBuffer = {};

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        VkRenderPass renderPass  = VK_NULL_HANDLE;
        VkFormat surfaceFormat   = {};
        VkExtent2D surfaceExtent = {};
        VkViewport viewport      = {};
        VkRect2D scissor         = {};
        std::vector<VkImage> images             = {};
        std::vector<VkImageView> views          = {};
        std::vector<VkFramebuffer> frameBuffers = {};
        int width  = 0;
        int height = 0;
    };

    class SyncObjectsWrapper
    {
    public:
        SyncObjectsWrapper() = default;
        ~SyncObjectsWrapper() = default;

        void create(SwapchainWrapper& swapchain);
        void destroy();

        [[nodiscard]] VkFence& image_in_flight(size_t i);
        [[nodiscard]] VkFence& in_flight_fence(size_t i);
        [[nodiscard]] VkSemaphore& image_available(size_t i);
        [[nodiscard]] VkSemaphore& rendering_finished(size_t i);

    private:
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> imageAvailableSemaphores = {};
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> renderingFinishedSemaphore = {};
        std::array<VkFence, MAX_FRAMES_IN_FLIGHT> inFlightFences = {};
        std::vector<VkFence> imagesInFlight = {};
    };

    struct RendererInternalData
    {
        SurfaceWrapper surface;
        SwapchainWrapper swapchain;
        SyncObjectsWrapper syncObjects;
        ShaderManager shaderManager;
        ObjectManager objectManager;
        TextureManager textureManager;
        RenderCallManager renderCallManager;
    };

    VkInstance GetInstance();
    VkPhysicalDevice GetRenderingDevice();

    struct LogicalDeviceWrapperV2
    {
        VkDevice handle  = VK_NULL_HANDLE;
        VkQueue graphics = VK_NULL_HANDLE;
        VkQueue present  = VK_NULL_HANDLE;
    };

    struct QueueIndices
    {
        unsigned present  = UINT_MAX;
        unsigned graphics = UINT_MAX;

        [[nodiscard]] inline bool complete() const
        {
            return present != UINT_MAX && graphics != UINT_MAX;
        }
    };

    namespace CommandSubmitter
    {
        using CmdFn = std::function<void(VkCommandBuffer)>;

        void Initialize();
        void Destroy();

        std::future<bool> SendAsync(CmdFn&& commands);
        std::future<VkCommandBuffer> Prerecord(CmdFn&& commands, VkCommandBufferUsageFlags flags);
    }

    QueueIndices GetQueueIndices();
    const LogicalDeviceWrapperV2& GetDevice();
    const VmaAllocator& GetMemoryAllocator();

    std::vector<const char*> GetRequiredDeviceExtensions();
    std::vector<const char*> GetAvailableOptionalExtensions(VkPhysicalDevice device);
    QueueIndices GetPhysicalDeviceQueues(VkPhysicalDevice device);

    std::vector<const char*> GetDebugLayers();

    void SignalRendererCreation();
    void SignalRendererDestroy();
}
