#include "render/renderer.hpp"
#include "render/window.hpp"
#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"

#include <fmt/ranges.h> // print vector 
#include <vulkan/vulkan.h>
#include <fstream>
#include <cassert>
#include <any>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

std::vector<char> ReadFile(const std::string& name)
{
    std::ifstream file(name, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Renderer-File-OpenFail");
    }

    size_t file_size;
    file_size = file.tellg();
    file.seekg(0);

    auto buffer = std::vector<char>(file_size);
    file.read(buffer.data(), file_size);
    file.close();

    return buffer;
}

namespace Vk
{
    void MemoryAllocatorWrapper::create(LogicalDeviceWrapper& device)
    {
        VmaAllocatorCreateInfo allocator_create_info =
        {
            .physicalDevice = GetRenderingDevice(),
            .device         = device.handle(),
            .instance       = GetInstance()
        };

        VkResult result;
        result = vmaCreateAllocator(&allocator_create_info, &vmaAllocator);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Memory allocator creation fail (vmaCreateAllocator did not return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-MemoryAllocator-CreationFail");
        }

        CDebug::Log("Vulkan Renderer | Memory allocator created.");
    }

    void MemoryAllocatorWrapper::destroy()
    {
        vmaDestroyAllocator(vmaAllocator);
        CDebug::Log("Vulkan Renderer | Memory allocator destroyed.");
    }

    VmaAllocator MemoryAllocatorWrapper::handle() const
    {
        return vmaAllocator;
    }

    void SemaphoreWrapper::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain)
    {
        pDevice = &device;

        VkSemaphoreCreateInfo semaphore_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        VkFenceCreateInfo fence_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderingFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapchain.image_views().size(), VK_NULL_HANDLE);

        for (auto i : range(0, MAX_FRAMES_IN_FLIGHT - 1))
        {
            VkResult result_a;
            VkResult result_b;
            VkResult result_c;
            result_a = vkCreateSemaphore(pDevice->handle(), &semaphore_create_info, nullptr, &imageAvailableSemaphores[i]);
            result_b = vkCreateSemaphore(pDevice->handle(), &semaphore_create_info, nullptr, &renderingFinishedSemaphores[i]);
            result_c = vkCreateFence(pDevice->handle(), &fence_create_info, nullptr, &inFlightFences[i]);

            if(result_a != VK_SUCCESS || result_b != VK_SUCCESS || result_c != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Could not create semaphores (either vkCreateSemaphore or vkCreateFence didn't return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-Semaphore-CreationFail");
            }
        }

        CDebug::Log("Vulkan Renderer | Semaphores successfully created.");
    }

    void SemaphoreWrapper::destroy()
    {
        for (auto i : range(0, MAX_FRAMES_IN_FLIGHT - 1))
        {
            vkDestroySemaphore(pDevice->handle(), imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(pDevice->handle(), renderingFinishedSemaphores[i], nullptr);
            vkDestroyFence(pDevice->handle(), inFlightFences[i], nullptr);
        }
    }

    std::vector<VkSemaphore>& SemaphoreWrapper::images_available()
    {
        return imageAvailableSemaphores;
    }

    std::vector<VkSemaphore>& SemaphoreWrapper::rendering_finished()
    {
        return renderingFinishedSemaphores;
    }

    std::vector<VkFence>& SemaphoreWrapper::in_flight_fences()
    {
        return inFlightFences;
    }

    std::vector<VkFence>& SemaphoreWrapper::images_in_flight()
    {
        return imagesInFlight;
    }
}

void GameRenderer::create(RendererCreateInfo createInfo)
{
    window_handle = createInfo.pWindow;
    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    internal_data->surface.create(*window_handle);
    internal_data->device.create(internal_data->surface);
    internal_data->hasOptionalDynamicRendering = true;

    auto optional_extensions = Vk::GetAvailableOptionalExtensions(Vk::GetRenderingDevice());

    for(const auto& extension : optional_extensions)
    {
        if (strcmp(extension, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
            internal_data->hasOptionalDynamicRendering = true;
    }

    internal_data->swapchain.create(*window_handle, internal_data->surface, internal_data->device);
    internal_data->memoryAllocator.create(internal_data->device);
    internal_data->semaphores.create(internal_data->device, internal_data->swapchain);

    auto vert_code = ReadFile("vert.spv");
    auto frag_code = ReadFile("frag.spv");

    GraphicsShaderCreateInfo shader_create_info =
    {
        .vertexCode = vert_code,
        .fragmentCode = frag_code
    };

    create_shaders(&shader_create_info, 1);

    internal_data->commandQueue.create(internal_data->device, internal_data->swapchain, internal_data->surface, internal_data->shaders[0].handle);
    internal_data->currentFrame = 0;

    CDebug::Log("Vulkan Renderer | Activated optional extensions: {}", optional_extensions);

    window_handle->subscribe(WindowEvent::eResized, [this](void* handle, const std::any& eventData)
    {
        auto new_size = std::any_cast<std::pair<int, int>>(eventData);
        if(new_size.first == 0 && new_size.second == 0)
        {
            is_window_minimized = true;
            return;
        }
        else if(is_window_minimized)
        {
            is_window_minimized = false;
        }

        GameWindow& window = *window_handle;
        auto* data = std::any_cast<Vk::RendererInternalData>(&backend_data);
        data->swapchain.resize(window, data->surface);
    });
}

void GameRenderer::destroy()
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);

    {
        for (auto& shader : internal_data->shaders)
        {
            vkDestroyPipeline(internal_data->device.handle(), shader.handle, nullptr);
        }

        vkDestroyPipelineLayout(internal_data->device.handle(), internal_data->shaders[0].layout, nullptr);

        CDebug::Log("Vulkan Renderer | Shader pipelines destroyed.");
    }

    internal_data->swapchain.destroy();
    internal_data->device.destroy();
    internal_data->surface.destroy();
}

void GameRenderer::draw()
{
    if(is_window_minimized)
    {
        return;
    }

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    auto& device = internal_data->device;
    auto& swapchain = internal_data->swapchain;
    auto& semaphores = internal_data->semaphores;
    auto& command_queue = internal_data->commandQueue;
    auto& current_frame = internal_data->currentFrame;

    vkWaitForFences(device.handle(), 1, &semaphores.in_flight_fences()[current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    vkAcquireNextImageKHR(device.handle(), swapchain.handle(), UINT64_MAX, semaphores.images_available()[current_frame], VK_NULL_HANDLE, &image_index);

    if (semaphores.images_in_flight()[image_index] != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.handle(), 1, &semaphores.images_in_flight()[image_index], VK_TRUE, UINT64_MAX);
    }

    semaphores.images_in_flight()[image_index] = semaphores.in_flight_fences()[current_frame];

    VkSemaphore wait_semaphores[] = 
    { 
        semaphores.images_available()[current_frame]
    };

    VkPipelineStageFlags wait_stages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSemaphore signal_semaphores[] =
    {
        semaphores.rendering_finished()[current_frame]
    };

    VkSubmitInfo submit_info =
    {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = wait_semaphores,
        .pWaitDstStageMask    = wait_stages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &command_queue.buffers()[image_index],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = signal_semaphores
    };

    vkResetFences(device.handle(), 1, &semaphores.in_flight_fences()[current_frame]);
    
    VkResult result;
    result = vkQueueSubmit(device.graphics_queue(), 1, &submit_info, nullptr);
    if (result != VK_SUCCESS)
    {
        CDebug::Error("Vulkan Renderer | Failed to render a frame (vkQueueSubmit didn't return VK_SUCCESS).");
        return;
    }

    VkSubpassDependency dependency =
    {
        .srcSubpass    = VK_SUBPASS_EXTERNAL,
        .dstSubpass    = 0,
        .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    VkSwapchainKHR swapchains[] =
    {
        swapchain.handle()
    };

    VkPresentInfoKHR present_info =
    {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = signal_semaphores,
        .swapchainCount     = 1,
        .pSwapchains        = swapchains,
        .pImageIndices      = &image_index,
        .pResults           = nullptr
    };

    vkQueuePresentKHR(device.present_queue(), &present_info);

    current_frame = (current_frame + 1) % Vk::MAX_FRAMES_IN_FLIGHT;
}
