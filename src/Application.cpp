#include <OrcaEngine/Application.hpp>
#include <OrcaEngine/VulkanUtils.hpp>

Application::Application() {}

Application::~Application()
{
	Shutdown();
}

void Application::Initialize()
{
	InitWindow();
	InitContext();
	InitSwapchain();
	InitRenderer();
}

void Application::Run() 
{
	while (!_window.ShouldClose()) {
		glfwPollEvents();
		_renderer.DrawFrame();
	}

	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());
}

void Application::Shutdown() 
{
	_renderer.DestroySwapchainResources();
	_swapchain.Shutdown();
	_renderer.Shutdown();
	_vulkan_context.Shutdown();
	_window.Shutdown();
}

void Application::InitWindow() 
{
	_window.Initialize();
	glfwSetWindowUserPointer(_window.GetHandle(), this);
	glfwSetFramebufferSizeCallback(_window.GetHandle(), Renderer::FramebufferResizeCallback);
}


void Application::InitContext() 
{
	_vulkan_context.Initialize(_window.GetHandle());
}

void Application::InitSwapchain()
{
	_swapchain.Initialize(_window.GetHandle(), _vulkan_context);
}

void Application::InitRenderer()
{
	_renderer.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain);
}

