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

		_vkSwapchain = vk_ctx.getDevice()
							.createSwapchainKHR(swap_info);
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
#endif
}
