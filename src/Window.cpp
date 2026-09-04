#include <OrcaEngine/Window.hpp>

Window::Window() {}

Window::~Window() {}

void Window::init()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void Window::cleanup()
{
	glfwDestroyWindow(_window);
	glfwTerminate();
}

bool Window::shouldClose()
{
	return glfwWindowShouldClose(_window);
}