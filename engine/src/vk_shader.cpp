#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <mutex>
#include <any>

namespace Vk
{
	struct ShaderData
	{
		ShaderType type;
		SwapchainWrapper* pSwapchain = nullptr;
		VkPipeline handle = VK_NULL_HANDLE;
		VkPipelineLayout layout = VK_NULL_HANDLE;
	};

	std::vector<ShaderData> SHADERS = {};
	std::mutex SHADERS_MUTEX = {};

	VkShaderModule CreateModule(LogicalDeviceWrapper& device, const std::vector<uint8_t>& code, int offset, int size)
	{
		VkShaderModuleCreateInfo module_create_info =
		{
			.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = static_cast<size_t>(size),
			.pCode    = reinterpret_cast<const uint32_t*>(code.data() + offset)
		};

		VkResult result;
		VkShaderModule module = VK_NULL_HANDLE;
		result = vkCreateShaderModule(device.handle(), &module_create_info, nullptr, &module);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Failed to create a shader module (vkCreateShaderModule didn't return VK_SUCCESS).");
			//throw std::runtime_error("Renderer-Vulkan-ShaderModule-CreationFail");
			return VK_NULL_HANDLE; // exceptions would be too slow here
		}
	
		return module;
	}

	bool CreateGraphicsShader(ShaderCreateInfo& info)
	{
		auto code_sizes = std::any_cast<std::pair<uint32_t, uint32_t>>(info.data);
		
		auto vertex_module = CreateModule(info.device, info.bytes, 0, code_sizes.first);
		auto fragment_module = CreateModule(info.device, info.bytes, code_sizes.first, code_sizes.second);

		VkPipelineShaderStageCreateInfo pipeline_stages[2] =
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
		};

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
			.width  = static_cast<float>(info.swapchain.get_width()),
			.height = static_cast<float>(info.swapchain.get_height())
		};

		VkRect2D scissor =
		{
			.offset = { 0, 0 },
			.extent = info.swapchain.surface_extent()
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
			.dynamicStateCount = 2,
			.pDynamicStates    = nullptr,
		};
		
		

		vkDestroyShaderModule(info.device.handle(), vertex_module, nullptr);
		vkDestroyShaderModule(info.device.handle(), fragment_module, nullptr);

		return true;
	}

	void CreateShaders(ShaderCreateInfo* pInfos, int count)
	{
		assert(pInfos != nullptr);

		for (int i = 0; i < count; i++)
		{
			auto& shader_create_info = pInfos[i];
			if (shader_create_info.type == ShaderType::eGraphics)
			{
				CreateGraphicsShader(shader_create_info);
			}
		}
	}
}
