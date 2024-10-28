#include <lunar/render/internal/render_vk.hpp>
#include <lunar/file/binary_file.hpp>
#include <lunar/debug.hpp>

namespace Render
{
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

	vk::ShaderModule LoadShaderModule(vk::Device& device, const std::vector<char>& content)
	{
		auto create_info = vk::ShaderModuleCreateInfo
		{
			.codeSize = content.size(),
			.pCode    = reinterpret_cast<const uint32_t*>(content.data()),
		};

		return device.createShaderModule(create_info);
	}

	vk::ShaderModule LoadShaderModule(vk::Device& device, const Fs::Path& path)
	{
		auto file = Fs::BinaryFile(path);
		return LoadShaderModule(device, file.content);
	}

	bool VulkanContext::createPipelines()
	{
		auto layout_info = vk::PipelineLayoutCreateInfo
		{
			.setLayoutCount = 1,
			.pSetLayouts    = &drawImageDescriptorLayout,
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

		deletionStack.push([this]() {
			device.destroyPipelineLayout(gradientPipelineLayout);
			device.destroyPipeline(gradientPipeline);
		});

		return true;
	}
}
