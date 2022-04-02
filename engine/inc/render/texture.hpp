#pragma once
#include <cstdint>
#include "utils/identifier.hpp"
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
#elif OPENGL_RENDERER
namespace GL
{
    struct RendererInternalData;
}
#endif

class TextureWrapper
{
public:
#ifdef VULKAN_RENDERER
    explicit TextureWrapper(Vk::TextureManager& textureManager, Identifer handle);
    [[nodiscard]] VkImageView view() const;
    [[nodiscard]] VkSampler sampler() const;
#elif OPENGL_RENDERER
    explicit TextureWrapper(GL::RendererInternalData& internalData, Identifier handle);
    [[nodiscard]] unsigned handle() const;
#endif
    ~TextureWrapper() = default;

private:
#ifdef VULKAN_RENDERER
    Vk::TextureManager& texture_manager;
#elif OPENGL_RENDERER
    GL::RendererInternalData& renderer_data;
#endif
    Identifier identifier;
};
