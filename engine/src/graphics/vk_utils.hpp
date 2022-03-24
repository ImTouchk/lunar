#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>

namespace Vk
{
    inline VkCommandPool CreateThreadPool()
    {
        const VkCommandPoolCreateInfo pool_create_info =
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = GetQueueIndices().graphics
        };

        VkResult result;
        VkCommandPool pool;
        result = vkCreateCommandPool(GetDevice().handle, &pool_create_info, nullptr, &pool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-PoolCreationFail");
        }

        return pool;
    }

    inline void WaitForFence(VkFence fence)
    {
        vkWaitForFences(GetDevice().handle, 1, &fence, VK_TRUE, -1);
        vkResetFences(GetDevice().handle, 1, &fence);
    }

    inline VkFence CreateFence()
    {
        const VkFenceCreateInfo fence_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        VkResult result;
        VkFence new_fence;
        result = vkCreateFence(GetDevice().handle, &fence_create_info, nullptr, &new_fence);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderrer-Vulkan-CmdSubmitter-FenceCreationFail");
        }

        return new_fence;
    }

    inline void BeginRecording(const VkCommandBuffer& buffer, const VkCommandBufferBeginInfo&& beginInfo)
    {
        VkResult result;
        result = vkBeginCommandBuffer(buffer, &beginInfo);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferBeginFail");
        }
    }

    inline void EndRecording(const VkCommandBuffer& buffer)
    {
        VkResult result;
        result = vkEndCommandBuffer(buffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferEndFail");
        }
    }

    inline VkCommandBufferBeginInfo DefaultCmdBufferBeginInfo()
    {
        return VkCommandBufferBeginInfo
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
        };
    }
}
