#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vector>

#if WINDOW_BACKEND == GLFW
#   include <GLFW/glfw3.h>
#endif

namespace Vk
{
    std::vector<const char*> REQUIRED_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    bool ENABLE_DEBUG_LAYERS = VULKAN_USE_DEBUG_LAYERS;

    VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback
    (
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData
    )
    {
        if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            CDebug::Warn("[VULKAN-LAYER] {}", pCallbackData->pMessage);
        }

        if(messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            CDebug::Error("[VULKAN-LAYER] {}", pCallbackData->pMessage);
            throw std::runtime_error("Renderer-Vulkan-ValidationError");
        }

        return VK_FALSE;
    }

    bool AreValidationLayersAvailable()
    {
        unsigned layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        auto available_layers = std::vector<VkLayerProperties>(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for(const auto required_layer : REQUIRED_LAYERS)
        {
            bool layer_found = false;
            for(const auto& available_layer : available_layers)
            {
                if(strcmp(required_layer, available_layer.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if(!layer_found)
                return false;
        }

        return true;
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo()
    {
        return VkDebugUtilsMessengerCreateInfoEXT
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT     |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
            .pfnUserCallback = DebugCallback,
            .pUserData       = nullptr
        };
    }

    std::vector<VkExtensionProperties> GetAvailableInstanceExtensions()
    {
        auto vkEnumInstExtProps = (PFN_vkEnumerateInstanceExtensionProperties)
        (
            vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties")
        );

        if (vkEnumInstExtProps == VK_NULL_HANDLE)
        {
            CDebug::Error
            (
                "Cannot enumerate Vulkan instance extensions "
                "(vkGetInstanceProcAddr returned NULL for vkEnumerateInstanceExtensionProperties)."
            );

            throw std::runtime_error("Renderer-Vulkan-NotSupported");
        }

        unsigned count;
        VkResult result;

        result = vkEnumInstExtProps(nullptr, &count, nullptr);
        if (result != VK_SUCCESS)
        {
            CDebug::Error("Cannot enumerate Vulkan instance extensions (vkEnumerateInstanceExtensionProperties did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-NotSupported");
        }

        auto extensions = std::vector<VkExtensionProperties>((size_t)count);
        vkEnumInstExtProps(nullptr, &count, extensions.data());

        return extensions;
    }

    std::vector<const char*> GetRequiredExtensions()
    {
        std::vector<const char*> required;
        std::array<const char*, 7> surface_extensions =
        {
            "VK_KHR_surface",
            "VK_KHR_win32_surface",
            "VK_MVK_macos_surface",
            "VK_EXT_metal_surface",
            "VK_KHR_xlib_surface",
            "VK_KHR_xcb_surface",
            "VK_KHR_wayland_surface"
        };

        auto available = GetAvailableInstanceExtensions();
        for (auto& extension : available)
        {
            auto& name = extension.extensionName;
            for (auto& surface_extension : surface_extensions)
            {
                if (strcmp(name, surface_extension) == 0)
                    required.push_back(surface_extension);
            }
        }

        if(ENABLE_DEBUG_LAYERS)
        {
            required.push_back("VK_EXT_debug_utils");
        }

        return required;
    }

    struct InstanceLifeguard
    {
        VkInstance instance = VK_NULL_HANDLE;

        InstanceLifeguard() noexcept
        {
            auto debug_layers = (ENABLE_DEBUG_LAYERS && AreValidationLayersAvailable())
                                ? REQUIRED_LAYERS
                                : std::vector<const char*>();

            auto extensions = GetRequiredExtensions();

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
                .enabledLayerCount       = (uint32_t)debug_layers.size(),
                .ppEnabledLayerNames     = debug_layers.data(),
                .enabledExtensionCount   = (uint32_t)extensions.size(),
                .ppEnabledExtensionNames = extensions.data(),
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

        ~InstanceLifeguard() noexcept
        {
            vkDestroyInstance(instance, nullptr);
            CDebug::Log("Vulkan instance destroyed.");
        }
    };

    struct DebugMessengerLifeguard
    {
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        DebugMessengerLifeguard() noexcept
        {
            if(VULKAN_USE_DEBUG_LAYERS == 0)
                return;

            auto create_info = DebugMessengerCreateInfo();
            auto vkCreateDebugMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)
            (
                vkGetInstanceProcAddr(GetInstance(), "vkCreateDebugUtilsMessengerEXT")
            );

            if(vkCreateDebugMessenger == VK_NULL_HANDLE)
            {
                CDebug::Warn("Could not set up Vulkan debug layers (vkGetInstanceProcAddr returned VK_NULL_HANDLE).");
                return;
            }

            VkResult result;
            result = vkCreateDebugMessenger(GetInstance(), &create_info, VK_NULL_HANDLE, &debugMessenger);

            if(result != VK_SUCCESS)
            {
                CDebug::Warn("Could not set up Vulkan debug layers (vkCreateDebugUtilsMessengerEXT did not return VK_SUCCESS).");
                return;
            }

            CDebug::Log("Vulkan validation layers set up.");
        }

        ~DebugMessengerLifeguard() noexcept
        {
            if(debugMessenger != VK_NULL_HANDLE)
            {
                auto vkDestroyDebugMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)
                (
                    vkGetInstanceProcAddr(GetInstance(), "vkDestroyDebugUtilsMessengerEXT")
                );


                vkDestroyDebugMessenger(GetInstance(), debugMessenger, nullptr);

                CDebug::Log("Vulkan validation layers shut down.");
            }
        }
    };
}

Vk::InstanceLifeguard VK_INSTANCE_LIFEGUARD = {};
Vk::DebugMessengerLifeguard VK_DEBUG_MESSENGER_LIFEGUARD = {};

namespace Vk
{
    VkInstance GetInstance()
    {
        return VK_INSTANCE_LIFEGUARD.instance;
    }
}
