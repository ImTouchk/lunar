#include "render/renderer.hpp"
#include "render/window.hpp"
#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <any>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void GameRenderer::create(RendererCreateInfo createInfo)
{
    window_handle = createInfo.pWindow;
    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    auto& surface = internal_data->surface;
    auto& device = internal_data->device;
    auto& swapchain = internal_data->swapchain;
    auto& sync_objects = internal_data->syncObjects;
    auto& object_manager = internal_data->objectManager;
    auto& shader_manager = internal_data->shaderManager;
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
    sync_objects.create(device, swapchain);
    shader_manager.create(device, swapchain);
    object_manager.create(device, swapchain, surface, shader_manager);

    //command_queue.create(device, swapchain, surface, internal_data->shaders[0].handle);

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
        data->objectManager.handle_resize();
    });
}

void GameRenderer::destroy()
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);

    vkDeviceWaitIdle(internal_data->device.handle());

    internal_data->objectManager.destroy();
    internal_data->shaderManager.destroy();
    internal_data->syncObjects.destroy();
    //internal_data->commandQueue.destroy();

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
    auto& object_manager = internal_data->objectManager;

    object_manager.update();

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

    VkCommandBuffer buffer = object_manager.get_main(image_index);

    VkSubmitInfo submit_info =
    {
        .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = wait_semaphores,
        .pWaitDstStageMask    = wait_stages,
        .commandBufferCount   = 1,
        .pCommandBuffers      = &buffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = signal_semaphores
    };

    vkResetFences(device.handle(), 1, &sync_objects.in_flight_fence(current_frame));
    
    VkResult result;
    result = vkQueueSubmit(device.graphics_queue(), 1, &submit_info, sync_objects.in_flight_fence(current_frame));
    if (result != VK_SUCCESS)
    {
        CDebug::Error("Vulkan Renderer | Failed to render a frame (vkQueueSubmit didn't return VK_SUCCESS).");
        return;
    }

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

std::vector<Shader> GameRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned int count)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->shaderManager.create_graphics(pCreateInfos, count);
}

CMesh GameRenderer::create_object(MeshCreateInfo meshCreateInfo)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->objectManager.create_object(meshCreateInfo);
}