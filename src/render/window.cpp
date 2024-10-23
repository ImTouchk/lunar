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
	WindowBuilder& WindowBuilder::setWidth(int width)
	{
		w = width;
		return *this;
	}

	WindowBuilder& WindowBuilder::setHeight(int height)
	{
		h = height;
		return *this;
	}

	WindowBuilder& WindowBuilder::setFullscreen(bool value)
	{
		fs = value;
		return *this;
	}

	WindowBuilder& WindowBuilder::setTitle(const std::string_view& title)
	{
		this->title = title;
		return *this;
	}

	WindowBuilder& WindowBuilder::setRenderContext(std::shared_ptr<RenderContext>& context)
	{
		renderContext = context;
		return *this;
	}

	WindowBuilder& WindowBuilder::setDefaultRenderContext()
	{
#		ifdef LUNAR_VULKAN
		vk::PhysicalDeviceVulkan12Features features12 = {};
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;

		vk::PhysicalDeviceVulkan13Features features13 = {};
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		renderContext = Render::VulkanContextBuilder()
			.setMinimumVersion(VK_VERSION_1_3)
			.setRequiredFeatures12(features12)
			.setRequiredFeatures13(features13)
			.enableDebugging(LUNAR_DEBUG_BUILD)
			.create();
#		else
		throw;
#		endif
		return *this;
	}

	Window WindowBuilder::create()
	{
		return Window(w, h, fs, title.data(), renderContext);
	}

	std::atomic<int> GlfwUsers = 0;

	inline Window& Glfw_CastUserPtr(GLFWwindow* window)
	{
		return *reinterpret_cast<Window*>(
			glfwGetWindowUserPointer(window)
		);
	}

	void Glfw_FramebufferSizeCb(GLFWwindow* handle, int width, int height)
	{
		auto& window = Glfw_CastUserPtr(handle);
#		ifdef LUNAR_VULKAN
		auto& context = window._getVkContext();
		window._vkDestroySwap();
		window._vkInitSwap();
		
		auto device = context.getDevice();
		for (size_t i = 0; i < 2; i++)
		{
			device.destroyFence(window._vkSwapImages[i].isInFlight);

			vk::FenceCreateInfo fence_info = { .flags = vk::FenceCreateFlagBits::eSignaled };
			window._vkSwapImages[i].isInFlight = device.createFence(fence_info);
		}

#		endif
	}

	Window::Window(
		int width,
		int height,
		bool fullscreen,
		const char* title,
		std::shared_ptr<RenderContext> context
	) : handle(nullptr),
		renderCtx(context),
		initialized(false),
		RenderTarget(RenderTargetType::eWindow),
		Identifiable()
	{
		init(width, height, fullscreen, title, renderCtx);
	}

	Window::~Window()
	{
		destroy();
	}

	void Window::init(int width, int height, bool fullscreen, const char* title, std::shared_ptr<RenderContext>& context)
	{
		if (initialized)
			return;

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
			title,
			(fullscreen)
				? glfwGetPrimaryMonitor() // TODO: monitor selection
				: nullptr,
			nullptr
		);

		if (handle == nullptr)
			DEBUG_ERROR("Failed to create a new window.");
		else
			DEBUG_LOG("Window created successfully.");

		glfwSetWindowUserPointer(handle, this);
		glfwSetFramebufferSizeCallback(handle, Glfw_FramebufferSizeCb);
		
		initialized = true;

#		ifdef LUNAR_VULKAN
		_vkInitialize();
#		endif
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
		handle = nullptr;
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

	bool Window::isMinimized() const
	{
		DEBUG_INIT_CHECK();
		int w, h;
		glfwGetFramebufferSize(handle, &w, &h);
		return w == 0 || h == 0;
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

	void Window::_vkInitSwap()
	{
		_vkUpdateSwapExtent();

		auto& vk_ctx = _getVkContext();
		auto phys_device = vk_ctx.getRenderingDevice();
		auto device = vk_ctx.getDevice();

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
		if (images.size() > 5)
		{
			DEBUG_ERROR("Too many swapchain images (max supported: 5 | existent: {})", images.size());
		}

		_vkSwapImgCount = images.size();
		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			vk::ImageViewCreateInfo view_info = {
				.image    = images[i],
				.viewType = vk::ImageViewType::e2D,
				.format   = _vkSurfaceFmt.format,
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
		}
	}

	void Window::_vkDestroySwap()
	{
		auto device = _getVkContext()
							.getDevice();

		device.waitIdle();
		for (size_t i = 0; i < _vkSwapImgCount; i++)
		{
			device.destroyImageView(_vkSwapImages[i].view);
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
		
		constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			device.destroyFence(_vkSwapImages[i].isInFlight);
			device.destroySemaphore(_vkSwapImages[i].renderFinished);
			device.destroySemaphore(_vkSwapImages[i].imageAvailable);
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
		constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
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
		_vkCurrentFrame = (_vkCurrentFrame + 1) % 2;
	}
#endif
}
