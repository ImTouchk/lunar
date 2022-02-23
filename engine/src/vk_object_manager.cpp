#include "utils/debug.hpp"
#include "utils/range.hpp"
#include "render/renderer.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <stdexcept>
#include <queue>

namespace Vk
{
    constexpr int BLOCK_SIZE = 10;

    void ObjectManager::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain, SurfaceWrapper& surface, ShaderManager& shaderManager)
    {
        pDevice = &device;
        pSurface = &surface;
        pSwapchain = &swapchain;
        pShaderManager = &shaderManager;

        auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

        commandPool = create_command_pool();

        VmaAllocatorCreateInfo allocator_create_info =
        {
            .physicalDevice   = GetRenderingDevice(),
            .device           = pDevice->handle(),
            .instance         = GetInstance(),
            .vulkanApiVersion = VK_API_VERSION_1_2
        };

        VkResult result;
        result = vmaCreateAllocator(&allocator_create_info, &memoryAllocator);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Object manager creation failed (vmaCreateAllocator didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-ObjectManager-MemAllocatorCreationFail");
        }

        mainCmdBuffers.resize(pSwapchain->frame_buffers().size());
        allocate_command_buffers(mainCmdBuffers.data(), mainCmdBuffers.size(), commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        CDebug::Log("Vulkan Renderer | Object manager created.");
    }

    void ObjectManager::destroy()
    {
        for(auto& mesh : meshes)
        {
            vmaDestroyBuffer(memoryAllocator, mesh.vertexBuffer, mesh.vbMemory);
            vmaDestroyBuffer(memoryAllocator, mesh.indexBuffer, mesh.ibMemory);
        }

        vmaDestroyAllocator(memoryAllocator);
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
