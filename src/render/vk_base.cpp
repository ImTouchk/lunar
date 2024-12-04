#ifdef WIN32
#	define NOMINMAX
#	define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_NONE
#define VMA_IMPLEMENTATION

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug.hpp>
#include <set>

#ifdef LUNAR_IMGUI
#	include <imgui_impl_glfw.h>
#	include <imgui_impl_vulkan.h>
#endif

namespace Render
{
	VKAPI_ATTR VkBool32 VKAPI_CALL DebugLayerCallback
	(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	)
	{
		if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			DEBUG_ERROR(pCallbackData->pMessage);
		else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			DEBUG_WARN(pCallbackData->pMessage);

		return VK_FALSE;
	}

	VulkanContextBuilder& VulkanContextBuilder::enableDebugging(bool value)
	{
		debugging = value;
		return *this;
	}

	VulkanContextBuilder& VulkanContextBuilder::setMinimumVersion(uint32_t version)
	{
		minimumVersion = version;
		return *this;
	}

	VulkanContextBuilder& VulkanContextBuilder::setRequiredFeatures12(vk::PhysicalDeviceVulkan12Features features)
	{
		features12 = features;
		return *this;
	}

	VulkanContextBuilder& VulkanContextBuilder::setRequiredFeatures13(vk::PhysicalDeviceVulkan13Features features)
	{
		features13 = features;
		return *this;
	}

	VulkanContextBuilder& VulkanContextBuilder::requireDeviceExtension(const char* name)
	{
		requiredDeviceExtensions.push_back(name);
		return *this;
	}

	std::shared_ptr<VulkanContext> VulkanContextBuilder::create()
	{
		return std::make_shared<VulkanContext>(
			debugging, minimumVersion, requiredDeviceExtensions, features12, features13
		);
	}

	std::vector<const char*> GetRequiredInstanceExtensions()
	{
		glfwInit();

		uint32_t ext_count = 0;
		const char** extensions;
		extensions = glfwGetRequiredInstanceExtensions(&ext_count);

		return std::vector<const char*>(extensions, extensions + ext_count);
	}

	bool VulkanContext::createInstance(bool debugging, uint32_t minVersion)
	{
		auto req_instance_ext = GetRequiredInstanceExtensions();
		auto req_instance_layers = std::vector<const char*>();
		if (debugging)
		{
			req_instance_ext.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			req_instance_layers.push_back("VK_LAYER_KHRONOS_validation");
		}

		auto available_ext = vk::enumerateInstanceExtensionProperties();
		auto available_layers = vk::enumerateInstanceLayerProperties();
		for (const char* req_ext : req_instance_ext)
		{
			auto it = std::find_if(
				available_ext.begin(),
				available_ext.end(),
				[req_ext](vk::ExtensionProperties& ext) {
					return strcmp(req_ext, ext.extensionName) == 0;
				}
			);

			if (it == available_ext.end())
			{
				DEBUG_ERROR("Vulkan extension \"{}\" not present on this device.", req_ext);
				return false;
			}
		}
		for (const char* req_layer : req_instance_layers)
		{
			auto it = std::find_if(
				available_layers.begin(),
				available_layers.end(),
				[req_layer](vk::LayerProperties& layer) {
					return strcmp(req_layer, layer.layerName) == 0;
				}
			);

			if (it == available_layers.end())
			{
				DEBUG_ERROR("Vulkan layer \"{}\" not present on this device.", req_layer);
				return false;
			}
		}

		auto app_info = vk::ApplicationInfo
		{
			.pApplicationName   = APP_NAME,
			.applicationVersion = VK_MAKE_API_VERSION(0, APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH),
			.pEngineName        = "Lunar",
			.engineVersion      = VK_MAKE_API_VERSION(0, LUNAR_VER_MAJOR, LUNAR_VER_MINOR, LUNAR_VER_PATCH),
			.apiVersion         = minVersion
		};

		auto debug_mess_info = vk::DebugUtilsMessengerCreateInfoEXT
		{
			.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
								vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
								vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,

			.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
							vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
							vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,

			.pfnUserCallback = DebugLayerCallback
		};

		auto instance_info = vk::InstanceCreateInfo
		{
			.pNext                   = (debugging) ? &debug_mess_info : nullptr,
			.pApplicationInfo        = &app_info,
			.enabledLayerCount       = (uint32_t)req_instance_layers.size(),
			.ppEnabledLayerNames     = req_instance_layers.data(),
			.enabledExtensionCount   = (uint32_t)req_instance_ext.size(),
			.ppEnabledExtensionNames = req_instance_ext.data(),
		};

		instance = vk::createInstance(instance_info);
		loader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);

		if (debugging)
			messenger = instance.createDebugUtilsMessengerEXT(debug_mess_info, nullptr, loader);

		deletionStack.push([debugging, this]() {
			if (debugging)
				instance.destroyDebugUtilsMessengerEXT(messenger, nullptr, loader);

			instance.destroy();
		});

		return true;
	}

	std::vector<vk::PhysicalDevice> GetCapableGpus
	(
		std::vector<vk::PhysicalDevice>& all,
		std::vector<const char*>& requiredDeviceExtensions,
		vk::PhysicalDeviceVulkan12Features features12,
		vk::PhysicalDeviceVulkan13Features features13
	)
	{
		std::vector<vk::PhysicalDevice> list = {};
		for (auto& device : all)
		{
			auto available_ext = device.enumerateDeviceExtensionProperties();
			auto ext_set = std::set<std::string>(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
			for (auto& ext : available_ext)
				ext_set.erase(ext.extensionName);

			if (!ext_set.empty())
				continue;

			list.push_back(device);
		}
		return list;
	}

	bool VulkanContext::createDevice
	(
		std::vector<const char*>& requiredDeviceExtensions,
		vk::PhysicalDeviceVulkan12Features features12,
		vk::PhysicalDeviceVulkan13Features features13
	)
	{
		auto phys_devices = instance.enumeratePhysicalDevices();
		auto capable_devices = GetCapableGpus(phys_devices, requiredDeviceExtensions, features12, features13);
		if (capable_devices.empty())
			return false;

		physicalDevice = capable_devices.at(0);

		int queue_families[2] = { -1, -1 };
		auto queues = physicalDevice.getQueueFamilyProperties();
		for (size_t i = 0; i < queues.size(); i++)
		{
			if (queues[i].queueFlags & vk::QueueFlagBits::eGraphics)
				queue_families[0] = i;

#			ifdef WIN32
			if (physicalDevice.getWin32PresentationSupportKHR(i) == VK_TRUE)
				queue_families[1] = i;
#			endif
		}

		if (queue_families[0] == -1 || queue_families[1] == -1)
			return false;

		queueFamilies[0] = queue_families[0];
		queueFamilies[1] = queue_families[1];

		float queue_prio = 1.f;
		vk::DeviceQueueCreateInfo queue_infos[2] =
		{
			{.queueFamilyIndex = queueFamilies[0], .queueCount = 1, .pQueuePriorities = &queue_prio },
			{.queueFamilyIndex = queueFamilies[1], .queueCount = 1, .pQueuePriorities = &queue_prio }
		};

		features12.pNext = &features13;

		auto features = vk::PhysicalDeviceFeatures2{ .pNext = &features12 };
		auto device_info = vk::DeviceCreateInfo
		{
			.pNext                   = &features,
			.queueCreateInfoCount    = 2,
			.pQueueCreateInfos       = queue_infos,
			.enabledExtensionCount   = static_cast<uint32_t>(requiredDeviceExtensions.size()),
			.ppEnabledExtensionNames = requiredDeviceExtensions.data(),
		};

		device = physicalDevice.createDevice(device_info);
		if (!device)
			return false;

		graphicsQueue = device.getQueue(queueFamilies[0], 0);
		presentQueue = device.getQueue(queueFamilies[1], 0);
		//transferQueue = device.getQueue(queueFamilies[2], 0);

		auto allocator_info = VmaAllocatorCreateInfo
		{
			.flags          = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = physicalDevice,
			.device         = device,
			.instance       = instance
		};

		vmaCreateAllocator(&allocator_info, &allocator);
		if (!allocator)
			return false;

		deletionStack.push([this]() { 
			vmaDestroyAllocator(allocator);
			device.destroy(); 
		});

		return true;
	}

	VulkanCommandPool VulkanContext::createCommandPool()
	{
		auto pool_info = vk::CommandPoolCreateInfo
		{
			.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = getQueueIndex(VulkanQueueType::eGraphics)
		};

		auto cmd_pool = device.createCommandPool(pool_info);
		return VulkanCommandPool(*this, cmd_pool);
	}

	bool VulkanContext::createMainCommandPool()
	{
		mainCmdPool = createCommandPool();
		mainCmdBuffer = mainCmdPool.allocateBuffer(vk::CommandBufferLevel::ePrimary);

		deletionStack.push([this]() {
			mainCmdBuffer.destroy();
			mainCmdPool.destroy();
		});
		return true;
	}

#	ifdef LUNAR_IMGUI
	void VulkanContext::_imguiInit()
	{
		vk::DescriptorPoolSize pool_sizes[] =
		{
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

		auto pool_info = vk::DescriptorPoolCreateInfo
		{
			.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			.maxSets       = 1000,
			.poolSizeCount = 11,
			.pPoolSizes    = pool_sizes
		};

		imguiPool = device.createDescriptorPool(pool_info);

		auto pipeline_color_attachments = VK_FORMAT_R16G16B16A16_SFLOAT;

		auto init_info = ImGui_ImplVulkan_InitInfo
		{
			.Instance                    = instance,
			.PhysicalDevice              = physicalDevice,
			.Device                      = device,
			.Queue                       = graphicsQueue,
			.DescriptorPool              = imguiPool,
			.MinImageCount               = 2,
			.ImageCount                  = 2,
			.MSAASamples                 = VK_SAMPLE_COUNT_1_BIT,
			.UseDynamicRendering         = true,
			.PipelineRenderingCreateInfo =
			{
				.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
				.colorAttachmentCount    = 1,
				.pColorAttachmentFormats = &pipeline_color_attachments
			}
		};

		ImGui_ImplVulkan_Init(&init_info);
		ImGui_ImplVulkan_CreateFontsTexture();

		deletionStack.push([this]() {
			device.destroyDescriptorPool(imguiPool);
			ImGui_ImplVulkan_Shutdown();
		});
	}
#	endif

	VulkanContext::VulkanContext
	(
		bool debugging,
		uint32_t minimumVersion,
		std::vector<const char*>& requiredDeviceExtensions,
		vk::PhysicalDeviceVulkan12Features features12,
		vk::PhysicalDeviceVulkan13Features features13
	)
	{
		if (!createInstance(debugging, minimumVersion))
			return;

		if (!createDevice(requiredDeviceExtensions, features12, features13))
			return;

		if (!createMainCommandPool())
			return;

		DEBUG_LOG("Ok");
	}

	VulkanContext::~VulkanContext()
	{
		DEBUG_LOG("Bye");

		device.waitIdle();

		while (!deletionQueue.empty())
		{
			deletionQueue.front()();
			deletionQueue.pop();
		}

		while (!deletionStack.empty())
		{
			deletionStack.top()();
			deletionStack.pop();
		}
	}

	void VulkanContext::flush()
	{
		while (!deletionQueue.empty())
		{
			deletionQueue.front()();
			deletionQueue.pop();
		}
	}

	vk::Instance& VulkanContext::getInstance()
	{
		return instance;
	}

	vk::PhysicalDevice& VulkanContext::getRenderingDevice()
	{
		return physicalDevice;
	}

	vk::Device& VulkanContext::getDevice()
	{
		return device;
	}

	vk::Queue& VulkanContext::getGraphicsQueue()
	{
		return graphicsQueue;
	}

	vk::Queue& VulkanContext::getPresentQueue()
	{
		return presentQueue;
	}

	std::array<uint32_t, 2> VulkanContext::getQueueFamilies() const
	{
		return { queueFamilies[0], queueFamilies[1] };
	}

	bool VulkanContext::areQueuesSeparate() const
	{
		return queueFamilies[0] != queueFamilies[1];
	}

	VmaAllocator VulkanContext::getAllocator()
	{
		return allocator;
	}

	void VulkanContext::init() {}
	void VulkanContext::destroy() {}

	vk::Queue& VulkanContext::getQueue(VulkanQueueType queue)
	{
		switch (queue)
		{
		case VulkanQueueType::eGraphics: return graphicsQueue;
		case VulkanQueueType::ePresent: return presentQueue;
		default: throw;
		}
	}

	uint32_t VulkanContext::getQueueIndex(VulkanQueueType queue)
	{
		return queueFamilies[static_cast<size_t>(queue)];
	}

	VulkanCommandPool::VulkanCommandPool(VulkanContext& context, vk::CommandPool pool)
		: value(pool),
		context(&context)
	{
	}

	VulkanCommandPool::VulkanCommandPool()
		: value(VK_NULL_HANDLE),
		context(nullptr)
	{
	}

	VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) noexcept
		: value(other.value),
		context(other.context)
	{
		other.value = VK_NULL_HANDLE;
		other.context = nullptr;
	}

	VulkanCommandPool& VulkanCommandPool::operator=(VulkanCommandPool&& other) noexcept
	{
		if (this != &other)
		{
			value = other.value;
			context = other.context;

			other.value = VK_NULL_HANDLE;
			other.context = nullptr;
		}

		return *this;
	}

	VulkanCommandBuffer VulkanCommandPool::allocateBuffer(vk::CommandBufferLevel level)
	{
		auto allocate_info = vk::CommandBufferAllocateInfo
		{
			.commandPool        = value,
			.level              = level,
			.commandBufferCount = 1
		};

		auto fence_create_info = vk::FenceCreateInfo { .flags = vk::FenceCreateFlagBits::eSignaled };
		
		auto cmd_buffer = context->device.allocateCommandBuffers(allocate_info).at(0);
		auto fence = context->device.createFence(fence_create_info);
		return VulkanCommandBuffer(*context, cmd_buffer, fence);
	}

	VulkanCommandPool::~VulkanCommandPool()
	{
		if (value == VK_NULL_HANDLE)
			return;

		destroy();
	}

	void VulkanCommandPool::destroy()
	{
		context->device.destroyCommandPool(value);
		
		value = VK_NULL_HANDLE;
		context = nullptr;
	}
	
	VulkanCommandBuffer::VulkanCommandBuffer(VulkanContext& context, vk::CommandBuffer buffer, vk::Fence fence)
		: value(buffer),
		ready(fence),
		context(&context),
		state(0)
	{
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanCommandBuffer&& other) noexcept
		: value(other.value),
		ready(other.ready),
		context(other.context),
		state(other.state)
	{
		other.value = VK_NULL_HANDLE;
		other.ready = VK_NULL_HANDLE;
		other.context = nullptr;
		other.state = 0;
	}

	VulkanCommandBuffer::VulkanCommandBuffer()
		: value(VK_NULL_HANDLE),
		ready(VK_NULL_HANDLE),
		context(nullptr),
		state(0)
	{
	}

	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		if (value == VK_NULL_HANDLE)
			return;

		destroy();
	}

	void VulkanCommandBuffer::destroy()
	{
		std::ignore = context->device.waitForFences(ready, VK_TRUE, UINT64_MAX);
		context->device.destroyFence(ready);

		value = VK_NULL_HANDLE;
		ready = VK_NULL_HANDLE;
		context = nullptr;
		state = 0;
	}

	VulkanCommandBuffer::operator vk::CommandBuffer& ()
	{
		return value;
	}

	vk::CommandBuffer* VulkanCommandBuffer::operator->()
	{
		return &value;
	}

	VulkanCommandBuffer& VulkanCommandBuffer::operator=(VulkanCommandBuffer&& other) noexcept
	{
		if (this != &other)
		{
			value = other.value;
			ready = other.ready;
			context = other.context;
			state = other.state;
			other.value = VK_NULL_HANDLE;
			other.ready = VK_NULL_HANDLE;
			other.context = nullptr;
			other.state = 0;
		}

		return *this;
	}

	void VulkanCommandBuffer::begin()
	{
		DEBUG_ASSERT(value != VK_NULL_HANDLE && state != 1);
		vk::Result result;
		result = context->device.waitForFences(ready, VK_TRUE, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			DEBUG_ERROR("device.waitForFences() failed: {}", static_cast<uint32_t>(result));
			throw;
		}

		context->device.resetFences(ready);
		
		auto cmd_begin_info = vk::CommandBufferBeginInfo{ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit };

		value.reset();
		value.begin(cmd_begin_info);

		state = 1;
	}

	void VulkanCommandBuffer::submit
	(
		const VulkanSemaphoreSubmit& waitSemaphores,
		const VulkanSemaphoreSubmit& signalSemaphores,
		bool waitForFinish
	)
	{
		DEBUG_ASSERT(value != VK_NULL_HANDLE && state == 1);

		value.end();

		auto cmd_submit_info = vk::CommandBufferSubmitInfo
		{
			.commandBuffer = value,
			.deviceMask = 0
		};

		auto submit_info = vk::SubmitInfo2
		{
			.waitSemaphoreInfoCount   = waitSemaphores.size(),
			.pWaitSemaphoreInfos      = waitSemaphores.data(),
			.commandBufferInfoCount   = 1,
			.pCommandBufferInfos      = &cmd_submit_info,
			.signalSemaphoreInfoCount = signalSemaphores.size(),
			.pSignalSemaphoreInfos    = signalSemaphores.data(),
		};

		context->graphicsQueue.submit2(submit_info, ready);

		if (waitForFinish)
			context->device.waitForFences(ready, VK_TRUE, UINT64_MAX);

		state = 0;
	}

	VulkanSemaphoreSubmit::VulkanSemaphoreSubmit
	(
		const std::initializer_list<VulkanSemaphoreSubmit::SubmitValue>& semaphores
	)
	{
		for (const auto& semaphore : semaphores)
		{
			if (semaphore.index() == 0)
			{
				value.push_back(vk::SemaphoreSubmitInfo
				{
					.semaphore   = std::get<vk::Semaphore>(semaphore),
					.value       = 1,
					.stageMask   = vk::PipelineStageFlagBits2::eAllCommands,
					.deviceIndex = 0
				});
			}
			else
				value.push_back(std::get<vk::SemaphoreSubmitInfo>(semaphore));
		}
	}

	uint32_t VulkanSemaphoreSubmit::size() const
	{
		return static_cast<uint32_t>(value.size());
	}

	const vk::SemaphoreSubmitInfo* VulkanSemaphoreSubmit::data() const
	{
		return value.data();
	}

	std::vector<vk::SemaphoreSubmitInfo>* VulkanSemaphoreSubmit::operator->()
	{
		return &value;
	}

	void VulkanContext::immediateSubmit(const std::function<void(vk::CommandBuffer)>& command, bool waitForFinish)
	{
		mainCmdBuffer.begin();
		command(mainCmdBuffer);
		mainCmdBuffer.submit({}, {}, waitForFinish);
	}

	std::shared_ptr<RenderContext> CreateDefaultContext()
	{
		vk::PhysicalDeviceVulkan12Features features12 = {};
		features12.bufferDeviceAddress = true;
		features12.descriptorIndexing = true;

		vk::PhysicalDeviceVulkan13Features features13 = {};
		features13.dynamicRendering = true;
		features13.synchronization2 = true;

		return VulkanContextBuilder()
			.setMinimumVersion(VK_API_VERSION_1_3)
			.setRequiredFeatures12(features12)
			.setRequiredFeatures13(features13)
			.enableDebugging(LUNAR_DEBUG_BUILD)
			.create();
	}
}
