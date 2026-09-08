#include <OrcaEngine/Application.hpp>
#include <OrcaEngine/VulkanUtils.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

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
	InitEditorUI();
}

void Application::Run() 
{
	while (!_window.ShouldClose()) {
		glfwPollEvents();

		_editor_ui.BeginFrame();
		_editor_ui.Draw();
		_editor_ui.EndFrame();

		_renderer.DrawFrame(framebuffer_resized, ImGui::GetDrawData());
		framebuffer_resized = false;
	}
	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());
}

void Application::Shutdown() 
{
	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());

	_editor_ui.Shutdown();
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
	glfwSetFramebufferSizeCallback(_window.GetHandle(), Application::FramebufferResizeCallback);
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

void Application::InitEditorUI()
{
	_editor_ui.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain, &_renderer);
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebuffer_resized = true;
}
