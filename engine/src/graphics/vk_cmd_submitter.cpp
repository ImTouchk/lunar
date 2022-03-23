#include "utils/debug.hpp"
#include "utils/fast_mutex.hpp"
#include "vk_renderer.hpp"
#include "vk_utils.hpp"

#include <vulkan/vulkan.h>
#include <mutex>

namespace Vk
{
    VkFence       GRAPHICS_QUEUE_FENCE = VK_NULL_HANDLE;
    VkCommandPool SHARED_POOL          = VK_NULL_HANDLE;
    CMutex        SHARED_POOL_MUTEX    = {};

    namespace CommandSubmitter
    {
        void Initialize()
        {
            SHARED_POOL = CreateThreadPool();
            GRAPHICS_QUEUE_FENCE = CreateFence();
        }

        void Destroy()
        {
            vkDestroyCommandPool(GetDevice().handle, SHARED_POOL, nullptr);
            vkDestroyFence(GetDevice().handle, GRAPHICS_QUEUE_FENCE, nullptr);
        }

        VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level)
        {
            CLockGuard lock(SHARED_POOL_MUTEX);
            
            const VkCommandBufferAllocateInfo buffer_allocate_info =
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext              = nullptr,
                .commandPool        = SHARED_POOL,
                .level              = level,
                .commandBufferCount = 1
            };

            VkResult result;
            VkCommandBuffer new_buffer;
            result = vkAllocateCommandBuffers(GetDevice().handle, &buffer_allocate_info, &new_buffer);
            if(result != VK_SUCCESS)
            {
                throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferAllocationFail");
            }

            return new_buffer;
        }

        void DestroyCommandBuffer(VkCommandBuffer& buffer)
        {
            CLockGuard lock(SHARED_POOL_MUTEX);
            vkFreeCommandBuffers(GetDevice().handle, SHARED_POOL, 1, &buffer);
        }

        inline void SubmitToQueue(const VkDevice& device, const VkQueue& queue, const VkSubmitInfo& submitInfo)
        {
            if(GRAPHICS_QUEUE_FENCE == VK_NULL_HANDLE)
            {
                GRAPHICS_QUEUE_FENCE = CreateFence();
            }
            else
            {
                vkWaitForFences(device, 1, &GRAPHICS_QUEUE_FENCE, VK_TRUE, -1);
            }

            vkResetFences(device, 1, &GRAPHICS_QUEUE_FENCE);
            vkQueueSubmit(queue, 1, &submitInfo, GRAPHICS_QUEUE_FENCE);
        }

        void SubmitSync(CommandRecordFn&& commands, bool waitForExecution, VkSubmitInfo submitInfo)
        {
            VkCommandBuffer one_time_buffer;
            one_time_buffer = CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            BeginRecording(one_time_buffer, DefaultCmdBufferBeginInfo());
            commands(one_time_buffer);
            EndRecording(one_time_buffer);

            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &one_time_buffer;

            vkQueueSubmit(GetDevice().graphics, 1, &submitInfo, GRAPHICS_QUEUE_FENCE);
            vkWaitForFences(GetDevice().handle, 1, &GRAPHICS_QUEUE_FENCE, VK_TRUE, -1);
            vkResetFences(GetDevice().handle, 1, &GRAPHICS_QUEUE_FENCE);

            DestroyCommandBuffer(one_time_buffer);
        }

        void SubmitSync(VkCommandBuffer command, bool waitForExecution, VkSubmitInfo submitInfo)
        {
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &command;

            vkQueueSubmit(GetDevice().graphics, 1, &submitInfo, GRAPHICS_QUEUE_FENCE);
            vkWaitForFences(GetDevice().handle, 1, &GRAPHICS_QUEUE_FENCE, VK_TRUE, -1);
            vkResetFences(GetDevice().handle, 1, &GRAPHICS_QUEUE_FENCE);
        }

        VkCommandBuffer RecordSync(CommandRecordFn&& commands, AdditionalRecordData&& recordData)
        {
            const auto buffer_inheritance_info = recordData.inheritanceInfo;
            auto buffer_begin_info             = recordData.beginInfo;

            if(buffer_begin_info.sType != VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO)
            {
                buffer_begin_info = DefaultCmdBufferBeginInfo();
                buffer_begin_info.flags = 0; // it is by default VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT which is not the use case a prerecorded buffer has
            }

            if(buffer_inheritance_info.sType == VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO)
            {
                buffer_begin_info.pInheritanceInfo = &buffer_inheritance_info;
            }

            VkCommandBuffer buffer;
            buffer = CreateCommandBuffer(recordData.level);
            BeginRecording(buffer, std::move(buffer_begin_info));
            commands(buffer);
            EndRecording(buffer);

            return buffer;
        }
    }

}
