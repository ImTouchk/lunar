#include "utils/range.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>

namespace Vk
{
    void SyncObjectsWrapper::create(LogicalDeviceWrapper& device, SwapchainWrapper& swapchain)
    {
        pDevice = &device;
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

            result_a = vkCreateSemaphore(pDevice->handle(), &semaphore_create_info, nullptr, &imageAvailableSemaphores[i]);
            result_b = vkCreateSemaphore(pDevice->handle(), &semaphore_create_info, nullptr, &renderingFinishedSemaphore[i]);
            result_c = vkCreateFence(pDevice->handle(), &fence_create_info, nullptr, &inFlightFences[i]);
            imagesInFlight[i] = VK_NULL_HANDLE;

            if(result_a != VK_SUCCESS || result_b != VK_SUCCESS | result_c != VK_SUCCESS)
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
            vkDestroySemaphore(pDevice->handle(), imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(pDevice->handle(), renderingFinishedSemaphore[i], nullptr);
            vkDestroyFence(pDevice->handle(), inFlightFences[i], nullptr);
        }
    }

    VkSemaphore& SyncObjectsWrapper::image_available(unsigned int i)
    {
        return imageAvailableSemaphores[i];
    }

    VkSemaphore& SyncObjectsWrapper::rendering_finished(unsigned int i)
    {
        return renderingFinishedSemaphore[i];
    }

    VkFence& SyncObjectsWrapper::in_flight_fence(unsigned int i)
    {
        return inFlightFences[i];
    }

    VkFence& SyncObjectsWrapper::image_in_flight(unsigned int i)
    {
        return imagesInFlight[i];
    }
}