#include "render/renderer.hpp"
#include "render/window.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <fmt/ranges.h> // print vector 
#include <vulkan/vulkan.h>
#include <fstream>
#include <cassert>
#include <any>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

void Vk::MemoryAllocatorWrapper::create(LogicalDeviceWrapper& device)
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

void Vk::MemoryAllocatorWrapper::destroy()
{
    vmaDestroyAllocator(vmaAllocator);
    CDebug::Log("Vulkan Renderer | Memory allocator destroyed.");
}

VmaAllocator Vk::MemoryAllocatorWrapper::handle() const
{
    return vmaAllocator;
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
}
