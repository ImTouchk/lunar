#include "utils/identifier.hpp"
#include "utils/debug.hpp"
#include "vk_renderer.hpp"
#include "vk_buffer.hpp"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace Vk
{
	VkBufferUsageFlags get_usage_flags(BufferType type)
	{
		switch (type)
		{
		case BufferType::eIndex:   return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case BufferType::eVertex:  return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case BufferType::eTexture: return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		default: return {};
		}
	}

	VmaMemoryUsage get_memory_flags(BufferMemoryType type)
	{
		switch (type)
		{
		case BufferMemoryType::eGpuStatic:  return VMA_MEMORY_USAGE_GPU_ONLY;
		case BufferMemoryType::eGpuDynamic: return VMA_MEMORY_USAGE_CPU_TO_GPU;
		case BufferMemoryType::eCpuAny:     return VMA_MEMORY_USAGE_CPU_ONLY;
		default: return {};
		}
	}

	void create_real_buffer
	(
		VkBufferUsageFlags usageFlags, 
		VmaMemoryUsage memoryUsage, 
		unsigned int size, 
		VkBuffer& buffer, 
		VmaAllocation& allocation,
		MemoryAllocatorWrapper* pMemoryAllocator
	)
	{
		VkBufferCreateInfo buffer_create_info =
		{
			.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size        = size,
			.usage       = usageFlags,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE
		};

		VmaAllocationCreateInfo allocation_create_info =
		{
			.flags          = 0,
			.usage          = memoryUsage,
			.requiredFlags  = 0,
			.preferredFlags = 0,
			.memoryTypeBits = 0,
			.pool           = VK_NULL_HANDLE,
			.pUserData      = nullptr,
			.priority       = 0,
		};

		VkResult result;
		result = vmaCreateBuffer(GetMemoryAllocator(), &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Buffer creation fail (vmaCreateBuffer didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-Buffer-CreationFail");
		}
	}

	void BufferManager::create(BufferManagerCreateInfo&& createInfo)
	{
        assert(not active);
        assert(createInfo.pCmdSubmitter != nullptr);

        pCmdSubmitter = createInfo.pCmdSubmitter;

		active = true;

		CDebug::Log("Vulkan Renderer | Buffer manager created.");
	}

	void BufferManager::destroy()
	{
		assert(active == true);
		
		// TODO: Improve this code

		while (!buffers.empty())
		{
			vmaDestroyBuffer(GetMemoryAllocator(), buffers[0].handle, buffers[0].memory);
			delete_element_with_identifier(buffers, buffers[0].identifier);
		}

        pCmdSubmitter = nullptr;
		buffers.clear();

		active = false;
	}

	BufferWrapper BufferManager::create_buffer(BufferCreateInfo&& createInfo)
	{
		assert(active == true);
		assert(createInfo.type != BufferType::eUnknown);
		assert(createInfo.memoryType != BufferMemoryType::eUnknown);
		assert(createInfo.pData != nullptr);
		assert(createInfo.dataSize != 0);

		BufferData new_buffer =
		{
			.identifier = get_unique_number(),
			.handle     = VK_NULL_HANDLE,
			.memory     = VK_NULL_HANDLE,
			.type       = createInfo.type,
			.memoryType = createInfo.memoryType,
			.dataSize   = createInfo.dataSize
		};

		create_real_buffer
		(
			get_usage_flags(createInfo.type),
			get_memory_flags(createInfo.memoryType),
			createInfo.dataSize,
			new_buffer.handle,
			new_buffer.memory,
			pMemoryAllocator
		);

		if (createInfo.memoryType == BufferMemoryType::eGpuStatic)
		{
			upload_to_gpu_buffer(createInfo.pData, createInfo.dataSize, new_buffer.handle);
		}
        else if (createInfo.memoryType == BufferMemoryType::eCpuAny)
        {
            void* mapped_buf = nullptr;
            vmaMapMemory(GetMemoryAllocator(), new_buffer.memory, &mapped_buf);
            memcpy(mapped_buf, createInfo.pData, createInfo.dataSize);
            vmaUnmapMemory(GetMemoryAllocator(), new_buffer.memory);
        }
		else
		{
			throw std::runtime_error("Not-Implemented");
		}

		unsigned identifier = new_buffer.identifier;

		buffers.push_back(std::move(new_buffer));
		return BufferWrapper(*this, identifier);
	}

	void BufferManager::upload_to_gpu_buffer(const void* pData, unsigned dataSize, VkBuffer dst)
	{
		VkBuffer staging_buffer;
		VmaAllocation staging_memory;

		create_real_buffer
		(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VMA_MEMORY_USAGE_CPU_ONLY,
			dataSize,
			staging_buffer,
			staging_memory,
			pMemoryAllocator
		);

		void* mapped_buf = nullptr;
		vmaMapMemory(GetMemoryAllocator(), staging_memory, &mapped_buf);
		memcpy(mapped_buf, pData, dataSize);
		vmaUnmapMemory(GetMemoryAllocator(), staging_memory);

		VkBufferCopy buffer_copy_region =
		{
			.srcOffset = 0,
			.dstOffset = 0,
			.size      = dataSize
		};

        std::promise<bool> cmd_sent = {};
        std::future<bool> cmd_exec = cmd_sent.get_future();
        pCmdSubmitter->submit([staging_buffer, dst, buffer_copy_region](VkCommandBuffer buffer)
        {
            vkCmdCopyBuffer(buffer, staging_buffer, dst, 1, &buffer_copy_region);
        }, cmd_sent);

        cmd_exec.wait();

		vmaDestroyBuffer(GetMemoryAllocator(), staging_buffer, staging_memory);
	}

	BufferWrapper::BufferWrapper(BufferManager& bufferManager, unsigned handle)
		: bufferManager(bufferManager),
		identifier(handle)
	{
		buffer = find_by_identifier_safe(bufferManager.buffers, identifier).handle;
	}

	void BufferWrapper::update(const void* pNewData, unsigned size)
	{	
		auto& data = find_by_identifier_safe(bufferManager.buffers, identifier);
		
		if (size > data.dataSize)
		{
			vmaDestroyBuffer(GetMemoryAllocator(), data.handle, data.memory);
			
			create_real_buffer
			(
				get_usage_flags(data.type),
				get_memory_flags(data.memoryType),
				size,
				data.handle,
				data.memory,
				bufferManager.pMemoryAllocator
			);

			data.dataSize = size;
		}

		if (data.memoryType == BufferMemoryType::eGpuStatic)
		{
			bufferManager.upload_to_gpu_buffer(pNewData, size, data.handle);
		}
		else
		{
			throw std::runtime_error("Not-Implemented");
		}
	}

	void BufferWrapper::destroy()
	{
		auto& data = find_by_identifier_safe(bufferManager.buffers, identifier);
		vmaDestroyBuffer(GetMemoryAllocator(), data.handle, data.memory);
		delete_element_with_identifier(bufferManager.buffers, identifier);

		identifier = 0;
	}

	VkBuffer BufferWrapper::handle() const
	{
		auto& data = find_by_identifier_safe(bufferManager.buffers, identifier);
		return data.handle;
	}

	VmaAllocation BufferWrapper::memory_handle() const
	{
		auto& data = find_by_identifier_safe(bufferManager.buffers, identifier);
		return data.memory;
	}
}
