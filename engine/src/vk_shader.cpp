#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"
#include "render/renderer.hpp"

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <mutex>
#include <any>

namespace Vk
{
    VkShaderModule CreateModule(LogicalDeviceWrapper& device, const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo module_create_info =
        {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = code.size(),
            .pCode    = reinterpret_cast<const uint32_t*>(code.data())
        };

        VkResult result;
        VkShaderModule module = VK_NULL_HANDLE;
        result = vkCreateShaderModule(device.handle(), &module_create_info, nullptr, &module);
        if (result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Failed to create a shader module (vkCreateShaderModule didn't return VK_SUCCESS).");
            //throw std::runtime_error("Renderer-Vulkan-ShaderModule-CreationFail");
            return VK_NULL_HANDLE; // exceptions are slow; it's not the end of the world if a shader doesn't compile
        }

        return module;
    }

    std::mutex SHADERS_VECTOR_MUTEX = {};
}

std::vector<Shader> GameRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned count)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);

    VkPipelineVertexInputStateCreateInfo vertex_input_info =
    {
        .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount   = 0,
        .pVertexBindingDescriptions      = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions    = nullptr
    };

    VkPipelineInputAssemblyStateCreateInfo assembly_state_create_info =
    {
        .sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport =
    {
        .x      = 0.f,
        .y      = 0.f,
        .width  = static_cast<float>(internal_data->swapchain.get_width()),
        .height = static_cast<float>(internal_data->swapchain.get_height())
    };

    VkRect2D scissor =
    {
        .offset = { 0, 0 },
        .extent = internal_data->swapchain.surface_extent()
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
        VK_DYNAMIC_STATE_LINE_WIDTH
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info =
    {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 0,
        .pDynamicStates    = nullptr,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info =
    {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = 0,
        .pSetLayouts            = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = nullptr
    };

    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkResult result;
    result = vkCreatePipelineLayout(internal_data->device.handle(), &pipeline_layout_create_info, nullptr, &layout);

    if(result != VK_SUCCESS)
    {
        CDebug::Error("Vulkan Renderer | Failed to create {} graphics pipelines (vkCreatePipelineLayout didn't return VK_SUCCESS).", count);
        return {};
    }

    struct CreateInfoData
    {
        VkShaderModule modules[2];
        VkPipelineShaderStageCreateInfo stageCreateInfos[2];
    };

    auto pipeline_stage_create_infos = std::unique_ptr<CreateInfoData[]>(new CreateInfoData[count]);
    auto pipeline_create_infos = std::unique_ptr<VkGraphicsPipelineCreateInfo[]>(new VkGraphicsPipelineCreateInfo[count]);
    auto result_pipelines = std::unique_ptr<VkPipeline[]>(new VkPipeline[count]);
    auto shaders_to_compile = 0;

    for(auto i : range(0, count - 1))
    {
        auto& create_info = pCreateInfos[i];

        auto vertex_module = Vk::CreateModule(internal_data->device, create_info.vertexCode);
        auto fragment_module = Vk::CreateModule(internal_data->device, create_info.fragmentCode);

        if(vertex_module == VK_NULL_HANDLE || fragment_module == VK_NULL_HANDLE)
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
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_VERTEX_BIT,
                    .module = vertex_module,
                    .pName = "main",
                },
                VkPipelineShaderStageCreateInfo
                {
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .module = fragment_module,
                    .pName = "main"
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
            .pDepthStencilState  = nullptr,
            .pColorBlendState    = &color_blend_create_info,
            .pDynamicState       = nullptr,
            .layout              = layout,
            .renderPass          = internal_data->swapchain.render_pass(),
            .subpass             = 0,
            .basePipelineHandle  = nullptr,
            .basePipelineIndex   = -1,
        };

        shaders_to_compile++;
    }

    vkCreateGraphicsPipelines(internal_data->device.handle(), nullptr, shaders_to_compile, pipeline_create_infos.get(), nullptr, result_pipelines.get());

    auto start_index = internal_data->shaders.size();
    auto final_handles = std::vector<unsigned>();

    std::lock_guard<std::mutex> lock_guard(Vk::SHADERS_VECTOR_MUTEX);
    auto compiled_pipelines = 0;
    for(auto i : range(0, shaders_to_compile - 1))
    {
        if(result_pipelines[i] == VK_NULL_HANDLE)
        {
            continue;
        }

        internal_data->shaders.push_back(Vk::ShaderData
        {
            .type   = Vk::ShaderType::eGraphics,
            .layout = layout,
            .handle = result_pipelines[i]
        });

        final_handles.push_back(start_index + i);
    }

    CDebug::Log("Vulkan Renderer | Compiled {} (graphics) pipelines.", final_handles.size());

    for(auto i : range(0, shaders_to_compile - 1))
    {
        vkDestroyShaderModule(internal_data->device.handle(), pipeline_stage_create_infos[i].modules[0], nullptr);
        vkDestroyShaderModule(internal_data->device.handle(), pipeline_stage_create_infos[i].modules[1], nullptr);
    }

    return std::move(final_handles);
}