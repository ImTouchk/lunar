#include "utils/debug.hpp"
#include "utils/identifier.hpp"
#include "render/mesh.hpp"
#include "vk_renderer.hpp"
#include "vk_object.hpp"

#include <vulkan/vulkan.h>
#include <cassert>

MeshWrapper::MeshWrapper(Vk::ObjectManager& objectManager, Identifier handle)
	: objectManager(objectManager),
	identifier(handle)
{
}

void MeshWrapper::set_vertices(std::vector<Vertex>&& vertices)
{
	auto& data = find_by_identifier_safe(objectManager.meshes, identifier);
	data.vertexBuffer.update(vertices.data(), vertices.size() * sizeof(Vertex));
	data.vertexCount = vertices.size();
	data.wasModified = true;
}

void MeshWrapper::set_indices(std::vector<Index>&& indices)
{
	auto& data = find_by_identifier_safe(objectManager.meshes, identifier);
	data.indexBuffer.update(indices.data(), indices.size() * sizeof(Index));
	data.indexCount = indices.size();
	data.wasModified = true;
}

void MeshWrapper::use_texture(TextureWrapper&& texture)
{
	auto& data = find_by_identifier_safe(objectManager.meshes, identifier);
	
}
