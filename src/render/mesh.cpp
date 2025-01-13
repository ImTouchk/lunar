#include <lunar/render/mesh.hpp>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/vk_mesh.hpp>
#	include <lunar/render/internal/render_vk.hpp>
#endif

namespace Render
{
	MeshBuilder& MeshBuilder::setVertices(const std::span<Vertex>& vertices)
	{
		this->vertices = vertices;
		return *this;
	}

	MeshBuilder& MeshBuilder::setIndices(const std::span<uint32_t>& indices)
	{
		this->indices = indices;
		return *this;
	}

	MeshBuilder& MeshBuilder::useRenderContext(std::shared_ptr<RenderContext>& context)
	{
		this->context = context;
		return *this;
	}

	MeshBuilder& MeshBuilder::build()
	{
#ifdef LUNAR_VULKAN
		auto& vk_ctx = getVulkanContext(context);

		auto builder = VulkanMeshBuilder();

		builder
			.setVertices(vertices)
			.setIndices(indices)
			.useRenderContext(&vk_ctx)
			.build();

		result = Mesh();
		result.indicesCount        = indices.size();
		result._vkIndexBuffer      = builder.getIndexBuffer();
		result._vkVertexBuffer     = builder.getVertexBuffer();
		result._vkVertexBufferAddr = builder.getVertexBufferAddress();
#endif
#ifdef LUNAR_OPENGL
		_glBuild();
#endif
		return *this;
	}

	Mesh MeshBuilder::getResult()
	{
		return std::move(result);
	}
}
