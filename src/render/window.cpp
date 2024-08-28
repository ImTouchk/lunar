#ifdef LUNAR_VULKAN
//#	define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#	define GLFW_INCLUDE_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#	include <vulkan/vulkan.hpp>
#endif

#include <lunar/render/window.hpp>
#include <lunar/debug.hpp>
#include <GLFW/glfw3.h>
#include <atomic>


namespace Render
{
	std::atomic<int> GlfwUsers = 0;

	Window::Window(std::shared_ptr<RenderContext> context, const Fs::ConfigFile& config)
		: handle(nullptr),
		renderCtx(context),
		initialized(false),
		RenderTarget(RenderTargetType::eWindow),
		Identifiable()
	{
		init(context, config);
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::init(std::shared_ptr<RenderContext>& context, const Fs::ConfigFile& config)
	{
		if (initialized)
			return;

		int width = config.get<int>("width");
		int height = config.get<int>("height");
		bool fullscreen = config.get<int>("fullscreen");

		if (GlfwUsers == 0 && glfwInit() == GLFW_FALSE)
			DEBUG_ERROR("Failed to initialize glfw.");
		else if (++GlfwUsers == 1)
		{
			int major, minor, rev;
			glfwGetVersion(&major, &minor, &rev);
			DEBUG_LOG("Loaded glfw version {}.{}.{}", major, minor, rev);
		}

#		ifdef LUNAR_VULKAN
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#		endif

		handle = glfwCreateWindow(
			width, height,
			"Lunar Engine Window",
			(fullscreen)
			? glfwGetPrimaryMonitor()
			: nullptr,
			nullptr
		);

		if (handle == nullptr)
			DEBUG_ERROR("Failed to create a new window.");
		else
			DEBUG_LOG("Window created successfully.");

#		ifdef LUNAR_VULKAN
		_vkInitialize();
#		endif

		initialized = true;
	}

	void Window::destroy()
	{
		if (!initialized)
			return;

#		ifdef LUNAR_VULKAN
		_vkDestroy();
#		endif	

		glfwDestroyWindow(handle);
		if (--GlfwUsers == 0)
		{
			glfwTerminate();
			DEBUG_LOG("Terminated glfw library.");
		}

		initialized = false;
	}

	void Window::close()
	{
		DEBUG_INIT_CHECK();
		glfwSetWindowShouldClose(handle, GLFW_TRUE);
	}

	bool Window::exists() const
	{
		return handle != nullptr;
	}

	bool Window::shouldClose() const
	{
		DEBUG_INIT_CHECK();
		return glfwWindowShouldClose(handle);
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

#ifdef LUNAR_VULKAN
	VulkanContext& Window::_getVkContext()
	{
		return *reinterpret_cast<VulkanContext*>(renderCtx.get());
	}

	void Window::_vkDestroy()
	{
		auto& vk_ctx = _getVkContext();
		auto& device = vk_ctx.getDevice();
		auto& inst = vk_ctx.getInstance();
		device.waitIdle();
		
		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			device.destroyFence(_vkSwapImages[i].isInFlight);
			device.destroySemaphore(_vkSwapImages[i].renderFinished);
			device.destroySemaphore(_vkSwapImages[i].imageAvailable);
			device.destroyFramebuffer(_vkSwapImages[i].fbuffer);
			device.destroyImageView(_vkSwapImages[i].view);
		}

		device.destroySwapchainKHR(_vkSwapchain);
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

		auto& phys_device = vk_ctx.getRenderingDevice();

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

		_vkUpdateSwapExtent();

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
			.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
			.preTransform     = capabilities.currentTransform,
			.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode      = _vkPresentMode,
			.clipped          = VK_TRUE,
			.oldSwapchain     = VK_NULL_HANDLE // todo
		};

		if (vk_ctx.areQueuesSeparate())
		{
			auto indices = vk_ctx.getQueueFamilies();

			swap_info.imageSharingMode      = vk::SharingMode::eConcurrent;
			swap_info.queueFamilyIndexCount = indices.size();
			swap_info.pQueueFamilyIndices   = indices.data();
		}
		else
		{
			swap_info.imageSharingMode = vk::SharingMode::eExclusive;
		}

		auto& device = vk_ctx.getDevice();
		_vkSwapchain = device.createSwapchainKHR(swap_info);
		
		auto images = device.getSwapchainImagesKHR(_vkSwapchain);
		if (images.size() > 5)
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
					.levelCount     = 1,
					.baseArrayLayer = 0,
					.layerCount     = 1
				}
			};

			_vkSwapImages[i].img = images[i];
			_vkSwapImages[i].view = device.createImageView(view_info);

			vk::FramebufferCreateInfo frame_buffer_info = {
				.renderPass      = _getVkContext().getDefaultRenderPass(),
				.attachmentCount = 1,
				.pAttachments    = &_vkSwapImages[i].view,
				.width           = _vkSwapExtent.width,
				.height          = _vkSwapExtent.height,
				.layers          = 1
			};

			_vkSwapImages[i].fbuffer = device.createFramebuffer(frame_buffer_info);

			vk::SemaphoreCreateInfo semaphore_info = {};
			vk::FenceCreateInfo fence_info = { .flags = vk::FenceCreateFlagBits::eSignaled };

			_vkSwapImages[i].renderFinished = device.createSemaphore(semaphore_info);
			_vkSwapImages[i].imageAvailable = device.createSemaphore(semaphore_info);
			_vkSwapImages[i].isInFlight = device.createFence(fence_info);
		}
	}

	void Window::_vkUpdateSwapExtent()
	{
		int width, height;
		glfwGetFramebufferSize(handle, &width, &height);

		auto capabilities = _getVkContext()
								.getRenderingDevice()
								.getSurfaceCapabilitiesKHR(_vkSurface);

		_vkSwapExtent = vk::Extent2D {
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

	vk::SurfaceKHR& Window::getVkSurface()
	{
		return _vkSurface;
	}

	vk::SwapchainKHR& Window::getVkSwapchain()
	{
		return _vkSwapchain;
	}

	size_t Window::getVkSwapImageCount()
	{
		return _vkSwapImgCount;
	}

	vk::Framebuffer& Window::getVkSwapFramebuffer(size_t idx)
	{
		// TODO: bounds check
		return _vkSwapImages[idx].fbuffer;
	}

	vk::Semaphore& Window::getVkImageAvailable(size_t idx)
	{
		// TODO: bounds check
		return _vkSwapImages[idx].imageAvailable;
	}

	vk::Semaphore& Window::getVkRenderFinished(size_t idx)
	{
		// TODO: bounds check
		return _vkSwapImages[idx].renderFinished;;
	}

	vk::Fence& Window::getVkInFlightFence(size_t idx)
	{
		return _vkSwapImages[idx].isInFlight;
	}

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
		_vkCurrentFrame = (_vkCurrentFrame + 1) % Vk::MAX_FRAMES_IN_FLIGHT;
	}
#endif
}
