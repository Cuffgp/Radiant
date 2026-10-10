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

		m_Width = 1280;
		m_Height = 720;
		m_Title = "Radiant Engine";

		if (!m_Window)
		{
			std::println("Failed to open window");
		}

	}

	Window::~Window()
	{
		glfwTerminate();
	}

}