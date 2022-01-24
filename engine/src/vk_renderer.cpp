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
    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    internal_data->surface.create(*createInfo.pWindow);
    internal_data->device.create(internal_data->surface);
}

void GameRenderer::destroy()
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    internal_data->device.destroy();
    internal_data->surface.destroy();
}
