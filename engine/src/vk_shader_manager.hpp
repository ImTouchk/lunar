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

        [[nodiscard]] VkPipeline try_get(Shader handle) const;
        [[nodiscard]] VkPipelineLayout get_graphics_layout() const;

        static VkVertexInputBindingDescription get_vertex_binding_desc();
        static std::vector<VkVertexInputAttributeDescription> get_vertex_attribute_desc();
    private:
        SwapchainWrapper* pSwapchain = nullptr;
        LogicalDeviceWrapper* pDevice = nullptr;
        std::vector<ShaderData> shaders;
        VkPipelineLayout graphicsLayout = VK_NULL_HANDLE;
    };
}