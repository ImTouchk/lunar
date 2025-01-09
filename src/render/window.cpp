#ifdef LUNAR_VULKAN
//#	define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#	define GLFW_INCLUDE_VULKAN
#	include <lunar/render/internal/render_vk.hpp>
#	include <vulkan/vulkan.hpp>
#endif

#ifdef LUNAR_OPENGL
#	include <glad/gl.h>
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

	WindowBuilder& WindowBuilder::setSize(int width, int height)
	{
		w = width;
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
		renderContext = CreateDefaultContext();
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
		window._vkHandleResize(width, height);
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

#		ifdef LUNAR_OPENGL
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
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

#		ifdef LUNAR_OPENGL
		_glInitialize();
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
}
