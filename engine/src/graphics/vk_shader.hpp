#pragma once
#include "render/shader.hpp"
#include "utils/identifier.hpp"
#include "vk_forward_decl.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

namespace Vk
{
    struct ShaderData
    {
        Identifier identifier      = 0;
        ShaderType type            = ShaderType::eUnknown;
        VkPipeline handle          = VK_NULL_HANDLE;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
    };

    struct ShaderManager
    {
    public:
        friend class ::ShaderWrapper;

        ShaderManager() = default;
        ~ShaderManager() = default;

        void create(SwapchainWrapper& swapchain);
        void destroy();

        std::vector<ShaderWrapper> create_graphics(GraphicsShaderCreateInfo* pCreateInfos, unsigned count);
        std::vector<ShaderWrapper> create_compute(ComputeShaderCreateInfo* pCreateInfos, unsigned count);
        //void create_compute();

        [[nodiscard]] VkPipelineLayout& get_graphics_layout();

        static VkVertexInputBindingDescription get_vertex_binding_desc();
        static std::vector<VkVertexInputAttributeDescription> get_vertex_attribute_desc();

    private:
        void create_descriptor_pool();
        void create_graphics_layout();
        void create_compute_layout();

    private:
        SwapchainWrapper* pSwapchain = nullptr;

        bool active = false;

        std::vector<ShaderData> shaders           = {};
        VkPipelineLayout graphicsLayout           = VK_NULL_HANDLE;
        VkPipelineLayout computePathTracingLayout = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool           = VK_NULL_HANDLE;
        VkDescriptorSetLayout descriptorLayout    = VK_NULL_HANDLE;
    };
}
