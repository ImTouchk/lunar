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

    auto& surface   = internal_data->surface;
    auto& swapchain = internal_data->swapchain;

    auto& shader_manager  = internal_data->shaderManager;
    auto& object_manager  = internal_data->objectManager;
    auto& texture_manager = internal_data->textureManager;

    Vk::SignalRendererCreation();

    surface.create(*window_handle);
    swapchain.create(*window_handle, surface);

    shader_manager.create(swapchain);

    object_manager.create
    (Vk::ObjectManagerCreateInfo
    {
        .pSwapchain       = &swapchain,
        .pSurface         = &surface,
        .pShaderManager   = &shader_manager,
    });

    texture_manager.create();

    VkSemaphoreCreateInfo semaphore_create_info =
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };

    for(auto i : range(0, Vk::MAX_FRAMES_IN_FLIGHT - 1))
    {
        vkCreateSemaphore(Vk::GetDevice().handle, &semaphore_create_info, nullptr, &internal_data->isImageAvailable[i]);
        vkCreateSemaphore(Vk::GetDevice().handle, &semaphore_create_info, nullptr, &internal_data->isRenderingFinished[i]);
    }

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

    for(auto i : range(0, Vk::MAX_FRAMES_IN_FLIGHT - 1))
    {
        vkDestroySemaphore(Vk::GetDevice().handle, internal_data->isImageAvailable[i], nullptr);
        vkDestroySemaphore(Vk::GetDevice().handle, internal_data->isRenderingFinished[i], nullptr);
    }

    internal_data->objectManager.destroy();
    internal_data->shaderManager.destroy();
    internal_data->textureManager.destroy();
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
    
    auto* internal_data  = std::any_cast<Vk::RendererInternalData>(&backend_data);
    auto& object_manager = internal_data->objectManager;
    auto& current_frame  = internal_data->currentFrame;
    auto& swapchain      = internal_data->swapchain;
    const auto& device   = Vk::GetDevice();

    object_manager.update();

    if(internal_data->recordedMeshes != object_manager.mesh_count())
    {
        for(int i = 0; i <= Vk::MAX_FRAMES_IN_FLIGHT; i++)
        {
            auto* pSwapchain = &internal_data->swapchain;
            auto* pObjectManager = &internal_data->objectManager;

            auto buffer = Vk::CommandSubmitter::RecordSync([pSwapchain, pObjectManager, i](VkCommandBuffer& buffer)
            {
                const auto meshes = pObjectManager->mesh_commands();

                const VkClearValue clear_values[2] =
                {
                    {.color = { 0.f, 0.f, 0.f, 1.f } },
                    {.depthStencil = { 1.f, 0 } }
                };

                const VkRenderPassBeginInfo render_pass_begin_info =
                {
                    .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                    .renderPass      = pSwapchain->render_pass(),
                    .framebuffer     = pSwapchain->frame_buffers()[i],
                    .renderArea      =
                    {
                        .offset      = { 0, 0 },
                        .extent      = pSwapchain->surface_extent()
                    },
                    .clearValueCount = 2,
                    .pClearValues    = clear_values,
                };

                vkCmdBeginRenderPass(buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
                if (!meshes.empty())
                {
                    vkCmdExecuteCommands(buffer, static_cast<uint32_t>(meshes.size()), meshes.data());
                }
                vkCmdEndRenderPass(buffer);
            });

            internal_data->submitCommands[i] = buffer;
        }

        internal_data->recordedMeshes = object_manager.mesh_count();
    }

    const VkPipelineStageFlags wait_stages[] =
    {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
     
    uint32_t image_index;
    vkAcquireNextImageKHR(device.handle, swapchain.handle(), UINT64_MAX, internal_data->isImageAvailable[current_frame], VK_NULL_HANDLE, &image_index);

    // img_indx
    Vk::CommandSubmitter::SubmitSync(internal_data->submitCommands[image_index], true,
    VkSubmitInfo
    {
        .waitSemaphoreCount   = 1,
        .pWaitSemaphores      = &internal_data->isImageAvailable[current_frame],
        .pWaitDstStageMask    = wait_stages,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores    = &internal_data->isRenderingFinished[current_frame]
    });

    VkSwapchainKHR swap = swapchain.handle();

    VkPresentInfoKHR present_info =
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &internal_data->isRenderingFinished[current_frame],
        .swapchainCount     = 1,
        .pSwapchains        = &swap,
        .pImageIndices      = &image_index,
        .pResults           = nullptr
    };

    vkQueuePresentKHR(Vk::GetDevice().present, &present_info);
    current_frame = (current_frame + 1) % Vk::MAX_FRAMES_IN_FLIGHT;
}

std::vector<ShaderWrapper> GameRenderer::create_shaders(GraphicsShaderCreateInfo* pCreateInfos, unsigned int count)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->shaderManager.create_graphics(pCreateInfos, count);
}

std::vector<ShaderWrapper> GameRenderer::create_shaders(ComputeShaderCreateInfo* pCreateInfos, unsigned count)
{
    auto* internal_data = std::any_cast<Vk::RendererInternalData>(&backend_data);
    return internal_data->shaderManager.create_compute(pCreateInfos, count);
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
