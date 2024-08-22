#define GLFW_INCLUDE_VULKAN
#include <lunar/render/window.hpp>
#include <lunar/debug/log.hpp>
#include <GLFW/glfw3.h>
#include <atomic>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
	std::atomic<int> GlfwUsers = 0;

	Window& getGameWindow()
	{
		static auto game_window = Window(Fs::baseDirectory().append("window.cfg"));
		return game_window;
	}

	Window::Window(const Fs::Path& path)
		: Window(Fs::ConfigFile(path))
	{
	}

	Window::Window(const Fs::ConfigFile& config)
		: handle(nullptr),
		Identifiable()
	{
		int width = config.get<int>("width");
		int height = config.get<int>("height");
		bool fullscreen = config.get<int>("fullscreen");

		if (GlfwUsers == 0 && glfwInit() == GLFW_FALSE)
			DEBUG_ERROR("Failed to initialize glfw.");
		else if(++GlfwUsers == 1)
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
		VkSurfaceKHR _surf;
		if (
			glfwCreateWindowSurface(static_cast<VkInstance>(Vk::getInstance()), handle, nullptr, &_surf) 
			!= VK_SUCCESS
		)
		{
			DEBUG_ERROR("Failed to create a window surface.");
		}

		surface = _surf;
#		endif
	}

	Window::~Window()
	{
#		ifdef LUNAR_VULKAN
		Vk::getInstance().destroySurfaceKHR(surface);
#		endif	

		glfwDestroyWindow(handle);
		if (--GlfwUsers == 0)
		{
			glfwTerminate();
			DEBUG_LOG("Terminated glfw library.");
		}
	}

	void Window::close()
	{
		glfwDestroyWindow(handle);
		handle = nullptr;
	}

	bool Window::exists() const
	{
		return handle != nullptr;
	}

	bool Window::shouldClose() const
	{
		return glfwWindowShouldClose(handle);
	}

	void Window::pollEvents()
	{
		glfwPollEvents();
	}

#ifdef LUNAR_VULKAN
	vk::SurfaceKHR& Window::getVkSurface()
	{
		return surface;
	}
#endif
}
