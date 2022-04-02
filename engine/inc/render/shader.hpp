#pragma once
#ifdef VULKAN_RENDERER
#	include <vulkan/vulkan.h>

namespace Vk
{
	class ShaderManager;
}
#endif

#include "utils/identifier.hpp"
#include "render/texture.hpp"
#include <vector>

enum class ShaderType
{
	eUnknown = 0,
	eGraphics,
	eCompute
};

struct GraphicsShaderCreateInfo
{
	const std::vector<char>& vertexCode;
	const std::vector<char>& fragmentCode;
};

struct ComputeShaderCreateInfo
{
	const std::vector<char>& code;
};

class ShaderWrapper
{
public:
#ifdef VULKAN_RENDERER
	explicit ShaderWrapper(Vk::ShaderManager& manager, Identifier handle);
	[[nodiscard]] VkPipeline pipeline() const;
	[[nodiscard]] VkDescriptorSet descriptor() const;
#elif OPENGL_RENDERER
	explicit ShaderWrapper(Identifier handle);
	[[nodiscard]] unsigned handle() const;
#endif
	~ShaderWrapper() = default;

	void use_texture(TextureWrapper& texture);

private:
#ifdef VULKAN_RENDERER
	Vk::ShaderManager& manager;
#endif
	Identifier identifier;
};
