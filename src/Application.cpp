#include <OrcaEngine/Application.hpp>
#include <OrcaEngine/VulkanUtils.hpp>

Application::Application() {}

Application::~Application()
{
	cleanup();
}

void Application::init()
{
	initWindow();
	initContext();
	initSwapchain();
	initRenderer();
}

void Application::run() 
{
	while (!_window.shouldClose()) {
		glfwPollEvents();
		_renderer.drawFrame();
	}

	vkDeviceWaitIdle(_vulkanContext.getLogicalDevice());
}

void Application::cleanup() 
{
	_renderer.cleanupSwapchainResources();
	_swapChain.cleanupSwapChain();
	_renderer.cleanup();
	_vulkanContext.cleanup();
	_window.cleanup();
}

void Application::initWindow() 
{
	_window.init();
	glfwSetWindowUserPointer(_window.getHandle(), this);
	glfwSetFramebufferSizeCallback(_window.getHandle(), Renderer::framebufferResizeCallback);
}


void Application::initContext() 
{
	_vulkanContext.init(_window.getHandle());
}

void Application::initSwapchain()
{
	_swapChain.init(_window.getHandle(), _vulkanContext);
}

void Application::initRenderer()
{
	_renderer.init(_window.getHandle(), &_vulkanContext, &_swapChain);
}

