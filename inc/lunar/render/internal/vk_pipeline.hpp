#pragma once
#include <lunar/render/internal/vk_base.hpp>
#include <lunar/file/filesystem.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <queue>

namespace Render
{
	struct LUNAR_API VulkanDescriptorLayoutBuilder
	{
	public:
		VulkanDescriptorLayoutBuilder() = default;
		~VulkanDescriptorLayoutBuilder() = default;

		VulkanDescriptorLayoutBuilder& clear();
		VulkanDescriptorLayoutBuilder& useVulkanContext(VulkanContext& context);
		VulkanDescriptorLayoutBuilder& addShaderStageFlag(vk::ShaderStageFlags flag);
		VulkanDescriptorLayoutBuilder& addLayoutCreateFlag(vk::DescriptorSetLayoutCreateFlagBits flag);
		VulkanDescriptorLayoutBuilder& addBinding(uint32_t binding, vk::DescriptorType type);
		VulkanDescriptorLayoutBuilder& setNext(void* pNext);
		[[nodiscard]] vk::DescriptorSetLayout build();

	private:
		VulkanContext* context = nullptr;
		void* pNext = nullptr;
		vk::ShaderStageFlags stageFlags = {};
		vk::DescriptorSetLayoutCreateFlags createFlags = {};
		std::vector<vk::DescriptorSetLayoutBinding> bindings;
	};

	class LUNAR_API VulkanDescriptorAllocator
	{
	public:
		struct PoolSizeRatio
		{
			vk::DescriptorType type;
			float ratio;
		};

		VulkanDescriptorAllocator(VulkanContext& context, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
		VulkanDescriptorAllocator();
		~VulkanDescriptorAllocator();

		VulkanDescriptorAllocator(VulkanDescriptorAllocator&&) noexcept;
		VulkanDescriptorAllocator& operator=(VulkanDescriptorAllocator&&) noexcept;

		void destroy();
		void clearDescriptors();
		[[nodiscard]] vk::DescriptorSet allocate(vk::DescriptorSetLayout layout);

		vk::DescriptorPool pool;

	private:
		VulkanContext* context;
	};

	class LUNAR_API VulkanGrowableDescriptorAllocator
	{
	public:
		struct PoolSizeRatio
		{
			vk::DescriptorType type;
			float ratio;
		};

		VulkanGrowableDescriptorAllocator(VulkanContext* context, uint32_t initialSets, std::span<PoolSizeRatio> poolRatios);
		VulkanGrowableDescriptorAllocator() = default;
		~VulkanGrowableDescriptorAllocator();

		VulkanGrowableDescriptorAllocator(VulkanGrowableDescriptorAllocator&&)  noexcept;
		VulkanGrowableDescriptorAllocator& operator=(VulkanGrowableDescriptorAllocator&&) noexcept;

		void destroy();
		void clearPools();

		[[nodiscard]] vk::DescriptorSet allocate(vk::DescriptorSetLayout layout);
	private:
		[[nodiscard]] vk::DescriptorPool getPool();
		[[nodiscard]] vk::DescriptorPool createPool(uint32_t setCount, std::span<PoolSizeRatio> poolRatios);

		VulkanContext* context = nullptr;
		std::vector<PoolSizeRatio> ratios = {};
		std::vector<vk::DescriptorPool> fullPools = {};
		std::vector<vk::DescriptorPool> readyPools = {};
		uint32_t setsPerPool = 0;
	};

	struct LUNAR_API VulkanDescriptorWriter
	{
		std::deque<vk::DescriptorImageInfo> imageInfos = {};
		std::deque<vk::DescriptorBufferInfo> bufferInfos = {};
		std::vector<vk::WriteDescriptorSet> writes = {};

		VulkanDescriptorWriter& clear();
		VulkanDescriptorWriter& writeImage(int binding, vk::ImageView view, vk::Sampler sampler, vk::ImageLayout layout, vk::DescriptorType type);
		VulkanDescriptorWriter& writeBuffer(int binding, vk::Buffer buffer, size_t size, size_t offset, vk::DescriptorType type);
		void updateSet(VulkanContext* context, vk::DescriptorSet set);
	};

	struct LUNAR_API VulkanPipelineBuilder
	{
	public:
		VulkanPipelineBuilder() = default;
		~VulkanPipelineBuilder() = default;

		VulkanPipelineBuilder& useVulkanContext(VulkanContext* ctx);
		VulkanPipelineBuilder& setVertexShader(const Fs::Path& path);
		VulkanPipelineBuilder& setFragmentShader(const Fs::Path& path);
		VulkanPipelineBuilder& setInputTopology(vk::PrimitiveTopology topology);
		VulkanPipelineBuilder& setPolygonMode(vk::PolygonMode mode);
		VulkanPipelineBuilder& setCullMode(vk::CullModeFlags cullMode, vk::FrontFace frontFace);
		VulkanPipelineBuilder& noMultisampling();
		VulkanPipelineBuilder& disableBlending();
		VulkanPipelineBuilder& additiveBlending();
		VulkanPipelineBuilder& alphaBlending();
		VulkanPipelineBuilder& setColorAttachmentFormat(vk::Format format);
		VulkanPipelineBuilder& setDepthFormat(vk::Format format);
		VulkanPipelineBuilder& disableDepthTesting();
		VulkanPipelineBuilder& useLayout(vk::PipelineLayout layout);
		vk::Pipeline create();

	private:
		VulkanContext* context = nullptr;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {};
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
		vk::PipelineRasterizationStateCreateInfo rasterizationState = {};
		vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
		vk::PipelineMultisampleStateCreateInfo multisampleState = {};
		vk::PipelineLayout pipelineLayout = {};
		vk::PipelineDepthStencilStateCreateInfo depthStencilState = {};
		vk::PipelineRenderingCreateInfo renderInfo = {};
		vk::Format colorAttachmentFormat = {};
	};
}
