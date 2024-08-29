#pragma once
#include <GLFW/glfw3.h>
#include <lunar/render/render_context.hpp>
#include <lunar/render/render_target.hpp>
#include <lunar/utils/identifiable.hpp>
#include <lunar/file/config_file.hpp>
#include <lunar/api.hpp>

#ifdef LUNAR_VULKAN
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
#	ifdef LUNAR_VULKAN
	class VulkanContext;
#	endif

	struct LUNAR_API WindowBuilder
	{
	public:
		WindowBuilder() = default;
		~WindowBuilder() = default;

		WindowBuilder& width(int w);
		WindowBuilder& height(int h);
		WindowBuilder& fullscreen(bool value);
		WindowBuilder& renderContext(std::shared_ptr<RenderContext>& context);
		WindowBuilder& fromConfigFile(const Fs::Path& path);
	private:
		int _width, _height;
		bool _fullscreen;
		const char* _title;
		std::shared_ptr<RenderContext> _renderCtx;

		friend class Window;
	};

	class LUNAR_API Window : public Identifiable, public RenderTarget
	{
	public:
		Window(const WindowBuilder& builder);
		~Window();

		void init(const WindowBuilder& buidler);
		void destroy();

		void close();
		bool shouldClose() const;
		bool isMinimized() const;
		bool exists() const;

		static void pollEvents();

#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR& getVkSurface();
		vk::SwapchainKHR& getVkSwapchain();
		size_t getVkSwapImageCount();
		vk::Framebuffer& getVkSwapFramebuffer(size_t idx);
		const vk::Extent2D& getVkSwapExtent() const;

		vk::Semaphore& getVkImageAvailable(size_t idx);
		vk::Semaphore& getVkRenderFinished(size_t idx);
		vk::Fence& getVkInFlightFence(size_t idx);

		size_t getVkCurrentFrame() const;
		void endVkFrame();
#		endif
	protected:
		GLFWwindow* handle;
		std::shared_ptr<RenderContext> renderCtx;
		bool initialized;

#		ifdef LUNAR_VULKAN
		VulkanContext& _getVkContext();

		void _vkInitialize();
		void _vkInitSwap();
		void _vkDestroy();
		void _vkDestroySwap();
		void _vkUpdateSwapExtent();

		vk::SurfaceKHR _vkSurface;
		vk::SurfaceFormatKHR _vkSurfaceFmt;
		vk::PresentModeKHR _vkPresentMode;
		vk::SwapchainKHR _vkSwapchain;
		vk::Extent2D _vkSwapExtent;
		struct
		{
			vk::Image img;
			vk::ImageView view;
			vk::Framebuffer fbuffer;
			vk::Semaphore imageAvailable;
			vk::Semaphore renderFinished;
			vk::Fence isInFlight;
		} _vkSwapImages[5];
		size_t _vkSwapImgCount;
		size_t _vkCurrentFrame;

		friend void Glfw_FramebufferSizeCb(GLFWwindow*, int, int);
#		endif
	};
}
