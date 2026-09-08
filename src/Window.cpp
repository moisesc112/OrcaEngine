#include <OrcaEngine/Window.hpp>

Window::Window() {}

Window::~Window() {}

void Window::Initialize()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Window::Shutdown()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(_window);
}