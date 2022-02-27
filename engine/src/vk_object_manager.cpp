#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Vk
{
    void ObjectManager::create(LogicalDeviceWrapper& device, MemoryAllocatorWrapper& memoryAllocator, SwapchainWrapper& swapchain, SurfaceWrapper& surface, ShaderManager& shaderManager)
    {
        pDevice = &device;
        pSurface = &surface;
        pSwapchain = &swapchain;
        pShaderManager = &shaderManager;
        pMemoryAllocator = &memoryAllocator;

        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        commandPool = create_command_pool();

        mainCmdBuffers.resize(pSwapchain->frame_buffers().size());
        allocate_command_buffers(mainCmdBuffers.data(), mainCmdBuffers.size(), commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        CDebug::Log("Vulkan Renderer | Object manager created.");
    }

    void ObjectManager::destroy()
    {
        for(auto& mesh : meshes)
        {
            vmaDestroyBuffer(pMemoryAllocator->handle(), mesh.vertexBuffer, mesh.vbMemory);
            vmaDestroyBuffer(pMemoryAllocator->handle(), mesh.indexBuffer, mesh.ibMemory);
        }

        vkDestroyCommandPool(pDevice->handle(), commandPool, nullptr);

        CDebug::Log("Vulkan Renderer | Object manager destroyed.");
    }

    void ObjectManager::update()
    {
        update_command_buffers();
    }

    VkCommandBuffer ObjectManager::get_main(unsigned int frame) const
    {
        return mainCmdBuffers[frame];
    }
}
