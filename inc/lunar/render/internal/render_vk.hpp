#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/core/scene.hpp>
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

	struct PipelineWrapper
	{
		PipelineWrapper() = default;
		~PipelineWrapper() = default;

		void init(VulkanContext& context);
		void destroy(VulkanContext& context);
	private:
		vk::RenderPass defaultRenderPass;
		vk::PipelineLayout defaultGraphicsLayout;

		friend class VulkanContext;
	};

	struct CmdBufferWrapper
	{
		CmdBufferWrapper() = default;
		~CmdBufferWrapper() = default;

		void init(VulkanContext& context);
		void destroy(VulkanContext& context);
	private:
		vk::CommandPool commandPool;
		vk::CommandBuffer primaryCmdBuffer;

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
		void draw(Core::Scene& scene, RenderTarget* target) override;

		vk::Instance& getInstance();
		vk::PhysicalDevice& getRenderingDevice();
		vk::Device& getDevice();

		vk::Queue& getGraphicsQueue();
		vk::Queue& getPresentQueue();
		const std::array<uint32_t, 2>& getQueueFamilies() const;
		bool areQueuesSeparate() const;

		vk::PipelineLayout& getDefaultGraphicsLayout();
		vk::RenderPass& getDefaultRenderPass();

	private:
		Vk::InstanceWrapper instanceWrapper;
		Vk::DeviceWrapper deviceWrapper;
		Vk::PipelineWrapper pipelineWrapper;
		Vk::CmdBufferWrapper cmdBufferWrapper;

		bool initialized;
	};

	inline VulkanContext& getVulkanContext(std::shared_ptr<RenderContext>& context)
	{
		return *reinterpret_cast<VulkanContext*>(context.get());
	}
}
