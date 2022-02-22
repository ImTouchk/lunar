#pragma once
#include "math/vec.hpp"
#include "render/mesh.hpp"
#include "render/renderer.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>
#include <vector>

namespace Vk
{
    struct LogicalDeviceWrapper;
    struct SwapchainWrapper;
    struct SurfaceWrapper;
    struct ShaderManager;

    struct MeshData
    {
        MeshType type = MeshType::eUnknown;
        Shader shader = 0;
        bool needsUpdating = false;
        std::optional<std::vector<vec3f>> vertices = {};
        std::optional<std::vector<unsigned>> indices = {};
        unsigned vertexCount = 0;
        unsigned indexCount = 0;

        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vbMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory ibMemory = VK_NULL_HANDLE;
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
        void allocate_command_buffers(void* pBuffer, unsigned count, VkCommandPool pool, VkCommandBufferLevel level);

        void update_command_buffers();
            bool try_allocate_new_command_buffers();
            void record_secondary_command_buffers();

    private:
        LogicalDeviceWrapper* pDevice = nullptr;
        SurfaceWrapper* pSurface = nullptr;
        SwapchainWrapper* pSwapchain = nullptr;
        ShaderManager* pShaderManager = nullptr;

        std::vector<MeshData> meshes = {};

        VkCommandPool commandPool = VK_NULL_HANDLE;
        VmaAllocator memoryAllocator = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> mainCmdBuffers = {};
        std::vector<VkCommandBuffer> secondaryCmdBuffers = {};
    };
}
