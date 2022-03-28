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

class CGameWindow;

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

        void create(CGameWindow& window);
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

        void create(CGameWindow& window, SurfaceWrapper& surface);
        void destroy();

        void resize(CGameWindow& window, SurfaceWrapper& surface);

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
        void create_swapchain(CGameWindow& window, SurfaceWrapper& surface);
        void create_render_pass();
        void create_image_views();
        void create_framebuffers();
        void create_depth_buffer();
        void create_shadow_map();

        struct
        {
            VmaAllocation allocation = VK_NULL_HANDLE;
            VkImage       image      = VK_NULL_HANDLE;
            VkImageView   view       = VK_NULL_HANDLE;
            VkFormat      format     = {};
        } depthBuffer = {};

        struct
        {
            VmaAllocation allocation  = VK_NULL_HANDLE;
            VkImage       image       = VK_NULL_HANDLE;
            VkImageView   view        = VK_NULL_HANDLE;
            VkFramebuffer framebuffer = VK_NULL_HANDLE;
            VkRenderPass  renderPass  = VK_NULL_HANDLE;
            VkFormat      format      = {};
        } shadowMap = {};

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

    struct RendererInternalData
    {
        SurfaceWrapper surface;
        SwapchainWrapper swapchain;
        ShaderManager shaderManager;
        ObjectManager objectManager;
        TextureManager textureManager;

        size_t currentFrame = 0;
        size_t recordedMeshes = 0;
        std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT + 1> submitCommands = {};
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> isImageAvailable       = {};
        std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> isRenderingFinished    = {};
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
        void Initialize();
        void Destroy();

        struct AdditionalRecordData
        {
            VkCommandBufferLevel           level           = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            VkCommandBufferInheritanceInfo inheritanceInfo = {};
            VkCommandBufferBeginInfo       beginInfo       = {};
        };

        void SubmitSync(VkCommandBuffer command, bool waitForExecution = false, VkSubmitInfo submitInfo = {});
        void SubmitSync(CommandRecordFn&& commands, bool waitForExecution = false, VkSubmitInfo submitInfo = {});
        VkCommandBuffer RecordSync(CommandRecordFn&& commands, AdditionalRecordData&& recordData = {});
        void DestroyCommandBuffer(VkCommandBuffer& buffer);
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
