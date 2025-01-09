#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/render/common.hpp>
#include <lunar/api.hpp>
#include <span>

#ifdef LUNAR_OPENGL
#	include <glad/gl.h>
#endif

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/vk_mesh.hpp>
#endif

namespace Render
{
	class LUNAR_API Mesh
	{
	public:
		Mesh() = default;

	private:
#ifdef LUNAR_VULKAN
		VulkanBuffer      _vkIndexBuffer      = {};
		VulkanBuffer      _vkVertexBuffer     = {};
		vk::DeviceAddress _vkVertexBufferAddr = {};

		friend class VulkanContext;
#endif
#ifdef LUNAR_OPENGL
		GLuint _glVao;
		GLuint _glVbo;
		GLuint _glEbo;

		friend class GLContext;
#endif
		//size_t vertexCount = 0;
		size_t indicesCount = 0;

		friend struct MeshBuilder;
	};

	struct LUNAR_API MeshBuilder
	{
	public:
		MeshBuilder() = default;
		~MeshBuilder() = default;

		MeshBuilder& setVertices(const std::span<Vertex>& vertices);
		MeshBuilder& setIndices(const std::span<uint32_t>& indices);
		MeshBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
		MeshBuilder& build();
		Mesh getResult();

	private:
		std::shared_ptr<RenderContext> context  = nullptr;
		std::span<Vertex>              vertices = {};
		std::span<uint32_t>            indices  = {};
		Mesh                           result   = {};

#		ifdef LUNAR_OPENGL
		void _glBuild();
#		endif
	};
}

