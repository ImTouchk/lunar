#pragma once
#include <cstdint>
#ifdef VULKAN_RENDERER
#   include <vulkan/vulkan.h>
#endif

enum class TextureFlags : uint8_t
{
    eNone    = 0,
    eDynamic = 1 << 0,
};

inline TextureFlags operator|(TextureFlags a, TextureFlags b)
{
    return static_cast<TextureFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

struct TextureCreateInfo
{
    TextureFlags flags;
    unsigned width;
    unsigned height;
    unsigned channels;
    void* pData;
};

#ifdef VULKAN_RENDERER
namespace Vk
{
    class TextureManager;
}
#endif

class TextureWrapper
{
public:
#ifdef VULKAN_RENDERER
    TextureWrapper(Vk::TextureManager& textureManager, unsigned handle);
    [[nodiscard]] VkImageView view() const;
    [[nodiscard]] VkSampler sampler() const;
#endif
    ~TextureWrapper() = default;

private:
#ifdef VULKAN_RENDERER
    Vk::TextureManager& texture_manager;
#endif
    unsigned identifier;

};
