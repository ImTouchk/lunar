#include <lunar/render/internal/render_vk.hpp>
#include <lunar/file/binary_file.hpp>
#include <lunar/debug.hpp>

namespace Render
{
	vk::ShaderModule LoadShaderModule(vk::Device& device, const std::vector<char>& content)
	{
		auto create_info = vk::ShaderModuleCreateInfo
		{
			.codeSize = content.size(),
			.pCode = reinterpret_cast<const uint32_t*>(content.data()),
		};

		return device.createShaderModule(create_info);
	}

	vk::ShaderModule LoadShaderModule(vk::Device& device, const Fs::Path& path)
	{
		auto file = Fs::BinaryFile(path);
		return LoadShaderModule(device, file.content);
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::useVulkanContext(VulkanContext* ctx)
	{
		context = ctx;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setVertexShader(const Fs::Path& path)
	{
		shaderStages.push_back(vk::PipelineShaderStageCreateInfo
		{
			.stage  = vk::ShaderStageFlagBits::eVertex,
			.module = LoadShaderModule(context->device, path),
			.pName  = "main"
		});
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setFragmentShader(const Fs::Path& path)
	{
		shaderStages.push_back(vk::PipelineShaderStageCreateInfo
		{
			.stage  = vk::ShaderStageFlagBits::eFragment,
			.module = LoadShaderModule(context->device, path),
			.pName  = "main"
		});
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setInputTopology(vk::PrimitiveTopology topology)
	{
		inputAssembly.topology = topology;
		inputAssembly.primitiveRestartEnable = vk::False;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setPolygonMode(vk::PolygonMode mode)
	{
		rasterizationState.polygonMode = mode;
		rasterizationState.lineWidth = 1.f;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace)
	{
		rasterizationState.cullMode = cullMode;
		rasterizationState.frontFace = frontFace;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::noMultisampling()
	{
		multisampleState.sampleShadingEnable = vk::False;
		multisampleState.rasterizationSamples = vk::SampleCountFlagBits::e1;
		multisampleState.minSampleShading = 1.f;
		multisampleState.alphaToCoverageEnable = vk::False;
		multisampleState.alphaToOneEnable = vk::False;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::disableBlending()
	{
		colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | 
			vk::ColorComponentFlagBits::eG | 
			vk::ColorComponentFlagBits::eB | 
			vk::ColorComponentFlagBits::eA;

		colorBlendAttachment.blendEnable = vk::False;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setColorAttachmentFormat(vk::Format format)
	{
		colorAttachmentFormat = format;
		renderInfo.colorAttachmentCount = 1;
		renderInfo.pColorAttachmentFormats = &colorAttachmentFormat;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::setDepthFormat(vk::Format format)
	{
		renderInfo.depthAttachmentFormat = format;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::disableDepthTesting()
	{
		depthStencilState.depthTestEnable = vk::False;
		depthStencilState.depthWriteEnable = vk::False;
		depthStencilState.depthCompareOp = vk::CompareOp::eNever;
		depthStencilState.depthBoundsTestEnable = vk::False;
		depthStencilState.stencilTestEnable = vk::False;
		depthStencilState.front = {};
		depthStencilState.back = {};
		depthStencilState.minDepthBounds = 0.f;
		depthStencilState.maxDepthBounds = 1.f;
		return *this;
	}

	VulkanPipelineBuilder& VulkanPipelineBuilder::useLayout(vk::PipelineLayout layout)
	{
		pipelineLayout = layout;
		return *this;
	}

	vk::Pipeline VulkanPipelineBuilder::create()
	{
		auto viewport_state = vk::PipelineViewportStateCreateInfo
		{
			.viewportCount = 1,
			.scissorCount  = 1,
		};

		auto color_blending = vk::PipelineColorBlendStateCreateInfo
		{
			.logicOpEnable   = vk::False,
			.logicOp         = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments    = &colorBlendAttachment
		};

		auto vertex_input_info = vk::PipelineVertexInputStateCreateInfo {};

		vk::DynamicState dyn_states[] =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
		};

		auto dynamic_state_info = vk::PipelineDynamicStateCreateInfo
		{
			.dynamicStateCount = 2,
			.pDynamicStates    = dyn_states,
		};

		auto pipeline_create_info = vk::GraphicsPipelineCreateInfo
		{
			.pNext               = &renderInfo,
			.stageCount          = static_cast<uint32_t>(shaderStages.size()),
			.pStages             = shaderStages.data(),
			.pVertexInputState   = &vertex_input_info,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState      = &viewport_state,
			.pRasterizationState = &rasterizationState,
			.pMultisampleState   = &multisampleState,
			.pDepthStencilState  = &depthStencilState,
			.pColorBlendState    = &color_blending,
			.pDynamicState       = &dynamic_state_info,
			.layout              = pipelineLayout,
		};

		auto res = context->device.createGraphicsPipeline(VK_NULL_HANDLE, pipeline_create_info);
		
		for (auto& stage : shaderStages)
			context->device.destroyShaderModule(stage.module);
		
		if (res.result != vk::Result::eSuccess)
		{
			DEBUG_ERROR("Failed to compile graphics pipeline: {}", static_cast<uint32_t>(res.result));
			return VK_NULL_HANDLE;
		}

		return res.value;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::clear()
	{
		bindings.clear();
		stageFlags = {};
		createFlags = {};
		return *this;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::addBinding(uint32_t binding, vk::DescriptorType type)
	{
		bindings.push_back(vk::DescriptorSetLayoutBinding 
		{
			.binding         = binding,
			.descriptorType  = type,
			.descriptorCount = 1,
		});

		return *this;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::useVulkanContext(VulkanContext& context)
	{
		this->context = &context;
		return *this;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::addShaderStageFlag(vk::ShaderStageFlagBits flag)
	{
		stageFlags |= flag;
		return *this;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::addLayoutCreateFlag(vk::DescriptorSetLayoutCreateFlagBits flag)
	{
		createFlags |= flag;
		return *this;
	}

	VulkanDescriptorLayoutBuilder& VulkanDescriptorLayoutBuilder::setNext(void* pNext)
	{
		this->pNext = pNext;
		return *this;
	}

	vk::DescriptorSetLayout VulkanDescriptorLayoutBuilder::build()
	{
		for (auto& binding : bindings)
		{
			binding.stageFlags |= stageFlags;
		}

		auto create_info = vk::DescriptorSetLayoutCreateInfo
		{
			.pNext        = pNext,
			.flags        = createFlags,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings    = bindings.data(),
		};

		return context->getDevice().createDescriptorSetLayout(create_info);
	}

	VulkanDescriptorAllocator::VulkanDescriptorAllocator()
		: context(nullptr),
		pool(VK_NULL_HANDLE)
	{
	}

	VulkanDescriptorAllocator::VulkanDescriptorAllocator(VulkanDescriptorAllocator&& other) noexcept
		: context(other.context),
		pool(other.pool)
	{
		other.pool = VK_NULL_HANDLE;
		other.context = nullptr;
	}

	VulkanDescriptorAllocator& VulkanDescriptorAllocator::operator=(VulkanDescriptorAllocator&& other) noexcept
	{
		if (this != &other)
		{
			context = other.context;
			pool = other.pool;
			other.context = nullptr;
			other.pool = VK_NULL_HANDLE;
		}

		return *this;
	}

	VulkanDescriptorAllocator::VulkanDescriptorAllocator
	(
		VulkanContext& context,
		uint32_t maxSets,
		std::span<PoolSizeRatio> poolRatios
	)
		: context(&context),
		pool(VK_NULL_HANDLE)
	{
		std::vector<vk::DescriptorPoolSize> poolSizes;
		for (auto& pool_ratio : poolRatios)
		{
			poolSizes.push_back(vk::DescriptorPoolSize
			{
				.type            = pool_ratio.type,
				.descriptorCount = static_cast<uint32_t>(pool_ratio.ratio * maxSets)
			});
		}

		auto pool_info = vk::DescriptorPoolCreateInfo
		{
			.maxSets       = maxSets,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes    = poolSizes.data()
		};

		pool = context.device.createDescriptorPool(pool_info);
	}

	VulkanDescriptorAllocator::~VulkanDescriptorAllocator()
	{
		if (pool != VK_NULL_HANDLE)
			destroy();
	}

	void VulkanDescriptorAllocator::clearDescriptors()
	{
		DEBUG_ASSERT(pool != VK_NULL_HANDLE);
		context->device.resetDescriptorPool(pool);
	}

	void VulkanDescriptorAllocator::destroy()
	{
		DEBUG_ASSERT(pool != VK_NULL_HANDLE);
		context->device.destroyDescriptorPool(pool);
		pool = VK_NULL_HANDLE;
		context = nullptr;
	}

	vk::DescriptorSet VulkanDescriptorAllocator::allocate(vk::DescriptorSetLayout layout)
	{
		DEBUG_ASSERT(pool != VK_NULL_HANDLE);
		auto allocate_info = vk::DescriptorSetAllocateInfo
		{
			.descriptorPool     = pool,
			.descriptorSetCount = 1,
			.pSetLayouts        = &layout
		};

		return context->device.allocateDescriptorSets(allocate_info).at(0);
	}

	bool VulkanContext::createPipelines()
	{
		auto push_constant = vk::PushConstantRange
		{
			.stageFlags = vk::ShaderStageFlagBits::eCompute,
			.offset     = 0,
			.size       = sizeof(glm::vec4) * 4,
		};

		auto layout_info = vk::PipelineLayoutCreateInfo
		{
			.setLayoutCount         = 1,
			.pSetLayouts            = &drawImageDescriptorLayout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges    = &push_constant
		};

		gradientPipelineLayout = device.createPipelineLayout(layout_info);

		auto shader_mod = LoadShaderModule(device, Fs::dataDirectory().append("shader-bin/gradient.comp.spv"));
		auto stage_info = vk::PipelineShaderStageCreateInfo
		{
			.stage  = vk::ShaderStageFlagBits::eCompute,
			.module = shader_mod,
			.pName  = "main"
		};

		auto compute_info = vk::ComputePipelineCreateInfo
		{
			.stage  = stage_info,
			.layout = gradientPipelineLayout,
		};

		gradientPipeline = device.createComputePipeline(VK_NULL_HANDLE, compute_info).value;
		device.destroyShaderModule(shader_mod);

		trianglePipelineLayout = device.createPipelineLayout({});
		trianglePipeline = VulkanPipelineBuilder()
			.useVulkanContext(this)
			.useLayout(trianglePipelineLayout)
			.setVertexShader(Fs::dataDirectory().append("shader-bin/default.vert.spv"))
			.setFragmentShader(Fs::dataDirectory().append("shader-bin/default.frag.spv"))
			.setInputTopology(vk::PrimitiveTopology::eTriangleList)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setCullMode(vk::CullModeFlagBits::eNone, vk::FrontFace::eClockwise)
			.setColorAttachmentFormat(drawImage.format)
			.setDepthFormat(vk::Format::eUndefined)
			.noMultisampling()
			.disableBlending()
			.disableDepthTesting()
			.create();

		deletionStack.push([this]() {
			device.destroyPipelineLayout(trianglePipelineLayout);
			device.destroyPipeline(trianglePipeline);

			device.destroyPipelineLayout(gradientPipelineLayout);
			device.destroyPipeline(gradientPipeline);
		});

		return true;
	}
}
