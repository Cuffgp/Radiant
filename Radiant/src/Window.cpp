#include <Window.h>

#include <print>

namespace Radiant{

	void ErrorCallback(int error, const char* description)
	{
		std::println(stderr, "[GLFW] {}", description);
	}

	Window::Window()
	{
		if (!glfwInit())
		{
			std::println("Failed to initilize window");
		}

		glfwSetErrorCallback(ErrorCallback);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_Window = glfwCreateWindow(1280, 720, "Radiant Engine", NULL, NULL);

		m_Data.Width = 1280;
		m_Data.Height = 720;
		m_Data.Title = "Radiant Engine";

		if (!m_Window)
		{
			std::println("Failed to open window");
		}

		glfwSetWindowUserPointer(m_Window, &m_Data);

		glfwSetKeyCallback(m_Window, nullptr);
		glfwSetCharCallback(m_Window, nullptr);
		glfwSetCursorPosCallback(m_Window, nullptr);
		glfwSetMouseButtonCallback(m_Window, nullptr);
		glfwSetScrollCallback(m_Window, nullptr);
		glfwSetWindowCloseCallback(m_Window, nullptr);
		glfwSetFramebufferSizeCallback(m_Window, nullptr);

		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				auto& data = *(WindowData*)(glfwGetWindowUserPointer(window));

				data.Width = width;
				data.Height = height;

				data.ResizeCallback(width, height);
			});

	}

	Window::~Window()
	{
		glfwTerminate();
	}

}