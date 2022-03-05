#include "utils/debug.hpp"
#include "utils/thread_pool.hpp"
#include "vk_cmd_submitter.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace Vk
{
    std::mutex SUBMITTER_MUTEX = {};
    std::condition_variable SUBMITTER_CONDITION = {};

    std::deque<std::pair<CmdFn, std::promise<bool>&&>> SUBMITTER_COMMANDS = {};
    bool SUBMITTER_STOP = false;
    bool SUBMITTER_FINISHED = false;

    void CmdSubmitter::create()
    {
        CThreadPool::DoTask([]
        {
            std::function<void(VkCommandPool)> task = nullptr;

            auto device = GetDevice();
            auto queue_index = GetQueueIndices().graphics;

            VkCommandPool thread_pool = VK_NULL_HANDLE;
            VkCommandBuffer thread_buffer = VK_NULL_HANDLE;

            VkCommandPoolCreateInfo pool_create_info =
            {
                .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = queue_index
            };

            VkResult result;
            result = vkCreateCommandPool(device.handle, &pool_create_info, nullptr, &thread_pool);
            if(result != VK_SUCCESS)
            {
                throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-CreationFail");
            }

            VkCommandBufferAllocateInfo command_buffer_allocate_info =
            {
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = thread_pool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };

            result = vkAllocateCommandBuffers(device.handle, &command_buffer_allocate_info, &thread_buffer);
            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-CreationFail");
            }

            while (true)
            {
                std::unique_lock<std::mutex> lock(SUBMITTER_MUTEX);
                SUBMITTER_CONDITION.wait(lock, []()
                {
                    return !SUBMITTER_COMMANDS.empty() || SUBMITTER_STOP;
                });

                if(SUBMITTER_STOP)
                    break;

                VkCommandBufferBeginInfo buffer_begin_info =
                {
                    .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                    .pNext            = nullptr,
                    .flags            = 0,
                    .pInheritanceInfo = nullptr
                };

                result = vkBeginCommandBuffer(thread_buffer, &buffer_begin_info);
                if (result != VK_SUCCESS)
                {
                    throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-UploadFail");
                }

                for(auto& pair : SUBMITTER_COMMANDS)
                {
                    pair.first(thread_buffer);
                }

                result = vkEndCommandBuffer(thread_buffer);
                if(result != VK_SUCCESS)
                {
                    throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-UploadFail");
                }

                VkSubmitInfo submit_info =
                {
                    .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                    .commandBufferCount = 1,
                    .pCommandBuffers    = &thread_buffer
                };

                vkQueueSubmit(device.graphics, 1, &submit_info, VK_NULL_HANDLE);
                vkQueueWaitIdle(device.graphics);

                vkResetCommandBuffer(thread_buffer, 0);

                for(auto& pair : SUBMITTER_COMMANDS)
                {
                    pair.second.set_value(true);
                }

                SUBMITTER_COMMANDS.clear();
            }

            vkFreeCommandBuffers(device.handle, thread_pool, 1, &thread_buffer);
            vkDestroyCommandPool(device.handle, thread_pool, nullptr);
            SUBMITTER_FINISHED = true;
        });
    }

    void CmdSubmitter::destroy()
    {
        SUBMITTER_STOP = true;
        SUBMITTER_CONDITION.notify_all();

        while(!SUBMITTER_FINISHED) {}
    }

    void CmdSubmitter::submit(CmdFn&& commands, std::promise<bool>& finished)
    {
        SUBMITTER_COMMANDS.emplace_back( commands, std::move(finished) );
        SUBMITTER_CONDITION.notify_all();
    }
}