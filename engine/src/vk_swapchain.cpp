#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"
#include "render/window.hpp"

#include <vulkan/vulkan.h>
#include <algorithm>
#include <cassert>

namespace Vk
{
    const SwapchainSupportDetails SwapchainSupportDetails::query(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        // This was originally cached ; however, if the window gets resized the values here become
        // invalid, so it needs to be calculated each time

        SwapchainSupportDetails value = {};

        unsigned format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
        if (format_count != 0)
        {
            value.surfaceFormats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, value.surfaceFormats.data());
        }

        unsigned present_modes = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes, nullptr);
        if (present_modes != 0)
        {
            value.surfacePresentModes.resize(present_modes);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_modes, value.surfacePresentModes.data());
        }

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &value.surfaceCapabilities);
        return value;
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
        VkExtent2D extent =
        {
            .width = std::clamp((unsigned)window.get_width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp((unsigned)window.get_height(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };

        return extent;
    }

    int SwapchainWrapper::get_width() const
    {
        return width;
    }

    int SwapchainWrapper::get_height() const
    {
        return height;
    }

    VkSwapchainKHR SwapchainWrapper::handle() const
    {
        return swapchain;
    }

    VkRenderPass SwapchainWrapper::render_pass() const
    {
        return renderPass;
    }

    const VkRect2D& SwapchainWrapper::get_scissor() const
    {
        return scissor;
    }

    const VkViewport& SwapchainWrapper::get_viewport() const
    {
        return viewport;
    }

    const std::vector<VkImageView>& SwapchainWrapper::image_views() const
    {
        return views;
    }

    const std::vector<VkFramebuffer>& SwapchainWrapper::frame_buffers() const
    {
        return frameBuffers;
    }

    VkExtent2D SwapchainWrapper::surface_extent() const
    {
        return surfaceExtent;
    }

    VkFormat SwapchainWrapper::surface_format() const
    {
        return surfaceFormat;
    }

    void SwapchainWrapper::create(GameWindow& window, SurfaceWrapper& surface, LogicalDeviceWrapper& device)
    {
        assert(swapchain == VK_NULL_HANDLE);

        pDevice = &device;
        width = window.get_width();
        height = window.get_height();

        create_swapchain(window, surface);
        create_image_views();

        if (renderPass == VK_NULL_HANDLE)
        {
            create_render_pass();
        }

        create_framebuffers();
    }

    void SwapchainWrapper::destroy()
    {
        assert(swapchain != VK_NULL_HANDLE);

        auto device = pDevice->handle();
        vkDeviceWaitIdle(device);

        for (auto& frame_buffer : frameBuffers)
        {
            vkDestroyFramebuffer(device, frame_buffer, nullptr);
        }

        vkDestroyRenderPass(device, renderPass, nullptr);

        for(auto& image_view : views)
        {
            vkDestroyImageView(device, image_view, nullptr);
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
        // resize optimization: render pass does not need to get recreated
        auto device = pDevice->handle();
        vkDeviceWaitIdle(device);

        for (auto& frame_buffer : frameBuffers)
        {
            vkDestroyFramebuffer(device, frame_buffer, nullptr);
        }

        for (auto& image_view : views)
        {
            vkDestroyImageView(device, image_view, nullptr);
        }

        vkDestroySwapchainKHR(device, swapchain, nullptr);

        swapchain = VK_NULL_HANDLE;
        frameBuffers.clear();
        images.clear();
        views.clear();
        surfaceFormat = {};
        surfaceExtent = {};
        
        create(window, surface, *pDevice);
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

        viewport = VkViewport
        {
            .x        = 0.f,
            .y        = 0.f,
            .width    = static_cast<float>(surfaceExtent.width),
            .height   = static_cast<float>(surfaceExtent.height),
            .minDepth = 0.f,
            .maxDepth = 1.f
        };

        scissor = VkRect2D
        {
            .offset = { 0, 0 },
            .extent = surfaceExtent
        };
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

    void SwapchainWrapper::create_framebuffers()
    {
        for (auto i : range(0, frameBuffers.size() - 1))
        {
            VkImageView attachments[] = { views[i] };

            VkFramebufferCreateInfo framebuffer_create_info =
            {
                .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass      = renderPass,
                .attachmentCount = 1,
                .pAttachments    = attachments,
                .width           = static_cast<uint32_t>(width),
                .height          = static_cast<uint32_t>(height),
                .layers          = 1
            };

            VkResult result;
            result = vkCreateFramebuffer(pDevice->handle(), &framebuffer_create_info, nullptr, &frameBuffers[i]);
            if (result != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Swapchain creation failed (vkCreateFramebuffer did not return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-Swapchain-FramebufferCreationFail");
            }
        }
    }
}
