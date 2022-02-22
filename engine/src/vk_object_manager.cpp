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

        rebuildNeeded = rebuildNeeded || try_allocate_new_command_buffers();

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
                .flags            = 0,
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

    CMesh ObjectManager::create_object(const MeshCreateInfo& meshCreateInfo)
    {
        

        return {};
    }
}
