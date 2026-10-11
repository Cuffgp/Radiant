#pragma once

#include<functional>
#include <string>

#include "volk.h"
#include <GLFW/glfw3.h>

namespace Radiant {

	using WindowResizeFunction = std::function<void(uint32_t width, uint32_t height)>;

	class Window
	{
	public:
		Window();
		~Window();

		void PollEvents() { glfwPollEvents(); }
		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }
		GLFWwindow* GetWindowPointer() { return m_Window; }

		void SetResizeCallback(const WindowResizeFunction& function) { m_Data.ResizeCallback = function; }
	private:
		struct WindowData
		{
			uint32_t Width;
			uint32_t Height;
			std::string Title;

			WindowResizeFunction ResizeCallback = nullptr;
		};

		WindowData m_Data;
		GLFWwindow* m_Window;
	};

}