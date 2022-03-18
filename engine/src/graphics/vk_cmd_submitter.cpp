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
    enum class CommandRequestType : uint8_t
    {
        eUnknown         = 0,
        ePrerecord       = 1,
        eSubmitOnce      = 2,
        eDestroyRecorded = 3,
    };

    struct CommandDestroyData
    {
        Identifier threadId;
        VkCommandBuffer commandBuffer;
    };

    struct CommandRecordData
    {
        CommandRecordFn recordFn;
        CommandSubmitter::AdditionalRecordData userData;
    };

    struct CommandSubmitData
    {
        CommandRecordFn command;
        VkSubmitInfo submitInfo;
    };

    struct CommandRequestData
    {
        CommandRequestType requestType;
        std::promise<std::any> returnValue;
        std::any requestValue; 
    };

    CThreadSafeQueue<CommandRequestData> REQUESTS          = {};
    std::condition_variable              WORKER_CONDITION  = {};
    std::mutex                           CONDITION_MUTEX   = {};
    std::atomic<int>                     WORKERS_STOPPED   = 0;
    bool                                 STOP_WORKER       = false;

    VkFence CreateThreadFence();
    VkCommandPool CreateThreadPool();
    VkCommandBuffer AllocateThreadBuffer(VkCommandPool& pool, VkCommandBufferLevel level);

    void BeginRecording(const VkCommandBuffer& buffer, const VkCommandBufferBeginInfo&& beginInfo);
    void EndRecording(const VkCommandBuffer& buffer);

    GpuCommand::GpuCommand(Identifier thread_id, VkCommandBuffer buffer)
        : thread_id(thread_id), buffer(buffer)
    {
    }

    VkCommandBuffer& GpuCommand::handle()
    {
        assert(buffer != VK_NULL_HANDLE);
        return buffer;
    }

    const VkCommandBuffer& GpuCommand::handle() const
    {
        assert(buffer != VK_NULL_HANDLE);
        return buffer;
    }

    bool GpuCommand::exists() const
    {
        return buffer != VK_NULL_HANDLE;
    }

    void GpuCommand::destroy()
    {
        assert(buffer != VK_NULL_HANDLE);

        auto promise = std::promise<std::any>();
        auto future = promise.get_future();

        REQUESTS.emplace
        ({
            CommandRequestType::eDestroyRecorded,
            std::move(promise),
            CommandDestroyData { thread_id, buffer }
        });

        WORKER_CONDITION.notify_one();

        buffer = VK_NULL_HANDLE;
        thread_id = 0;
    }

    namespace CommandSubmitter
    {
        VkCommandBufferBeginInfo DefaultBeginInfo()
        {
            return VkCommandBufferBeginInfo
            {
                .sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .pNext            = nullptr,
                .flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                .pInheritanceInfo = nullptr
            };
        }

        void Initialize()
        {
            const auto total_threads = CThreadPool::GetThreadCount();

            for (auto i : range(1, total_threads / 2))
            {
                CThreadPool::DoTask([]{
                    const auto device = GetDevice().handle;
                    const auto graphics_queue = GetDevice().graphics;
                    const auto queue_index = GetQueueIndices().graphics;

                    VkCommandPool   thread_pool    = CreateThreadPool();
                    VkCommandBuffer thread_buffer  = AllocateThreadBuffer(thread_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
                    VkFence         executed_fence = CreateThreadFence();
                    Identifier      thread_id      = get_unique_number();

                    while (true)
                    {
                        std::unique_lock lock(CONDITION_MUTEX);
                        WORKER_CONDITION.wait(lock, []()
                        {
                            return !REQUESTS.empty() || STOP_WORKER;
                        });

                        if (STOP_WORKER)
                        {
                            break;
                        }

                        auto request = REQUESTS.get_and_pop_front();
                        switch(request.requestType)
                        {
                        case CommandRequestType::eDestroyRecorded:
                        {
                            auto& destroy_data = std::any_cast<CommandDestroyData&>(request.requestValue);
                            if(destroy_data.threadId == thread_id)
                            {
                                vkFreeCommandBuffers(device, thread_pool, 1, &destroy_data.commandBuffer);
                                request.returnValue.set_value({ true });
                            }
                            else
                            {
                                REQUESTS.emplace(std::move(request));
                            }

                            break;
                        }

                        case CommandRequestType::ePrerecord:
                        {
                            auto record_data = std::any_cast<CommandRecordData&>(request.requestValue);
                            auto buffer_begin_info = record_data.userData.beginInfo.value_or(DefaultBeginInfo());
                            auto inheritance_info = record_data.userData.inheritanceInfo.value_or(VkCommandBufferInheritanceInfo { });
                            if(record_data.userData.inheritanceInfo.has_value())
                            {
                                buffer_begin_info.pInheritanceInfo = &inheritance_info;
                            }
                            else
                            {
                                buffer_begin_info.pInheritanceInfo = nullptr;
                            }

                            VkCommandBuffer new_buffer = AllocateThreadBuffer(thread_pool, record_data.userData.level);
                            BeginRecording(new_buffer, std::move(buffer_begin_info));
                            record_data.recordFn(new_buffer);
                            EndRecording(new_buffer);

                            request.returnValue.set_value(GpuCommand{ thread_id, new_buffer });
                            break;
                        }

                        case CommandRequestType::eSubmitOnce:
                        {
                            auto submit_data = std::any_cast<CommandSubmitData&>(request.requestValue);

                            BeginRecording(thread_buffer, DefaultBeginInfo());
                            submit_data.command(thread_buffer);
                            EndRecording(thread_buffer);

                            auto submit_info = submit_data.submitInfo;
                            submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                            submit_info.commandBufferCount = 1;
                            submit_info.pCommandBuffers    = &thread_buffer;

                            vkQueueWaitIdle(graphics_queue);
                            vkQueueSubmit(graphics_queue, 1, &submit_info, executed_fence);

                            vkWaitForFences(device, 1, &executed_fence, VK_TRUE, -1);
                            vkResetFences(device, 1, &executed_fence);
                            vkResetCommandBuffer(thread_buffer, 0);

                            request.returnValue.set_value({ true });
                            break;
                        }

                        default:
                            break;
                        }
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

        std::future<std::any> RecordAsync(CommandRecordFn&& commands, AdditionalRecordData&& recordData)
        {
            auto promise = std::promise<std::any>();
            auto future = promise.get_future();

            REQUESTS.emplace(std::move(CommandRequestData
            {
                CommandRequestType::ePrerecord,
                std::move(promise),
                CommandRecordData
                {
                    .recordFn = std::move(commands),
                    .userData = std::move(recordData)
                }
            }));

            WORKER_CONDITION.notify_one();

            return future;
        }

        std::future<std::any> SubmitAsync(CommandRecordFn&& commands, VkSubmitInfo submitInfo)
        {
            auto promise = std::promise<std::any>();
            auto future = promise.get_future();

            REQUESTS.emplace
            ({
                CommandRequestType::eSubmitOnce,
                std::move(promise),
                CommandSubmitData
                {
                    .command    = std::move(commands),
                    .submitInfo = std::move(submitInfo)
                }
            });

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

    VkCommandBuffer AllocateThreadBuffer(VkCommandPool& pool, VkCommandBufferLevel level)
    {
        const VkCommandBufferAllocateInfo command_buffer_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool        = pool,
            .level              = level,
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

    void BeginRecording(const VkCommandBuffer& buffer, const VkCommandBufferBeginInfo&& beginInfo)
    {
        VkResult result;
        result = vkBeginCommandBuffer(buffer, &beginInfo);
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
