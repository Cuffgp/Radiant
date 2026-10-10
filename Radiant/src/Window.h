#pragma once

#include <string>

#include "volk.h"
#include <GLFW/glfw3.h>

namespace Radiant {

	class Window
	{
	public:
		Window();
		~Window();

		void PollEvents() { glfwPollEvents(); }
		bool ShouldClose() { return glfwWindowShouldClose(m_Window); }
		GLFWwindow* GetWindowPointer() { return m_Window; }
	private:
		GLFWwindow* m_Window;
		uint32_t m_Width;
		uint32_t m_Height;
		std::string m_Title;
	};

}