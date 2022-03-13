#include "render/renderer.hpp"
#include "render/window.hpp"
#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <any>

void GameRenderer::create(RendererCreateInfo&& createInfo)
{
    window_handle = createInfo.pWindow;
    backend_data = Vk::RendererInternalData();

    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);

    auto& surface      = internal_data->surface;
    auto& swapchain    = internal_data->swapchain;
    auto& sync_objects = internal_data->syncObjects;

    auto& shader_manager      = internal_data->shaderManager;
    auto& object_manager      = internal_data->objectManager;
    auto& texture_manager     = internal_data->textureManager;
    auto& render_call_manager = internal_data->renderCallManager;

    Vk::SignalRendererCreation();

    surface.create(*window_handle);

    swapchain.create(*window_handle, surface);
    sync_objects.create(swapchain);
    shader_manager.create(swapchain);

    object_manager.create
    (Vk::ObjectManagerCreateInfo
    {
        .pSwapchain       = &swapchain,
        .pSurface         = &surface,
        .pShaderManager   = &shader_manager,
    });

    texture_manager.create();

    render_call_manager.create
    (Vk::RenderCallManagerCreateInfo
    {
        .pSurface       = &surface,
        .pSwapchain     = &swapchain,
        .pObjectManager = &object_manager,
        .pSyncObjects   = &sync_objects
    });

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

    vkDeviceWaitIdle(Vk::GetDevice().handle);

    internal_data->renderCallManager.destroy();
    internal_data->objectManager.destroy();
    internal_data->shaderManager.destroy();
    internal_data->textureManager.destroy();
    internal_data->syncObjects.destroy();
    internal_data->swapchain.destroy();
    internal_data->surface.destroy();

    Vk::SignalRendererDestroy();
}

void GameRenderer::draw()
{
    if(is_window_minimized)
    {
        return;
    }
    
    auto* internal_data       = std::any_cast<Vk::RendererInternalData>(&backend_data);
    auto& render_call_manager = internal_data->renderCallManager;
    auto& object_manager      = internal_data->objectManager;
    auto& sync_objects        = internal_data->syncObjects;
    auto& swapchain           = internal_data->swapchain;
    auto current_frame        = render_call_manager.current_image();
    const auto& device        = Vk::GetDevice();

    vkWaitForFences(device.handle, 1, &sync_objects.in_flight_fence(render_call_manager.current_image()), VK_TRUE, UINT64_MAX);

    object_manager.update();
    render_call_manager.update();

    uint32_t image_index;
    vkAcquireNextImageKHR(device.handle, swapchain.handle(), UINT64_MAX, sync_objects.image_available(current_frame), VK_NULL_HANDLE, &image_index);

    if (sync_objects.image_in_flight(image_index) != VK_NULL_HANDLE)
    {
        vkWaitForFences(device.handle, 1, &sync_objects.image_in_flight(image_index), VK_TRUE, UINT64_MAX);
    }

    render_call_manager.draw(image_index);
}

std::vector<ShaderWrapper> GameRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned int count)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->shaderManager.create_graphics(pCreateInfos, count);
}

MeshWrapper GameRenderer::create_object(MeshCreateInfo&& meshCreateInfo)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->objectManager.create_mesh(std::move(meshCreateInfo));
}

TextureWrapper GameRenderer::create_texture(TextureCreateInfo&& textureCreateInfo)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->textureManager.create_texture(std::move(textureCreateInfo));
}
