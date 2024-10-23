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

	class LUNAR_API Window : public Identifiable, public RenderTarget
	{
	public:
		Window(
			int width,
			int height,
			bool fullscreen,
			const char* title,
			std::shared_ptr<RenderContext> context
		);

		~Window();

		void init(int width, int height, bool fullscreen, const char* title, std::shared_ptr<RenderContext>& context);
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
			vk::Semaphore imageAvailable;
			vk::Semaphore renderFinished;
			vk::Fence isInFlight;
		} _vkSwapImages[5];
		size_t _vkSwapImgCount;
		size_t _vkCurrentFrame;

		friend void Glfw_FramebufferSizeCb(GLFWwindow*, int, int);
#		endif
	};

	struct LUNAR_API WindowBuilder
	{
	public:
		WindowBuilder& setWidth(int width);
		WindowBuilder& setHeight(int height);
		WindowBuilder& setFullscreen(bool value = true);
		WindowBuilder& setTitle(const std::string_view& title);
		WindowBuilder& setRenderContext(std::shared_ptr<RenderContext>& context);
		WindowBuilder& setDefaultRenderContext();
		WindowBuilder& loadFromConfigFile(const Fs::Path& path);
		Window create();

	private:
		int w = 800, h = 600;
		bool fs = false;
		std::string_view title = "lunar";
		std::shared_ptr<RenderContext> renderContext = nullptr;
	};
}
