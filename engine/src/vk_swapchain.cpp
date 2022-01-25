#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"
#include "render/window.hpp"

#include <vulkan/vulkan.h>
#include <cassert>

namespace Vk
{
    const SwapchainSupportDetails& SwapchainSupportDetails::query(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        static VkPhysicalDevice cached_device = VK_NULL_HANDLE;
        static SwapchainSupportDetails cached_value = {};

        if(cached_device == device)
        {
            return cached_value;
        }

        cached_device = device;
        cached_value = {};

        auto query_surface_formats = [&]()
        {
            unsigned format_count;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
            if(format_count != 0)
            {
                cached_value.surfaceFormats.resize(format_count);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, cached_value.surfaceFormats.data());
            }
        };

        auto query_present_modes = [&]()
        {
            unsigned present_modes;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes, nullptr);
            if(present_modes != 0)
            {
                cached_value.surfacePresentModes.resize(present_modes);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes, cached_value.surfacePresentModes.data());
            }
        };

        query_surface_formats();
        query_present_modes();

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &cached_value.surfaceCapabilities);
        return cached_value;
    }

    VkSurfaceFormatKHR PickSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        if(formats.empty())
        {
            CDebug::Error("Vulkan Renderer | No surface formats are available on the selected graphics device (formats.empty() == true).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-NoFormatsAvailable");
        }

        for(const auto& format : formats)
        {
            auto desired_format = VK_FORMAT_B8G8R8A8_SRGB;
            auto desired_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

            if(format.colorSpace == desired_color_space && format.format == desired_format)
            {
                return format;
            }
        }

        return formats.at(0);
    }

    VkPresentModeKHR PickPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        if(presentModes.empty())
        {
            CDebug::Error("Vulkan Renderer | No surface present modes are available on the selected graphics device (presentModes.empty() == true).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-NoPresentModesAvailable");
        }

        for(const auto& mode : presentModes)
        {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }

        return VK_PRESENT_MODE_IMMEDIATE_KHR; // available everywhere (makes the empty check redundant?)
    }

    VkExtent2D PickSwapExtent(GameWindow& window, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if(capabilities.currentExtent.width != UINT32_MAX && capabilities.currentExtent.height != 0)
        {
            return capabilities.currentExtent;
        }

        VkExtent2D extent =
        {
            .width  = std::max(capabilities.minImageExtent.width, (unsigned)window.get_width()),
            .height = std::max(capabilities.minImageExtent.height, (unsigned)window.get_height())
        };

        return extent;
    }

    void SwapchainWrapper::create(GameWindow& window, SurfaceWrapper& surface, LogicalDeviceWrapper& device)
    {
        assert(swapchain == VK_NULL_HANDLE);

        pDevice = &device;

        create_swapchain(window, surface);
        create_image_views();
        create_render_pass();
        CDebug::Log("Vulkan Renderer | Swapchain created.");
    }

    void SwapchainWrapper::destroy()
    {
        assert(swapchain != VK_NULL_HANDLE);

        auto device = pDevice->handle();
        vkDeviceWaitIdle(device);
        vkDestroyRenderPass(device, renderPass, nullptr);

        for(auto i : range(0, views.size() - 1))
        {
            vkDestroyImageView(device, views[i], nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);

        swapchain = VK_NULL_HANDLE;
        frameBuffers.clear();
        images.clear();
        views.clear();
        surfaceFormat = {};
        surfaceExtent = {};
    }

    void SwapchainWrapper::resize(GameWindow& window, SurfaceWrapper& surface)
    {
        destroy();
        create(window, surface, *pDevice);
        CDebug::Log("Vulkan Renderer | Swapchain resized.");
    }

    void SwapchainWrapper::create_swapchain(GameWindow& window, SurfaceWrapper& surface)
    {
        auto render_device = GetRenderingDevice();
        auto swapchain_details = SwapchainSupportDetails::query(render_device, surface.handle());
        auto surface_format = PickSurfaceFormat(swapchain_details.surfaceFormats);
        auto surface_present_mode = PickPresentMode(swapchain_details.surfacePresentModes);
        auto swapchain_extent = PickSwapExtent(window, swapchain_details.surfaceCapabilities);

        auto& surface_capabilities = swapchain_details.surfaceCapabilities;

        unsigned image_count = surface_capabilities.minImageCount + 1;
        if(surface_capabilities.minImageCount > 0 && image_count > surface_capabilities.maxImageCount)
        {
            image_count = surface_capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapchain_create_info =
        {
            .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface          = surface.handle(),
            .minImageCount    = image_count,
            .imageFormat      = surface_format.format,
            .imageColorSpace  = surface_format.colorSpace,
            .imageExtent      = swapchain_extent,
            .imageArrayLayers = 1,
            .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform     = swapchain_details.surfaceCapabilities.currentTransform,
            .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode      = surface_present_mode,
            .clipped          = VK_TRUE,
            .oldSwapchain     = VK_NULL_HANDLE
        };

        const auto& queue_families = QueueFamilyIndices::query(render_device, surface.handle());
        const unsigned queue_indices[] =
        {
            queue_families.graphics.value(),
            queue_families.present.value()
        };

        if(queue_families.graphics.value() != queue_families.present.value())
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2;
            swapchain_create_info.pQueueFamilyIndices = queue_indices;
        }
        else
        {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 1;
            swapchain_create_info.pQueueFamilyIndices = nullptr;
        }

        VkResult result;
        result = vkCreateSwapchainKHR(pDevice->handle(), &swapchain_create_info, nullptr, &swapchain);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Swapchain creation failed (vkCreateSwapchainKHR did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-CreationFail");
        }

        views.resize(image_count);
        images.resize(image_count);
        frameBuffers.resize(image_count);
        surfaceFormat = surface_format.format;
        surfaceExtent = swapchain_extent;
    }

    void SwapchainWrapper::create_image_views()
    {
        unsigned image_count = images.size();

        VkResult result;
        result = vkGetSwapchainImagesKHR(pDevice->handle(), swapchain, &image_count, nullptr);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Swapchain creation failed (vkGetSwapchainImagesKHR did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-CreationFailed");
        }

        vkGetSwapchainImagesKHR(pDevice->handle(), swapchain, &image_count, images.data());

        for(auto i : range(0, image_count - 1))
        {
            VkImageViewCreateInfo view_create_info =
            {
                .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image      = images.at(i),
                .viewType   = VK_IMAGE_VIEW_TYPE_2D,
                .format     = surfaceFormat,
                .components =
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY
                },
                .subresourceRange =
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                }
            };

            result = vkCreateImageView(pDevice->handle(), &view_create_info, nullptr, &views[i]);
            if(result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Swapchain creation failed (vkCreateImageView did not return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-Swapchain-CreationFail");
            }
        }
    }

    void SwapchainWrapper::create_render_pass()
    {
        VkAttachmentDescription color_attachment =
        {
            .format         = surfaceFormat,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };

        VkAttachmentReference attachment_reference =
        {
            .attachment = 0,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass_description =
        {
            .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments    = &attachment_reference
        };

        VkRenderPassCreateInfo render_pass_create_info =
        {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments    = &color_attachment,
            .subpassCount    = 1,
            .pSubpasses      = &subpass_description
        };

        VkResult result;
        result = vkCreateRenderPass(pDevice->handle(), &render_pass_create_info, nullptr, &renderPass);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Swapchain creation failed (vkCreateRenderPass did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-CreationFail");
        }
    }
}
