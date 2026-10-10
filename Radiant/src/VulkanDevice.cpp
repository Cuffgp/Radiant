
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "VulkanDevice.h"

#include "vulkan/vk_enum_string_helper.h"

namespace Radiant {

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			std::println("[Vulkan]: {}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	void VulkanDevice::Init()
	{
		s_VulkanDevice = new VulkanDevice();
	}

	void VulkanDevice::Destroy()
	{
		delete s_VulkanDevice;
	}

	VulkanDevice::VulkanDevice()
	{
		CreateInstance();
		CreateDebugMessenger();
		SelectPhysicalDevice();
		CreateDevice();
	}

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyDevice(m_Device, nullptr);
		if (m_Validation)
		{
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
		}
		vkDestroyInstance(m_Instance, nullptr);
	}

	void VulkanDevice::CreateInstance()
	{
		// Initialize Volk and load Vk function pointers
		if (volkInitialize() != VK_SUCCESS)
		{
			std::println("Failed to initilize Volk");
		}

		VkApplicationInfo appInfo
		{
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "Radiant",
			.apiVersion = VK_API_VERSION_1_4
		};

		uint32_t instanceExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
		std::vector<VkExtensionProperties> instanceExtensions(instanceExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data());

		std::println("Avalible Instance Extensions:");
		for (const auto& extension : instanceExtensions)
		{
			std::println("\t {}", extension.extensionName);
		}

		std::vector<const char*> requestedInstanceExtensions;
		requestedInstanceExtensions.emplace_back("VK_KHR_surface");
		requestedInstanceExtensions.emplace_back("VK_KHR_win32_surface");
		requestedInstanceExtensions.emplace_back("VK_EXT_debug_utils");

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::println("Avalible Layers:");
		for (const auto& layer : availableLayers)
		{
			std::println("\t {}", layer.layerName);
		}

		std::vector<const char*> enabledLayers
		{
			"VK_LAYER_KHRONOS_validation"
		};

		VkInstanceCreateInfo instanceCI
		{
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = nullptr,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size()),
			.ppEnabledLayerNames = enabledLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(requestedInstanceExtensions.size()),
			.ppEnabledExtensionNames = requestedInstanceExtensions.data(),
		};

		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = DebugCallback,
			.pUserData = nullptr // Optional
		};

		if (m_Validation)
		{
			instanceCI.pNext = &debugMessengerCreateInfo;
			instanceCI.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
			instanceCI.ppEnabledLayerNames = enabledLayers.data();
		}

		if (vkCreateInstance(&instanceCI, nullptr, &m_Instance) != VK_SUCCESS) 
		{
			std::println("failed to create instance!");
		}

		volkLoadInstance(m_Instance);
	}

	void VulkanDevice::CreateDebugMessenger()
	{
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
			.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
			.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
			.pfnUserCallback = DebugCallback,
			.pUserData = nullptr // Optional
		};

		if (vkCreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerCreateInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		{
			std::println("failed to set up debug messenger!");
		}
	}

	void VulkanDevice::SelectPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

		std::println("Physical Device:");
		for (const auto& device : physicalDevices)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(device, &properties);

			std::println("\t {}", properties.deviceName);
			std::println("\t {}", string_VkPhysicalDeviceType(properties.deviceType));

			VkPhysicalDeviceFeatures features;
			vkGetPhysicalDeviceFeatures(device, &features);
		}

		m_PhysicalDevice = physicalDevices[0];

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

		int index = 0;
		for (const auto& queue : queueFamilies)
		{
			if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				m_GraphicsIndice = index;
			}
			index++;
		}
	}

	void VulkanDevice::CreateDevice()
	{
		const float qfpriorities{ 1.0f };
		VkDeviceQueueCreateInfo queueCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_GraphicsIndice,
			.queueCount = 1,
			.pQueuePriorities = &qfpriorities
		};

		VkPhysicalDeviceVulkan11Features enabledVk11Features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.shaderDrawParameters = true
		};

		VkPhysicalDeviceVulkan12Features enabledVk12Features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &enabledVk11Features,
			.descriptorIndexing = true,
			.shaderSampledImageArrayNonUniformIndexing = true,
			.descriptorBindingVariableDescriptorCount = true,
			.runtimeDescriptorArray = true,
			.bufferDeviceAddress = true
		};

		VkPhysicalDeviceVulkan13Features enabledVk13Features
		{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &enabledVk12Features,
			.synchronization2 = true,
			.dynamicRendering = true,
		};

		VkPhysicalDeviceFeatures enabledVk10Features
		{
			.samplerAnisotropy = VK_TRUE
		};

		std::vector<const char*> deviceExtensions
		{
			"VK_KHR_swapchain"
		};

		VkDeviceCreateInfo deviceCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &enabledVk13Features,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
			.ppEnabledExtensionNames = deviceExtensions.data(),
			.pEnabledFeatures = &enabledVk10Features
		};

		if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
		{
			std::println("Failed to create device");
		}

		vkGetDeviceQueue(m_Device, m_GraphicsIndice, 0, &m_GraphicsQueue);

		// Create the command pool
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = m_GraphicsIndice;

		if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
		{
			std::println("Failed to create command pool!");
		}

		VmaVulkanFunctions vkFunctions
		{
			.vkGetInstanceProcAddr = vkGetInstanceProcAddr,
			.vkGetDeviceProcAddr = vkGetDeviceProcAddr,
			.vkCreateImage = vkCreateImage
		};

		VmaAllocatorCreateInfo allocatorCI
		{
			.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
			.physicalDevice = m_PhysicalDevice,
			.device = m_Device,
			.pVulkanFunctions = &vkFunctions,
			.instance = m_Instance
		};

		if (vmaCreateAllocator(&allocatorCI, &m_Allocator) != VK_SUCCESS)
		{
			std::println("Failed to create allocator");
		}
	}

}