#pragma once
#include <lunar/render/internal/vk_base.hpp>
#include <lunar/render/internal/vk_buffer.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace Render
{
	class LUNAR_API VulkanImage
	{
	public:
		VulkanImage() = default;
		~VulkanImage();

		void destroy();

		VulkanImage(VulkanImage&&) noexcept;
		VulkanImage& operator=(VulkanImage&&) noexcept;

		vk::Image      handle      = VK_NULL_HANDLE;
		vk::ImageView  view        = VK_NULL_HANDLE;
		VmaAllocation  allocation  = VK_NULL_HANDLE;
		vk::Extent3D   extent      = {};
		vk::Format     format      = {};
	private:
		VulkanContext* context     = nullptr;

		friend struct VulkanImageBuilder;
	};

	struct LUNAR_API VulkanImageBuilder
	{
	public:
		VulkanImageBuilder() = default;
		~VulkanImageBuilder() = default;

		VulkanImageBuilder& useVulkanContext(VulkanContext* ctx);
		VulkanImageBuilder& setFormat(vk::Format fmt);
		VulkanImageBuilder& setExtent(vk::Extent3D extent);
		VulkanImageBuilder& addUsageFlags(vk::ImageUsageFlags flags);
		VulkanImageBuilder& setUsageFlags(vk::ImageUsageFlags flags);
		VulkanImageBuilder& build();
		VulkanImage getResult();

	private:
		VulkanContext*      context    = nullptr;
		vk::Format          format     = {};
		vk::Extent3D        extent     = {};
		vk::ImageUsageFlags usageFlags = {};
		VulkanImage         result     = {};
	};
}
