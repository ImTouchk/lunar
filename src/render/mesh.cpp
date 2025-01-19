#include <lunar/render/mesh.hpp>
#include <lunar/render/material.hpp>

#ifdef LUNAR_VULKAN
#	include <lunar/render/internal/vk_mesh.hpp>
#	include <lunar/render/internal/render_vk.hpp>
#endif

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

namespace Render
{
	MeshBuilder& MeshBuilder::setVertices(const std::span<const Vertex>& vertices)
	{
		this->vertices = { vertices.begin(), vertices.end() };
		return *this;
	}

	MeshBuilder& MeshBuilder::setIndices(const std::span<const uint32_t>& indices)
	{
		this->indices = { indices.begin(), indices.end() };
		return *this;
	}

	MeshBuilder& MeshBuilder::useRenderContext(std::shared_ptr<RenderContext>& context)
	{
		this->context = context;
		return *this;
	}

	void MeshBuilder::aggregateMaterials(const fastgltf::Asset& asset)
	{
		resMaterials.count = asset.materials.size();
		for (size_t i = 0; i < asset.materials.size(); i++)
		{ 
			auto material_builder     = MaterialBuilder();
			resMaterials.materials[i] = material_builder
				.useRenderContext(context)
				.fromGltfObject(asset.materials[i])
				.build()
				.getGpuResult();
		}
	}

	void MeshBuilder::aggregateMesh(const fastgltf::Asset& asset, const fastgltf::Mesh& mesh)
	{
		for (auto&& p : mesh.primitives)
		{
			size_t initial_idx = vertices.size();
			auto& index_accessor = asset.accessors[p.indicesAccessor.value()];

			int material_idx = p.materialIndex.value_or(-1);

			indices.reserve(index_accessor.count);
			fastgltf::iterateAccessor<uint32_t>(asset, index_accessor, [&](uint32_t idx) {
				indices.emplace_back(idx + initial_idx);
			});

			auto& pos_accessor = asset.accessors[p.findAttribute("POSITION")->accessorIndex];
			vertices.resize(vertices.size() + pos_accessor.count);

			fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, pos_accessor, [&](glm::vec3 v, size_t idx) {
				auto& vertex = vertices[initial_idx + idx];
				vertex.position   = v;
				vertex.normal     = { 0.f, 0.f, 0.f };
				vertex.uv_x       = 0.f;
				vertex.uv_y       = 0.f;
				vertex.color      = glm::vec4{ 1.f };
			});

			auto normals = p.findAttribute("NORMAL");
			if (normals != p.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec3>(
					asset,
					asset.accessors[normals->accessorIndex],
					[&](glm::vec3 v, size_t idx) {
						vertices[initial_idx + idx].normal = v;
					}
				);
				DEBUG_LOG("?");
			}

			auto tangents = p.findAttribute("TANGENT");
			if (tangents != p.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec4>(
					asset,
					asset.accessors[tangents->accessorIndex],
					[&](glm::vec4 v, size_t idx) {
						auto& vertex = vertices[initial_idx];
						vertex.tangent = v;
						vertex.bitangent = glm::cross(vertex.normal, vertex.tangent);
					}
				);
			}

			auto uv = p.findAttribute("TEXCOORD_0");
			if (uv != p.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec2>(
					asset,
					asset.accessors[uv->accessorIndex],
					[&](glm::vec2 v, size_t idx) {
						vertices[initial_idx + idx].uv_x = v.x;
						vertices[initial_idx + idx].uv_y = v.y;
					}
				);
			}

			auto colors = p.findAttribute("COLOR_0");
			if (colors != p.attributes.end())
			{
				fastgltf::iterateAccessorWithIndex<glm::vec4>(
					asset,
					asset.accessors[colors->accessorIndex],
					[&](glm::vec4 v, size_t idx) {
						vertices[initial_idx + idx].color = v;
					}
				);
			}
		}

	}

	MeshBuilder& MeshBuilder::fromGltfFile(const Fs::Path& path)
	{
		auto options = fastgltf::Options::LoadGLBBuffers | 
						fastgltf::Options::LoadExternalBuffers | 
						fastgltf::Options::LoadExternalImages;

		auto parser  = fastgltf::Parser {};
		auto data    = fastgltf::GltfDataBuffer::FromPath(path);
		if (data.error() != fastgltf::Error::None)
		{
			DEBUG_ERROR("Couldn't open GLTF file at '{}'.", path.generic_string());
			return *this;
		}

		auto asset = parser.loadGltf(data.get(), path.parent_path(), options);
		// todo; error

		auto& meshes = asset->meshes;
		
		if (meshes.size() <= 0)
		{
			DEBUG_WARN("File '{}' does not contain any meshes.", path.generic_string());
			return *this;
		}

		aggregateMaterials(asset.get());

		for (auto& mesh : meshes)
			aggregateMesh(asset.get(), mesh);

		// TODO: textures, materials

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

	const std::vector<Vertex>& GetCubeVertices()
	{
		const static auto vertices = std::vector<Vertex>
		{
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 0, { 0, 0, 0 }, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 0, { 0, 0, 0 }, 0, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f,-1.f}, 1, { 0, 0, 1.f}, 1, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f,-1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f, 1.f}, 0, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f,-1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },

			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 1.f, 0.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 0.f, 1.f, 1.f } },
			Vertex{ {  1.f, 1.f,-1.f}, 1, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ {  1.f, 1.f, 1.f}, 1, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f,-1.f}, 0, { 0, 0, 1.f}, 1, { 0.f, 1.f, 0.f, 1.f } },
			Vertex{ { -1.f, 1.f, 1.f}, 0, { 0, 0, 1.f}, 0, { 0.f, 1.f, 0.f, 1.f } },
		};

		return vertices;
	}

	const std::vector<uint32_t>& GetCubeIndices()
	{
		const static auto indices = std::vector<uint32_t>
		{
			0, 1, 2, 3, 4, 5,
			6, 7, 8, 9, 10, 11,
			12, 13, 14, 15, 16, 17,
			18, 19, 20, 21, 22, 23,
			24, 25, 26, 27, 28, 29,
			30, 31, 32, 33, 34, 35
		};

		return indices;
	}

	const std::vector<Vertex>& GetQuadVertices()
	{
		const static auto vertices = std::vector<Vertex>
		{
			Vertex { { -1.f, 1.f, 0.f }, 0.f, { 0.f, 0.f, 0.f }, 1.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { { -1.f,-1.f, 0.f }, 0.f, { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { {  1.f, 1.f, 0.f }, 1.f, { 0.f, 0.f, 0.f }, 1.f, { 0.f, 0.f, 0.f, 0.f } },
			Vertex { {  1.f,-1.f, 0.f }, 1.f, { 0.f, 0.f, 0.f }, 0.f, { 0.f, 0.f, 0.f, 0.f } },
		};

		return vertices;
	}

	const std::vector<uint32_t>& GetQuadIndices()
	{
		const static auto indices = std::vector<uint32_t>
		{
			0, 1, 2,
			2, 1, 3
		};

		return indices;
	}
}
