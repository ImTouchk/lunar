#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug.hpp>

namespace Render
{
	VulkanImage::VulkanImage
	(
		VulkanContext& context, 
		vk::Image image, 
		vk::ImageView view, 
		VmaAllocation allocation, 
		vk::Extent3D extent, 
		vk::Format format
	)
		: context(&context),
		handle(image),
		view(view),
		allocation(allocation),
		extent(extent),
		format(format)
	{
	}

	VulkanImage::VulkanImage()
		: context(nullptr),
		handle(VK_NULL_HANDLE),
		view(VK_NULL_HANDLE),
		allocation(VK_NULL_HANDLE),
		extent(),
		format()
	{
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
		context->deletionQueue.push([
			image = handle,
			view = view,
			allocation = allocation,
			allocator = context->allocator,
			device = context->device
		]() {
			device.destroyImageView(view);
			vmaDestroyImage(allocator, image, allocation);
		});
	}

	VulkanImage VulkanContext::createImage
	(
		vk::Format format,
		vk::Extent3D extent,
		vk::ImageUsageFlags flags
	)
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
			.usage       = flags
		};

		auto image_alloc_info = VmaAllocationCreateInfo
		{
			.usage         = VMA_MEMORY_USAGE_GPU_ONLY,
			.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
		};

		VkImage _image;
		VmaAllocation _allocation;
		VkResult _result;
		_result = vmaCreateImage(
			allocator, &image_info.operator const VkImageCreateInfo &(), &image_alloc_info, &_image, &_allocation, nullptr
		);

		if (_result != VK_SUCCESS)
		{
			DEBUG_ERROR("Failed to create a VulkanImage: {}", static_cast<uint32_t>(_result));
			return VulkanImage();
		}

		auto view_info = vk::ImageViewCreateInfo
		{
			.image    = _image,
			.viewType = vk::ImageViewType::e2D,
			.format   = format,
			.subresourceRange = {
				.aspectMask     = vk::ImageAspectFlagBits::eColor,
				.baseMipLevel   = 0,
				.levelCount     = 1,
				.baseArrayLayer = 0,
				.layerCount     = 1,
			}
		};

		VkImageView _view = device.createImageView(view_info);

		return VulkanImage(*this, _image, _view, _allocation, extent, format);
	}
}
