
#include "Application.h"

#include "VulkanSwapchain.h"

namespace Radiant {

	VulkanSwapchain::VulkanSwapchain()
	{
        VkResult err = glfwCreateWindowSurface(
            VulkanDevice::Get().Instance(),
            Application::Get().GetWindow().GetWindowPointer(),
            NULL,
            &m_Surface);

        CreateSwapchain();
        CreateImages();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{

	}

    void VulkanSwapchain::CreateSwapchain()
    {
        auto device = VulkanDevice::Get().Device();

        VkSurfaceCapabilitiesKHR surfaceCaps{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VulkanDevice::Get().PhysicalDevice(), m_Surface, &surfaceCaps);

        m_Extent = surfaceCaps.currentExtent;
        m_ImageFormat = VK_FORMAT_B8G8R8A8_SRGB;
        m_PresentMode = VK_PRESENT_MODE_FIFO_KHR;
        m_DepthFormat = VK_FORMAT_D32_SFLOAT;

        s_SwapchainImageFormat = m_ImageFormat;
        s_SwapchainDepthFormat = m_DepthFormat;

        VkSwapchainCreateInfoKHR swapchainCI
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = m_Surface,
            .minImageCount = surfaceCaps.minImageCount,
            .imageFormat = m_ImageFormat,
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent{.width = m_Extent.width, .height = m_Extent.height },
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = m_PresentMode
        };


        auto res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &m_Swapchain);
        if ( res != VK_SUCCESS)
        {
            std::println("Failed to create swapchain");
        }
    }

    void VulkanSwapchain::CreateImages()
    {
        auto device = VulkanDevice::Get().Device();


        vkGetSwapchainImagesKHR(device, m_Swapchain, &m_SwapchainImageCount, nullptr);
        m_Images.resize(m_SwapchainImageCount);
        vkGetSwapchainImagesKHR(device, m_Swapchain, &m_SwapchainImageCount, m_Images.data());

        m_ImageViews.resize(m_SwapchainImageCount);
        for (auto i = 0; i < m_SwapchainImageCount; i++)
        {
            VkImageViewCreateInfo viewCI
            { 
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_Images[i],
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_ImageFormat,
                .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .levelCount = 1, .layerCount = 1 } 
            };
            vkCreateImageView(device, &viewCI, nullptr, &m_ImageViews[i]);
        }

        VkImageCreateInfo depthImageCI
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = m_DepthFormat,
            .extent{.width = m_Extent.width, .height = m_Extent.height , .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        VmaAllocationCreateInfo allocCI{ .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
        vmaCreateImage(VulkanDevice::Get().Allocator(), &depthImageCI, &allocCI, &m_DepthImage, &m_DepthAllocation, nullptr);

        VkImageViewCreateInfo depthViewCI
        { 
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_DepthImage,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_DepthFormat,
            .subresourceRange{.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT, .levelCount = 1, .layerCount = 1 }
        };

        vkCreateImageView(device, &depthViewCI, nullptr, &m_DepthImageView);
    }

    VkResult VulkanSwapchain::AquireNextImage(uint32_t* imageIndex, VkSemaphore imageAvalible)
    {
        auto device = VulkanDevice::Get().Device();

        VkResult result = vkAcquireNextImageKHR(
            device,
            m_Swapchain,
            UINT64_MAX,
            imageAvalible,  // must be a not signaled semaphore
            VK_NULL_HANDLE,
            imageIndex);

        return result;
    }

    VkResult VulkanSwapchain::QueuePresent(uint32_t* imageIndex, VkSemaphore renderFinished)
    {
        VkSwapchainKHR swapChains[] = { m_Swapchain };

        VkPresentInfoKHR presentInfo
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinished,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = imageIndex
        };

        auto result = vkQueuePresentKHR(VulkanDevice::Get().GraphicsQueue(), &presentInfo);

        return result;
    }


}