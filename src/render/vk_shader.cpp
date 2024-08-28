#include <lunar/render/internal/render_vk.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/render/shader.hpp>
#include <lunar/debug.hpp>

namespace Render::Vk
{
	void PipelineWrapper::init(VulkanContext& context)
	{
		auto& device = context.getDevice();

		vk::PipelineLayoutCreateInfo layout_info = {};
		defaultGraphicsLayout = device.createPipelineLayout(layout_info);

		vk::AttachmentDescription color_attachment = {
			.format         = vk::Format::eB8G8R8A8Srgb, // TODO: take this from the window swapchain
			.samples        = vk::SampleCountFlagBits::e1,
			.loadOp         = vk::AttachmentLoadOp::eClear,
			.storeOp        = vk::AttachmentStoreOp::eStore,
			.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
			.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
			.initialLayout  = vk::ImageLayout::eUndefined,
			.finalLayout    = vk::ImageLayout::ePresentSrcKHR
		};
		
		vk::AttachmentReference color_attachment_ref = {
			.attachment = 0,
			.layout     = vk::ImageLayout::eColorAttachmentOptimal
		};

		vk::SubpassDescription subpass = {
			.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics,
			.colorAttachmentCount = 1,
			.pColorAttachments    = &color_attachment_ref
		};

		vk::SubpassDependency subpass_dep = {
			.srcSubpass    = vk::SubpassExternal,
			.dstSubpass    = 0,
			.srcStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.dstStageMask  = vk::PipelineStageFlagBits::eColorAttachmentOutput,
			.srcAccessMask = vk::AccessFlagBits::eNone,
			.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite
		};

		vk::RenderPassCreateInfo render_pass_info = {
			.attachmentCount = 1,
			.pAttachments    = &color_attachment,
			.subpassCount    = 1,
			.pSubpasses      = &subpass,
			.dependencyCount = 1,
			.pDependencies   = &subpass_dep
		};

		defaultRenderPass = device.createRenderPass(render_pass_info);
	}

	void PipelineWrapper::destroy(VulkanContext& context)
	{
		auto& device = context.getDevice();
		device.waitIdle();
		device.destroyRenderPass(defaultRenderPass);
		device.destroyPipelineLayout(defaultGraphicsLayout);
	}
}
