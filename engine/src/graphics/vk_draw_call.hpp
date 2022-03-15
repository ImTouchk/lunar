#pragma once
#include "vk_forward_decl.hpp"
#include <vulkan/vulkan.h>
#include <vector>

namespace Vk
{
	struct RenderCallManagerCreateInfo
	{
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
		SwapchainWrapper* pSwapchain     = nullptr;
		SurfaceWrapper* pSurface         = nullptr;
		ObjectManager* pObjectManager    = nullptr;
		SyncObjectsWrapper* pSyncObjects = nullptr;

		bool active = false;

		size_t current_frame = 0;
	};

}