
#include "Application.h"

#include <filesystem>
#include "hlsl++.h"

namespace Radiant {

	Application::Application()
	{
		s_Application = this;

		m_Window = CreateScope<Window>();
		m_Window->SetResizeCallback(RD_BIND_FN(Application::OnWindowResize));

		m_Renderer = CreateScope<VulkanRenderer>();
		m_Pipeline = CreateScope<VulkanPipeline>("assets/simple_shader.spv");

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

	void Application::OnWindowResize(uint32_t width, uint32_t height)
	{
		//std::println("Window resized to {}x{}", width, height);
		m_Renderer->OnWindowResize(width, height);
	}
}
