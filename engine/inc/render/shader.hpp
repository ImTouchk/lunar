#pragma once
#include "utils/identifier.hpp"
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
#endif
	~ShaderWrapper() = default;

private:
#ifdef VULKAN_RENDERER
	Vk::ShaderManager& manager;
#endif
	Identifier identifier;
};
