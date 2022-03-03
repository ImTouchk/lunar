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
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }

        VkExtent2D extent =
        {
            .width = std::clamp((unsigned)window.get_width(), capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            .height = std::clamp((unsigned)window.get_height(), capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
        };

        return extent;
    }

    VkFormat PickSupportedDepthFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for(auto& format : candidates)
        {
            VkFormatProperties properties;
            vkGetPhysicalDeviceFormatProperties(GetRenderingDevice(), format, &properties);

            if(tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if(tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        CDebug::Error("Vulkan Renderer | Swapchain creation failed (no depth buffer format is available).");
        throw std::runtime_error("Renderer-Vulkan-Swapchain-DepthBuffer-NoFormatAvailable");
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

    void SwapchainWrapper::create(GameWindow& window, SurfaceWrapper& surface, LogicalDeviceWrapper& device, MemoryAllocatorWrapper& memoryAllocator)
    {
        assert(swapchain == VK_NULL_HANDLE);

        pDevice = &device;
        pMemoryAllocator = &memoryAllocator;

        width = window.get_width();
        height = window.get_height();

        create_swapchain(window, surface);
        create_depth_buffer();
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

        vkDestroyImageView(device, depthBuffer.view, nullptr);
        vmaDestroyImage(pMemoryAllocator->handle(), depthBuffer.image, depthBuffer.allocation);

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

        vkDestroyImageView(device, depthBuffer.view, nullptr);
        vmaDestroyImage(pMemoryAllocator->handle(), depthBuffer.image, depthBuffer.allocation);

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
        
        create(window, surface, *pDevice, *pMemoryAllocator);
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

        VkAttachmentDescription depth_attachment =
        {
            .format         = depthBuffer.format,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference depth_attachment_reference =
        {
            .attachment = 1,
            .layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };

        VkAttachmentReference attachment_reference =
        {
            .attachment = 0,
            .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass_description =
        {
            .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount    = 1,
            .pColorAttachments       = &attachment_reference,
            .pDepthStencilAttachment = &depth_attachment_reference
        };

        VkAttachmentDescription attachments[2] =
        {
            color_attachment,
            depth_attachment
        };

        VkSubpassDependency dependency =
        {
            .srcSubpass    = VK_SUBPASS_EXTERNAL,
            .dstSubpass    = 0,
            .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        };

        VkRenderPassCreateInfo render_pass_create_info =
        {
            .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 2,
            .pAttachments    = attachments,
            .subpassCount    = 1,
            .pSubpasses      = &subpass_description,
            .dependencyCount = 1,
            .pDependencies   = &dependency
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
            VkImageView attachments[2] =
            {
                views[i],
                depthBuffer.view
            };

            VkFramebufferCreateInfo framebuffer_create_info =
            {
                .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass      = renderPass,
                .attachmentCount = 2,
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

    void SwapchainWrapper::create_depth_buffer()
    {
        depthBuffer.format = PickSupportedDepthFormat
        (
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

        VkImageCreateInfo image_create_info =
        {
            .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext       = nullptr,
            .imageType   = VK_IMAGE_TYPE_2D,
            .format      = depthBuffer.format,
            .extent      = VkExtent3D
            {
                surfaceExtent.width,
                surfaceExtent.height,
                1
            },
            .mipLevels   = 1,
            .arrayLayers = 1,
            .samples     = VK_SAMPLE_COUNT_1_BIT,
            .tiling      = VK_IMAGE_TILING_OPTIMAL,
            .usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
        };

        VmaAllocationCreateInfo image_allocation =
        {
            .usage         = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        };

        VkResult result;
        result = vmaCreateImage(pMemoryAllocator->handle(), &image_create_info, &image_allocation, &depthBuffer.image, &depthBuffer.allocation, nullptr);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Swapchain creation fail (vmaCreateImage didn't return VK_SUCCESS for the depth buffer image).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-DepthBuffer-CreationFail");
        }

        VkImageViewCreateInfo view_create_info =
        {
            .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext    = nullptr,
            .image    = depthBuffer.image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format   = depthBuffer.format,
            .subresourceRange =
            {
                .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1,
            }
        };

        result = vkCreateImageView(pDevice->handle(), &view_create_info, nullptr, &depthBuffer.view);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Swapchain creation fail (vkCreateImageView didn't return VK_SUCCESS for the depth buffer image).");
            throw std::runtime_error("Renderer-Vulkan-Swapchain-DepthBuffer-CreationFail");
        }
    }
}
