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
		case BufferType::eIndex:  return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case BufferType::eVertex: return VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		default: return {};
		}
	}

	VmaMemoryUsage get_memory_flags(BufferMemoryType type)
	{
		switch (type)
		{
		case BufferMemoryType::eGpuStatic:  return VMA_MEMORY_USAGE_GPU_ONLY;
		case BufferMemoryType::eGpuDynamic: return VMA_MEMORY_USAGE_CPU_TO_GPU;
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
		result = vmaCreateBuffer(pMemoryAllocator->handle(), &buffer_create_info, &allocation_create_info, &buffer, &allocation, nullptr);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Buffer creation fail (vmaCreateBuffer didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-Buffer-CreationFail");
		}
	}

	void BufferManager::create(BufferManagerCreateInfo&& createInfo)
	{
		assert(createInfo.pDevice != nullptr);
		assert(createInfo.pSurface != nullptr);
		assert(createInfo.pMemoryAllocator != nullptr);
		assert(not active);

		pDevice = createInfo.pDevice;
		pMemoryAllocator = createInfo.pMemoryAllocator;

		auto& queue_indices = QueueFamilyIndices::query(GetRenderingDevice(), createInfo.pSurface->handle());
		assert(queue_indices.is_complete());

		VkCommandPoolCreateInfo pool_create_info =
		{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			.queueFamilyIndex = queue_indices.graphics.value()
		};

		VkResult result;
		result = vkCreateCommandPool(pDevice->handle(), &pool_create_info, nullptr, &command_pool);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Buffer manager creation failed (vkCreateCommandPool didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-BufferManager-CreationFail");
		}

		VkCommandBufferAllocateInfo command_buffer_allocate_info =
		{
			.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool        = command_pool,
			.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};

		result = vkAllocateCommandBuffers(pDevice->handle(), &command_buffer_allocate_info, &staging_command);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | Buffer manager creation failed (vkAllocateCommandBuffers didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-BufferManager-CreationFail");
		}

		active = true;

		CDebug::Log("Vulkan Renderer | Buffer manager created.");
	}

	void BufferManager::destroy()
	{
		assert(active == true);
		
		vkFreeCommandBuffers(pDevice->handle(), command_pool, 1, &staging_command);
		vkDestroyCommandPool(pDevice->handle(), command_pool, nullptr);
		
		// TODO: Destroy all buffers
		
		pDevice = nullptr;
		pMemoryAllocator = nullptr;
		command_pool = VK_NULL_HANDLE;
		staging_command = VK_NULL_HANDLE;
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
		vmaMapMemory(pMemoryAllocator->handle(), staging_memory, &mapped_buf);
		memcpy(mapped_buf, pData, dataSize);
		vmaUnmapMemory(pMemoryAllocator->handle(), staging_memory);

		VkCommandBufferBeginInfo command_buffer_begin_info =
		{
			.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext            = nullptr, 
			.flags            = 0,
			.pInheritanceInfo = nullptr
		};

		VkResult result;
		result = vkBeginCommandBuffer(staging_command, &command_buffer_begin_info);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | GPU upload of buffer failed (vkBeginCommandBuffer didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-BufferManager-UploadFail");
		}

		VkBufferCopy buffer_copy_region =
		{
			.srcOffset = 0,
			.dstOffset = 0,
			.size      = dataSize
		};

		vkCmdCopyBuffer(staging_command, staging_buffer, dst, 1, &buffer_copy_region);
		
		result = vkEndCommandBuffer(staging_command);
		if (result != VK_SUCCESS)
		{
			CDebug::Error("Vulkan Renderer | GPU upload of buffer failed (vkEndCommandBuffer didn't return VK_SUCCESS).");
			throw std::runtime_error("Renderer-Vulkan-BufferManager-UploadFail");
		}

		VkSubmitInfo submit_info =
		{
			.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.commandBufferCount = 1,
			.pCommandBuffers    = &staging_command
		};

		vkQueueSubmit(pDevice->graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(pDevice->graphics_queue());

		vkResetCommandBuffer(staging_command, 0);

		vmaDestroyBuffer(pMemoryAllocator->handle(), staging_buffer, staging_memory);
	}

	BufferWrapper::BufferWrapper(BufferManager& bufferManager, unsigned handle)
		: bufferManager(bufferManager),
		identifier(handle)
	{
		buffer = get_handle_safe().handle;
	}

	BufferData& BufferWrapper::get_handle_safe() const
	{
		assert(identifier != 0);
		BufferData* buffer = find_by_identifier(bufferManager.buffers, identifier);
		if (buffer == nullptr)
		{
			throw std::runtime_error("Renderer-Vulkan-BufferWrapper-Nullptr");
		}

		return *buffer;
	}

	void BufferWrapper::update(const void* pNewData, unsigned size)
	{	
		auto& data = get_handle_safe();
		
		if (size > data.dataSize)
		{
			vmaDestroyBuffer(bufferManager.pMemoryAllocator->handle(), data.handle, data.memory);
			
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
		auto& data = get_handle_safe();
		vmaDestroyBuffer(bufferManager.pMemoryAllocator->handle(), data.handle, data.memory);
		delete_element_with_identifier(bufferManager.buffers, identifier);

		identifier = 0;
	}

	VkBuffer BufferWrapper::handle() const
	{
		auto& data = get_handle_safe();
		return data.handle;
	}

	VmaAllocation BufferWrapper::memory_handle() const
	{
		auto& data = get_handle_safe();
		return data.memory;
	}
}
