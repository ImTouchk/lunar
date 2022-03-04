#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Vk
{
	class ObjectManager;
	struct LogicalDeviceWrapper;
	struct SwapchainWrapper;
	struct SurfaceWrapper;
	class SyncObjectsWrapper;

	struct RenderCallManagerCreateInfo
	{
		LogicalDeviceWrapper* pDevice;
		SurfaceWrapper* pSurface;
		SwapchainWrapper* pSwapchain;
		ObjectManager* pObjectManager;
		SyncObjectsWrapper* pSyncObjects;
	};

	class RenderCallManager
	{
	public:
		RenderCallManager() = default;
		~RenderCallManager() = default;

		void create(RenderCallManagerCreateInfo&& createInfo);
		void destroy();

		void update();
		void draw(uint32_t image);

		size_t current_image() const;

	private:
		LogicalDeviceWrapper* pDevice    = nullptr;
		SwapchainWrapper* pSwapchain     = nullptr;
		SurfaceWrapper* pSurface         = nullptr;
		ObjectManager* pObjectManager    = nullptr;
		SyncObjectsWrapper* pSyncObjects = nullptr;

		bool active = false;

		VkCommandPool cmd_pool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> cmd_buffers = {};
		size_t current_frame = 0;
	};

}