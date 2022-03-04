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
		assert(createInfo.pDevice != nullptr);
		assert(createInfo.pSurface != nullptr);
		assert(createInfo.pSwapchain != nullptr);
		assert(createInfo.pSyncObjects != nullptr);
		assert(createInfo.pObjectManager != nullptr);

		pDevice = createInfo.pDevice;
		pSurface = createInfo.pSurface;
		pSwapchain = createInfo.pSwapchain;
		pSyncObjects = createInfo.pSyncObjects;
		pObjectManager = createInfo.pObjectManager;

		auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

		VkCommandPoolCreateInfo pool_create_info =
		{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queue_indices.graphics.value()
		};

		VkResult result;
		result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &cmd_pool);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Render call manager creation fail (vkCreateCommandPool didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-RenderCallManager-CreationFail");
		}

		cmd_buffers.resize(pSwapchain->frame_buffers().size(), VK_NULL_HANDLE);

		VkCommandBufferAllocateInfo buffer_allocate_info =
		{
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool        = cmd_pool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(cmd_buffers.size())
		};

		result = vkAllocateCommandBuffers(pDevice->handle(), &buffer_allocate_info, cmd_buffers.data());
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Render call manager creation fail (vkAllocateCommandBuffers didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-RenderCallManager-CreationFail");
		}

		active = true;
		
		CDebug::Log("Vulkan Renderer | Render call manager created.");
	}

	void RenderCallManager::destroy()
	{
		assert(active == true);

		vkFreeCommandBuffers(pDevice->handle(), cmd_pool, static_cast<uint32_t>(cmd_buffers.size()), cmd_buffers.data());
		vkDestroyCommandPool(pDevice->handle(), cmd_pool, nullptr);

		pDevice = nullptr;
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
		assert(active == true);

		if (!pObjectManager->cmd_buffers_need_rebuilding())
		{
			return;
		}

		auto& mesh_commands = pObjectManager->mesh_commands();

		for (int i = 0; i < cmd_buffers.size(); i++)
		{
			auto& command_buffer = cmd_buffers[i];
			vkResetCommandBuffer(command_buffer, 0);
			
			VkCommandBufferBeginInfo buffer_begin_info =
			{
				.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags            = 0,
				.pInheritanceInfo = nullptr
			};

			VkResult result;
			result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
			if (result != VK_SUCCESS)
			{
				CDebug::Error("Vulkan Renderer | Failed to prepare command buffers for frame {} (vkBeginCommandBuffer didn't return VK_SUCCESS).", i);
				continue;
			}

			VkClearValue clear_values[2] =
			{
				{.color        = { 0.f, 0.f, 0.f, 1.f } },
				{.depthStencil = { 1.f, 0 } }
			};

			VkRenderPassBeginInfo render_pass_begin_info =
			{
				.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
				.renderPass  = pSwapchain->render_pass(),
				.framebuffer = pSwapchain->frame_buffers()[i],
				.renderArea  =
				{
					.offset = { 0, 0 },
					.extent = pSwapchain->surface_extent()
				},
				.clearValueCount = 2,
				.pClearValues    = clear_values,
			};

			vkCmdBeginRenderPass(command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			if (mesh_commands.size() != 0)
			{
				vkCmdExecuteCommands(command_buffer, static_cast<uint32_t>(mesh_commands.size()), mesh_commands.data());
			}

			vkCmdEndRenderPass(command_buffer);

			result = vkEndCommandBuffer(command_buffer);
			if (result != VK_SUCCESS)
			{
				CDebug::Error("Vulkan Renderer | Failed to prepare command buffers for frame {} (vkEndCommandBuffer didn't return VK_SUCCESS).", i);
				continue;
			}
		}
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

		VkCommandBuffer& buffer = cmd_buffers[image];

		VkSubmitInfo submit_info =
		{
			.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount   = 1,
			.pWaitSemaphores      = wait_semaphores,
			.pWaitDstStageMask    = wait_stages,
			.commandBufferCount   = 1,
			.pCommandBuffers      = &buffer,
			.signalSemaphoreCount = 1,
			.pSignalSemaphores    = signal_semaphores
		};

		vkResetFences(pDevice->handle(), 1, &pSyncObjects->in_flight_fence(current_frame));

		VkResult result;
		result = vkQueueSubmit(pDevice->graphics_queue(), 1, &submit_info, pSyncObjects->in_flight_fence(current_frame));
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Failed to render a frame (vkQueueSubmit didn't return VK_SUCCESS).");
			return;
		}

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

		vkQueuePresentKHR(pDevice->present_queue(), &present_info);

		current_frame = (current_frame + 1) % Vk::MAX_FRAMES_IN_FLIGHT;
	}
}
