#include "utils/debug.hpp"

#include <vulkan/vulkan.h>

#if WINDOW_BACKEND == GLFW
#   include <GLFW/glfw3.h>
#endif

struct VkInstanceLifeguard
{
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

    VkInstanceLifeguard() noexcept
    {
        unsigned extension_count = 0;
        const char** ppExtensions = nullptr;

#   if WINDOW_BACKEND == GLFW
        ppExtensions = glfwGetRequiredInstanceExtensions(&extension_count);
#   endif

        VkApplicationInfo application_info =
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "Lunar-App",
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName        = "Lunar",
            .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion         = VK_API_VERSION_1_2
        };

        VkInstanceCreateInfo instance_create_info =
        {
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo        = &application_info,
            .enabledLayerCount       = 0,
            .ppEnabledLayerNames     = nullptr,
            .enabledExtensionCount   = extension_count,
            .ppEnabledExtensionNames = ppExtensions,
        };

        VkResult result;
        result = vkCreateInstance(&instance_create_info, nullptr, &instance);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Failed to create a Vulkan instance (vkCreateInstance returned {}).", result);
            std::exit(-1);
        }

        CDebug::Log("Global vulkan instance created.");
    }

    ~VkInstanceLifeguard() noexcept
    {

    }
};

VkInstanceLifeguard VK_INSTANCE_LIFEGUARD = {};