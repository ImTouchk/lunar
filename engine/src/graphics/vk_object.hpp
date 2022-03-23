#pragma once
#include "render/mesh.hpp"
#include "vk_forward_decl.hpp"
#include "vk_buffer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <optional>
#include <vector>
#include <future>

namespace Vk
{
	struct ObjectManagerCreateInfo
	{
		SwapchainWrapper* pSwapchain;
		SurfaceWrapper* pSurface;
		ShaderManager* pShaderManager;
	};

	struct DrawableObjectData
	{
		Identifier identifier;
		MeshType type;
		ShaderWrapper shader;
		BufferWrapper vertexBuffer;
		BufferWrapper indexBuffer;
		bool wasModified;
		unsigned vertexCount;
		unsigned indexCount;
		glm::mat4 transform;
		VkCommandBuffer command;
		
		std::optional<std::vector<Vertex>> vertices;
		std::optional<std::vector<unsigned>> indices;
	};

	class ObjectManager
	{
	public:
		friend class ::MeshWrapper;

		ObjectManager() = default;
		~ObjectManager() = default;

		void create(ObjectManagerCreateInfo&& createInfo);
		void destroy();

		void update();

		[[nodiscard]] size_t mesh_count() const;
		[[nodiscard]] std::vector<VkCommandBuffer> mesh_commands() const;

		MeshWrapper create_mesh(MeshCreateInfo&& createInfo);

	private:
		void rebuild_mesh_command(DrawableObjectData& object);

	private:
		SwapchainWrapper* pSwapchain  = nullptr;
		SurfaceWrapper* pSurface      = nullptr;
		ShaderManager* pShaderManager = nullptr;

		bool active = false;

		std::vector<DrawableObjectData> meshes = {};
		std::vector<VkCommandBuffer> command_buffers = {};
		std::vector<std::pair<Identifier, std::shared_future<VkCommandBuffer>>> pending_buffers = {};

		VkRect2D scissor    = {};
		VkViewport viewport = {};
	};
}
