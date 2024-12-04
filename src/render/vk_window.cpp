#define GLFW_INCLUDE_VULKAN
#include <lunar/render/internal/render_vk.hpp>
#include <vulkan/vulkan.hpp>
#include <lunar/render/window.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_vulkan.h>
#	include <imgui_impl_glfw.h>
#endif

namespace Render
{

	VulkanContext& Window::_getVkContext()
	{
		return *reinterpret_cast<VulkanContext*>(renderCtx.get());
	}

	void Window::_vkHandleResize(int width, int height)
	{
		auto& context = _getVkContext();
		_vkDestroySwap();
		_vkInitSwap();

		//auto device = context.getDevice();
		//for (size_t i = 0; i < 2; i++)
		//{
		//	device.destroyFence(_vkSwapImages[i].isInFlight);

		//	vk::FenceCreateInfo fence_info = { .flags = vk::FenceCreateFlagBits::eSignaled };
		//	_vkSwapImages[i].isInFlight = device.createFence(fence_info);
		//}
	}

	void Window::_vkInitSwap()
	{
		_vkUpdateSwapExtent();

		auto& vk_ctx = _getVkContext();
		auto phys_device = vk_ctx.getRenderingDevice();
		auto device = vk_ctx.getDevice();
		device.waitIdle();

		auto capabilities = phys_device.getSurfaceCapabilitiesKHR(_vkSurface);
		auto image_count = (capabilities.maxImageCount != 0)
			? std::clamp(capabilities.minImageCount + 1, capabilities.minImageCount, capabilities.maxImageCount)
			: capabilities.minImageCount + 1;

		vk::SwapchainCreateInfoKHR swap_info = {
			.surface          = _vkSurface,
			.minImageCount    = image_count,
			.imageFormat      = _vkSurfaceFmt.format,
			.imageColorSpace  = _vkSurfaceFmt.colorSpace,
			.imageExtent      = _vkSwapExtent,
			.imageArrayLayers = 1,
			.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst,
			.preTransform     = capabilities.currentTransform,
			.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode      = _vkPresentMode,
			.clipped          = VK_TRUE,
			.oldSwapchain     = VK_NULL_HANDLE // todo
		};

		if (vk_ctx.areQueuesSeparate())
		{
			auto indices = vk_ctx.getQueueFamilies();

			swap_info.imageSharingMode = vk::SharingMode::eConcurrent;
			swap_info.queueFamilyIndexCount = indices.size();
			swap_info.pQueueFamilyIndices = indices.data();
		}
		else
		{
			swap_info.imageSharingMode = vk::SharingMode::eExclusive;
		}

		_vkSwapchain = device.createSwapchainKHR(swap_info);

		auto images = device.getSwapchainImagesKHR(_vkSwapchain);
		if (images.size() > FRAME_OVERLAP)
		{
			DEBUG_ERROR("Too many swapchain images (max supported: 5 | existent: {})", images.size());
		}

		_vkSwapImgCount = images.size();
		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			vk::ImageViewCreateInfo view_info = {
				.image      = images[i],
				.viewType   = vk::ImageViewType::e2D,
				.format     = _vkSurfaceFmt.format,
				.components = {
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity,
					vk::ComponentSwizzle::eIdentity
				},
				.subresourceRange = {
					.aspectMask     = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel   = 0,
					.levelCount     = vk::RemainingMipLevels,
					.baseArrayLayer = 0,
					.layerCount     = vk::RemainingArrayLayers
				}
			};

			_vkFrameData[i].swapchain.image = images[i];
			_vkFrameData[i].swapchain.view = device.createImageView(view_info);
		}
	}

	void Window::_vkDestroySwap()
	{
		auto device = _getVkContext()
			.getDevice();

		device.waitIdle();
		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			device.destroyImageView(_vkFrameData[i].swapchain.view);
		}

		device.destroySwapchainKHR(_vkSwapchain);
	}

	void Window::_vkDestroy()
	{
		auto& vk_ctx = _getVkContext();
		auto device = vk_ctx.getDevice();
		auto inst = vk_ctx.getInstance();
		device.waitIdle();

		_vkDestroySwap();

		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			device.destroyDescriptorSetLayout(_vkFrameData[i].uniformBuffer.descriptorLayout);
			device.destroySemaphore(_vkFrameData[i].internal.renderFinished);
			device.destroySemaphore(_vkFrameData[i].swapchain.imageAvailable);
		}

		inst.destroySurfaceKHR(_vkSurface);
	}

	void Window::_vkInitialize()
	{
		auto& vk_ctx = _getVkContext();

		VkSurfaceKHR _surf;
		if
		(
			glfwCreateWindowSurface(static_cast<VkInstance>(vk_ctx.getInstance()), handle, nullptr, &_surf)
				!= VK_SUCCESS
		)
		{
			DEBUG_ERROR("Failed to create a window surface.");
		}

		_vkSurface = _surf;
		_vkCurrentFrame = 0;

		auto phys_device = vk_ctx.getRenderingDevice();

		auto surface_formats = phys_device.getSurfaceFormatsKHR(_vkSurface);
		// TODO: size 0 check

		bool found_optimal = false;
		for (const auto& format : surface_formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				_vkSurfaceFmt = format;
				found_optimal = true;
				break;
			}
		}

		if (!found_optimal)
			_vkSurfaceFmt = surface_formats.at(0);

		auto surface_present_modes = phys_device.getSurfacePresentModesKHR(_vkSurface);
		// TODO: size 0 check

		found_optimal = false;
		for (const auto& present_mode : surface_present_modes)
		{
			if (present_mode == vk::PresentModeKHR::eMailbox)
			{
				_vkPresentMode = present_mode;
				found_optimal = true;
				break;
			}
		}

		if (!found_optimal)
			_vkPresentMode = vk::PresentModeKHR::eImmediate;

		_vkInitSwap();

		auto device = vk_ctx.getDevice();
		

		auto sizes = std::vector<VulkanGrowableDescriptorAllocator::PoolSizeRatio>
		{
			{ vk::DescriptorType::eStorageImage, 1 }
		};

		_vkCommandPool = vk_ctx.createCommandPool();
		_vkDescriptorAlloc = VulkanGrowableDescriptorAllocator(&vk_ctx, 10, sizes);

		for (size_t i = 0; i < FRAME_OVERLAP; i++)
		{
			auto draw_img_extent = vk::Extent3D{ 1920, 1080, 1 };
			auto draw_img_format = vk::Format::eR16G16B16A16Sfloat;
			auto draw_img_usage = vk::ImageUsageFlagBits::eColorAttachment |
				vk::ImageUsageFlagBits::eStorage |
				vk::ImageUsageFlagBits::eTransferSrc |
				vk::ImageUsageFlagBits::eTransferDst;

			vk::SemaphoreCreateInfo semaphore_info = {};
			vk::FenceCreateInfo fence_info = { .flags = vk::FenceCreateFlagBits::eSignaled };

			_vkFrameData[i].internal.image = vk_ctx.createImage(draw_img_format, draw_img_extent, draw_img_usage);
			_vkFrameData[i].internal.renderFinished = device.createSemaphore(semaphore_info);
			_vkFrameData[i].swapchain.imageAvailable = device.createSemaphore(semaphore_info);
			_vkFrameData[i].commandBuffer = _vkCommandPool.allocateBuffer(vk::CommandBufferLevel::ePrimary);
			_vkFrameData[i].uniformBuffer.descriptorLayout = VulkanDescriptorLayoutBuilder()
				.useVulkanContext(vk_ctx)
				.addBinding(0, vk::DescriptorType::eUniformBuffer)
				.addShaderStageFlag(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment)
				.build();
			_vkFrameData[i].uniformBuffer.descriptorSet = _vkDescriptorAlloc.allocate(_vkFrameData[i].uniformBuffer.descriptorLayout);
			_vkFrameData[i].uniformBuffer.buffer = VulkanBufferBuilder()
				.useRenderContext(&vk_ctx)
				.setAllocationSize(sizeof(UniformBufferData))
				.setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
				.addUsageFlags(vk::BufferUsageFlagBits::eUniformBuffer)
				.build();

			VulkanDescriptorWriter()
				.writeBuffer(0, _vkFrameData[i].uniformBuffer.buffer, sizeof(UniformBufferData), 0, vk::DescriptorType::eUniformBuffer)
				.updateSet(&vk_ctx, _vkFrameData[i].uniformBuffer.descriptorSet);
		}

#		ifdef LUNAR_IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
		ImGui_ImplGlfw_InitForVulkan(handle, true);
		vk_ctx._imguiInit();
#		endif
	}

	void Window::_vkUpdateSwapExtent()
	{
		int width, height;
		glfwGetFramebufferSize(handle, &width, &height);

		auto capabilities = _getVkContext()
			.getRenderingDevice()
			.getSurfaceCapabilitiesKHR(_vkSurface);

		_vkSwapExtent = vk::Extent2D 
		{
			std::clamp(
				(uint32_t)width,
				capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width
			),
			std::clamp(
				(uint32_t)height,
				capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height
			)
		};
	}

	VulkanFrameData& Window::getVkFrameData(size_t idx)
	{
		return _vkFrameData[idx];
	}

	vk::SurfaceKHR& Window::getVkSurface()
	{
		return _vkSurface;
	}

	vk::SwapchainKHR& Window::getVkSwapchain()
	{
		return _vkSwapchain;
	}

	vk::Extent2D& Window::getVkSwapExtent()
	{
		return _vkSwapExtent;
	}

	size_t Window::getVkSwapImageCount()
	{
		return _vkSwapImgCount;
	}

	//vk::Semaphore& Window::getVkImageAvailable(size_t idx)
	//{
	//	// TODO: bounds check
	//	return _vkSwapImages[idx].imageAvailable;
	//}

	//vk::Semaphore& Window::getVkImagePresentable(size_t idx)
	//{
	//	// TODO: bounds check
	//	return _vkSwapImages[idx].imagePresentable;;
	//}

	//VulkanCommandBuffer& Window::getVkCommandBuffer(size_t idx)
	//{
	//	return _vkSwapImages[idx].cmdBuffer;
	//}

	//vk::Image& Window::getVkSwapImage(size_t idx)
	//{
	//	return _vkSwapImages[idx].img;
	//}

	const vk::Extent2D& Window::getVkSwapExtent() const
	{
		return _vkSwapExtent;
	}

	size_t Window::getVkCurrentFrame() const
	{
		return _vkCurrentFrame;
	}

	void Window::endVkFrame()
	{
		_vkCurrentFrame = (_vkCurrentFrame + 1) % FRAME_OVERLAP;
	}
}
