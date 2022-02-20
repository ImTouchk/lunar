#pragma once
#include "render/mesh.hpp"
#include "render/renderer.hpp"
#include <vulkan/vulkan.h>

namespace Vk
{
    class SwapchainWrapper;
    class LogicalDeviceWrapper;

    enum class ShaderType
    {
        eUnknown = 0,
        eGraphics,
        eCompute,
    };

    struct ShaderData
    {
        ShaderType type = ShaderType::eUnknown;
        VkPipeline handle = VK_NULL_HANDLE;
    };

    struct ShaderManager
    {
    public:
        ShaderManager() = default;
        ~ShaderManager() = default;

        void create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain);
        void destroy();

        std::vector<Shader> create_graphics(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);
        //void create_compute();

        VkPipeline get(Shader handle);

    private:
        SwapchainWrapper* pSwapchain = nullptr;
        LogicalDeviceWrapper* pDevice = nullptr;
        std::vector<ShaderData> shaders;
        VkPipelineLayout layout = VK_NULL_HANDLE;
    };
}