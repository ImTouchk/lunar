#pragma once
#include "render/mesh.hpp"
#include "vk_buffer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>
#include <vector>

namespace Vk
{
	struct LogicalDeviceWrapper;
	struct MemoryAllocatorWrapper;
	struct SwapchainWrapper;
	struct SurfaceWrapper;
	struct ShaderManager;
	class BufferManager;

	struct ObjectManagerCreateInfo
	{
		LogicalDeviceWrapper* pDevice;
		MemoryAllocatorWrapper* pMemoryAllocator;
		SwapchainWrapper* pSwapchain;
		SurfaceWrapper* pSurface;
		ShaderManager* pShaderManager;
		BufferManager* pBufferManager;
	};

	struct DrawableObjectData
	{
		unsigned identifier;
		MeshType type;
		Shader shader;
		BufferWrapper vertexBuffer;
		BufferWrapper indexBuffer;
		bool wasModified;
		unsigned vertexCount;
		unsigned indexCount;
		glm::mat4 transform;
		
		std::optional<std::vector<Vertex>> vertices;
		std::optional<std::vector<unsigned>> indices;
	};

	class ObjectManager
	{
	public:
		friend class MeshWrapper;

		ObjectManager() = default;
		~ObjectManager() = default;

		void create(ObjectManagerCreateInfo&& createInfo);
		void destroy();

		void update();
		void handle_resize();

		bool cmd_buffers_need_rebuilding();
		const std::vector<VkCommandBuffer>& mesh_commands() const;

		MeshWrapper create_mesh(MeshCreateInfo&& createInfo);

	private:
		void create_cmd_buffers(VkCommandPool pool, VkCommandBuffer* pBuffer, unsigned count);
		void rebuild_cmd_buffers();

	private:
		LogicalDeviceWrapper* pDevice            = nullptr;
		MemoryAllocatorWrapper* pMemoryAllocator = nullptr;
		SwapchainWrapper* pSwapchain             = nullptr;
		SurfaceWrapper* pSurface                 = nullptr;
		ShaderManager* pShaderManager            = nullptr;
		BufferManager* pBufferManager            = nullptr;

		bool active = false;

		std::vector<DrawableObjectData> meshes = {};

		VkRect2D scissor    = {};
		VkViewport viewport = {};

		VkCommandPool command_pool                        = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> mesh_command_buffers = {};
		bool command_buffers_modified                     = true; // needs to be true so the primary command buffers get built at least once
	};
}
