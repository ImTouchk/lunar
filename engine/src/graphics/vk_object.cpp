#include "utils/identifier.hpp"
#include "utils/debug.hpp"
#include "render/mesh.hpp"
#include "vk_object.hpp"
#include "vk_shader.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

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
	data.shader.use_texture(texture);
}

void MeshWrapper::set_transform(glm::mat4&& transform)
{
	auto& data = find_by_identifier_safe(objectManager.meshes, identifier);
	data.transform = transform;
}

namespace Vk
{
	void ObjectManager::create(ObjectManagerCreateInfo&& createInfo)
	{
		assert(not active);
		assert(createInfo.pSurface != nullptr);
		assert(createInfo.pSwapchain != nullptr);
		assert(createInfo.pShaderManager != nullptr);

		pSurface = createInfo.pSurface;
		pSwapchain = createInfo.pSwapchain;
		pShaderManager = createInfo.pShaderManager;

		active = true;
		
		CDebug::Log("Vulkan Renderer | Object manager created.");
	}

	void ObjectManager::destroy()
	{
		assert(active == true);

		for (auto& mesh : meshes)
		{
			mesh.indexBuffer.destroy();
			mesh.vertexBuffer.destroy();
		}

		meshes.clear();

		scissor  = {};
		viewport = {};
		active   = false;

		pSurface         = nullptr;
		pSwapchain       = nullptr;
		pShaderManager   = nullptr;
	}

	bool ObjectManager::update()
	{
		bool buffers_rebuilt = false;

		for(auto& mesh : meshes)
		{
			if(mesh.wasModified)
			{
				rebuild_mesh_command(mesh);
				mesh.wasModified = false;
				buffers_rebuilt = true;
			}
		}

		return buffers_rebuilt;
	}

	MeshWrapper ObjectManager::create_mesh(MeshCreateInfo&& createInfo)
	{
		auto data_buffer_type = (createInfo.type == MeshType::eDynamic)
									? BufferMemoryType::eGpuDynamic
									: BufferMemoryType::eGpuStatic;

		BufferWrapper vertex_buffer = BufferManager::CreateBuffer(BufferCreateInfo
		{
			.type       = BufferType::eVertex,
			.memoryType = data_buffer_type,
			.pData      = createInfo.vertices.data(),
			.dataSize   = static_cast<unsigned>(createInfo.vertices.size() * sizeof(Vertex)),
		});

		BufferWrapper index_buffer = BufferManager::CreateBuffer(BufferCreateInfo
		{
			.type       = BufferType::eIndex,
			.memoryType = data_buffer_type,
			.pData      = createInfo.indices.data(),
			.dataSize   = static_cast<unsigned>(createInfo.indices.size() * sizeof(Index))
		});

		DrawableObjectData object_data =
		{
			.identifier   = get_unique_number(),
			.type         = createInfo.type,
			.shader       = createInfo.shader,
			.vertexBuffer = std::move(vertex_buffer),
			.indexBuffer  = std::move(index_buffer),
			.wasModified  = true,
			.vertexCount  = static_cast<unsigned>(createInfo.vertices.size()),
			.indexCount   = static_cast<unsigned>(createInfo.indices.size()),
			.transform    = glm::mat4(1.f),
		};

		auto identifier = object_data.identifier;

		meshes.push_back(std::move(object_data));
		return MeshWrapper { *this, identifier };
	}

	void ObjectManager::rebuild_mesh_command(DrawableObjectData& object)
	{
		if(object.command != VK_NULL_HANDLE)
		{
			CommandSubmitter::DestroyCommandBuffer(object.command);
		}

		VkCommandBufferInheritanceInfo buffer_inheritance_info =
		{
			.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
			.pNext                = nullptr,
			.renderPass           = pSwapchain->render_pass(),
			.subpass              = 0,
			.framebuffer          = VK_NULL_HANDLE,
			.occlusionQueryEnable = VK_FALSE,
			.queryFlags           = 0,
			.pipelineStatistics   = 0
		};

		VkCommandBufferBeginInfo buffer_begin_info =
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
		};

		SwapchainWrapper* swapchain = pSwapchain;

		const auto graphics_layout = pShaderManager->get_graphics_layout();

		const auto res = CommandSubmitter::RecordSync([&object, graphics_layout, swapchain](VkCommandBuffer buffer)
		{
			const auto shader = object.shader.pipeline();
			const auto shader_desc = object.shader.descriptor();
			const auto index_buffer = object.indexBuffer.handle();
			const auto vertex_buffer = object.vertexBuffer.handle();

			if(shader == VK_NULL_HANDLE || vertex_buffer == VK_NULL_HANDLE)
			{
				return;
			}

			VkDeviceSize offsets[] = { 0 };

			vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader);
			vkCmdBindDescriptorSets(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_layout, 0, 1, &shader_desc, 0, nullptr);
			vkCmdPushConstants(buffer, graphics_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &object.transform);
			vkCmdSetViewport(buffer, 0, 1, &swapchain->get_viewport());
			vkCmdSetScissor(buffer, 0, 1, &swapchain->get_scissor());
			vkCmdBindVertexBuffers(buffer, 0, 1, &vertex_buffer, offsets);
			vkCmdBindIndexBuffer(buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdDrawIndexed(buffer, object.indexCount, 1, 0, 0, 0);
		}, 
		CommandSubmitter::AdditionalRecordData 
		{
			.level           = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.inheritanceInfo = buffer_inheritance_info,
			.beginInfo       = buffer_begin_info,
		});

		object.command = res;
	}

	size_t ObjectManager::mesh_count() const
	{
		return meshes.size();
	}

	std::vector<VkCommandBuffer> ObjectManager::mesh_commands() const
	{
		auto final_handles = std::vector<VkCommandBuffer>();
		for(int i = 0; i < meshes.size(); i++)
		{
			if(meshes[i].command != VK_NULL_HANDLE)
			{
				final_handles.push_back(meshes[i].command);
			}
		}
		return final_handles;
	}
}
