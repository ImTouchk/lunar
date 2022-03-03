#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"
#include "vk_object_manager.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
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
            rebuild_primary_buffers();

            CDebug::Log("Vulkan Renderer | Object command buffers rebuilt.");
        }
    }

    void ObjectManager::rebuild_primary_buffers()
    {
        for(auto i : range(0, mainCmdBuffers.size() - 1))
        {
            auto& command_buffer = mainCmdBuffers[i];

            if(command_buffer != VK_NULL_HANDLE)
            {
                vkResetCommandBuffer(command_buffer, 0);
            }

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

            VkClearValue clear_values[2] =
            {
                { .color = { 0.f, 0.f, 0.f, 1.f } },
                { .depthStencil = { 1.f, 0 } }
            };

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
                .clearValueCount = 2,
                .pClearValues    = clear_values,
            };

            vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            vkCmdExecuteCommands(command_buffer, secondaryCmdBuffers.size(), secondaryCmdBuffers.data());
            vkCmdEndRenderPass(command_buffer);

            result = vkEndCommandBuffer(command_buffer);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkEndCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-EndFail");
            }
        }
    }

    void ObjectManager::handle_resize()
    {
        vkDeviceWaitIdle(pDevice->handle());
        rebuild_primary_buffers();
    }

    void ObjectManager::record_secondary_command_buffers()
    {
        for (auto i: range(0, meshes.size() - 1))
        {
            auto& command_buffer = secondaryCmdBuffers[i];
            auto& mesh = meshes[i];

            if(!mesh.needsUpdating)
                continue;

            if (mesh.vertices.has_value())
            {
                update_buffer(mesh.vertexBuffer, mesh.vbMemory, mesh.vertices.value().data(), mesh.vertices.value().size());
                mesh.vertices.reset();
                CDebug::Log("Vulkan Renderer | Vertex buffer updated.");
            }

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
                .flags            = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
                .pInheritanceInfo = &buffer_inheritance_info
            };

            VkResult result;
            result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkBeginCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-BeginFail");
            }


            auto graphics_layout = pShaderManager->get_graphics_layout();
            auto pipeline = pShaderManager->try_get(mesh.shader);
            if(pipeline != VK_NULL_HANDLE)
            {
                VkDeviceSize offsets[] = { 0 };

                vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdPushConstants(command_buffer, graphics_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mesh.transform);
                vkCmdSetViewport(command_buffer, 0, 1, &pSwapchain->get_viewport());
                vkCmdSetScissor(command_buffer, 0, 1, &pSwapchain->get_scissor());
                vkCmdBindVertexBuffers(command_buffer, 0, 1, &mesh.vertexBuffer, offsets);
                vkCmdBindIndexBuffer(command_buffer, mesh.indexBuffer, 0, VK_INDEX_TYPE_UINT16);
                vkCmdDrawIndexed(command_buffer, mesh.indexCount, 1, 0, 0, 0);
            }

            result = vkEndCommandBuffer(command_buffer);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Failed to rebuild object command buffers (vkEndCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-ObjectManager-CommandBuffers-EndFail");
            }

            mesh.needsUpdating = false;
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
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
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
}