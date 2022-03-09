#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <future>

namespace Vk
{
    // Function that registers commands to provided buffer
    using CmdFn = std::function<void(VkCommandBuffer)>;

    // Yeets itself onto a secondary thread that does one-time command submits
    class CmdSubmitter
    {
    public:
        CmdSubmitter() = default;
        ~CmdSubmitter() = default;

        void create();
        void destroy();

        std::future<bool> submit(CmdFn&& commands);
    };
}
