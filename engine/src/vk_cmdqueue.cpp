#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <cassert>
#include <vector>

namespace Vk
{
    void CommandQueueWrapper::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface, VkPipeline pipeline)
    {
        pDevice = &device;
        pSurface = &surface;
        pSwapchain = &swapchain;

        auto queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());
        auto pool_create_info = VkCommandPoolCreateInfo
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = 0,
            .queueFamilyIndex = queue_indices.graphics.value(),
        };

        VkResult result;
        result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &commandPool);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Command queue creation failed (vkCreateCommandPool didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-CommandQueue-PoolCreationFail");
        }

        commandBuffers.resize(pSwapchain->frame_buffers().size());

        VkCommandBufferAllocateInfo buffer_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = commandPool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
        };

        result = vkAllocateCommandBuffers(pDevice->handle(), &buffer_allocate_info, commandBuffers.data());
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Command queue creation failed (vkAllocateCommandBuffers didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-CommandQueue-BufferAllocationFail");
        }

        for(auto i : range(0, commandBuffers.size() - 1))
        {
            auto& command_buffer = commandBuffers[i];

            VkCommandBufferBeginInfo buffer_begin_info =
            {
                .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags            = 0,
                .pInheritanceInfo = nullptr
            };

            result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
            if(result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Command queue creation failed (vkBeginCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-CommandQueue-BufferBeginFail");
            }

            VkClearValue clear_color = { 0.f, 0.f, 0.f, 1.f };

            VkRenderPassBeginInfo render_pass_begin_info =
            {
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = swapchain.render_pass(),
                .framebuffer = swapchain.frame_buffers()[i],
                .renderArea =
                {
                    { 0, 0 },
                    swapchain.surface_extent()
                },
                .clearValueCount = 1,
                .pClearValues    = &clear_color
            };

            vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
            vkCmdDraw(command_buffer, 3, 1, 0, 0);
            vkCmdEndRenderPass(command_buffer);

            result = vkEndCommandBuffer(command_buffer);
            if(result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Command queue creation failed (vkEndCommandBuffer didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-CommandQueue-BufferEndFail");
            }
        }
    }

    void CommandQueueWrapper::destroy()
    {
        assert(commandPool != VK_NULL_HANDLE);
        vkDestroyCommandPool(pDevice->handle(), commandPool, nullptr);
    }

    std::vector<VkCommandBuffer>& CommandQueueWrapper::buffers()
    {
        return commandBuffers;
    }
}


