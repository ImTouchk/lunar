#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <array>

namespace Render { class VulkanContext; }

namespace Render::Vk
{
	struct InstanceWrapper
	{
		InstanceWrapper() = default;
		~InstanceWrapper() = default;

		void init();
		void destroy();

	private:
		vk::Instance instance;
		vk::DispatchLoaderDynamic loader;
		vk::DebugUtilsMessengerEXT debugMessenger;

		friend class VulkanContext;
	};

	struct DeviceWrapper
	{
		DeviceWrapper() = default;
		~DeviceWrapper() = default;

		void init(VulkanContext& context);
		void destroy(VulkanContext& context);

	private:
		vk::PhysicalDevice physDevice;
		vk::Device device;
		vk::Queue presentQueue;
		vk::Queue graphicsQueue;
		std::array<uint32_t, 2> queueFamilies;

		friend class VulkanContext;
	};
}

namespace Render
{
	class LUNAR_API VulkanContext : public RenderContext
	{
	public:
		VulkanContext();
		~VulkanContext() override;
		
		void init() override;
		void destroy() override;

		vk::Instance& getInstance();
		vk::PhysicalDevice& getRenderingDevice();
		vk::Device& getDevice();

		vk::Queue& getGraphicsQueue();
		vk::Queue& getPresentQueue();
		const std::array<uint32_t, 2>& getQueueFamilies() const;
		bool areQueuesSeparate() const;

	private:
		Vk::InstanceWrapper instanceWrapper;
		Vk::DeviceWrapper deviceWrapper;
		bool initialized;
	};
}
