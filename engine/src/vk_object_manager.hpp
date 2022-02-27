#pragma once
#include "math/vec.hpp"
#include "render/mesh.hpp"
#include "render/renderer.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <optional>
#include <vector>

namespace Vk
{
    struct LogicalDeviceWrapper;
    struct SwapchainWrapper;
    struct SurfaceWrapper;
    struct ShaderManager;
    struct MemoryAllocatorWrapper;

    struct MeshData
    {
        MeshType type = MeshType::eUnknown;
        Shader shader = 0;
        bool needsUpdating = false;
        std::optional<std::vector<Vertex>> vertices = {};
        std::optional<std::vector<unsigned>> indices = {};
        unsigned vertexCount = 0;
        unsigned indexCount = 0;
        glm::mat4 transform = glm::mat4(1.f);

        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VmaAllocation vbMemory = VK_NULL_HANDLE;
        VkBuffer indexBuffer = VK_NULL_HANDLE;
        VmaAllocation ibMemory = VK_NULL_HANDLE;
    };

    struct ObjectManager
    {
    public:
        ObjectManager() = default;
        ~ObjectManager() = default;

        void create(LogicalDeviceWrapper& device, MemoryAllocatorWrapper& memoryAllocator, SwapchainWrapper& swapchain, SurfaceWrapper& surface, ShaderManager& shaderManager);
        void destroy();

        void update();
        void handle_resize();

        [[nodiscard]] VkCommandBuffer get_main(unsigned frame) const;

        CMesh create_object(const MeshCreateInfo& meshCreateInfo);

    private:
        VkCommandPool create_command_pool();
        void allocate_command_buffers(void* pBuffer, unsigned count, VkCommandPool pool, VkCommandBufferLevel level);

        void update_command_buffers();
            bool try_allocate_new_command_buffers();
            void record_secondary_command_buffers();
            void rebuild_primary_buffers();

        void create_index_buffer(MeshData& meshData, const std::vector<Index>& indices);
        void create_vertex_buffer(MeshData& meshData, const std::vector<Vertex>& vertices);
            void create_buffer(unsigned size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation);
            void copy_buffer(VkBuffer src, VkBuffer dst, unsigned size);
    private:
        LogicalDeviceWrapper* pDevice = nullptr;
        MemoryAllocatorWrapper* pMemoryAllocator = nullptr;
        SurfaceWrapper* pSurface = nullptr;
        SwapchainWrapper* pSwapchain = nullptr;
        ShaderManager* pShaderManager = nullptr;

        std::vector<MeshData> meshes = {};

        VkRect2D scissor = {};
        VkViewport viewport = {};

        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> mainCmdBuffers = {};
        std::vector<VkCommandBuffer> secondaryCmdBuffers = {};
    };
}
