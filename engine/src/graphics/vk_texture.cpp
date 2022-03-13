#include "utils/debug.hpp"
#include "utils/identifier.hpp"
#include "vk_buffer.hpp"
#include "vk_texture.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <future>

namespace Vk
{
    void TextureManager::create()
    {
        assert(not active);

        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(GetRenderingDevice(), &device_properties);

        VkSamplerCreateInfo sampler_create_info =
        {
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter               = VK_FILTER_LINEAR,
            .minFilter               = VK_FILTER_LINEAR,
            .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias              = 0.f,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = device_properties.limits.maxSamplerAnisotropy,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .minLod                  = 0.f,
            .maxLod                  = 0.f,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };

        VkResult result;
        result = vkCreateSampler(GetDevice().handle, &sampler_create_info, nullptr, &texture_sampler);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Texture manager creation failed (vkCreateSampler didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-TextureManager-CreationFail");
        }

        active = true;

        CDebug::Log("Vulkan Renderer | Texture manager created.");
    }

    void TextureManager::destroy()
    {
        assert(active == true);

        while(!textures.empty())
        {
            vkDestroyImageView(GetDevice().handle, textures[0].view, nullptr);
            vmaDestroyImage(GetMemoryAllocator(), textures[0].image, textures[0].memory);
            delete_element_with_identifier(textures, textures[0].identifier);
        }

        vkDestroySampler(GetDevice().handle, texture_sampler, nullptr);
        active = false;

        CDebug::Log("Vulkan Renderer | Texture manager destroyed.");
    }

    TextureWrapper TextureManager::create_texture(TextureCreateInfo&& createInfo)
    {
        assert(active == true);

        auto buffer = BufferManager::CreateBuffer(BufferCreateInfo
        {
            .type       = BufferType::eTexture,
            .memoryType = BufferMemoryType::eCpuAny,
            .pData      = createInfo.pData,
            .dataSize   = createInfo.width * createInfo.height * createInfo.channels
        });

        VkImageCreateInfo image_create_info =
        {
            .sType      = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags      = 0,
            .imageType  = VK_IMAGE_TYPE_2D,
            .format     = VK_FORMAT_R8G8B8A8_SRGB,
            .extent     =
            {
                .width  = static_cast<uint32_t>(createInfo.width),
                .height = static_cast<uint32_t>(createInfo.height),
                .depth  = 1,
            },
            .mipLevels     = 1,
            .arrayLayers   = 1,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo allocation_create_info =
        {
            .flags          = 0,
            .usage          = VMA_MEMORY_USAGE_GPU_ONLY,
            .requiredFlags  = 0,
            .preferredFlags = 0,
            .memoryTypeBits = 0,
            .pool           = VK_NULL_HANDLE,
            .pUserData      = nullptr,
            .priority       = 0,
        };

        TextureData new_texture =
        {
            .identifier = get_unique_number(),
            .flags      = createInfo.flags,
            .image      = VK_NULL_HANDLE,
            .view       = VK_NULL_HANDLE,
            .memory     = VK_NULL_HANDLE,
            .width      = createInfo.width,
            .height     = createInfo.height,
            .channels   = createInfo.channels,
        };

        VkResult result;
        result = vmaCreateImage(GetMemoryAllocator(), &image_create_info, &allocation_create_info, &new_texture.image, &new_texture.memory, nullptr);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Failed to create a new texture (vmaCreateImage didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-TextureManager-TextureCreationFail");
        }

        convert_tex_layout(new_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        upload_texture_to_gpu(buffer, new_texture.image, new_texture.width, new_texture.height, new_texture.channels);
        convert_tex_layout(new_texture.image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        VkImageViewCreateInfo view_create_info =
        {
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = new_texture.image,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = VK_FORMAT_R8G8B8A8_SRGB,
            .subresourceRange =
            {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };

        result = vkCreateImageView(GetDevice().handle, &view_create_info, nullptr, &new_texture.view);
        if(result != VK_SUCCESS)
        {
            CDebug::Error("Vulkan Renderer | Failed to create a texture (vkCreateImageView didn't return VK_SUCCESS).");
            throw std::runtime_error("Renderer-Vulkan-TextureManager-TextureCreationFail");
        }

        buffer.destroy();
    }

    void TextureManager::convert_tex_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        SubmitCommand([&](VkCommandBuffer buffer)
        {
            VkPipelineStageFlags source_stage;
            VkPipelineStageFlags dest_stage;

            VkImageMemoryBarrier barrier =
            {
                .sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask       = 0,
                .dstAccessMask       = 0,
                .oldLayout           = oldLayout,
                .newLayout           = newLayout,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .image               = image,
                .subresourceRange    =
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0,
                    .levelCount     = 1,
                    .baseArrayLayer = 0,
                    .layerCount     = 1,
                },
            };

            if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
            {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                dest_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            }
            else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
            {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                dest_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            }
            else
            {
                throw std::runtime_error("Renderer-Vulkan-TextureManager-LayoutChangeFail");
            }

            vkCmdPipelineBarrier(buffer, source_stage, dest_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        }).wait();
    }

    void TextureManager::upload_texture_to_gpu(BufferWrapper& src, VkImage dst, unsigned width, unsigned height, unsigned channels)
    {
        SubmitCommand([&](VkCommandBuffer buffer)
        {
            VkBufferImageCopy copy_region =
            {
                .bufferOffset      = 0,
                .bufferRowLength   = 0,
                .bufferImageHeight = 0,
                .imageSubresource  =
                {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .mipLevel       = 0,
                    .baseArrayLayer = 0,
                    .layerCount     = 1
                },
                .imageOffset = { 0, 0, 0 },
                .imageExtent = { width, height, 1 }
            };

            vkCmdCopyBufferToImage(buffer, src.handle(), dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        }).wait();
    }
}
