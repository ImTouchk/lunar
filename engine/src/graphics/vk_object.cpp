#include "utils/identifier.hpp"
#include "utils/debug.hpp"
#include "vk_object.hpp"
#include "vk_shader.hpp"
#include "vk_renderer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

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
			mesh.command.destroy();
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

	void ObjectManager::update()
	{
		for(auto& mesh : meshes)
		{
			if(mesh.wasModified)
			{
				rebuild_mesh_command(mesh);
				mesh.wasModified = false;
			}
		}

		for (int i = 0; i < pending_buffers.size();)
		{
			const auto& identifier = pending_buffers[i].first;
			auto& future_value = pending_buffers[i].second;

			if(future_value.valid())
			{
				auto& mesh = find_by_identifier_safe(meshes, identifier);
				mesh.command = std::any_cast<GpuCommand>(future_value.get());

				pending_buffers.erase(pending_buffers.begin() + i);
			}
			else
			{
				i++;
			}
		}
	}

	MeshWrapper ObjectManager::create_mesh(MeshCreateInfo&& createInfo)
	{
		BufferWrapper vertex_buffer = BufferManager::CreateBuffer(BufferCreateInfo
		{
			.type       = BufferType::eVertex,
			.memoryType = BufferMemoryType::eGpuStatic,
			.pData      = createInfo.vertices.data(),
			.dataSize   = static_cast<unsigned>(createInfo.vertices.size() * sizeof(Vertex)),
		});

		BufferWrapper index_buffer = BufferManager::CreateBuffer(BufferCreateInfo
		{
			.type       = BufferType::eIndex,
			.memoryType = BufferMemoryType::eGpuStatic,
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
			.vertices     = {},
			.indices      = {},
		};

		auto identifier = object_data.identifier;

		meshes.push_back(std::move(object_data));
		return MeshWrapper { *this, identifier };
	}

	void ObjectManager::rebuild_mesh_command(DrawableObjectData& object)
	{
		if(object.command.exists())
		{
			object.command.destroy();
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

		auto res = CommandSubmitter::RecordAsync([&object, graphics_layout, swapchain](VkCommandBuffer buffer)
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

		pending_buffers.push_back({ object.identifier, res.share() });
	}

	std::vector<VkCommandBuffer> ObjectManager::mesh_commands() const
	{
		auto final_handles = std::vector<VkCommandBuffer>();
		for(int i = 0; i < meshes.size(); i++)
		{
			if(meshes[i].command.handle() != VK_NULL_HANDLE)
			{
				final_handles.push_back(meshes[i].command.handle());
			}
		}
		return final_handles;
	}
}
