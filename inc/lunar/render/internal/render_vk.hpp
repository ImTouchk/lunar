#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/render/common.hpp>
#include <lunar/utils/stopwatch.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <variant>
#include <queue>
#include <stack>
#include <array>
#include <span>

namespace Render
{
	class VulkanContext;

	enum class LUNAR_API VulkanQueueType : size_t
	{
		eGraphics = 0,
		ePresent = 1,
		eTransfer = 2
	};

	struct LUNAR_API VulkanSemaphoreSubmit
	{
		using SubmitValue = std::variant<vk::Semaphore, vk::SemaphoreSubmitInfo>;

		VulkanSemaphoreSubmit(const std::initializer_list<SubmitValue>& semaphores);
		VulkanSemaphoreSubmit() = default;
		~VulkanSemaphoreSubmit() = default;

		[[nodiscard]] uint32_t size() const;
		[[nodiscard]] const vk::SemaphoreSubmitInfo* data() const;

		std::vector<vk::SemaphoreSubmitInfo>* operator->();
		std::vector<vk::SemaphoreSubmitInfo> value = { };
	};

	class LUNAR_API VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanContext& context, vk::CommandBuffer buffer, vk::Fence fence);
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

		void begin();

		void submit(
			const VulkanSemaphoreSubmit& waitSemaphores, 
			const VulkanSemaphoreSubmit& signalSemaphores,
			bool waitForFinish = false
		);

		void destroy();

		VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept;

		[[nodiscard]] operator vk::CommandBuffer& ();
		[[nodiscard]] vk::CommandBuffer* operator->();


		vk::CommandBuffer value;
		vk::Fence ready;
	private:
		VulkanContext* context;
		int state;
	};

	class LUNAR_API VulkanCommandPool
	{
	public:
		VulkanCommandPool(VulkanContext& context, vk::CommandPool pool);
		VulkanCommandPool();
		~VulkanCommandPool();

		VulkanCommandPool(VulkanCommandPool&&) noexcept;
		VulkanCommandPool& operator=(VulkanCommandPool&&) noexcept;

		[[nodiscard]] VulkanCommandBuffer allocateBuffer(vk::CommandBufferLevel level);

		void destroy();

		vk::CommandPool value;
	private:
		VulkanContext* context;
	};

	class LUNAR_API VulkanImage
	{
	public:
		VulkanImage(
			VulkanContext& context,
			vk::Image image,
			vk::ImageView view,
			VmaAllocation allocation,
			vk::Extent3D extent,
			vk::Format format
		);
		VulkanImage();
		~VulkanImage();

		void destroy();

		VulkanImage(VulkanImage&&) noexcept;
		VulkanImage& operator=(VulkanImage&&) noexcept;

		vk::Image handle;
		vk::ImageView view;
		VmaAllocation allocation;
		vk::Extent3D extent;
		vk::Format format;
	private:
		VulkanContext* context;
	};

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

	class LUNAR_API VulkanBuffer
	{
	public:
		VulkanBuffer(
			VulkanContext* context,
			vk::Buffer buffer,
			VmaAllocation allocation,
			VmaAllocationInfo info
		);
		VulkanBuffer() = default;
		~VulkanBuffer();

		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;
		VulkanBuffer(VulkanBuffer&&) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

		void destroy();

		operator vk::Buffer& ();

		vk::Buffer handle = VK_NULL_HANDLE;
		VmaAllocation allocation = VK_NULL_HANDLE;
		VmaAllocationInfo info = {};
	private:
		VulkanContext* context = nullptr;
	};

	class LUNAR_API VulkanMesh
	{
	public:
		VulkanBuffer indexBuffer;
		VulkanBuffer vertexBuffer;
		vk::DeviceAddress vertexBufferAddr;
	};

	class LUNAR_API VulkanShader
	{
	public:
		VulkanShader(
			VulkanContext* context,
			vk::Pipeline& pipeline,
			vk::PipelineLayout& layout,
			vk::Fence& wasCompiled
		);

		

		vk::Pipeline       pipeline    = VK_NULL_HANDLE;
		vk::PipelineLayout layout      = VK_NULL_HANDLE;
		vk::Fence          wasCompiled = VK_NULL_HANDLE;
	private:
		VulkanContext*     context     = nullptr;
	};

	class LUNAR_API VulkanContext : public RenderContext
	{
	public:
		VulkanContext(
			bool debugging,
			uint32_t minimumVersion,
			std::vector<const char*>& requiredDeviceExtensions,
			vk::PhysicalDeviceVulkan12Features features12,
			vk::PhysicalDeviceVulkan13Features features13
		);
		virtual ~VulkanContext();

		void init() override;
		void destroy() override;

		//void render(Core::Scene& scene);
		//void draw(Core::Scene& scene, RenderTarget* target) override;
		void draw(Core::Scene& scene, RenderTarget* target) override;
		void flush();

		VulkanCommandPool createCommandPool();
		VulkanImage createImage(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags flags);
		VulkanBuffer createBuffer(size_t allocationSize, vk::BufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage);
		VulkanMesh uploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		void immediateSubmit(const std::function<void(vk::CommandBuffer)>&, bool waitForFinish = false);

		vk::Instance& getInstance();
		vk::PhysicalDevice& getRenderingDevice();
		vk::Device& getDevice();

		vk::Queue& getGraphicsQueue();
		vk::Queue& getPresentQueue();
		std::array<uint32_t, 2> getQueueFamilies() const;
		uint32_t getQueueIndex(VulkanQueueType queue);
		vk::Queue& getQueue(VulkanQueueType queue);
		bool areQueuesSeparate() const;

	private:
		bool createInstance(bool debugging, uint32_t minVersion);
		bool createDevice(
			std::vector<const char*>& requiredDeviceExtensions,
			vk::PhysicalDeviceVulkan12Features features12,
			vk::PhysicalDeviceVulkan13Features features13
		);
		bool createMainCommandPool();

		std::queue<std::function<void()>> deletionQueue;
		std::stack<std::function<void()>> deletionStack;

		vk::Instance instance;
		vk::DispatchLoaderDynamic loader;
		vk::DebugUtilsMessengerEXT messenger;

		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::Queue transferQueue;
		uint32_t queueFamilies[2];

		VulkanCommandPool mainCmdPool;
		VulkanCommandBuffer mainCmdBuffer;
		VulkanGrowableDescriptorAllocator mainDescriptorAllocator;
		VmaAllocator allocator;

		//vk::Semaphore drawFinished;
		//VulkanImage drawImage;
		//vk::Extent2D drawExtent;

		struct {
			vk::DescriptorSet descriptorSet;
			vk::DescriptorSetLayout descriptorLayout;
			VulkanBuffer buffer;
		} sceneData;

		friend class VulkanImage;
		friend class VulkanBuffer;
		friend class VulkanCommandPool;
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorAllocator;
		friend class VulkanGrowableDescriptorAllocator;
		friend struct VulkanPipelineBuilder;
	};

	struct LUNAR_API VulkanContextBuilder
	{
	public:
		VulkanContextBuilder() = default;
		~VulkanContextBuilder() = default;

		VulkanContextBuilder& enableDebugging(bool value = true);
		VulkanContextBuilder& setMinimumVersion(uint32_t version);
		VulkanContextBuilder& setRequiredFeatures12(vk::PhysicalDeviceVulkan12Features features);
		VulkanContextBuilder& setRequiredFeatures13(vk::PhysicalDeviceVulkan13Features features);
		VulkanContextBuilder& requireDeviceExtension(const char* extension);
		std::shared_ptr<VulkanContext> create();

	private:
		bool debugging = false;
		uint32_t minimumVersion = VK_API_VERSION_1_0;
		vk::PhysicalDeviceVulkan12Features features12 = {};
		vk::PhysicalDeviceVulkan13Features features13 = {};
		std::vector<const char*> requiredDeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	};

	inline VulkanContext& getVulkanContext(std::shared_ptr<RenderContext>& context)
	{
		return *reinterpret_cast<VulkanContext*>(context.get());
	}
}
