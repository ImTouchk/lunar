#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug.hpp>
#include <vk_mem_alloc.h>

namespace Render
{
	VulkanBuffer::VulkanBuffer(
		VulkanContext* context,
		vk::Buffer buffer,
		VmaAllocation allocation,
		VmaAllocationInfo info
	) : context(context),
		handle(buffer),
		allocation(allocation),
		info(info)
	{
	}

	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
		: context(other.context),
		handle(other.handle),
		allocation(other.allocation),
		info(other.info)
	{
		other.context = nullptr;
		other.handle = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.info = {};
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (handle == VK_NULL_HANDLE)
			return;

		context->device.destroyBuffer(handle);
		vmaDestroyBuffer(context->allocator, handle, allocation);
	}

	VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
	{
		if (this != &other)
		{
			context = other.context;
			handle = other.handle;
			allocation = other.allocation;
			info = other.info;

			other.context = nullptr;
			other.handle = VK_NULL_HANDLE;
			other.allocation = VK_NULL_HANDLE;
			other.info = {};
		}
		return *this;
	}

	VulkanBuffer VulkanContext::createBuffer(size_t allocationSize, vk::BufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage)
	{
		auto buffer_info = vk::BufferCreateInfo
		{
			.size = allocationSize,
			.usage = usageFlags,
		};

		auto allocation_info = VmaAllocationCreateInfo
		{
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = memoryUsage,
		};

		VkBuffer _buffer = VK_NULL_HANDLE;
		VmaAllocation _alloc = VK_NULL_HANDLE;
		VmaAllocationInfo _alloc_info = {};
		VkResult result;
		result = vmaCreateBuffer(
			allocator,
			&buffer_info.operator const VkBufferCreateInfo & (),
			&allocation_info,
			&_buffer,
			&_alloc,
			&_alloc_info
		);

		if (result != VK_SUCCESS)
		{
			DEBUG_ERROR("Failed to create GPU buffer: {}", static_cast<uint32_t>(result));
			return VulkanBuffer();
		}

		return VulkanBuffer(this, _buffer, _alloc, _alloc_info);
	}

	VulkanMesh VulkanContext::uploadMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		const size_t vertex_buf_size = vertices.size() * sizeof(Vertex);
		const size_t index_buf_size = indices.size() * sizeof(uint32_t);

		auto index_buffer = createBuffer(
			index_buf_size,
			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eIndexBuffer,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		auto vertex_buffer = createBuffer(
			vertex_buf_size,
			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eStorageBuffer |
			vk::BufferUsageFlagBits::eShaderDeviceAddress,
			VMA_MEMORY_USAGE_GPU_ONLY
		);

		auto device_address_info = vk::BufferDeviceAddressInfo { .buffer = vertex_buffer.handle };
		auto vertex_buffer_addr = device.getBufferAddress(device_address_info);

		auto staging_buffer = createBuffer(
			vertex_buf_size + index_buf_size,
			vk::BufferUsageFlagBits::eTransferSrc,
			VMA_MEMORY_USAGE_CPU_ONLY
		);

		void* pData;
		vmaMapMemory(allocator, staging_buffer.allocation, &pData);
		memcpy(pData, vertices.data(), vertex_buf_size);
		memcpy((char*)pData + vertex_buf_size, indices.data(), index_buf_size);

		immediateSubmit([&](vk::CommandBuffer cmd) {
			auto vertex_copy = vk::BufferCopy 
			{
				.srcOffset = 0,
				.dstOffset = 0,
				.size      = vertex_buf_size
			};

			cmd.copyBuffer(staging_buffer.handle, vertex_buffer.handle, vertex_copy);

			auto index_copy = vk::BufferCopy
			{
				.srcOffset = vertex_buf_size,
				.dstOffset = 0,
				.size      = index_buf_size
			};

			cmd.copyBuffer(staging_buffer.handle, index_buffer.handle, index_copy);
		}, true);

		return VulkanMesh
		{
			.indexBuffer      = std::move(index_buffer),
			.vertexBuffer     = std::move(vertex_buffer),
			.vertexBufferAddr = vertex_buffer_addr,
		};
	}
}
