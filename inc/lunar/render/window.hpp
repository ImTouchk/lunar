#pragma once
#include <GLFW/glfw3.h>
#include <lunar/render/render_context.hpp>
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

	class LUNAR_API Window : public Identifiable
	{
	public:
		Window(std::shared_ptr<RenderContext> context, const Fs::ConfigFile& config);
		~Window();

		void init(std::shared_ptr<RenderContext>& context, const Fs::ConfigFile& config);
		void destroy();

		void close();
		bool shouldClose() const;
		bool exists() const;

		static void pollEvents();

#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR& getVkSurface();
		vk::SwapchainKHR& getVkSwapchain();
#		endif
	protected:
		GLFWwindow* handle;
		std::shared_ptr<RenderContext> renderCtx;
		bool initialized;

#		ifdef LUNAR_VULKAN
		VulkanContext& _getVkContext();

		void _vkInitialize();
		void _vkDestroy();
		void _vkUpdateSwapExtent();

		vk::SurfaceKHR _vkSurface;
		vk::SurfaceFormatKHR _vkSurfaceFmt;
		vk::PresentModeKHR _vkPresentMode;
		vk::SwapchainKHR _vkSwapchain;
		vk::Extent2D _vkSwapExtent;
#		endif
	};
}
