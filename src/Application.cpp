#include <OrcaEngine/Application.hpp>
#include <OrcaEngine/VulkanUtils.hpp>

Application::Application() {}

Application::~Application()
{
	cleanup();
}

void Application::run() {
	initWindow();
	initVulkan();
	mainLoop();
}

void Application::initWindow() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, Renderer::framebufferResizeCallback);
}

void Application::initVulkan() {
	
	initContext();
	initSwapchain();
	initRenderer();
}

void Application::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		_renderer.drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void Application::cleanup() {

	_renderer.cleanupSwapchainResources();

	_swapChain.cleanupSwapChain();

	_renderer.cleanup();

	vulkanContext.cleanup();

	glfwDestroyWindow(window);

	glfwTerminate();
}

void Application::initContext() 
{
	vulkanContext.init(window);

	device = vulkanContext.getLogicalDevice();
}

void Application::initSwapchain()
{
	_swapChain.init(window, vulkanContext);
}

void Application::initRenderer()
{
	_renderer.init(window, &vulkanContext, &_swapChain);
}

