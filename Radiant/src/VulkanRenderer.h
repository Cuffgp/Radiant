#pragma once

#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

namespace Radiant {

	class VulkanRenderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		VkCommandBuffer BeginFrame();
		void BeginSwapchainRendering();
		void EndSwapchainRendering();
		void EndFrame();

		void OnWindowResize(uint32_t width, uint32_t height);
		uint32_t CurrentFrame() { return m_CurrentFrameIndex; }

	public:
		static inline uint32_t MaxFramesInFlight = 3;
	private:

		Scope<VulkanSwapchain> m_Swapchain;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		uint32_t m_CurrentImageIndex = 0;
		uint32_t m_CurrentFrameIndex = 0;

		std::vector<VkSemaphore> m_ImageAvailableSemaphores;
		std::vector<VkSemaphore> m_DrawFinishedSemaphores;
		std::vector<VkFence> m_FrameInFlightFences;
		std::vector<VkFence> m_ImageInFlightFences;
	};

}