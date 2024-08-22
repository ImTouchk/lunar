#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define GLFW_INCLUDE_NONE
#include <lunar/debug/log.hpp>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>
#include <map>

#ifdef NDEBUG
#	define VK_DEBUG 0
#else
#	define VK_DEBUG 1
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

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
			else
				DEBUG_LOG(pCallbackData->pMessage);

			return VK_FALSE;
		}

		struct InstanceWrapper
		{
			bool hasRequiredLayers(const std::vector<const char*> requiredLayers)
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

			bool hasRequiredExtensions(const std::vector<const char*> requiredExt)
			{
				auto available_ext = vk::enumerateInstanceExtensionProperties();
				for (const char* required : requiredExt)
				{
					// ?????????????????????????????
					if (required == nullptr)
						continue;

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

			inline std::vector<const char*> getRequiredExtensions()
			{
				uint32_t ext_count = 0;
				const char** extensions;
				extensions = glfwGetRequiredInstanceExtensions(&ext_count);
				
				auto ext_vector = std::vector<const char*>();
				for (uint32_t i = 0; i < ext_count; i++)
				{
					ext_vector.push_back(extensions[i]);
				}

				if (VK_DEBUG)
				{
					ext_vector.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
				}

				return ext_vector;
			}

			inline std::vector<const char*> getRequiredLayers()
			{
				auto layers = std::vector<const char*>();
				if (VK_DEBUG)
				{
					layers.push_back("VK_LAYER_KHRONOS_validation");
				}

				return layers;
			}

			InstanceWrapper()
			{
				auto _vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");

				VULKAN_HPP_DEFAULT_DISPATCHER.init(_vkGetInstanceProcAddr);

				auto required_ext = getRequiredExtensions();
				auto required_layers = getRequiredLayers();

				if (!hasRequiredExtensions(required_ext))
					return;

				if (!hasRequiredLayers(required_layers))
					return;

				auto app_info = vk::ApplicationInfo {
					.pApplicationName   = "UntitledGame",
					.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0),
					.pEngineName        = "Lunar",
					.engineVersion      = VK_MAKE_API_VERSION(0, 0, 1, 0),
					.apiVersion         = VK_API_VERSION_1_3
				};

				auto debug_mess_info = vk::DebugUtilsMessengerCreateInfoEXT {
					.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
										vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
										vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,

					.messageType     = 
										vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
										vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,

					.pfnUserCallback = DebugLayerCallback
				};

				auto instance_info = vk::InstanceCreateInfo {
					.pNext                   = (VK_DEBUG) ? &debug_mess_info : nullptr,
					.pApplicationInfo        = &app_info,
					.enabledLayerCount       = (uint32_t)required_layers.size(),
					.ppEnabledLayerNames     = required_layers.data(),
					.enabledExtensionCount   = (uint32_t)required_ext.size(),
					.ppEnabledExtensionNames = required_ext.data(),
				};

				instance = vk::createInstance(instance_info);
				VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

				if(VK_DEBUG)
					debugMessenger = instance.createDebugUtilsMessengerEXT(debug_mess_info);

				DEBUG_LOG("Vulkan instance created.");
			}

			~InstanceWrapper()
			{
				if(VK_DEBUG)
					instance.destroyDebugUtilsMessengerEXT(debugMessenger);

				DEBUG_LOG("Vulkan instance destroyed.");
			}

			vk::DynamicLoader dl;
			vk::Instance instance;
			vk::DebugUtilsMessengerEXT debugMessenger;
		};

		InstanceWrapper& getInstanceWrapper()
		{
			static auto wrapper = InstanceWrapper();
			return wrapper;
		}

		vk::Instance& getInstance()
		{
			return getInstanceWrapper()
						.instance;
		}

		struct DeviceWrapper
		{
			int getDeviceScore(const vk::PhysicalDevice& device)
			{
				int score = 0;

				auto properties = device.getProperties();
				auto features = device.getFeatures();
				
				switch (properties.deviceType)
				{
				case vk::PhysicalDeviceType::eDiscreteGpu:   score += 2; break;
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
						score += 2;

					if(queue_family.queueFlags & vk::QueueFlagBits::eCompute)
						score += 1;
				}

				return score;
			}

			DeviceWrapper() : graphicsQueueIdx(-1), presentQueueIdx(-1)
			{
				auto devices = getInstance().enumeratePhysicalDevices();
				std::sort(devices.begin(), devices.end(), [this](const auto& a, const auto& b) {
					return getDeviceScore(a) > getDeviceScore(b);
				});
				

				DEBUG_LOG("Using device \"{}\" for rendering.", devices[0].getProperties().deviceName.data());

				physDevice = devices.at(0);
				auto queues = physDevice.getQueueFamilyProperties();
				for (size_t i = 0; i < queues.size(); i++)
				{
					if (queues[i].queueFlags & vk::QueueFlagBits::eGraphics)
						graphicsQueueIdx = i;

					// TODO: do this for other platforms
					// (vkGetPhysicalDeviceSurfaceSupportKHR requires a valid vkSurfaceKHR object)
#					ifdef WIN32
					if (physDevice.getWin32PresentationSupportKHR(i) == VK_TRUE)
						presentQueueIdx = i;
#					endif
				}

				if (presentQueueIdx == -1 || graphicsQueueIdx == -1)
					DEBUG_ERROR("Graphics device does not support Vulkan rendering.");

				auto device_features = physDevice.getFeatures();

				float queue_prio = 1.f;

				vk::DeviceQueueCreateInfo queue_infos[2] = {
					{
						.queueFamilyIndex = (unsigned)graphicsQueueIdx,
						.queueCount       = 1,
						.pQueuePriorities = &queue_prio
					},
					{
						.queueFamilyIndex = (unsigned)presentQueueIdx,
						.queueCount       = 1,
						.pQueuePriorities = &queue_prio
					}
				};

				auto required_layers = getInstanceWrapper().getRequiredLayers();

				auto device_info = vk::DeviceCreateInfo {
					.queueCreateInfoCount  = sizeof(queue_infos),
					.pQueueCreateInfos     = queue_infos,
					.enabledLayerCount     = (uint32_t)required_layers.size(),
					.ppEnabledLayerNames   = required_layers.data(),
					.enabledExtensionCount = 0,
					.pEnabledFeatures      = &device_features,
				};

				device = physDevice.createDevice(device_info);
				graphicsQueue = device.getQueue(graphicsQueueIdx, 0);
				presentQueue = device.getQueue(presentQueueIdx, 0);
				VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
			}

			~DeviceWrapper()
			{
				
			}

			vk::Device device;
			vk::PhysicalDevice physDevice;
			vk::Queue graphicsQueue;
			vk::Queue presentQueue;
			int graphicsQueueIdx;
			int presentQueueIdx;
		};

		Vk::DeviceWrapper& getDeviceWrapper()
		{
			static auto wrapper = Vk::DeviceWrapper();
			return wrapper;
		}

		vk::Device& getDevice()
		{
			return getDeviceWrapper()
				.device;
		}

		vk::Queue& getGraphicsQueue()
		{
			return getDeviceWrapper()
					.graphicsQueue;
		}

		vk::Queue& getPresentQueue()
		{
			return getDeviceWrapper()
					.presentQueue;
		}
	}
}
