#pragma once
#include <lunar/api.hpp>

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <initializer_list>
#include <variant>
#include <vector>

// TODO: Move to namespace Render::imp

namespace Render
{
	class VulkanContext;

	enum class LUNAR_API VulkanQueueType : size_t
	{
		eGraphics = 0,
		ePresent = 1,
		eTransfer = 2
	};

	struct LUNAR_API VulkanSemaphoreSubmit
	{
		using SubmitValue = std::variant<vk::Semaphore, vk::SemaphoreSubmitInfo>;

		VulkanSemaphoreSubmit(const std::initializer_list<SubmitValue>& semaphores);
		VulkanSemaphoreSubmit() = default;
		~VulkanSemaphoreSubmit() = default;

		[[nodiscard]] uint32_t size() const;
		[[nodiscard]] const vk::SemaphoreSubmitInfo* data() const;

		std::vector<vk::SemaphoreSubmitInfo>* operator->();
		std::vector<vk::SemaphoreSubmitInfo> value = { };
	};

	class LUNAR_API VulkanCommandBuffer
	{
	public:
		VulkanCommandBuffer(VulkanContext& context, vk::CommandBuffer buffer, vk::Fence fence);
		VulkanCommandBuffer();
		~VulkanCommandBuffer();

		void begin();

		void submit(
			const VulkanSemaphoreSubmit& waitSemaphores,
			const VulkanSemaphoreSubmit& signalSemaphores,
			bool waitForFinish = false
		);

		void destroy();

		VulkanCommandBuffer(VulkanCommandBuffer&&) noexcept;
		VulkanCommandBuffer& operator=(VulkanCommandBuffer&&) noexcept;

		[[nodiscard]] operator vk::CommandBuffer& ();
		[[nodiscard]] vk::CommandBuffer* operator->();


		vk::CommandBuffer value;
		vk::Fence ready;
	private:
		VulkanContext* context;
		int state;
	};

	class LUNAR_API VulkanCommandPool
	{
	public:
		VulkanCommandPool(VulkanContext& context, vk::CommandPool pool);
		VulkanCommandPool();
		~VulkanCommandPool();

		VulkanCommandPool(VulkanCommandPool&&) noexcept;
		VulkanCommandPool& operator=(VulkanCommandPool&&) noexcept;

		[[nodiscard]] VulkanCommandBuffer allocateBuffer(vk::CommandBufferLevel level);

		void destroy();

		vk::CommandPool value;
	private:
		VulkanContext* context;
	};
}
