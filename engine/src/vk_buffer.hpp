#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace Vk
{
	struct SurfaceWrapper;
	struct LogicalDeviceWrapper;
	struct MemoryAllocatorWrapper;

	enum class BufferType
	{
		eUnknown = 0,
		eVertex,
		eIndex,
	};

	enum class BufferMemoryType
	{
		eUnknown = 0,
		eGpuStatic,
		eGpuDynamic,
	};

	struct BufferCreateInfo
	{
		BufferType type;
		BufferMemoryType memoryType;
		void* pData;
		unsigned dataSize;
	};

	struct BufferManagerCreateInfo
	{
		LogicalDeviceWrapper* pDevice;
		SurfaceWrapper* pSurface;
		MemoryAllocatorWrapper* pMemoryAllocator;
	};

	struct BufferData
	{
		unsigned identifier;
		VkBuffer handle;
		VmaAllocation memory;
	};

	class BufferManager
	{
	public:
		friend class BufferWrapper;
		
		BufferManager() = default;
		~BufferManager() = default;

		void create(BufferManagerCreateInfo&& createInfo);
		void destroy();

		BufferWrapper create_buffer(BufferCreateInfo&& createInfo);

	private:
		void upload_to_gpu_buffer(void* pData, unsigned dataSize, VkBuffer dst);

	private:
		LogicalDeviceWrapper* pDevice = nullptr;
		MemoryAllocatorWrapper* pMemoryAllocator = nullptr;
		bool active = false;

		VkCommandPool command_pool = VK_NULL_HANDLE;

		struct
		{
			VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
			VkBuffer buffer = VK_NULL_HANDLE;
			VmaAllocation memory = VK_NULL_HANDLE;
			unsigned size = 0;
		} staging;

		std::vector<BufferData> buffers = {};
	};

	class BufferWrapper
	{
	public:
		BufferWrapper(BufferManager& bufferManager, unsigned handle);
		~BufferWrapper() = default;

		void destroy();
		void update(void* pNewData, unsigned size);

		[[nodiscard]] VkBuffer handle() const;
		[[nodiscard]] VmaAllocation memory_handle() const;
	private:
		BufferData& get_handle_safe() const;

	private:
		unsigned identifier;
		BufferManager& bufferManager;
	};
}