#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/utils/stopwatch.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <queue>
#include <stack>
#include <array>

namespace Render
{
	class VulkanContext;

	enum class LUNAR_API VulkanQueueType : size_t
	{
		eGraphics = 0,
		ePresent = 1,
		eTransfer = 2
	};

	class LUNAR_API VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanContext& context, vk::CommandBuffer buffer, vk::Fence fence);
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

		void begin();

		void submit(
			const std::initializer_list<vk::SemaphoreSubmitInfo>& waitSubmitInfos,
			const std::initializer_list<vk::SemaphoreSubmitInfo>& signalSubmitInfos
		);

		void submit(
			const std::initializer_list<vk::Semaphore>& waitSemaphores,
			const std::initializer_list<vk::Semaphore>& signalSemaphores
		);

		void destroy();

		VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept;

		[[nodiscard]] operator vk::CommandBuffer& ();

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
		VmaAllocator allocator;

		vk::Semaphore drawFinished;
		VulkanImage drawImage;
		vk::Extent2D drawExtent;

		Utils::Stopwatch stopwatch;

		friend class VulkanImage;
		friend class VulkanCommandPool;
		friend class VulkanCommandBuffer;
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
