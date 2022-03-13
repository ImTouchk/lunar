#pragma once
#include "render/texture.hpp"
#include "vk_forward_decl.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace Vk
{
    struct TextureData
    {
        unsigned identifier;
        TextureFlags flags;
        VkImage image;
        VkImageView view;
        VmaAllocation memory;
        unsigned width;
        unsigned height;
        unsigned channels;
    };

    class TextureManager
    {
    public:
        friend class TextureWrapper;

        TextureManager() = default;
        ~TextureManager() = default;

        void create();
        void destroy();

        TextureWrapper create_texture(TextureCreateInfo&& createInfo);

    private:
        void upload_texture_to_gpu(BufferWrapper& src, VkImage dst, unsigned width, unsigned height, unsigned channels);
        void convert_tex_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    private:
        VkSampler texture_sampler         = VK_NULL_HANDLE;
        bool active                       = false;
        std::vector<TextureData> textures = {};
    };
}