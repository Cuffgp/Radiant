#pragma once

#include "Base.h"
#include "Window.h"
#include "VulkanRenderer.h"
#include "VulkanPipeline.h"

namespace Radiant {

	class Application
	{
	public:
		Application();
		~Application();

		void Run();

		Window& GetWindow() { return *m_Window; }
		static Application& Get() { return *s_Application; }

		void OnWindowResize(uint32_t width, uint32_t height);
	private:
		inline static Application* s_Application;
		Scope<Window> m_Window;
		Scope<VulkanRenderer> m_Renderer;
		Scope<VulkanPipeline> m_Pipeline;

	};

	Application* CreateApplication();

}