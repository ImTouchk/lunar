#pragma once
#include <lunar/render/render_context.hpp>
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
			const VulkanSemaphoreSubmit& signalSemaphores
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
		VulkanDescriptorLayoutBuilder& addShaderStageFlag(vk::ShaderStageFlagBits flag);
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

		void render(Core::Scene& scene) override;
		void output(RenderTarget* target) override;

		//void render(Core::Scene& scene);
		//void draw(Core::Scene& scene, RenderTarget* target) override;
		void flush();

		VulkanCommandPool createCommandPool();
		VulkanImage createImage(vk::Format format, vk::Extent3D extent, vk::ImageUsageFlags flags);

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
		bool createDrawImage();
		bool createPipelines();

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
		VulkanDescriptorAllocator mainDescriptorAllocator;
		VmaAllocator allocator;

		vk::Semaphore drawFinished;
		VulkanImage drawImage;
		vk::Extent2D drawExtent;
		vk::DescriptorSet drawImageDescriptors;
		vk::DescriptorSetLayout drawImageDescriptorLayout;

		vk::Pipeline gradientPipeline;
		vk::PipelineLayout gradientPipelineLayout;

		Utils::Stopwatch stopwatch;

		friend class VulkanImage;
		friend class VulkanCommandPool;
		friend class VulkanCommandBuffer;
		friend class VulkanDescriptorAllocator;
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
