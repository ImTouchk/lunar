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

    using CommandRecordFn = std::function<void(VkCommandBuffer&)>;
}
