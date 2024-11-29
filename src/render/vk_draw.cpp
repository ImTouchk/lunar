#include <lunar/render/internal/render_vk.hpp>
#include <lunar/render/render_components.hpp>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug.hpp>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

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

	void VulkanContext::draw(Core::Scene& scene, RenderTarget* target)
	{
		if(target->getType() != RenderTargetType::eWindow)
			return; // TODO

		auto& target_window = *static_cast<Render::Window*>(target);
		if(target_window.isMinimized())
			return;

		auto current_frame = target_window.getVkCurrentFrame();
		auto& frame_data = target_window.getVkFrameData(current_frame);
		auto& swapchain = target_window.getVkSwapchain();
		auto& command_buffer = frame_data.commandBuffer;

		command_buffer.begin();

#		ifdef LUNAR_IMGUI
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
#		endif

		uint32_t img_idx;
		std::ignore = device.acquireNextImageKHR(swapchain, UINT64_MAX, frame_data.swapchain.imageAvailable, {}, &img_idx);
		
		auto& swap_image = frame_data.swapchain.image;
		auto& draw_image = frame_data.internal;

		draw_image.extent = {
			.width = draw_image.image.extent.width,
			.height = draw_image.image.extent.height
		};

		TransitionImage(command_buffer, draw_image.image.handle, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral);

		auto clear_value = vk::ClearColorValue{ {{ 1.f, 1.f, 1.f, 1.f }} };
		auto clear_range = vk::ImageSubresourceRange
		{
			.aspectMask     = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel   = 0,
			.levelCount     = vk::RemainingMipLevels,
			.baseArrayLayer = 0,
			.layerCount     = vk::RemainingArrayLayers
		};

		command_buffer->clearColorImage(draw_image.image.handle, vk::ImageLayout::eGeneral, &clear_value, 1, &clear_range);

		TransitionImage(command_buffer, draw_image.image.handle, vk::ImageLayout::eGeneral, vk::ImageLayout::eColorAttachmentOptimal);

		auto color_attachment = vk::RenderingAttachmentInfo
		{
			.imageView   = draw_image.image.view,
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp      = vk::AttachmentLoadOp::eLoad,
			.storeOp     = vk::AttachmentStoreOp::eStore,
		};

		auto render_info = vk::RenderingInfo
		{
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = draw_image.extent,
			},
			.layerCount           = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments    = &color_attachment,
		};

		command_buffer->beginRendering(render_info);

		auto viewport = vk::Viewport
		{
			.x      = 0,
			.y      = 0,
			.width  = (float)draw_image.extent.width,
			.height = (float)draw_image.extent.height
		};

		auto scissor = vk::Rect2D
		{
			.offset = { 0, 0 },
			.extent = { draw_image.extent.width, draw_image.extent.height }
		};

		command_buffer->setViewport(0, viewport);
		command_buffer->setScissor(0, scissor);

		ImGui::ShowDemoWindow();
		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.operator vk::CommandBuffer &());
		
		//mainCmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, trianglePipeline);
		//mainCmdBuffer->draw(3, 1, 0, 0);

		command_buffer->endRendering();

		TransitionImage(command_buffer, draw_image.image.handle, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

		TransitionImage(command_buffer, swap_image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		CopyImageToImage(command_buffer, draw_image.image.handle, swap_image, draw_image.extent, target_window.getVkSwapExtent());
		TransitionImage(command_buffer, swap_image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);

		command_buffer.submit({ frame_data.swapchain.imageAvailable }, { frame_data.internal.renderFinished });

		auto present_info = vk::PresentInfoKHR
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores    = &frame_data.internal.renderFinished,
			.swapchainCount     = 1,
			.pSwapchains        = &swapchain,
			.pImageIndices      = &img_idx,
		};

		std::ignore = presentQueue.presentKHR(present_info);

		target_window.endVkFrame();
	}
}
