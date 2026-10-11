#pragma once

#include "VulkanDevice.h"

namespace Radiant {

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		VkExtent2D GetExtent() { return m_Extent; }
		VkFormat GetImageFormat() { return m_ImageFormat; }
		uint32_t ImageCount() { return m_SwapchainImageCount; }
		VkImageView GetImageView(uint32_t index) { return m_ImageViews[index]; }
		VkImageView GetDepthView() { return m_DepthImageView; }
		VkImage GetImage(uint32_t index) { return m_Images[index]; }
		VkImage GetDepthImage() { return m_DepthImage; }

		VkResult AquireNextImage(uint32_t* imageIndex, VkSemaphore imageAvalible);
		VkResult QueuePresent(uint32_t* imageIndex, VkSemaphore imageAvalible);

		static inline VkFormat s_SwapchainImageFormat;
		static inline VkFormat s_SwapchainDepthFormat;

		void RecreateSwapchain();

	private:
		void CreateSwapchain();
		void CreateImages();
		void CleanupSwapchain();

	private:
		VkSurfaceKHR m_Surface;
		VkSwapchainKHR m_Swapchain;
		VkSwapchainKHR m_OldSwapchain = nullptr;

		VkFormat m_ImageFormat;
		VkPresentModeKHR m_PresentMode;
		VkExtent2D m_Extent;

		uint32_t m_SwapchainImageCount;
		std::vector<VkImage> m_Images;
		std::vector<VkImageView> m_ImageViews;

		VkFormat m_DepthFormat;
		VkImage m_DepthImage;
		VkImageView m_DepthImageView;
		VmaAllocation m_DepthAllocation;

	};

}