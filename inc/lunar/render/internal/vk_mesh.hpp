#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <lunar/api.hpp>
#include <lunar/render/internal/vk_base.hpp>
#include <lunar/render/internal/vk_buffer.hpp>
#include <lunar/render/common.hpp>

namespace Render
{
	class LUNAR_API VulkanMesh
	{
	public:
		VulkanMesh(VulkanBuffer& idx, VulkanBuffer& vert, vk::DeviceAddress vertBufAddr);
		VulkanMesh() = default;
		~VulkanMesh() = default;

		VulkanBuffer      indexBuffer      = {};
		VulkanBuffer      vertexBuffer     = {};
		vk::DeviceAddress vertexBufferAddr = {};
	};

	struct LUNAR_API VulkanMeshBuilder
	{
		VulkanMeshBuilder() = default;
		~VulkanMeshBuilder() = default;

		VulkanMeshBuilder& setVertices(const std::span<Vertex>& vertices);
		VulkanMeshBuilder& setIndices(const std::span<uint32_t>& indices);
		VulkanMeshBuilder& useRenderContext(VulkanContext* context);
		VulkanMeshBuilder& build();
		VulkanBuffer getIndexBuffer();
		VulkanBuffer getVertexBuffer();
		vk::DeviceAddress getVertexBufferAddress();
		
	private:
		VulkanContext*        context       = nullptr;
		std::vector<Vertex>   vertices      = {};
		std::vector<uint32_t> indices       = {};

		VulkanBuffer          indexBuf      = {};
		VulkanBuffer          vertexBuf     = {};
		vk::DeviceAddress     vertexBufAddr = {};
	};
}