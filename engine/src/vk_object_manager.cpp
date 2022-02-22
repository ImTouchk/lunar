#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "utils/thread_pool.hpp"
#include "render/renderer.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <shared_mutex>
#include <stdexcept>
#include <semaphore>
#include <mutex>
#include <queue>

namespace Vk
{
    constexpr int BLOCK_SIZE = 10;

    void ObjectManager::allocate_command_buffers(void* pBuffer, unsigned count, VkCommandPool pool, VkCommandBufferLevel level)
    {
        VkCommandBufferAllocateInfo buffer_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = pool,
            .level              = level,
            .commandBufferCount = static_cast<uint32_t>(count)
        };

        VkResult result;
        result = vkAllocateCommandBuffers(pDevice->handle(), &buffer_allocate_info, reinterpret_cast<VkCommandBuffer*>(pBuffer));
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-AllocationFail");
        }
    }

    VkCommandPool ObjectManager::create_command_pool()
    {
        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        VkCommandPoolCreateInfo pool_create_info =
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = 0,
            .queueFamilyIndex = queue_indices.graphics.value()
        };

        VkResult result;
        VkCommandPool pool;
        result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &pool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandPool-CreationFail");
        }

        return pool;
    }

    void ObjectManager::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface, ShaderManager& shaderManager)
    {
        pDevice = &device;
        pSurface = &surface;
        pSwapchain = &swapchain;
        pShaderManager = &shaderManager;

        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        commandPool = create_command_pool();

        VmaAllocatorCreateInfo allocator_create_info =
        {
            .physicalDevice   = GetRenderingDevice(),
            .device           = pDevice->handle(),
            .instance         = GetInstance(),
            .vulkanApiVersion = VK_API_VERSION_1_2
        };

        VkResult result;
        result = vmaCreateAllocator(&allocator_create_info, &memoryAllocator);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Object manager creation failed (vmaCreateAllocator didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-MemAllocatorCreationFail");
        }

        mainCmdBuffers.resize(pSwapchain->frame_buffers().size());
        allocate_command_buffers(mainCmdBuffers.data(), mainCmdBuffers.size(), commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        CDebug::Log("Vulkan Renderer | Object manager created.");
    }

    void ObjectManager::destroy()
    {
        // TODO: destroy command buffers

        vmaDestroyAllocator(memoryAllocator);
        vkDestroyCommandPool(pDevice->handle(), commandPool, nullptr);

        CDebug::Log("Vulkan Renderer | Object manager destroyed.");
    }

    void ObjectManager::update_command_buffers()
    {
        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        bool rebuildNeeded = false;
        for(const auto& mesh : meshes)
        {
            if(mesh.needsUpdating)
                rebuildNeeded = true;
        }

        rebuildNeeded = try_allocate_new_command_buffers() || rebuildNeeded;

        if(rebuildNeeded)
        {
            record_secondary_command_buffers();

            for(auto i : range(0, mainCmdBuffers.size() - 1))
            {
                auto& command_buffer = mainCmdBuffers[i];

                VkCommandBufferBeginInfo buffer_begin_info =
                {
                    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .flags            = 0,
                    .pInheritanceInfo = nullptr
                };

                VkResult result;
                result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
                if(result != VK_SUCCESS)
                {
                    CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkBeginCommandBuffer didn't return VK_SUCCESS).");
                    throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-BeginFail");
                }

                VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

                VkRenderPassBeginInfo render_pass_begin_info =
                {
                    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass      = pSwapchain->render_pass(),
                    .framebuffer     = pSwapchain->frame_buffers()[i],
                    .renderArea      =
                    {
                        .offset = { 0, 0 },
                        .extent = pSwapchain->surface_extent()
                    },
                    .clearValueCount = 1,
                    .pClearValues    = &clear_color
                };

                vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdExecuteCommands(command_buffer, secondaryCmdBuffers.size(), secondaryCmdBuffers.data());
                vkCmdEndRenderPass(command_buffer);

                result = vkEndCommandBuffer(command_buffer);
                if (result != VK_SUCCESS)
                {
                    CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkEndCommandBuffer didn't return VK_SUCCESS).");
                    throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-EndFail");
                }
            }

            CDebug::Log("Vulkan Renderer | Object command buffers rebuilt.");
        }
    }

    void ObjectManager::record_secondary_command_buffers()
    {
        for (auto i: range(0, meshes.size() - 1))
        {
            auto& command_buffer = secondaryCmdBuffers[i];
            auto& mesh = meshes[i];

            if(!mesh.needsUpdating)
                continue;

            VkCommandBufferInheritanceInfo buffer_inheritance_info =
            {
                .sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                .pNext                = nullptr,
                .renderPass           = pSwapchain->render_pass(),
                .subpass              = 0,
                .framebuffer          = VK_NULL_HANDLE,
                .occlusionQueryEnable = VK_FALSE,
                .queryFlags           = 0,
                .pipelineStatistics   = 0
            };

            VkCommandBufferBeginInfo buffer_begin_info =
            {
                .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags            = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                .pInheritanceInfo = &buffer_inheritance_info
            };

            VkResult result;
            result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkBeginCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-BeginFail");
            }

            // TODO: record commands here

            result = vkEndCommandBuffer(command_buffer);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkEndCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-EndFail");
            }
        }
    }

    bool ObjectManager::try_allocate_new_command_buffers()
    {
        bool needsRebuilding = false;

        if(secondaryCmdBuffers.size() != meshes.size())
        {
            if(secondaryCmdBuffers.empty())
            {
                secondaryCmdBuffers.resize(meshes.size(), VK_NULL_HANDLE);
                allocate_command_buffers(secondaryCmdBuffers.data(), meshes.size(), commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
                CDebug::Log("Vulkan Renderer | Object manager: allocated {} initial command buffers.", meshes.size());
            }
            else
            {
                auto old_size = secondaryCmdBuffers.size();
                for (auto i : range(old_size, meshes.size() - 1))
                {
                    secondaryCmdBuffers.push_back(VK_NULL_HANDLE);
                }

                auto* pBuf = secondaryCmdBuffers.data() + old_size;
                allocate_command_buffers(pBuf, meshes.size() - old_size, commandPool, VK_COMMAND_BUFFER_LEVEL_SECONDARY);
                CDebug::Log("Vulkan Renderer | Object manager: allocated {} new command buffers.", meshes.size() - old_size);
            }

            needsRebuilding = true;
        }

        return needsRebuilding;
    }

    void ObjectManager::update()
    {
        update_command_buffers();
    }

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
            .usage = memoryUsage
        };

        VkResult result;
        result = vmaCreateBuffer(memoryAllocator, &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr);
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

    // TODO: Do different buffer allocations depending on the mesh type

    void ObjectManager::create_index_buffer(MeshData& meshData, const std::vector<unsigned>& indices)
    {

    }

    void ObjectManager::create_vertex_buffer(MeshData& meshData, const std::vector<vec3f>& vertices)
    {
        if(meshData.vertexBuffer != VK_NULL_HANDLE)
        {
            vmaDestroyBuffer(memoryAllocator, meshData.vertexBuffer, meshData.vbMemory);
        }

        auto buffer_size = meshData.vertexCount * sizeof(vec3f);

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
        vmaMapMemory(memoryAllocator, staging_allocation, &pData);
        memcpy(pData, vertices.data(), buffer_size);
        vmaUnmapMemory(memoryAllocator, staging_allocation);

        copy_buffer(staging_buffer, meshData.vertexBuffer, buffer_size);
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

        auto vb_size = sizeof(meshCreateInfo.vertices[0]) * meshCreateInfo.vertices.size();
        auto ib_size = sizeof(meshCreateInfo.indices[0]) * meshCreateInfo.indices.size();

        create_vertex_buffer(new_mesh, meshCreateInfo.vertices);

        CDebug::Log("Allocation worked :D");

        //meshes.push_back(std::move(new_mesh));
        return {};
    }
}
