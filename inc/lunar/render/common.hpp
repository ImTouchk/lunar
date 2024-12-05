#pragma once
#include <lunar/api.hpp>
#include <glm/glm.hpp>

#ifdef LUNAR_VULKAN
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
	struct LUNAR_API Vertex
	{
		glm::vec3 position;
		float uv_x;
		glm::vec3 normal;
		float uv_y;
		glm::vec4 color;
	};

	struct LUNAR_API UniformBufferData
	{
		glm::mat4 worldMatrix;
#		ifdef LUNAR_VULKAN
	private:
		vk::DeviceAddress _vkVertexBuffer;

		friend class VulkanContext;
#		endif
	};
}
