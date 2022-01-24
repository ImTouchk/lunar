#if WIN32 && WINDOW_BACKEND == GLFW
#   define VK_USE_PLATFORM_WIN32_KHR
#   define GLFW_INCLUDE_VULKAN
#   define GLFW_EXPOSE_NATIVE_WIN32
#   include <GLFW/glfw3.h>
#   include <GLFW/glfw3native.h>
#endif

#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"
#include "render/window.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
    void SurfaceWrapper::create(GameWindow& window)
    {
#       if WINDOW_BACKEND == GLFW
        VkWin32SurfaceCreateInfoKHR  surface_create_info =
        {
            .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
            .hinstance = GetModuleHandleW(nullptr),
            .hwnd      = glfwGetWin32Window((GLFWwindow*)window.handle()),
        };

        VkResult result;
        result = vkCreateWin32SurfaceKHR(GetInstance(), &surface_create_info, nullptr, &surface);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Cannot create an abstract surface (vkCreateWin32SurfaceKHR did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-Surface-CreationFail");
        }

        CDebug::Log("Vulkan Renderer | Abstract surface created.");
#       else
#           error "Not implemented"
#       endif
    }

    void SurfaceWrapper::destroy()
    {
        vkDestroySurfaceKHR(GetInstance(), surface, nullptr);
    }

    VkSurfaceKHR SurfaceWrapper::handle() const
    {
        return surface;
    }
}
