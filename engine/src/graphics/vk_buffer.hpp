#pragma once
#include "vk_forward_decl.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

namespace Vk
{
	enum class BufferType
	{
		eUnknown = 0,
		eVertex,
		eIndex,
		eTexture,
	};

	enum class BufferMemoryType
	{
		eUnknown = 0,
		eGpuStatic,
		eGpuDynamic,
		eCpuAny,
	};

	struct BufferCreateInfo
	{
		BufferType type;
		BufferMemoryType memoryType;
		const void* pData;
		unsigned dataSize;
	};

	struct BufferManagerCreateInfo
	{
		LogicalDeviceWrapper* pDevice;
        CmdSubmitter* pCmdSubmitter;
		MemoryAllocatorWrapper* pMemoryAllocator;
	};

	struct BufferData
	{
		unsigned identifier;
		VkBuffer handle;
		VmaAllocation memory;
		BufferType type;
		BufferMemoryType memoryType;
		unsigned dataSize;
	};

    class BufferWrapper;

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
		void upload_to_gpu_buffer(const void* pData, unsigned dataSize, VkBuffer dst);

	private:
		LogicalDeviceWrapper* pDevice            = nullptr;
		MemoryAllocatorWrapper* pMemoryAllocator = nullptr;
        CmdSubmitter* pCmdSubmitter              = nullptr;
		bool active = false;

		std::vector<BufferData> buffers = {};
	};

	class BufferWrapper
	{
	public:
		BufferWrapper(BufferManager& bufferManager, unsigned handle);
		~BufferWrapper() = default;

		void destroy();
		void update(const void* pNewData, unsigned size);

		[[nodiscard]] VkBuffer handle() const;
		[[nodiscard]] VmaAllocation memory_handle() const;
	private:
		unsigned identifier;
		BufferManager& bufferManager;
		VkBuffer buffer;
	};
}