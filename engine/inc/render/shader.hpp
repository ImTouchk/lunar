#pragma once
#include "utils/identifier.hpp"
#include "render/texture.hpp"
#include <vector>

#ifdef VULKAN_RENDERER
#	include <vulkan/vulkan.h>

namespace Vk
{
	class ShaderManager;
}

#endif

struct GraphicsShaderCreateInfo
{
	const std::vector<char>& vertexCode;
	const std::vector<char>& fragmentCode;
};

class ShaderWrapper
{
public:
#ifdef VULKAN_RENDERER
	explicit ShaderWrapper(Vk::ShaderManager& manager, Identifier handle);
	[[nodiscard]] VkPipeline pipeline() const;
	[[nodiscard]] VkDescriptorSet descriptor() const;
#endif
	~ShaderWrapper() = default;

	void use_texture(TextureWrapper& texture);

private:
#ifdef VULKAN_RENDERER
	Vk::ShaderManager& manager;
#endif
	Identifier identifier;
};
