#include "utils/debug.hpp"
#include "utils/thread_pool.hpp"
#include "utils/thread_safe_queue.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <condition_variable>
#include <atomic>
#include <mutex>

namespace Vk
{
    CThreadSafeQueue<std::pair<CommandSubmitter::CmdFn, std::promise<bool>>> COMMAND_QUEUE = {};
    std::condition_variable WORKER_CONDITION                             = {};
    std::mutex CONDITION_MUTEX                                           = {};
    bool STOP_WORKER                                                     = false;
    std::atomic<int> WORKERS_STOPPED                                     = 0;

    VkCommandPool CreateThreadPool();
    VkCommandBuffer AllocateThreadBuffer(VkCommandPool& pool);
    VkFence CreateThreadFence();

    void BeginRecording(const VkCommandBuffer& buffer);
    void EndRecording(const VkCommandBuffer& buffer);

    namespace CommandSubmitter
    {
        void Initialize()
        {
            const auto total_threads = CThreadPool::GetThreadCount();

            for (auto i : range(1, total_threads / 2))
            {
                CThreadPool::DoTask([]{
                    const auto device = GetDevice().handle;
                    const auto graphics_queue = GetDevice().graphics;
                    const auto queue_index = GetQueueIndices().graphics;

                    VkCommandPool   thread_pool = CreateThreadPool();
                    VkCommandBuffer thread_buffer = AllocateThreadBuffer(thread_pool);
                    VkFence         executed_fence = CreateThreadFence();

                    while (true)
                    {
                        std::unique_lock lock(CONDITION_MUTEX);
                        WORKER_CONDITION.wait(lock, []()
                        {
                            return !COMMAND_QUEUE.empty() || STOP_WORKER;
                        });

                        if (STOP_WORKER)
                        {
                            break;
                        }

                        auto command_data = COMMAND_QUEUE.get_and_pop_front();

                        BeginRecording(thread_buffer);
                        command_data.first(thread_buffer);
                        EndRecording(thread_buffer);

                        VkSubmitInfo submit_info =
                        {
                            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                            .commandBufferCount = 1,
                            .pCommandBuffers    = &thread_buffer
                        };

                        vkQueueWaitIdle(graphics_queue);
                        vkQueueSubmit(graphics_queue, 1, &submit_info, executed_fence);

                        vkWaitForFences(device, 1, &executed_fence, VK_TRUE, -1);
                        vkResetFences(device, 1, &executed_fence);
                        vkResetCommandBuffer(thread_buffer, 0);
                        command_data.second.set_value(true);
                    }

                    ++WORKERS_STOPPED;
                    vkDestroyFence(device, executed_fence, nullptr);
                    vkDestroyCommandPool(device, thread_pool, nullptr);
                });
            }
        }

        void Destroy()
        {
            STOP_WORKER = true;
            WORKER_CONDITION.notify_all();

            while (WORKERS_STOPPED != CThreadPool::GetThreadCount() / 2) {}
        }

        std::future<bool> SendAsync(CmdFn&& commands)
        {
            auto promise = std::promise<bool>();

            COMMAND_QUEUE.emplace({ std::move(commands), std::move(promise) });
            auto future = COMMAND_QUEUE.front()
                            .second
                            .get_future();

            WORKER_CONDITION.notify_one();
            return future;
        }
    }

    VkCommandPool CreateThreadPool()
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

    VkCommandBuffer AllocateThreadBuffer(VkCommandPool& pool)
    {
        const VkCommandBufferAllocateInfo command_buffer_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = pool,
            .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1,
        };

        VkResult result;
        VkCommandBuffer new_buffer;
        result = vkAllocateCommandBuffers(GetDevice().handle, &command_buffer_allocate_info, &new_buffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferAllocationFail");
        }

        return new_buffer;
    }

    VkFence CreateThreadFence()
    {
        const VkFenceCreateInfo fence_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
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

    void BeginRecording(const VkCommandBuffer& buffer)
    {
        const VkCommandBufferBeginInfo buffer_begin_info =
        {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext            = nullptr,
            .flags            = 0,
            .pInheritanceInfo = nullptr
        };

        VkResult result;
        result = vkBeginCommandBuffer(buffer, &buffer_begin_info);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferBeginFail");
        }
    }

    void EndRecording(const VkCommandBuffer& buffer)
    {
        VkResult result;
        result = vkEndCommandBuffer(buffer);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Renderer-Vulkan-CmdSubmitter-BufferEndFail");
        }
    }

}
