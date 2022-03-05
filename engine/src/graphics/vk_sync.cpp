#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
    void SyncObjectsWrapper::create(SwapchainWrapper& swapchain)
    {
        imagesInFlight = std::vector<VkFence>(swapchain.image_views().size());

        VkSemaphoreCreateInfo semaphore_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };

        VkFenceCreateInfo fence_create_info =
        {
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        for(auto i : range(0, MAX_FRAMES_IN_FLIGHT - 1))
        {
            VkResult result_a;
            VkResult result_b;
            VkResult result_c;

            result_a = vkCreateSemaphore(GetDevice().handle, &semaphore_create_info, nullptr, &imageAvailableSemaphores[i]);
            result_b = vkCreateSemaphore(GetDevice().handle, &semaphore_create_info, nullptr, &renderingFinishedSemaphore[i]);
            result_c = vkCreateFence(GetDevice().handle, &fence_create_info, nullptr, &inFlightFences[i]);
            imagesInFlight[i] = VK_NULL_HANDLE;

            if(result_a != VK_SUCCESS || result_b != VK_SUCCESS || result_c != VK_SUCCESS)
            {
                CDebug::Error("Vulkan Renderer | Could not create synchronization objects (either vkCreateSemaphore or vkCreateFence did not return VK_SUCCESS).");
                throw std::runtime_error("Renderer-Vulkan-SyncObjects-CreationFail");
            }
        }

        CDebug::Log("Vulkan Renderer | Sync objects created.");
    }

    void SyncObjectsWrapper::destroy()
    {
        for(auto i : range(0, MAX_FRAMES_IN_FLIGHT - 1))
        {
            vkDestroySemaphore(GetDevice().handle, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(GetDevice().handle, renderingFinishedSemaphore[i], nullptr);
            vkDestroyFence(GetDevice().handle, inFlightFences[i], nullptr);
        }
    }

    VkSemaphore& SyncObjectsWrapper::image_available(size_t i)
    {
        return imageAvailableSemaphores[i];
    }

    VkSemaphore& SyncObjectsWrapper::rendering_finished(size_t i)
    {
        return renderingFinishedSemaphore[i];
    }

    VkFence& SyncObjectsWrapper::in_flight_fence(size_t i)
    {
        return inFlightFences[i];
    }

    VkFence& SyncObjectsWrapper::image_in_flight(size_t i)
    {
        return imagesInFlight[i];
    }
}
