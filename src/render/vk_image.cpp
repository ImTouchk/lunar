#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug.hpp>

namespace Render
{
	VulkanImageBuilder& VulkanImageBuilder::useVulkanContext(VulkanContext* ctx)
	{
		context = ctx;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setFormat(vk::Format fmt)
	{
		format = fmt;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setExtent(vk::Extent3D extent)
	{
		this->extent = extent;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::setUsageFlags(vk::ImageUsageFlags flags)
	{
		usageFlags = flags;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::addUsageFlags(vk::ImageUsageFlags flags)
	{
		usageFlags |= flags;
		return *this;
	}

	VulkanImageBuilder& VulkanImageBuilder::build()
	{
		auto image_info = vk::ImageCreateInfo
		{
			.imageType   = vk::ImageType::e2D,
			.format      = format,
			.extent      = extent,
			.mipLevels   = 1,
			.arrayLayers = 1,
			.samples     = vk::SampleCountFlagBits::e1,
			.tiling      = vk::ImageTiling::eOptimal,
			.usage       = usageFlags
		};

		auto image_alloc_info = VmaAllocationCreateInfo
		{
			.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
			.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		VkImage       _image;
		VmaAllocation _allocation;
		VkResult      _result;
		_result = vmaCreateImage(
			context->getAllocator(), 
			&image_info.operator const VkImageCreateInfo & (), 
			&image_alloc_info, 
			&_image, 
			&_allocation, 
			nullptr
		);

		if (_result != VK_SUCCESS)
		{
			DEBUG_ERROR("Failed to create a VulkanImage: {}", static_cast<uint32_t>(_result));
			return *this;
		}

		auto view_info = vk::ImageViewCreateInfo
		{
			.image    = _image,
			.viewType = vk::ImageViewType::e2D,
			.format   = format,
			.subresourceRange = {
				.aspectMask     = (format == vk::Format::eD32Sfloat) 
					? vk::ImageAspectFlagBits::eDepth 
					: vk::ImageAspectFlagBits::eColor,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1,
			}
		};

		VkImageView _view = context
			->getDevice()
			.createImageView(view_info);

		result = VulkanImage();
		result.allocation = _allocation;
		result.handle     = _image;
		result.context    = context;
		result.extent     = extent;
		result.format     = format;
		result.view       = _view;
		return *this;
	}

	VulkanImage VulkanImageBuilder::getResult()
	{
		return std::move(result);
	}

	VulkanImage::~VulkanImage()
	{
		if (handle == VK_NULL_HANDLE)
			return;

		destroy();
	}

	VulkanImage::VulkanImage(VulkanImage&& other) noexcept
		: context(other.context),
		handle(other.handle),
		view(other.view),
		allocation(other.allocation),
		extent(other.extent),
		format(other.format)
	{
		other.context = nullptr;
		other.handle = VK_NULL_HANDLE;
		other.view = VK_NULL_HANDLE;
		other.allocation = VK_NULL_HANDLE;
		other.extent = {};
		other.format = {};
	}

	VulkanImage& VulkanImage::operator=(VulkanImage&& other) noexcept
	{
		if (this != &other)
		{
			context = other.context;
			handle = other.handle;
			view = other.view;
			allocation = other.allocation;
			extent = other.extent;
			format = other.format;

			other.context = nullptr;
			other.handle = VK_NULL_HANDLE;
			other.view = VK_NULL_HANDLE;
			other.allocation = VK_NULL_HANDLE;
			other.extent = {};
			other.format = {};
		}

		return *this;
	}

	void VulkanImage::destroy()
	{
		DEBUG_ASSERT(handle != VK_NULL_HANDLE);
		context->device.destroyImageView(view);
		vmaDestroyImage(context->allocator, handle, allocation);
		view = VK_NULL_HANDLE;
		handle = VK_NULL_HANDLE;
		allocation = VK_NULL_HANDLE;
	}
}
