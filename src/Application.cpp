#include <OrcaEngine/Application.hpp>

#include <OrcaEngine/TransformComponent.hpp>
#include <OrcaEngine/MeshComponent.hpp>

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
	InitRegistry();
	InitEditorUI();
}

void Application::Run() 
{
	while (!_window.ShouldClose()) {
		glfwPollEvents();

		_editor_ui.BeginFrame();
		_editor_ui.Draw();
		_editor_ui.EndFrame();

		auto& transform = _registry.GetComponent<TransformComponent>(_entity);
		auto& mesh = _registry.GetComponent<MeshComponent>(_entity);
		std::cout << "transform position: " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << std::endl;
		transform.position.x += 0.0001f; 

		_renderer.DrawFrame(_framebuffer_resized, _editor_ui.GetDrawData(), transform);
		_framebuffer_resized = false;
	}
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

void Application::InitRegistry()
{
	_entity = _registry.CreateEntity();

	_registry.AddComponent<TransformComponent>(_entity, TransformComponent{});
	_registry.AddComponent<MeshComponent>(_entity, MeshComponent{ 0 });

	auto& transform = _registry.GetComponent<TransformComponent>(_entity);
	//transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
	std::cout << "transform position: " << transform.position.x << ", " << transform.position.y << ", " << transform.position.z << std::endl;
}

void Application::InitEditorUI()
{
	_editor_ui.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain, &_renderer, &_registry);
	_editor_ui.SetSelectedEntity(_entity);
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_framebuffer_resized = true;
}
