#pragma once
#include "math/vec.hpp"
#include "render/mesh.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace Vk
{
    struct LogicalDeviceWrapper;
    struct SwapchainWrapper;
    struct SurfaceWrapper;
    struct ShaderManager;

    enum class MeshType
    {
        eUnknown = 0,
        eStatic,
        eDynamic
    };

    struct MeshData
    {
        bool needsUpdating = false;
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vbMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory ibMemory = VK_NULL_HANDLE;
        unsigned shader = 0;
        std::vector<vec3f> vertices = {};
        std::vector<unsigned> indices = {};
    };

    struct ObjectManager
    {
    public:
        ObjectManager() = default;
        ~ObjectManager() = default;

        void create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface, ShaderManager& shaderManager);
        void destroy();

        void update();

        CMesh create_object(const MeshCreateInfo& meshCreateInfo);

    private:
        VkCommandPool create_command_pool();
        void allocate_command_buffers(void* pBuffer, int count, VkCommandPool pool, VkCommandBufferLevel level);

        void create_render_threads();
        void update_main_buffers();


    private:
        LogicalDeviceWrapper* pDevice = nullptr;
        SurfaceWrapper* pSurface = nullptr;
        SwapchainWrapper* pSwapchain = nullptr;
        ShaderManager* pShaderManager = nullptr;

        std::vector<MeshData> meshes = {};

        VmaAllocator memoryAllocator = VK_NULL_HANDLE;
        VkCommandPool mainPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> mainCmdBuffers = {};
        std::vector<std::vector<VkCommandBuffer>> secondaryCmdBuffers = {};
    };
}
