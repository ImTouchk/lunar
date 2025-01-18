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
		glm::vec3 tangent;
		glm::vec3 bitangent;
	};

	struct LUNAR_API UniformBufferData
	{
#		ifdef LUNAR_VULKAN
	private:
		vk::DeviceAddress _vkVertexBuffer;

		friend class VulkanContext;
#		endif
	};

	struct LUNAR_API SceneGpuData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 model;
		glm::vec3 cameraPos;
	};

	struct LUNAR_API SceneLightData
	{
		float metallic;
		float roughness;
		float ao;
		int count;
		glm::vec4 positions[10];
		glm::vec4 colors[10];
		int isCubemapHdr;
	};

	LUNAR_API const std::vector<Vertex>& GetCubeVertices();
	LUNAR_API const std::vector<uint32_t>& GetCubeIndices();
	LUNAR_API const std::vector<Vertex>& GetQuadVertices();
	LUNAR_API const std::vector<uint32_t>& GetQuadIndices();
}
