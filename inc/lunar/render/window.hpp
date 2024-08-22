#pragma once
#include <GLFW/glfw3.h>
#include <lunar/utils/identifiable.hpp>
#include <lunar/file/config_file.hpp>
#include <lunar/api.hpp>

#ifdef LUNAR_VULKAN
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
	class LUNAR_API Window : public Identifiable
	{
	public:
		Window(const Fs::ConfigFile& config);
		Window(const Fs::Path& path);
		~Window();

		void close();
		bool shouldClose() const;
		bool exists() const;

		static void pollEvents();

#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR& getVkSurface();
#		endif
	protected:
		GLFWwindow* handle;
#		ifdef LUNAR_VULKAN
		vk::SurfaceKHR surface;
#		endif
	};

	LUNAR_API Window& getGameWindow();
}
