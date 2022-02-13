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
}

void GameRenderer::create(RendererCreateInfo createInfo)
{
    window_handle = createInfo.pWindow;
    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    auto& surface = internal_data->surface;
    auto& device = internal_data->device;
    auto& swapchain = internal_data->swapchain;
    auto& memory_allocator = internal_data->memoryAllocator;
    auto& sync_objects = internal_data->syncObjects;
    auto& command_queue = internal_data->commandQueue;

    surface.create(*window_handle);
    device.create(surface);

    auto optional_extensions = Vk::GetAvailableOptionalExtensions(Vk::GetRenderingDevice());

    for(const auto& extension : optional_extensions)
    {
        if (strcmp(extension, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
            internal_data->hasOptionalDynamicRendering = true;
    }

    swapchain.create(*window_handle, surface, device);
    memory_allocator.create(device);
    sync_objects.create(device, swapchain);

    auto vert_code = ReadFile("vert.spv");
    auto frag_code = ReadFile("frag.spv");

    GraphicsShaderCreateInfo shader_create_info =
    {
        .vertexCode = vert_code,
        .fragmentCode = frag_code
    };

    create_shaders(&shader_create_info, 1);

    command_queue.create(device, swapchain, surface, internal_data->shaders[0].handle);

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
    auto& sync_objects = internal_data->syncObjects;
    auto& command_queue = internal_data->commandQueue;
    auto& current_frame = internal_data->currentFrame;

    vkWaitForFences(device.handle(), 1, &sync_objects.in_flight_fence(current_frame), VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    vkAcquireNextImageKHR(device.handle(), swapchain.handle(), UINT64_MAX, sync_objects.image_available(current_frame), VK_NULL_HANDLE, &image_index);

    if (sync_objects.image_in_flight(image_index) != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.handle(), 1, &sync_objects.image_in_flight(image_index), VK_TRUE, UINT64_MAX);
    }

    sync_objects.image_in_flight(image_index) = sync_objects.in_flight_fence(current_frame);

    VkSemaphore wait_semaphores[] = 
    {
        sync_objects.image_available(current_frame)
    };

    VkPipelineStageFlags wait_stages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    VkSemaphore signal_semaphores[] =
    {
        sync_objects.rendering_finished(current_frame)
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

    vkResetFences(device.handle(), 1, &sync_objects.in_flight_fence(current_frame));
    
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
