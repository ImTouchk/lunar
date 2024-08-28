//#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1

#ifdef WIN32
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_NONE
#include <lunar/render/internal/render_vk.hpp>
#include <lunar/debug/log.hpp>
#include <lunar/debug/assert.hpp>
#include <lunar/api.hpp>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include <set>

//VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Render
{
	namespace Vk
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

		bool HasRequiredLayers(const std::vector<const char*> requiredLayers)
		{
			auto available_layers = vk::enumerateInstanceLayerProperties();
			for (const char* required : requiredLayers)
			{
				auto it = std::find_if(
					available_layers.begin(),
					available_layers.end(),
					[required](vk::LayerProperties& layer) {
						return strcmp(required, layer.layerName) == 0;
					}
				);

				if (it == available_layers.end())
				{
					DEBUG_ERROR("Vulkan layer \"{}\" is not available on this device.", required);
					return false;
				}
			}
			return true;
		}

		bool HasRequiredExtensions(const std::vector<const char*> requiredExt)
		{
			auto available_ext = vk::enumerateInstanceExtensionProperties();
			for (const char* required : requiredExt)
			{
				auto it = std::find_if(
					available_ext.begin(),
					available_ext.end(),
					[required](vk::ExtensionProperties& ext) {
						return strcmp(required, ext.extensionName) == 0;
					}
				);

				if (it == available_ext.end())
				{
					DEBUG_ERROR("Vulkan extension \"{}\" is not available on this device.", required);
					return false;
				}
			}

			return true;
		}

		inline std::vector<const char*> GetRequiredExtensions()
		{
			glfwInit();

			uint32_t ext_count = 0;
			const char** extensions;
			extensions = glfwGetRequiredInstanceExtensions(&ext_count);

			auto ext_vector = std::vector<const char*>();
			for (uint32_t i = 0; i < ext_count; i++)
			{
				ext_vector.push_back(extensions[i]);
			}

			DEBUG_ONLY_EXPR(ext_vector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));

			return ext_vector;
		}

		inline std::vector<const char*> GetRequiredLayers()
		{
			auto layers = std::vector<const char*>();
			DEBUG_ONLY_EXPR(layers.push_back("VK_LAYER_KHRONOS_validation"));

			return layers;
		}

		void InstanceWrapper::init()
		{
			//VULKAN_HPP_DEFAULT_DISPATCHER.init();

			auto required_ext = GetRequiredExtensions();
			auto required_layers = GetRequiredLayers();

			if (!HasRequiredExtensions(required_ext))
				return;

			if (!HasRequiredLayers(required_layers))
				return;

			auto app_info = vk::ApplicationInfo{
				.pApplicationName = APP_NAME,
				.applicationVersion = VK_MAKE_API_VERSION(0, APP_VER_MAJOR, APP_VER_MINOR, APP_VER_PATCH),
				.pEngineName = "Lunar",
				.engineVersion = VK_MAKE_API_VERSION(0, LUNAR_VER_MAJOR, LUNAR_VER_MINOR, LUNAR_VER_PATCH),
				.apiVersion = VK_API_VERSION_1_3
			};

			auto debug_mess_info = vk::DebugUtilsMessengerCreateInfoEXT{
				.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
									vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
									vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,

				.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
									vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
									vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,

				.pfnUserCallback = DebugLayerCallback
			};

			auto instance_info = vk::InstanceCreateInfo{
				.pNext = (LUNAR_DEBUG_BUILD) ? &debug_mess_info : nullptr,
				.pApplicationInfo = &app_info,
				.enabledLayerCount = (uint32_t)required_layers.size(),
				.ppEnabledLayerNames = required_layers.data(),
				.enabledExtensionCount = (uint32_t)required_ext.size(),
				.ppEnabledExtensionNames = required_ext.data(),
			};

			instance = vk::createInstance(instance_info);
			loader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
			//VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

			DEBUG_ONLY_EXPR(debugMessenger = instance.createDebugUtilsMessengerEXT(debug_mess_info, nullptr, loader));
		}

		void InstanceWrapper::destroy()
		{
			DEBUG_ONLY_EXPR(instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, loader));

			instance.destroy();
		}

		inline std::vector<const char*> GetRequiredDeviceExtensions()
		{
			return std::vector<const char*> {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};
		}

		int GetDeviceScore(const vk::PhysicalDevice& device)
		{
			int score = 0;

			auto properties = device.getProperties();
			auto features = device.getFeatures();

			switch (properties.deviceType)
			{
			case vk::PhysicalDeviceType::eDiscreteGpu:   score += 3; break;
			case vk::PhysicalDeviceType::eIntegratedGpu: score += 1; break;
			case vk::PhysicalDeviceType::eCpu:           score -= 1; break;
			default: break;
			}

			if (features.geometryShader)
				score += 1;

			auto queue_families = device.getQueueFamilyProperties();
			for (const auto& queue_family : queue_families)
			{
				if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
					score += 3;

				if (queue_family.queueFlags & vk::QueueFlagBits::eCompute)
					score += 1;
			}


			auto required_ext = GetRequiredDeviceExtensions();
			auto extension_set = std::set<std::string>(required_ext.begin(), required_ext.end());
			auto device_ext = device.enumerateDeviceExtensionProperties();
			for (auto& ext : device_ext)
			{
				extension_set.erase(ext.extensionName);
			}

			if (!extension_set.empty())
				score -= 10;

			return score;
		}

		void DeviceWrapper::init(VulkanContext& context)
		{
			auto devices = context.getInstance()
				.enumeratePhysicalDevices();

			std::sort(devices.begin(), devices.end(), [this](const auto& a, const auto& b) {
				return GetDeviceScore(a) > GetDeviceScore(b);
				});


			DEBUG_LOG("Using device \"{}\" for rendering.", devices[0].getProperties().deviceName.data());

			physDevice = devices.at(0);
			auto queues = physDevice.getQueueFamilyProperties();
			for (size_t i = 0; i < queues.size(); i++)
			{
				if (queues[i].queueFlags & vk::QueueFlagBits::eGraphics)
					queueFamilies[0] = i;

				// TODO: do this for other platforms
				// (vkGetPhysicalDeviceSurfaceSupportKHR requires a valid vkSurfaceKHR object)
#					ifdef WIN32
				if (physDevice.getWin32PresentationSupportKHR(i) == VK_TRUE)
					queueFamilies[1] = i;
#					endif
			}

			if (queueFamilies[0] == -1 || queueFamilies[1] == -1)
				DEBUG_ERROR("Graphics device does not support Vulkan rendering.");

			auto device_features = physDevice.getFeatures();

			float queue_prio = 1.f;

			vk::DeviceQueueCreateInfo queue_infos[2] = {
				{
					.queueFamilyIndex = (unsigned)queueFamilies[0],
					.queueCount = 1,
					.pQueuePriorities = &queue_prio
				},
				{
					.queueFamilyIndex = (unsigned)queueFamilies[1],
					.queueCount = 1,
					.pQueuePriorities = &queue_prio
				}
			};

			auto required_layers = GetRequiredLayers();
			auto required_ext = GetRequiredDeviceExtensions();

			auto device_info = vk::DeviceCreateInfo{
				.queueCreateInfoCount = 2,
				.pQueueCreateInfos = queue_infos,
				.enabledLayerCount = (uint32_t)required_layers.size(),
				.ppEnabledLayerNames = required_layers.data(),
				.enabledExtensionCount = (uint32_t)required_ext.size(),
				.ppEnabledExtensionNames = required_ext.data(),
				.pEnabledFeatures = &device_features,
			};

			device = physDevice.createDevice(device_info);
			graphicsQueue = device.getQueue(queueFamilies[0], 0);
			presentQueue = device.getQueue(queueFamilies[1], 0);
		}

		void DeviceWrapper::destroy(VulkanContext& context)
		{
			device.waitIdle();
			device.destroy();
		}
	}

	VulkanContext::VulkanContext() : initialized(false)
	{
		init();
	}

	VulkanContext::~VulkanContext()
	{
		destroy();
	}

	void VulkanContext::init()
	{
		if (initialized)
			return;

		instanceWrapper.init();
		deviceWrapper.init(*this);
		pipelineWrapper.init(*this);
		initialized = true;

		DEBUG_LOG("Vulkan render context initialized.");
	}

	void VulkanContext::destroy()
	{
		if (!initialized)
			return;

		pipelineWrapper.destroy(*this);
		deviceWrapper.destroy(*this);
		instanceWrapper.destroy();
		initialized = false;

		DEBUG_LOG("Vulkan render context destroyed.");
	}

	vk::Instance& VulkanContext::getInstance() { return instanceWrapper.instance; }
	vk::PhysicalDevice& VulkanContext::getRenderingDevice() { return deviceWrapper.physDevice; }
	vk::Device& VulkanContext::getDevice() { return deviceWrapper.device; }
	vk::Queue& VulkanContext::getGraphicsQueue() { return deviceWrapper.graphicsQueue; }
	vk::Queue& VulkanContext::getPresentQueue() { return deviceWrapper.presentQueue; }
	const std::array<uint32_t, 2>& VulkanContext::getQueueFamilies() const { return deviceWrapper.queueFamilies; }
	bool VulkanContext::areQueuesSeparate() const { return deviceWrapper.queueFamilies[0] != deviceWrapper.queueFamilies[1]; }
	vk::PipelineLayout& VulkanContext::getDefaultGraphicsLayout() { return pipelineWrapper.defaultGraphicsLayout; }
	vk::RenderPass& VulkanContext::getDefaultRenderPass() { return pipelineWrapper.defaultRenderPass; }
}
