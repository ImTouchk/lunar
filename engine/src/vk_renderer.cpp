#include "render/renderer.hpp"
#include "render/window.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <cassert>
#include <any>

#include <vk_mem_alloc.h>

void GameRenderer::create(RendererCreateInfo createInfo)
{
    window_handle = createInfo.pWindow;
    backend_data = Vk::RendererInternalData();

    CDebug::Log("sizeof(RendererInternalData): {}", sizeof(Vk::RendererInternalData));

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    internal_data->surface.create(*window_handle);
    internal_data->device.create(internal_data->surface);
    internal_data->swapchain.create(*window_handle, internal_data->surface, internal_data->device);

    window_handle->subscribe(WindowEvent::eResized, [this](void* handle, const std::any&)
    {
        // memory gets corrupted here

        GameWindow& window = *reinterpret_cast<GameWindow*>(handle);
        auto* data = std::any_cast<Vk::RendererInternalData>(&backend_data);
        data->swapchain.resize(window, data->surface);
    });
    
    internal_data->swapchain.resize(*window_handle, internal_data->surface); // this works fine
}

void GameRenderer::destroy()
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
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
