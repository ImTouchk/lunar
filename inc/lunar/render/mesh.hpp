#pragma once
#include <lunar/render/common.hpp>
#include <lunar/render/material.hpp>
#include <lunar/api.hpp>
#include <span>

#ifdef LUNAR_OPENGL
#	include <glad/gl.h>
#endif

namespace fastgltf { class Mesh; class Asset; class Material; }

//namespace Render
//{
//	enum class LUNAR_API MeshPrimitive
//	{
//		eCube  = 0,
//		eQuad  = 1,
//		_count = 2,
//	};
//
//
//	class LUNAR_API Mesh
//	{
//	public:
//		Mesh() = default;
//
//		Material material = {};
//	private:
//#ifdef LUNAR_VULKAN
//		VulkanBuffer      _vkIndexBuffer      = {};
//		VulkanBuffer      _vkVertexBuffer     = {};
//		vk::DeviceAddress _vkVertexBufferAddr = {};
//
//		friend class VulkanContext;
//#endif
//#ifdef LUNAR_OPENGL
//		GLuint _glVao;
//		GLuint _glVbo;
//		GLuint _glEbo;
//		GLuint glSsbo;
//
//#endif
//		//size_t vertexCount = 0;
//		size_t indicesCount = 0;
//
//		friend struct CubemapBuilder;
//		friend struct MeshBuilder;
//		friend class MeshRenderer;
//		friend class RenderContext;
//	};
//
//	struct LUNAR_API MeshBuilder
//	{
//	public:
//		MeshBuilder() = default;
//		~MeshBuilder() = default;
//
//		MeshBuilder& setVertices(const std::span<const Vertex>& vertices);
//		MeshBuilder& setIndices(const std::span<const uint32_t>& indices);
//		MeshBuilder& useRenderContext(std::shared_ptr<RenderContext>& context);
//		MeshBuilder& fromGltfFile(const Fs::Path& path);
//		MeshBuilder& build();
//		Mesh getResult();
//
//	private:
//		std::shared_ptr<RenderContext> context      = nullptr;
//		std::vector<Vertex>            vertices     = {};
//		std::vector<uint32_t>          indices      = {};
//		Mesh                           result       = {};
//		GpuMaterialData                resMaterials = {};
//		
//		void aggregateMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh);
//		void aggregateMaterials(const fastgltf::Asset& asset);
//
//#		ifdef LUNAR_OPENGL
//		void _glBuild();
//#		endif
//	};
//}

namespace lunar::Render
{
	enum class LUNAR_API MeshTopology
	{
		eTriangles = GL_TRIANGLES,
	};

	class LUNAR_API GpuMesh_T
	{
	public:
		GpuMesh_T
		(
			RenderContext_T*     context,
			GpuVertexArrayObject vertexArray,
			GpuBuffer            vertexBuffer,
			GpuBuffer            indexBuffer,
			MeshTopology         topology
		) noexcept;
		GpuMesh_T() noexcept = default;
		~GpuMesh_T() noexcept;

		GpuVertexArrayObject getVertexArray();
		size_t               getVertexCount()  const;
		size_t               getIndicesCount() const;
		MeshTopology         getTopology()     const;

	private:
		GpuVertexArrayObject vertexArray  = nullptr;
		GpuBuffer            vertexBuffer = nullptr;
		GpuBuffer            indexBuffer  = nullptr;
		RenderContext_T*     context      = nullptr;
		size_t               vertexCount  = 0;
		size_t               indexCount   = 0;
		MeshTopology         meshTopology = MeshTopology::eTriangles;
	};

	enum class LUNAR_API MeshPrimitive
	{
		eCube = 0,
		eQuad = 1
	};

	struct LUNAR_API GpuMeshBuilder
	{
	public:
		GpuMeshBuilder()  noexcept = default;
		~GpuMeshBuilder() noexcept = default;

		GpuMeshBuilder& useRenderContext(RenderContext_T* context);
		GpuMeshBuilder& fromVertexArray(const std::span<const Vertex>& vertices);
		GpuMeshBuilder& fromIndexArray(const std::span<const uint32_t>& indices);
		GpuMeshBuilder& defaultVertexArray();
		GpuMesh         build();

	private:
		GpuVertexArrayObject vertexArray  = nullptr;
		GpuBuffer            vertexBuffer = nullptr;
		GpuBuffer            indexBuffer  = nullptr;
		RenderContext_T*     context      = nullptr;
		size_t               vertexCount  = 0;
		size_t               indicesCount = 0;
	};
}
