#include "utils/debug.hpp"
#include "utils/thread_pool.hpp"
#include "utils/thread_safe_queue.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <condition_variable>
#include <mutex>

namespace Vk
{
    CThreadSafeQueue<std::pair<CmdFn, std::promise<bool>>> COMMAND_QUEUE = {};
    std::condition_variable WORKER_CONDITION                             = {};
    std::mutex CONDITION_MUTEX                                           = {};
    bool STOP_WORKER                                                     = false;
    bool WORKER_STOPPED                                                  = false;

    void CreateCommandSubmitter()
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
                std::unique_lock lock(CONDITION_MUTEX);
                WORKER_CONDITION.wait(lock, []()
                {
                    return !COMMAND_QUEUE.empty() || STOP_WORKER;
                });

                if(STOP_WORKER)
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

                for (auto& command : COMMAND_QUEUE)
                {
                    command.first(thread_buffer);
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

                while (!COMMAND_QUEUE.empty())
                {
                    auto& command = COMMAND_QUEUE.front();
                    command.second.set_value(true);
                    COMMAND_QUEUE.pop();
                }

                COMMAND_QUEUE.clear();

                vkResetCommandBuffer(thread_buffer, 0);
            }

            vkFreeCommandBuffers(device.handle, thread_pool, 1, &thread_buffer);
            vkDestroyCommandPool(device.handle, thread_pool, nullptr);
            WORKER_STOPPED = true;
        });
    }

    void DestroyCommandSubmitter()
    {
        STOP_WORKER = true;
        WORKER_CONDITION.notify_all();

        while(!WORKER_STOPPED) {}
    }

    std::future<bool> SubmitCommand(CmdFn&& commands)
    {
        auto promise = std::promise<bool>();
        COMMAND_QUEUE.emplace({ std::move(commands), std::move(promise) });

        auto future = COMMAND_QUEUE.front()
                        .second
                        .get_future();

        WORKER_CONDITION.notify_all();

        return future;
    }
}
