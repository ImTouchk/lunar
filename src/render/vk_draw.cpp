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

	void CopyImageToImage
	(
		vk::CommandBuffer cmd,
		vk::Image src,
		vk::Image dst,
		vk::Extent2D srcSize,
		vk::Extent2D dstSize
	)
	{
		auto blit_region = vk::ImageBlit2
		{
			.srcSubresource =
			{
				.aspectMask     = vk::ImageAspectFlagBits::eColor,
				.mipLevel       = 0,
				.baseArrayLayer = 0,
				.layerCount     = 1,
			},
			.dstSubresource =
			{
				.aspectMask     = vk::ImageAspectFlagBits::eColor,
				.mipLevel       = 0,
				.baseArrayLayer = 0,
				.layerCount     = 1,
			}
		};

		blit_region.srcOffsets[1] = { .x = static_cast<int>(srcSize.width), .y = static_cast<int>(srcSize.height), .z = 1 };
		blit_region.dstOffsets[1] = { .x = static_cast<int>(dstSize.width), .y = static_cast<int>(dstSize.height), .z = 1 };

		auto blit_info = vk::BlitImageInfo2
		{
			.srcImage       = src,
			.srcImageLayout = vk::ImageLayout::eTransferSrcOptimal,
			.dstImage       = dst,
			.dstImageLayout = vk::ImageLayout::eTransferDstOptimal,
			.regionCount    = 1,
			.pRegions       = &blit_region,
			.filter         = vk::Filter::eLinear,
		};

		cmd.blitImage2(blit_info);
	}

	void VulkanContext::render(Core::Scene& scene)
	{
		mainCmdBuffer.begin();

		drawExtent = {
			.width  = drawImage.extent.width,
			.height = drawImage.extent.height
		};

		TransitionImage(mainCmdBuffer, drawImage.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

		auto clear_value = vk::ClearColorValue{ {{ 1.f, 1.f, 1.f, 1.f }} };
		auto clear_range = vk::ImageSubresourceRange
		{
			.aspectMask     = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel   = 0,
			.levelCount     = vk::RemainingMipLevels,
			.baseArrayLayer = 0,
			.layerCount     = vk::RemainingArrayLayers
		};

		mainCmdBuffer->clearColorImage(drawImage.handle, vk::ImageLayout::eGeneral, &clear_value, 1, &clear_range);
		
		mainCmdBuffer->bindPipeline(vk::PipelineBindPoint::eCompute, gradientPipeline);
		mainCmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, gradientPipelineLayout, 0, drawImageDescriptors, {});

		glm::vec4 push_constants[4];
		push_constants[0] = glm::vec4(1, 0, 0, 1);
		push_constants[1] = glm::vec4(0, 0, 1, 1);

		mainCmdBuffer->pushConstants(gradientPipelineLayout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(glm::vec4) * 4, push_constants);
		mainCmdBuffer->dispatch(std::ceil(drawExtent.width / 16), std::ceil(drawExtent.height / 16), 1);

		TransitionImage(mainCmdBuffer, drawImage.handle, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);

		auto color_attachment = vk::RenderingAttachmentInfo
		{
			.imageView   = drawImage.view,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp      = vk::AttachmentLoadOp::eLoad,
			.storeOp     = vk::AttachmentStoreOp::eStore,
		};

		auto render_info = vk::RenderingInfo
		{
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = drawExtent,
			},
			.layerCount           = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments    = &color_attachment,
		};

		mainCmdBuffer->beginRendering(render_info);

		auto viewport = vk::Viewport
		{
			.x      = 0,
			.y      = 0,
			.width  = (float)drawExtent.width,
			.height = (float)drawExtent.height
		};

		auto scissor = vk::Rect2D
		{
			.offset = { 0, 0 },
			.extent = { drawExtent.width, drawExtent.height }
		};

		mainCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);
		mainCmdBuffer->setViewport(0, viewport);
		mainCmdBuffer->setScissor(0, scissor);
		mainCmdBuffer->draw(3, 1, 0, 0);


		mainCmdBuffer->endRendering();

		TransitionImage(mainCmdBuffer, drawImage.handle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);
			
		mainCmdBuffer.submit({  }, { drawFinished });
	}

	void VulkanContext::output(RenderTarget* target)
	{
		DEBUG_ASSERT(target->getType() == RenderTargetType::eWindow);
		auto& target_window = *static_cast<Render::Window*>(target);

		if (target_window.isMinimized())
			return;

		auto current_frame = target_window.getVkCurrentFrame();
		auto& cmd_buffer = target_window.getVkCommandBuffer(current_frame);
		auto& image_available = target_window.getVkImageAvailable(current_frame);
		auto& image_presentable = target_window.getVkImagePresentable(current_frame);
		auto& swapchain = target_window.getVkSwapchain();

		cmd_buffer.begin();

		uint32_t img_idx;
		std::ignore = device.acquireNextImageKHR(swapchain, UINT64_MAX, image_available, {}, &img_idx);
		
		auto& swap_image = target_window.getVkSwapImage(img_idx);

		TransitionImage(cmd_buffer, swap_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		CopyImageToImage(cmd_buffer, drawImage.handle, swap_image, drawExtent, target_window.getVkSwapExtent());
		TransitionImage(cmd_buffer, swap_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

		cmd_buffer.submit({ image_available, drawFinished }, { image_presentable });

		auto present_info = vk::PresentInfoKHR
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores    = &image_presentable,
			.swapchainCount     = 1,
			.pSwapchains        = &swapchain,
			.pImageIndices      = &img_idx,
		};

		std::ignore = presentQueue.presentKHR(present_info);

		target_window.endVkFrame();
	}
}
