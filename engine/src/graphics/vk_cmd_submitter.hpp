#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <future>

namespace Vk
{
    struct CmdSubmitterCreateInfo
    {
        VkDevice device;
        VkQueue graphicsQueue;
        unsigned queueIndex;
    };

    // Function that registers commands to provided buffer
    using CmdFn = std::function<void(VkCommandBuffer)>;

    // Yeets itself onto a secondary thread that does one-time command submits
    class CmdSubmitter
    {
    public:
        CmdSubmitter() = default;
        ~CmdSubmitter() = default;

        void create(CmdSubmitterCreateInfo&& createInfo);
        void destroy();

        void submit(CmdFn&& commands, std::promise<bool>& finished);
    };
}
