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
		assert(createInfo.pDevice != nullptr);
		assert(createInfo.pSurface != nullptr);
		assert(createInfo.pSwapchain != nullptr);
		assert(createInfo.pShaderManager != nullptr);
		assert(createInfo.pBufferManager != nullptr);
		assert(createInfo.pMemoryAllocator != nullptr);

		pDevice = createInfo.pDevice;
		pSurface = createInfo.pSurface;
		pSwapchain = createInfo.pSwapchain;
		pShaderManager = createInfo.pShaderManager;
		pBufferManager = createInfo.pBufferManager;
		pMemoryAllocator = createInfo.pMemoryAllocator;

		auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), pSurface->handle());

		VkCommandPoolCreateInfo pool_create_info =
		{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queue_indices.graphics.value()
		};

		VkResult result;
		result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &command_pool);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Object manager creation failed (vkCreateCommandPool didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-ObjectManager-CreationFail");
		}

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

		vkDestroyCommandPool(pDevice->handle(), command_pool, nullptr);

		meshes.clear();

		scissor  = {};
		viewport = {};
		active   = false;

		pDevice          = nullptr;
		pSurface         = nullptr;
		pSwapchain       = nullptr;
		pShaderManager   = nullptr;
		pMemoryAllocator = nullptr;
		pBufferManager   = nullptr;
	}

	void ObjectManager::update()
	{
		bool do_rebuild = false;
		
		if (mesh_command_buffers.size() != meshes.size())
		{
			if (mesh_command_buffers.empty())
			{
				mesh_command_buffers.resize(meshes.size(), VK_NULL_HANDLE);
				create_cmd_buffers(command_pool, mesh_command_buffers.data(), meshes.size());
				CDebug::Log("Vulkan Renderer | Created {} initial secondary command buffers.", meshes.size());
			}
			else
			{
				auto old_size = mesh_command_buffers.size();
				mesh_command_buffers.insert
				(
					mesh_command_buffers.end(),
					meshes.size() - old_size,
					VK_NULL_HANDLE
				);

				auto* pBuffer = mesh_command_buffers.data() + old_size;
				create_cmd_buffers(command_pool, mesh_command_buffers.data(), meshes.size());
				CDebug::Log("Vulkan Renderer | Created {} new secondary command buffers.", meshes.size() - old_size);
			}

			do_rebuild = true;
		}

		if (not do_rebuild)
		{
			for (const auto& mesh : meshes)
			{
				if (mesh.wasModified)
				{
					do_rebuild = true;
				}
			}
		}

		if (do_rebuild)
		{
			rebuild_cmd_buffers();
			command_buffers_modified = true;
		}
	}

	MeshWrapper ObjectManager::create_mesh(MeshCreateInfo&& createInfo)
	{
		BufferWrapper vertex_buffer = pBufferManager->create_buffer(BufferCreateInfo
		{
			.type       = BufferType::eVertex,
			.memoryType = BufferMemoryType::eGpuStatic,
			.pData      = createInfo.vertices.data(),
			.dataSize   = static_cast<unsigned>(createInfo.vertices.size() * sizeof(Vertex)),
		});

		BufferWrapper index_buffer = pBufferManager->create_buffer(BufferCreateInfo
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

		meshes.push_back(std::move(object_data));
		return {};
	}

	bool ObjectManager::cmd_buffers_need_rebuilding()
	{
		bool last_value = command_buffers_modified;
		command_buffers_modified = false;
		return last_value;
	}

	void ObjectManager::create_cmd_buffers(VkCommandPool pool, VkCommandBuffer* pBuffer, unsigned count)
	{
		VkCommandBufferAllocateInfo buffer_allocate_info =
		{
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool        = pool,
			.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = static_cast<uint32_t>(count)
		};

		VkResult result;
		result = vkAllocateCommandBuffers(pDevice->handle(), &buffer_allocate_info, pBuffer);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Failed to update object manager (vkAllocateCommandBuffers didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-ObjectManager-UpdateFail");
		}
	}

	void ObjectManager::rebuild_cmd_buffers()
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			auto& mesh = meshes[i];
			auto& command_buffer = mesh_command_buffers[i];

			if (!mesh.wasModified)
				continue;

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
				.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags            = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
				.pInheritanceInfo = &buffer_inheritance_info
			};

			VkResult result;
			result = vkBeginCommandBuffer(command_buffer, &buffer_begin_info);
			if (result != VK_SUCCESS)
			{
				CDebug::Error("Vulkan Renderer | Failed to update object manager (vkBeginCommandBuffer didn't return VK_SUCCESS).");
				throw std::runtime_error("Renderer-Vulkan-ObjectManager-UpdateFail");
			}

			auto graphics_layout = pShaderManager->get_graphics_layout();
			auto pipeline = pShaderManager->try_get(mesh.shader);
			if (pipeline != VK_NULL_HANDLE)
			{
				VkDeviceSize offsets[] = { 0 };

				auto index_buffer = mesh.indexBuffer.handle();
				auto vertex_buffer = mesh.vertexBuffer.handle();

				vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
				vkCmdPushConstants(command_buffer, graphics_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mesh.transform);
				vkCmdSetViewport(command_buffer, 0, 1, &pSwapchain->get_viewport());
				vkCmdSetScissor(command_buffer, 0, 1, &pSwapchain->get_scissor());
				vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offsets);
				vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT16);
				vkCmdDrawIndexed(command_buffer, mesh.indexCount, 1, 0, 0, 0);
			}

			result = vkEndCommandBuffer(command_buffer);
			if (result != VK_SUCCESS)
			{
				CDebug::Error("Vulkan Renderer | Failed to update object manager (vkEndCommandBuffer didn't return VK_SUCCESS).");
				throw std::runtime_error("Renderer-Vulkan-ObjectManager-UpdateFail");
			}

			mesh.wasModified = false;
		}
	}

	const std::vector<VkCommandBuffer>& ObjectManager::mesh_commands() const
	{
		return mesh_command_buffers;
	}
}