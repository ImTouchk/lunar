#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_shader.hpp"
#include "vk_renderer.hpp"
#include "render/renderer.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <mutex>
#include <any>

ShaderWrapper::ShaderWrapper(Vk::ShaderManager& manager, Identifier handle)
    : manager(manager), identifier(handle)
{
}

VkPipeline ShaderWrapper::pipeline() const
{
    const auto& data = find_by_identifier_safe(manager.shaders, identifier);
    return data.handle;
}

namespace Vk
{
    VkShaderModule CreateModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo module_create_info =
        {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkResult result;
        VkShaderModule module = VK_NULL_HANDLE;
        result = vkCreateShaderModule(GetDevice().handle, &module_create_info, nullptr, &module);
        if (result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Failed to create a shader module (vkCreateShaderModule didn't return VK_SUCCESS).");
            //throw std::runtime_error("Renderer-Vulkan-ShaderModule-CreationFail");
            return VK_NULL_HANDLE; // exceptions are slow; it's not the end of the world if a shader doesn't compile
        }

        return module;
    }

    void ShaderManager::create_descriptor_pool()
    {
        std::array<VkDescriptorSetLayoutBinding, 2> layout_bindings =
        {
            VkDescriptorSetLayoutBinding
            {
                .binding            = 0,
                .descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_VERTEX_BIT,
                .pImmutableSamplers = nullptr,
            },
            VkDescriptorSetLayoutBinding
            {
                .binding            = 1,
                .descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount    = 1,
                .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            }
        };

        const VkDescriptorSetLayoutCreateInfo layout_create_info =
        {
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = nullptr,
            .bindingCount = static_cast<uint32_t>(layout_bindings.size()),
            .pBindings    = layout_bindings.data()
        };

        VkResult result;
        result = vkCreateDescriptorSetLayout(GetDevice().handle, &layout_create_info, nullptr, &descriptorLayout);
        if (result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Shader manager creation fail (vkCreateDescriptorSetLayout didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ShaderManager-CreationFail");
        }

        const std::array<VkDescriptorPoolSize, 2> pool_sizes =
        {
            VkDescriptorPoolSize
            {
                .type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
            },
            VkDescriptorPoolSize
            {
                .type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT)
            }
        };

        const VkDescriptorPoolCreateInfo pool_create_info =
        {
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext         = nullptr,
            .flags         = 0,
            .maxSets       = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
            .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
            .pPoolSizes    = pool_sizes.data()
        };

        result = vkCreateDescriptorPool(GetDevice().handle, &pool_create_info, nullptr, &descriptorPool);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Shader manager creation failed (vkCreateDescriptorPool didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ShaderManager-CreationFail");
        }

        descriptorSets.fill(VK_NULL_HANDLE);

        std::array<VkDescriptorSetLayout, MAX_FRAMES_IN_FLIGHT> layouts;
        layouts.fill(descriptorLayout);

        VkDescriptorSetAllocateInfo set_allocate_info =
        {
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = nullptr,
            .descriptorPool     = descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
            .pSetLayouts        = layouts.data()
        };

        result = vkAllocateDescriptorSets(GetDevice().handle, &set_allocate_info, descriptorSets.data());
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Shader manager creation failed (vkAllocateDescriptorSets didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ShaderManager-CreationFail");
        }
    }

    void ShaderManager::create_graphics_layout()
    {
        VkPushConstantRange push_constant =
        {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset     = 0,
            .size       = sizeof(glm::mat4),
        };

        const VkPipelineLayoutCreateInfo pipeline_layout_create_info =
        {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = 1,
            .pSetLayouts            = &descriptorLayout,
            .pushConstantRangeCount = 1,
            .pPushConstantRanges    = &push_constant
        };

        VkResult result;
        result = vkCreatePipelineLayout(GetDevice().handle, &pipeline_layout_create_info, nullptr, &graphicsLayout);
        if (result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Shader manager creation fail (vkCreatePipelineLayout didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ShaderManager-CreationFail");
        }
    }

    void ShaderManager::create(SwapchainWrapper& swapchain)
    {
        assert(not active);

        pSwapchain = &swapchain;

        create_descriptor_pool();
        create_graphics_layout();

        active = true;
    }

    void ShaderManager::destroy()
    {
        assert(active == true);

        for(const auto& shader : shaders)
        {
            vkDestroyPipeline(GetDevice().handle, shader.handle, nullptr);
        }

        vkDestroyPipelineLayout(GetDevice().handle, graphicsLayout, nullptr);
        vkDestroyDescriptorPool(GetDevice().handle, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(GetDevice().handle, descriptorLayout, nullptr);

        descriptorSets.fill(VK_NULL_HANDLE);

        active = false;

        CDebug::Log("Vulkan Renderer | Shader manager destroyed.");
    }

    VkPipelineLayout& ShaderManager::get_graphics_layout()
    {
        assert(active == true);
        return graphicsLayout;
    }

    VkDescriptorSet& ShaderManager::get_descriptor_set(size_t frame)
    {
        assert(active == true);
        return descriptorSets.at(frame);
    }

    VkVertexInputBindingDescription ShaderManager::get_vertex_binding_desc()
    {
        return VkVertexInputBindingDescription
        {
            .binding   = 0,
            .stride    = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
    }

    std::vector<VkVertexInputAttributeDescription> ShaderManager::get_vertex_attribute_desc()
    {
        return std::vector<VkVertexInputAttributeDescription>
        {
            VkVertexInputAttributeDescription
            {
                .location = 0,
                .binding  = 0,
                .format   = VK_FORMAT_R32G32B32_SFLOAT,
                .offset   = 0,
            }
        };
    }

    std::vector<ShaderWrapper> ShaderManager::create_graphics(GraphicsShaderCreateInfo* pCreateInfos, unsigned count)
    {
        assert(active == true);

        auto vertex_binding_desc = get_vertex_binding_desc();
        auto vertex_attribute_desc = get_vertex_attribute_desc();

        VkPipelineVertexInputStateCreateInfo vertex_input_info =
        {
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = 1,
            .pVertexBindingDescriptions      = &vertex_binding_desc,
            .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_attribute_desc.size()),
            .pVertexAttributeDescriptions    = vertex_attribute_desc.data()
        };

        VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info =
        {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        VkViewport viewport =
        {
            .x      = 0.f,
            .y      = 0.f,
            .width  = 0.f,
            .height = 0.f,
        };

        VkRect2D scissor =
        {
            .offset = { 0, 0 },
            .extent = { 0 }
        };

        VkPipelineViewportStateCreateInfo viewport_state_create_info =
        {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports    = &viewport,
            .scissorCount  = 1,
            .pScissors     = &scissor
        };

        VkPipelineRasterizationStateCreateInfo rasterizer =
        {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .cullMode                = VK_CULL_MODE_BACK_BIT,
            .frontFace               = VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.f,
            .depthBiasClamp          = 0.f,
            .depthBiasSlopeFactor    = 0.f,
            .lineWidth               = 1.f,
        };

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info =
        {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable   = VK_FALSE,
            .minSampleShading      = 1.f,
            .pSampleMask           = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable      = VK_FALSE,
        };

        VkPipelineColorBlendAttachmentState color_attachment_state =
        {
            .blendEnable         = VK_FALSE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
            .colorBlendOp        = VK_BLEND_OP_ADD,
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
            .alphaBlendOp        = VK_BLEND_OP_ADD,
            .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo color_blend_create_info =
        {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable   = VK_FALSE,
            .logicOp         = VK_LOGIC_OP_COPY,
            .attachmentCount = 1,
            .pAttachments    = &color_attachment_state,
            .blendConstants  = { 0.f, 0.f, 0.f, 0.f }
        };

        VkDynamicState dynamic_states[] =
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info =
        {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates    = dynamic_states,
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info =
        {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable       = VK_TRUE,
            .depthWriteEnable      = VK_TRUE,
            .depthCompareOp        = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE,
            .front                 = {},
            .back                  = {},
            .minDepthBounds        = 0.f,
            .maxDepthBounds        = 1.f,
        };

        struct CreateInfoData
        {
            VkShaderModule modules[2];
            VkPipelineShaderStageCreateInfo stageCreateInfos[2];
        };

        auto pipeline_stage_create_infos = std::unique_ptr<CreateInfoData[]>(new CreateInfoData[count]);
        auto pipeline_create_infos       = std::unique_ptr<VkGraphicsPipelineCreateInfo[]>(new VkGraphicsPipelineCreateInfo[count]);
        auto result_pipelines            = std::unique_ptr<VkPipeline[]>(new VkPipeline[count]);
        auto shaders_to_compile          = 0;

        for (auto i : range(0, count - 1))
        {
            auto& create_info = pCreateInfos[i];

            auto vertex_module   = Vk::CreateModule(create_info.vertexCode);
            auto fragment_module = Vk::CreateModule(create_info.fragmentCode);

            if (vertex_module == VK_NULL_HANDLE || fragment_module == VK_NULL_HANDLE)
            {
                CDebug::Error("Vulkan Renderer | Failed to create shader pipeline {}. Skipping...", i);
                continue;
            }

            pipeline_stage_create_infos[shaders_to_compile] = CreateInfoData
            {
                .modules = { vertex_module, fragment_module },
                .stageCreateInfos =
                {
                    VkPipelineShaderStageCreateInfo
                    {
                        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage  = VK_SHADER_STAGE_VERTEX_BIT,
                        .module = vertex_module,
                        .pName  = "main",
                    },
                    VkPipelineShaderStageCreateInfo
                    {
                        .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                        .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
                        .module = fragment_module,
                        .pName  = "main"
                    }
                }
            };

            pipeline_create_infos[shaders_to_compile] = VkGraphicsPipelineCreateInfo
            {
                .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount          = 2,
                .pStages             = pipeline_stage_create_infos[shaders_to_compile].stageCreateInfos,
                .pVertexInputState   = &vertex_input_info,
                .pInputAssemblyState = &assembly_state_create_info,
                .pViewportState      = &viewport_state_create_info,
                .pRasterizationState = &rasterizer,
                .pMultisampleState   = &multisample_state_create_info,
                .pDepthStencilState  = &depth_stencil_state_create_info,
                .pColorBlendState    = &color_blend_create_info,
                .pDynamicState       = &dynamic_state_create_info,
                .layout              = graphicsLayout,
                .renderPass          = pSwapchain->render_pass(),
                .subpass             = 0,
                .basePipelineHandle  = nullptr,
                .basePipelineIndex   = -1,
            };

            shaders_to_compile++;
        }

        vkCreateGraphicsPipelines(GetDevice().handle, nullptr, shaders_to_compile, pipeline_create_infos.get(), nullptr, result_pipelines.get());

        auto start_index = shaders.size();
        auto final_handles = std::vector<ShaderWrapper>();
        auto compiled_pipelines = 0;
        for (auto i : range(0, shaders_to_compile - 1))
        {
            if (result_pipelines[i] == VK_NULL_HANDLE)
            {
                continue;
            }

            auto final_data = ShaderData
            {
                .identifier = get_unique_number(),
                .type       = ShaderType::eGraphics,
                .handle     = result_pipelines[i]
            };

            final_handles.emplace_back(ShaderWrapper(*this, final_data.identifier));
            shaders.push_back(std::move(final_data));
        }

        CDebug::Log("Vulkan Renderer | Compiled {} (graphics) pipelines.", final_handles.size());

        for (auto i : range(0, shaders_to_compile - 1))
        {
            vkDestroyShaderModule(GetDevice().handle, pipeline_stage_create_infos[i].modules[0], nullptr);
            vkDestroyShaderModule(GetDevice().handle, pipeline_stage_create_infos[i].modules[1], nullptr);
        }

        return final_handles;
    }
}
