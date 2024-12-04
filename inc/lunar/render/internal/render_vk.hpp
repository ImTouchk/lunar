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

#ifdef LUNAR_IMGUI
#	include <imgui_impl_vulkan.h>
#endif

#include <lunar/render/internal/vk_base.hpp>
#include <lunar/render/internal/vk_pipeline.hpp>
#include <lunar/render/internal/vk_buffer.hpp>
#include <lunar/render/internal/vk_mesh.hpp>

namespace Render
{
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

#		ifdef LUNAR_IMGUI
		void _imguiInit();
#		endif
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

		VmaAllocator getAllocator();

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

#		ifdef LUNAR_IMGUI
		vk::DescriptorPool imguiPool;
#		endif

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
