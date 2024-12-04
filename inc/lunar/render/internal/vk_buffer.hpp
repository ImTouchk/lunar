#pragma once
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

#include <lunar/render/internal/vk_base.hpp>

namespace Render
{
	class LUNAR_API VulkanBuffer
	{
	public:
		VulkanBuffer(
			VulkanContext* context,
			vk::Buffer buffer,
			VmaAllocation allocation,
			VmaAllocationInfo info
		);
		VulkanBuffer() = default;
		~VulkanBuffer();

		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;
		VulkanBuffer(VulkanBuffer&&) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&&) noexcept;

		void destroy();

		operator vk::Buffer& ();

		vk::Buffer        handle     = VK_NULL_HANDLE;
		VmaAllocation     allocation = VK_NULL_HANDLE;
		VmaAllocationInfo info       = {};
	private:
		VulkanContext* context = nullptr;
	};

	enum class LUNAR_API VulkanBufferFlags : uint32_t
	{
		eNone = 0,
		eAsyncUpload = 1
	};

	inline VulkanBufferFlags operator|(VulkanBufferFlags a, VulkanBufferFlags b)
	{
		return static_cast<VulkanBufferFlags>(
			static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
		);
	}

	inline bool operator&(VulkanBufferFlags a, VulkanBufferFlags b)
	{
		return static_cast<uint32_t>(a) & static_cast<uint32_t>(b);
	}

	struct LUNAR_API VulkanBufferBuilder
	{
		VulkanBufferBuilder() = default;
		~VulkanBufferBuilder() = default;

		VulkanBufferBuilder& useRenderContext(VulkanContext* ctx);
		VulkanBufferBuilder& addFlags(VulkanBufferFlags flags);
		VulkanBufferBuilder& addUsageFlags(vk::BufferUsageFlags flags);
		VulkanBufferBuilder& setAllocationSize(size_t size);
		VulkanBufferBuilder& setMemoryUsage(VmaMemoryUsage usage);
		VulkanBuffer build();
	private:
		VulkanContext*       context     = nullptr;
		VulkanBufferFlags    flags       = VulkanBufferFlags::eNone;
		vk::BufferUsageFlags usageFlags  = {};
		VmaMemoryUsage       memoryUsage = {};
		size_t               allocSize   = 0;
	};
}
