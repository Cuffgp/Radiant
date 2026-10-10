#pragma once

#include <print>
#include <vector>
#include <memory>

#include "Base.h"

#include "volk.h"
#include <vk_mem_alloc.h>

namespace Radiant {

	class VulkanDevice
	{
	public:
		static void Init();
		static void Destroy();

		static VulkanDevice& Get() { return *s_VulkanDevice; }

		VkInstance Instance() { return m_Instance; }
		VkDevice Device() { return m_Device; }
		VkPhysicalDevice PhysicalDevice() { return m_PhysicalDevice; }
		VmaAllocator Allocator() { return m_Allocator; }
		VkQueue GraphicsQueue() { return m_GraphicsQueue; }
		VkCommandPool CommandPool() { return m_CommandPool; }

	private:
		VulkanDevice();
		~VulkanDevice();

		void CreateInstance();
		void CreateDebugMessenger();
		void SelectPhysicalDevice();
		void CreateDevice();

	private:
		static inline VulkanDevice* s_VulkanDevice;
		static constexpr bool m_Validation = true;

		VkInstance m_Instance;
		VkDebugUtilsMessengerEXT m_DebugMessenger;
		VkPhysicalDevice m_PhysicalDevice;
		uint32_t m_GraphicsIndice;
		VkQueue m_GraphicsQueue;
		VkDevice m_Device;
		VkCommandPool m_CommandPool;
		VmaAllocator m_Allocator;

	};

}