#pragma once
#include <lunar/api.hpp>
#include <lunar/core/handle.hpp>
#include <glm/glm.hpp>

#ifdef LUNAR_VULKAN
#	include <vulkan/vulkan.hpp>
#endif

namespace Render
{
	struct LUNAR_API Vertex
	{
		glm::vec3 position;
		float     uv_x;
		glm::vec3 normal;
		float     uv_y;
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

	struct LUNAR_API GpuMaterial
	{
		int   albedoMap;
		float metallic;
		float roughness;
		float ao;
	};

	struct LUNAR_API GpuMaterialData
	{
		int         count         = 0;
		GpuMaterial materials[15] = {};
	};

	LUNAR_API const std::vector<Vertex>& GetCubeVertices();
	LUNAR_API const std::vector<uint32_t>& GetCubeIndices();
	LUNAR_API const std::vector<Vertex>& GetQuadVertices();
	LUNAR_API const std::vector<uint32_t>& GetQuadIndices();

}

namespace lunar::Render
{
	class LUNAR_API GpuVertexArrayObject_T;
	class LUNAR_API GpuBuffer_T;
	class LUNAR_API GpuProgram_T;
	class LUNAR_API GpuTexture_T;
	class LUNAR_API GpuCubemap_T;
	class LUNAR_API GpuMesh_T;
	class LUNAR_API RenderContext_T;
	LUNAR_HANDLE(GpuVertexArrayObject);
	LUNAR_HANDLE(GpuBuffer);
	LUNAR_HANDLE(GpuProgram);
	LUNAR_HANDLE(GpuTexture);
	LUNAR_HANDLE(GpuCubemap);
	LUNAR_HANDLE(GpuMesh);
	LUNAR_SHARED_HANDLE(RenderContext);
}