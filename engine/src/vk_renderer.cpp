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
    auto& window = *createInfo.pWindow;

    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    internal_data->surface.create(window);
    internal_data->device.create(internal_data->surface);

    window.subscribe(WindowEvent::eMinimized, [&](void*, const std::any&){ is_window_minimized = true; });
    window.subscribe(WindowEvent::eRestored, [&](void*, const std::any&) { is_window_minimized = false; });
}

void GameRenderer::destroy()
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
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
