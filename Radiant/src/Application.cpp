
#include "Application.h"

#include <filesystem>

namespace Radiant {

	Application::Application()
	{
		s_Application = this;

		m_Window = CreateScope<Window>();
		m_Renderer = CreateScope<VulkanRenderer>();
		m_Pipeline = CreateScope<VulkanPipeline>("Path");

		std::println("Current Path: {}", std::filesystem::current_path().string());
	}

	Application::~Application()
	{
		vkDeviceWaitIdle(VulkanDevice::Get().Device());
	}

	void Application::Run()
	{
		while (!m_Window->ShouldClose())
		{
			m_Window->PollEvents();

			auto cmd = m_Renderer->BeginFrame();
			m_Renderer->BeginSwapchainRendering();

			m_Pipeline->Bind(cmd);
			vkCmdDraw(cmd, 3, 1, 0, 0);

			m_Renderer->EndSwapchainRendering();
			m_Renderer->EndFrame();
			//std::println("Frame Finished");
		}
	}
}
