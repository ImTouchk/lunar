#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug.hpp>
#include <vk_mem_alloc.h>

namespace Render
{
	VulkanBufferBuilder& VulkanBufferBuilder::useRenderContext(VulkanContext* context)
	{
		this->context = context;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::addFlags(VulkanBufferFlags flags)
	{
		this->flags = this->flags | flags;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::setMemoryUsage(VmaMemoryUsage usage)
	{
		memoryUsage = usage;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::addUsageFlags(vk::BufferUsageFlags flags)
	{
		usageFlags |= flags;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::setAllocationSize(size_t size)
	{
		allocSize = size;
		return *this;
	}

	VulkanBufferBuilder& VulkanBufferBuilder::build()
	{
		auto buffer_info = VkBufferCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.pNext = nullptr,
			.size  = allocSize,
			.usage = static_cast<VkBufferUsageFlags>(usageFlags)
		};

		auto allocation_info = VmaAllocationCreateInfo
		{
			.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
			.usage = memoryUsage,
		};

		VkBuffer          _buffer     = VK_NULL_HANDLE;
		VmaAllocation     _alloc      = VK_NULL_HANDLE;
		VmaAllocationInfo _alloc_info = {};
		VkResult          _result;

		_result = vmaCreateBuffer(
			context->getAllocator(),
			&buffer_info,
			&allocation_info,
			&_buffer,
			&_alloc,
			&_alloc_info
		);

		if (_result != VK_SUCCESS)
		{
			DEBUG_ERROR("Failed to create GPU buffer: {}", static_cast<uint32_t>(_result));
			//return VulkanBuffer();
		}

		result = VulkanBuffer(context, _buffer, _alloc, _alloc_info);
		return *this;
		//return VulkanBuffer(context, _buffer, _alloc, _alloc_info);
	}

	vk::Buffer VulkanBufferBuilder::getBuffer()
	{
		return result.handle;
	}

	VmaAllocation VulkanBufferBuilder::getAllocation()
	{
		return result.allocation;
	}

	VmaAllocationInfo VulkanBufferBuilder::getAllocationInfo()
	{
		return result.info;
	}

	VulkanBuffer VulkanBufferBuilder::getResult()
	{
		return std::move(result);
	}

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

		DEBUG_LOG("Managed Vulkan buffer destroyed (handle: {})", (void*)handle);
		destroy();
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

	void VulkanBuffer::destroy()
	{
		DEBUG_ASSERT(handle != VK_NULL_HANDLE, "VulkanBuffer object not initialized");
		
		vmaUnmapMemory(context->allocator, allocation);
		vmaDestroyBuffer(context->allocator, handle, allocation);

		context = nullptr;
		handle = VK_NULL_HANDLE;
		allocation = VK_NULL_HANDLE;
		info = {};
	}

	VulkanMeshBuilder& VulkanMeshBuilder::setVertices(const std::span<Vertex>& vertices)
	{
		this->vertices = std::vector<Vertex>(vertices.begin(), vertices.end());
		return *this;
	}

	VulkanMeshBuilder& VulkanMeshBuilder::setIndices(const std::span<uint32_t>& indices)
	{
		this->indices = std::vector<uint32_t>(indices.begin(), indices.end());
		return *this;
	}

	VulkanMeshBuilder& VulkanMeshBuilder::useRenderContext(VulkanContext* context)
	{
		this->context = context;
		return *this;
	}

	VulkanMeshBuilder& VulkanMeshBuilder::build()
	{
		const size_t vertex_buf_size = vertices.size() * sizeof(Vertex);
		const size_t index_buf_size  = indices.size() * sizeof(uint32_t);

		auto buffer_builder = VulkanBufferBuilder();
		indexBuf = buffer_builder
			.useRenderContext(context)
			.addUsageFlags(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer)
			.setMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.setAllocationSize(index_buf_size)
			.build()
			.getResult();

		vertexBuf = buffer_builder
			.useRenderContext(context)
			.addUsageFlags(vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eStorageBuffer)
			.addUsageFlags(vk::BufferUsageFlagBits::eShaderDeviceAddress)
			.setMemoryUsage(VMA_MEMORY_USAGE_GPU_ONLY)
			.setAllocationSize(vertex_buf_size)
			.build()
			.getResult();

		auto staging_buffer = buffer_builder
			.useRenderContext(context)
			.addUsageFlags(vk::BufferUsageFlagBits::eTransferSrc)
			.setMemoryUsage(VMA_MEMORY_USAGE_CPU_TO_GPU)
			.setAllocationSize(vertex_buf_size + index_buf_size)
			.build()
			.getResult();

		auto device_address_info = vk::BufferDeviceAddressInfo{ .buffer = vertexBuf.handle };
		vertexBufAddr  = context->getDevice().getBufferAddress(device_address_info);

		void* pData;
		vmaMapMemory(context->getAllocator(), staging_buffer.allocation, &pData);
		memcpy(pData, vertices.data(), vertex_buf_size);
		memcpy((char*)pData + vertex_buf_size, indices.data(), index_buf_size);

		context->immediateSubmit([&](vk::CommandBuffer cmd) {
			auto vertex_copy = vk::BufferCopy
			{
				.srcOffset = 0,
				.dstOffset = 0,
				.size      = vertex_buf_size
			};

			cmd.copyBuffer(staging_buffer.handle, vertexBuf.handle, vertex_copy);

			auto index_copy = vk::BufferCopy
			{
				.srcOffset = vertex_buf_size,
				.dstOffset = 0,
				.size     = index_buf_size
			};

			cmd.copyBuffer(staging_buffer.handle, indexBuf.handle, index_copy);
		}, true);

		//return VulkanMesh(index_buffer, vertex_buffer, vertex_buffer_addr);
		return *this;
	}

	VulkanBuffer VulkanMeshBuilder::getIndexBuffer()
	{
		return std::move(indexBuf);
	}

	VulkanBuffer VulkanMeshBuilder::getVertexBuffer()
	{
		return std::move(vertexBuf);
	}

	vk::DeviceAddress VulkanMeshBuilder::getVertexBufferAddress()
	{
		return vertexBufAddr;
	}

	VulkanMesh::VulkanMesh(VulkanBuffer& idx, VulkanBuffer& vert, vk::DeviceAddress vertBufAddr)
		: vertexBuffer(std::move(vert)),
		vertexBufferAddr(vertBufAddr),
		indexBuffer(std::move(idx))
	{
	}

	VulkanBuffer::operator vk::Buffer& ()
	{
		return handle;
	}
}
