#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/core/scene.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <stack>
#include <array>

namespace Render
{
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
		~VulkanContext() override;

		void init() override {}
		void destroy() override {}
		void draw(Core::Scene& scene, RenderTarget* target) override {}

		vk::Instance getInstance();
		vk::PhysicalDevice getRenderingDevice();
		vk::Device getDevice();

		vk::Queue getGraphicsQueue();
		vk::Queue getPresentQueue();
		std::array<uint32_t, 2> getQueueFamilies() const;
		bool areQueuesSeparate() const;

	private:
		bool createInstance(bool debugging, uint32_t minVersion);
		bool createDevice(
			std::vector<const char*>& requiredDeviceExtensions,
			vk::PhysicalDeviceVulkan12Features features12,
			vk::PhysicalDeviceVulkan13Features features13
		);

		std::stack<std::function<void()>> deletionStack;

		vk::Instance instance;
		vk::DispatchLoaderDynamic loader;
		vk::DebugUtilsMessengerEXT messenger;

		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		uint32_t queueFamilies[2];
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
