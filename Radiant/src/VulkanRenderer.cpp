
#include <array>

#include "VulkanRenderer.h"

namespace Radiant {

	VulkanRenderer::VulkanRenderer()
	{
		auto device = VulkanDevice::Get().Device();

		m_Swapchain = CreateScope<VulkanSwapchain>();

		//auto extent = m_Swapchain->GetExtent();
		//SL_INFO("Swapchain extent {}, {}", extent.width, extent.height);

		// Create synchronization objects
		m_ImageAvailableSemaphores.resize(VulkanRenderer::MaxFramesInFlight);
		m_DrawFinishedSemaphores.resize(VulkanRenderer::MaxFramesInFlight);
		m_FrameInFlightFences.resize(VulkanRenderer::MaxFramesInFlight);
		m_ImageInFlightFences.resize(m_Swapchain->ImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo
		{
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		VkFenceCreateInfo fenceInfo
		{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT
		};

		for (size_t i = 0; i < VulkanRenderer::MaxFramesInFlight; i++)
		{
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_DrawFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &m_FrameInFlightFences[i]) != VK_SUCCESS)
			{
				std::println("Failed to create synchronization objects for a frame!");
			}
		}

		// Create command buffers
		m_CommandBuffers.resize(VulkanRenderer::MaxFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = VulkanDevice::Get().CommandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size())
		};

		if (vkAllocateCommandBuffers(device, &allocInfo, m_CommandBuffers.data()) !=
			VK_SUCCESS)
		{
			std::println("Failed to allocate command buffers!");
		}
	}

	VulkanRenderer::~VulkanRenderer()
	{
		auto device = VulkanDevice::Get().Device();

		vkDeviceWaitIdle(device);

		for (size_t i = 0; i < VulkanRenderer::MaxFramesInFlight; i++)
		{
			vkDestroySemaphore(device, m_DrawFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, m_ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, m_FrameInFlightFences[i], nullptr);
		}
	}

	VkCommandBuffer VulkanRenderer::BeginFrame()
	{
		auto device = VulkanDevice::Get().Device();

		vkWaitForFences(
			device,
			1,
			&m_FrameInFlightFences[m_CurrentFrameIndex],
			VK_TRUE,
			UINT64_MAX);

		auto result = m_Swapchain->AquireNextImage(&m_CurrentImageIndex, m_ImageAvailableSemaphores[m_CurrentFrameIndex]);

		//std::println("Frame {}, Image {}", m_CurrentFrameIndex, m_CurrentImageIndex);

		// Begin recording the command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_CommandBuffers[m_CurrentFrameIndex], &beginInfo) != VK_SUCCESS)
		{
			std::println("failed to begin recording command buffer!");
		}

		return m_CommandBuffers[m_CurrentFrameIndex];
	}

	void VulkanRenderer::BeginSwapchainRendering()
	{
		auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];

		VkImageSubresourceRange colourSubresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.levelCount = 1,
			.layerCount = 1
		};

		VkImageMemoryBarrier2 colourBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.image = m_Swapchain->GetImage(m_CurrentImageIndex),
			.subresourceRange = colourSubresourceRange
		};

		VkImageSubresourceRange depthSubresourceRange
		{
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.levelCount = 1,
			.layerCount = 1
		};

		VkImageMemoryBarrier2 depthBarrier
		{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.image = m_Swapchain->GetDepthImage(),
			.subresourceRange = depthSubresourceRange
		};

		std::array<VkImageMemoryBarrier2, 2> outputBarriers{ colourBarrier , depthBarrier };

		VkDependencyInfo barrierDependencyInfo{};
		barrierDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		barrierDependencyInfo.imageMemoryBarrierCount = 2;
		barrierDependencyInfo.pImageMemoryBarriers = outputBarriers.data();

		vkCmdPipelineBarrier2(commandBuffer, &barrierDependencyInfo);

		VkViewport viewport{};
		viewport.x = 0.0f;
		//viewport.y = static_cast<float>(m_Swapchain->GetExtent().height);
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Swapchain->GetExtent().width);
		//viewport.height = -static_cast<float>(m_Swapchain->GetExtent().height);
		viewport.height = static_cast<float>(m_Swapchain->GetExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		VkRect2D scissor{ {0, 0}, m_Swapchain->GetExtent() };
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		VkRenderingAttachmentInfoKHR colorAttachment{};
		colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		colorAttachment.imageView = m_Swapchain->GetImageView(m_CurrentImageIndex);
		colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.clearValue.color = { 0.1f, 0.1f, 0.7f, 0.0f };

		VkRenderingAttachmentInfoKHR depthStencilAttachment{};
		depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		depthStencilAttachment.imageView = m_Swapchain->GetDepthView();
		depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthStencilAttachment.clearValue.depthStencil = { 1.0f,  0 };

		VkRenderingInfoKHR renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.renderArea = { 0, 0, m_Swapchain->GetExtent().width, m_Swapchain->GetExtent().height };
		renderingInfo.layerCount = 1;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachment;
		renderingInfo.pDepthAttachment = &depthStencilAttachment;
		renderingInfo.pStencilAttachment = nullptr;

		vkCmdBeginRendering(commandBuffer, &renderingInfo);
	}

	void VulkanRenderer::EndSwapchainRendering()
	{
		auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];

		vkCmdEndRendering(commandBuffer);

		VkImageSubresourceRange colourSubresourceRange{};
		colourSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colourSubresourceRange.levelCount = 1;
		colourSubresourceRange.layerCount = 1;

		VkImageMemoryBarrier2 colourBarrier{};
		colourBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
		colourBarrier.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colourBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		colourBarrier.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colourBarrier.dstAccessMask = 0;
		colourBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colourBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		colourBarrier.image = m_Swapchain->GetImage(m_CurrentImageIndex);
		colourBarrier.subresourceRange = colourSubresourceRange;

		VkDependencyInfo barrierDependencyInfo{};
		barrierDependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		barrierDependencyInfo.imageMemoryBarrierCount = 1;
		barrierDependencyInfo.pImageMemoryBarriers = &colourBarrier;

		vkCmdPipelineBarrier2(commandBuffer, &barrierDependencyInfo);
	}

	void VulkanRenderer::EndFrame()
	{
		auto device = VulkanDevice::Get().Device();
		auto commandBuffer = m_CommandBuffers[m_CurrentFrameIndex];

		// Finish and submit the command buffer to the graphics queue
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			std::println("Failed to record command buffer!");
		}

		if (m_ImageInFlightFences[m_CurrentImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device, 1, &m_ImageInFlightFences[m_CurrentImageIndex], VK_TRUE, UINT64_MAX);
		}
		m_ImageInFlightFences[m_CurrentImageIndex] = m_FrameInFlightFences[m_CurrentFrameIndex];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrameIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { m_DrawFinishedSemaphores[m_CurrentFrameIndex] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device, 1, &m_FrameInFlightFences[m_CurrentFrameIndex]);
		if (vkQueueSubmit(VulkanDevice::Get().GraphicsQueue(), 1, &submitInfo, m_FrameInFlightFences[m_CurrentFrameIndex]) !=
			VK_SUCCESS)
		{
			std::println("Failed to submit draw command buffer!");
		}

		// Submit the swapchain image for presentation
		auto result = m_Swapchain->QueuePresent(&m_CurrentImageIndex, m_DrawFinishedSemaphores[m_CurrentFrameIndex]);

		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VulkanRenderer::MaxFramesInFlight;
	}

}