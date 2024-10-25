#include <lunar/render/internal/render_vk.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

namespace Render
{
	void TransitionImage
	(
		vk::CommandBuffer cmd, 
		vk::Image image, 
		vk::ImageLayout currentLayout, 
		vk::ImageLayout newLayout
	)
	{
		vk::ImageAspectFlags aspect_mask = (newLayout == vk::ImageLayout::eDepthAttachmentOptimal)
												? vk::ImageAspectFlagBits::eDepth
												: vk::ImageAspectFlagBits::eColor;

		auto image_barrier = vk::ImageMemoryBarrier2
		{
			.srcStageMask     = vk::PipelineStageFlagBits2::eAllCommands,
			.srcAccessMask    = vk::AccessFlagBits2::eMemoryWrite,
			.dstStageMask     = vk::PipelineStageFlagBits2::eAllCommands,
			.dstAccessMask    = vk::AccessFlagBits2::eMemoryWrite | vk::AccessFlagBits2::eMemoryRead,
			.oldLayout        = currentLayout,
			.newLayout        = newLayout,
			.image            = image,
			.subresourceRange = 
			{ 
				.aspectMask     = aspect_mask,
				.baseMipLevel   = 0,
				.levelCount     = vk::RemainingMipLevels,
				.baseArrayLayer = 0,
				.layerCount     = vk::RemainingArrayLayers
			}
		};

		auto dep_info = vk::DependencyInfo
		{
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers    = &image_barrier
		};

		cmd.pipelineBarrier2(dep_info);
	}

	void VulkanContext::draw(Core::Scene& scene, RenderTarget* target)
	{
		stopwatch.reset();

		DEBUG_ASSERT(target->getType() == RenderTargetType::eWindow);
		auto& target_window = *static_cast<Render::Window*>(target);

		if (target_window.isMinimized())
			return;

		auto device = getDevice();
		auto current_frame = target_window.getVkCurrentFrame();
		auto& cmd_buffer = target_window.getVkCommandBuffer(current_frame);
		auto& image_available = target_window.getVkImageAvailable(current_frame);
		auto& render_finished = target_window.getVkRenderFinished(current_frame);
		auto& in_flight = target_window.getVkInFlightFence(current_frame);
		auto& swapchain = target_window.getVkSwapchain();
		//auto queue_idx = getQueueIndex(VulkanQueueType::eTransfer);

		vk::Result result;
		result = device.waitForFences(in_flight, VK_TRUE, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			DEBUG_ERROR("{}", static_cast<uint32_t>(result));
			return;
		}

		device.resetFences(in_flight);

		uint32_t img_idx;
		result = device.acquireNextImageKHR(swapchain, UINT64_MAX, image_available, {}, &img_idx);
		if (result != vk::Result::eSuccess)
		{
			DEBUG_ERROR("{}", static_cast<uint32_t>(result));
			return;
		}

		auto& swap_image = target_window.getVkSwapImage(img_idx);

		auto cmd_begin_info = vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };
		cmd_buffer.reset();
		cmd_buffer.begin(cmd_begin_info);

		TransitionImage(cmd_buffer, swap_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

		auto clear_value = vk::ClearColorValue{ {{ 1.f, 1.f, 1.f, 1.f }} };
		auto clear_range = vk::ImageSubresourceRange
		{
			.aspectMask     = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel   = 0,
			.levelCount     = vk::RemainingMipLevels,
			.baseArrayLayer = 0,
			.layerCount     = vk::RemainingArrayLayers
		};

		cmd_buffer.clearColorImage(swap_image, vk::ImageLayout::eGeneral, &clear_value, 1, &clear_range);

		TransitionImage(cmd_buffer, swap_image, vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);

		cmd_buffer.end();

		auto cmd_submit_info = vk::CommandBufferSubmitInfo
		{
			.commandBuffer = cmd_buffer,
			.deviceMask    = 0
		};

		auto wait_semaphore = vk::SemaphoreSubmitInfo
		{
			.semaphore   = image_available,
			.value       = 1,
			.stageMask   = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			.deviceIndex = 0,
		};

		auto signal_semaphore = vk::SemaphoreSubmitInfo
		{
			.semaphore   = render_finished,
			.value       = 1,
			.stageMask   = vk::PipelineStageFlagBits2::eAllCommands,
			.deviceIndex = 0,
		};

		auto submit_info = vk::SubmitInfo2
		{
			.waitSemaphoreInfoCount   = 1,
			.pWaitSemaphoreInfos      = &wait_semaphore,
			.commandBufferInfoCount   = 1,
			.pCommandBufferInfos      = &cmd_submit_info,
			.signalSemaphoreInfoCount = 1,
			.pSignalSemaphoreInfos    = &signal_semaphore,
		};

		graphicsQueue.submit2(submit_info, in_flight, loader);

		auto present_info = vk::PresentInfoKHR
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores    = &render_finished,
			.swapchainCount     = 1,
			.pSwapchains        = &swapchain,
			.pImageIndices      = &img_idx,
		};

		presentQueue.presentKHR(present_info, loader);

		target_window.endVkFrame();

		// TODO: improve this code
		flush();
	}

	//namespace Vk
	//{
	//	void TransitionImage(vk::CommandBuffer cmd, vk::Image image, vk::ImageLayout current, vk::ImageLayout newLayout)
	//	{

	//	}

	//}

	//void VulkanContext::draw(Core::Scene& scene, RenderTarget* target)
	//{
	//	// TODO: make Core::Scene const
	//	// TODO: implement for RenderTargetType::eTexture
	//	// TODO: check for initialization

	//	auto& target_window = *reinterpret_cast<Render::Window*>(target);
	//	if (target_window.isMinimized())
	//	{
	//		return;
	//	}

	//	auto current_frame = target_window.getVkCurrentFrame();
	//	auto& img_available = target_window.getVkImageAvailable(current_frame);
	//	auto& render_finished = target_window.getVkRenderFinished(current_frame);
	//	auto& in_flight = target_window.getVkInFlightFence(current_frame);
	//	auto& cmd_buffer = cmdBufferWrapper.renderCmdBuffers[current_frame];

	//	auto& swapchain = target_window.getVkSwapchain();
	//	auto& swap_extent = target_window.getVkSwapExtent();

	//	auto& device = getDevice();

	//	/* 
	//		TODO: When rendering on multiple windows, weird sync issues
	//		happen due to the reuse of only MAX_FRAMES_IN_FLIGHT (2) command buffers
	//		for M windows with N frames in flight each
	//		Need a better strategy for rendering
	//	*/
	//	//device.waitForFences(in_flight, VK_TRUE, UINT64_MAX);


	//	//device.resetFences(in_flight);

	//	//uint32_t img_idx;
	//	//device.acquireNextImageKHR(swapchain, UINT64_MAX, img_available, {}, &img_idx);

	//	//vk::CommandBufferBeginInfo begin_info = {};

	//	//cmd_buffer.reset();
	//	//cmd_buffer.begin(begin_info);

	//	//vk::ClearValue clear_value = { .color = { {{ 1.f, 1.f, 1.f, 1.f }} } };
	//	//vk::RenderPassBeginInfo render_pass_begin_info = {
	//	//	.renderPass  = getDefaultRenderPass(),
	//	//	.framebuffer = target_window.getVkSwapFramebuffer(img_idx),
	//	//	.renderArea = {
	//	//		.offset = { 0, 0 },
	//	//		.extent = swap_extent
	//	//	},
	//	//	.clearValueCount = 1,
	//	//	.pClearValues    = &clear_value
	//	//};

	//	//vk::Viewport viewport = {
	//	//	.x        = 0.f,
	//	//	.y        = 0.f,
	//	//	.width    = static_cast<float>(swap_extent.width),
	//	//	.height   = static_cast<float>(swap_extent.height),
	//	//	.minDepth = 0.f,
	//	//	.maxDepth = 1.f
	//	//};

	//	//vk::Rect2D scissor = {
	//	//	.offset = { 0, 0 },
	//	//	.extent = swap_extent
	//	//};

	//	//cmd_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
	//	//cmd_buffer.setViewport(0, viewport);
	//	//cmd_buffer.setScissor(0, scissor);

	//	//for (auto& gameObject : scene.getGameObjects())
	//	//{
	//	//	auto* mesh_renderer = gameObject.getComponent<MeshRenderer>();
	//	//	
	//	//	if (mesh_renderer == nullptr)
	//	//		continue;

	//	//	auto& shader = mesh_renderer->getShader();
	//	//	cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, shader.getVkPipeline());
	//	//	cmd_buffer.draw(3, 1, 0, 0);
	//	//}

	//	//// draw call

	//	//cmd_buffer.endRenderPass();
	//	//cmd_buffer.end();
	//
	//	//vk::PipelineStageFlags wait_stages[] = {
	//	//	vk::PipelineStageFlagBits::eColorAttachmentOutput
	//	//};

	//	//vk::SubmitInfo submit_info = {
	//	//	.waitSemaphoreCount   = 1,
	//	//	.pWaitSemaphores      = &img_available,
	//	//	.pWaitDstStageMask    = wait_stages,
	//	//	.commandBufferCount   = 1,
	//	//	.pCommandBuffers      = &cmd_buffer,
	//	//	.signalSemaphoreCount = 1,
	//	//	.pSignalSemaphores    = &render_finished
	//	//};

	//	//getGraphicsQueue()
	//	//	.submit(submit_info, in_flight);

	//	//vk::PresentInfoKHR present_info = {
	//	//	.waitSemaphoreCount = 1,
	//	//	.pWaitSemaphores    = &render_finished,
	//	//	.swapchainCount     = 1,
	//	//	.pSwapchains        = &swapchain,
	//	//	.pImageIndices      = &img_idx
	//	//};

	//	//getPresentQueue()
	//	//	.presentKHR(present_info);

	//	//target_window.endVkFrame();
	//}
}
