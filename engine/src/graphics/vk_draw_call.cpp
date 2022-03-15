#include "utils/debug.hpp"
#include "vk_draw_call.hpp"
#include "vk_renderer.hpp"
#include "vk_object.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
	void RenderCallManager::create(RenderCallManagerCreateInfo&& createInfo)
	{
		assert(not active);
		assert(createInfo.pSurface != nullptr);
		assert(createInfo.pSwapchain != nullptr);
		assert(createInfo.pSyncObjects != nullptr);
		assert(createInfo.pObjectManager != nullptr);

		pSurface = createInfo.pSurface;
		pSwapchain = createInfo.pSwapchain;
		pSyncObjects = createInfo.pSyncObjects;
		pObjectManager = createInfo.pObjectManager;

		active = true;
	}

	void RenderCallManager::destroy()
	{
		assert(active == true);

		pSurface = nullptr;
		pSwapchain = nullptr;
		pSyncObjects = nullptr;
		pObjectManager = nullptr;

		active = false;

		CDebug::Log("Vulkan Renderer | Render call manager destroyed.");
	}

	size_t RenderCallManager::current_image() const
	{
		return current_frame;
	}

	void RenderCallManager::update()
	{
	}

	void RenderCallManager::draw(uint32_t image)
	{
		assert(active == true);

		pSyncObjects->image_in_flight(image) = pSyncObjects->in_flight_fence(current_frame);

		VkSemaphore wait_semaphores[] = 
		{
			pSyncObjects->image_available(current_frame)
		};

		VkPipelineStageFlags wait_stages[] =
		{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		};

		VkSemaphore signal_semaphores[] =
		{
			pSyncObjects->rendering_finished(current_frame)
		};

		VkSubmitInfo submit_info =
		{
			.waitSemaphoreCount   = 1,
			.pWaitSemaphores      = wait_semaphores,
			.pWaitDstStageMask    = wait_stages,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores    = signal_semaphores
		};

		vkResetFences(GetDevice().handle, 1, &pSyncObjects->in_flight_fence(current_frame));

		auto frame = current_frame;
		auto swapchain = pSwapchain;
		auto mesh_buffers = pObjectManager->mesh_commands();

		CommandSubmitter::SubmitAsync([mesh_buffers, swapchain, frame](VkCommandBuffer& buffer)
		{
			VkClearValue clear_values[2] =
			{
				{.color = { 0.f, 0.f, 0.f, 1.f } },
				{.depthStencil = { 1.f, 0 } }
			};

			VkRenderPassBeginInfo render_pass_begin_info =
			{
				.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass  = swapchain->render_pass(),
				.framebuffer = swapchain->frame_buffers()[frame],
				.renderArea  =
				{
					.offset = { 0, 0 },
					.extent = swapchain->surface_extent()
				},
				.clearValueCount = 2,
				.pClearValues    = clear_values,
			};

			vkCmdBeginRenderPass(buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			if (mesh_buffers.size() != 0)
			{
				vkCmdExecuteCommands(buffer, static_cast<uint32_t>(mesh_buffers.size()), mesh_buffers.data());
			}
			vkCmdEndRenderPass(buffer);
		}, submit_info).wait();

		VkSwapchainKHR swapchains[] =
		{
			pSwapchain->handle()
		};

		VkPresentInfoKHR present_info =
		{
			.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1,
			.pWaitSemaphores    = signal_semaphores,
			.swapchainCount     = 1,
			.pSwapchains        = swapchains,
			.pImageIndices      = &image,
			.pResults           = nullptr
		};

		vkQueuePresentKHR(GetDevice().present, &present_info);

		current_frame = (current_frame + 1) % Vk::MAX_FRAMES_IN_FLIGHT;
	}
}
