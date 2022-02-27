#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"
#include "vk_object_manager.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
    void
    ObjectManager::create_buffer
    (
        unsigned int size,
        VkBufferUsageFlags usageFlags,
        VmaMemoryUsage memoryUsage,
        VkBuffer& buffer,
        VmaAllocation& allocation
    )
    {
        VkBufferCreateInfo buffer_create_info =
        {
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = size,
            .usage       = usageFlags,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };

        VmaAllocationCreateInfo allocation_create_info =
        {
            .flags          = 0,
            .usage          = memoryUsage,
            .requiredFlags  = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = VK_NULL_HANDLE,
            .pUserData      = nullptr,
            .priority       = 0,
        };

        VkResult result;
        result = vmaCreateBuffer(pMemoryAllocator->handle(), &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Could not create new buffer.");
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-BufferCreationFail");
        }
    }

    void ObjectManager::copy_buffer(VkBuffer src, VkBuffer dst, unsigned int size)
    {
        VkCommandBufferAllocateInfo buffer_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = commandPool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkCommandBuffer command_buffer;
        vkAllocateCommandBuffers(pDevice->handle(), &buffer_allocate_info, &command_buffer);

        VkCommandBufferBeginInfo buffer_begin_info =
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        vkBeginCommandBuffer(command_buffer, &buffer_begin_info);

        VkBufferCopy buffer_copy_region =
        {
            .srcOffset = 0,
            .dstOffset = 0,
            .size      = size,
        };

        vkCmdCopyBuffer(command_buffer, src, dst, 1, &buffer_copy_region);
        vkEndCommandBuffer(command_buffer);

        VkSubmitInfo submit_info =
        {
            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers    = &command_buffer,
        };

        vkQueueSubmit(pDevice->graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(pDevice->graphics_queue());

        vkFreeCommandBuffers(pDevice->handle(), commandPool, 1, &command_buffer);
    }

    // TODO: Do different buffer allocations depending on the mesh type ; right now it only accounts for static meshes

    void ObjectManager::create_index_buffer(MeshData& meshData, const std::vector<Index>& indices)
    {
        if(meshData.indexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(pMemoryAllocator->handle(), meshData.indexBuffer, meshData.ibMemory);
        }

        auto buffer_size = sizeof(Index) * indices.size();

        if(buffer_size == 0)
        {
            CDebug::Warn("Vulkan Renderer | ObjectManager::create_index_buffer() called on empty vector \"indices\".");
            meshData.indexBuffer = VK_NULL_HANDLE;
            meshData.ibMemory = VK_NULL_HANDLE;
            meshData.indexCount = 0;
            return;
        }

        VkBuffer staging_buffer;
        VmaAllocation staging_allocation;
        create_buffer
        (
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY,
            staging_buffer,
            staging_allocation
        );

        create_buffer
        (
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            meshData.indexBuffer,
            meshData.ibMemory
        );

        void* pData = nullptr;
        vmaMapMemory(pMemoryAllocator->handle(), staging_allocation, &pData);
        memcpy(pData, indices.data(), buffer_size);
        vmaUnmapMemory(pMemoryAllocator->handle(), staging_allocation);

        copy_buffer(staging_buffer, meshData.indexBuffer, buffer_size);

        vmaDestroyBuffer(pMemoryAllocator->handle(), staging_buffer, staging_allocation);
    }

    void ObjectManager::create_vertex_buffer(MeshData& meshData, const std::vector<Vertex>& vertices)
    {
        if(meshData.vertexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(pMemoryAllocator->handle(), meshData.vertexBuffer, meshData.vbMemory);
        }

        auto buffer_size = sizeof(Vertex) * vertices.size();

        if(buffer_size == 0)
        {
            CDebug::Warn("Vulkan Renderer | ObjectManager::create_vertex_buffer() called on empty vector \"vertices\".");
            meshData.vertexBuffer = VK_NULL_HANDLE;
            meshData.vbMemory = VK_NULL_HANDLE;
            meshData.vertexCount = 0;
            return;
        }

        VkBuffer staging_buffer;
        VmaAllocation staging_allocation;
        create_buffer
        (
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            staging_buffer,
            staging_allocation
        );

        create_buffer
        (
            buffer_size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY,
            meshData.vertexBuffer,
            meshData.vbMemory
        );

        void* pData = nullptr;
        vmaMapMemory(pMemoryAllocator->handle(), staging_allocation, &pData);
        memcpy(pData, (char*)vertices.data(), buffer_size);
        vmaUnmapMemory(pMemoryAllocator->handle(), staging_allocation);

        copy_buffer(staging_buffer, meshData.vertexBuffer, buffer_size);

        vmaDestroyBuffer(pMemoryAllocator->handle(), staging_buffer, staging_allocation);
    }

    CMesh ObjectManager::create_object(const MeshCreateInfo& meshCreateInfo)
    {
        MeshData new_mesh =
        {
            .type          = meshCreateInfo.type,
            .shader        = meshCreateInfo.shader,
            .needsUpdating = true,
            .vertexCount   = static_cast<unsigned int>(meshCreateInfo.vertices.size()),
            .indexCount    = static_cast<unsigned int>(meshCreateInfo.indices.size())
        };

        create_index_buffer(new_mesh, meshCreateInfo.indices);
        create_vertex_buffer(new_mesh, meshCreateInfo.vertices);

        meshes.push_back(std::move(new_mesh));
        return {};
    }
}