#pragma once
#include <lunar/render/render_context.hpp>
#include <lunar/render/common.hpp>
#include <lunar/render/material.hpp>
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
	enum class LUNAR_API MeshPrimitive
	{
		eCube  = 0,
		eQuad  = 1,
		_count = 2,
	};

	class LUNAR_API Mesh
	{
	public:
		Mesh() = default;

		Material material = {};
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

		friend struct CubemapBuilder;
		friend struct MeshBuilder;
		friend class MeshRenderer;
	};

	struct LUNAR_API MeshBuilder
	{
	public:
		MeshBuilder() = default;
		~MeshBuilder() = default;

		MeshBuilder& setVertices(const std::span<const Vertex>& vertices);
		MeshBuilder& setIndices(const std::span<const uint32_t>& indices);
		MeshBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
		MeshBuilder& fromGltfFile(const Fs::Path& path);
		MeshBuilder& build();
		Mesh getResult();

	private:
		std::shared_ptr<RenderContext> context  = nullptr;
		std::vector<Vertex>            vertices = {};
		std::vector<uint32_t>          indices  = {};
		Mesh                           result   = {};

#		ifdef LUNAR_OPENGL
		void _glBuild();
#		endif
	};
}

