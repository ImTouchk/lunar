#pragma once
#include "utils/identifier.hpp"
#include <vulkan/vulkan.h>
#include <functional>

namespace Vk
{
    struct SurfaceWrapper;
    struct SwapchainWrapper;
    struct CommandQueueWrapper;
    struct SyncObjectsWrapper;

    struct ShaderManager;
    class ObjectManager;
    class RenderCallManager;

    constexpr unsigned MAX_FRAMES_IN_FLIGHT = 2;

    struct GpuCommand
    {
    public:
        GpuCommand(Identifier thread_id, VkCommandBuffer buffer);
        GpuCommand() = default;

        void destroy();
        [[nodiscard]] VkCommandBuffer& handle();
        [[nodiscard]] const VkCommandBuffer& handle() const;

    private:
        Identifier thread_id = 0;
        VkCommandBuffer buffer = VK_NULL_HANDLE;
    };

    enum class GpuCommandType
    {
        eUnknown = 0,
        eSecondaryMesh,
    };

    using CommandRecordFn = std::function<void(VkCommandBuffer&)>;
}
