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

	struct BufferData
	{
		unsigned identifier;
		VkBuffer handle;
		VmaAllocation memory;
		BufferType type;
		BufferMemoryType memoryType;
		unsigned dataSize;
	};

	class BufferWrapper
	{
	public:
		BufferWrapper(unsigned identifier);
		~BufferWrapper() = default;

		void destroy();
		void update(const void* pNewData, unsigned size);

		[[nodiscard]] VkBuffer handle() const;
		[[nodiscard]] VmaAllocation memory_handle() const;

	private:
		unsigned identifier;
	};

	namespace BufferManager
	{
		void Initialize();
		void Destroy();

		BufferWrapper CreateBuffer(BufferCreateInfo&& createInfo);
	}
}