#include <OrcaEngine/Core/Application.hpp>
#include <OrcaEngine/Rendering/RenderExtraction.hpp>

#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>

#include <glm/glm.hpp>

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

		UpdateScene();
		RenderFrame();
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
	{
	Entity entity = _registry.CreateEntity();

	_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(0.0f),
																		   .rotation = glm::vec3(0.0f),
																		   .scale = glm::vec3(1.0f) });

	_registry.AddComponent<MeshComponent>(entity, MeshComponent{ .mesh_id = 0 });
	_registry.AddComponent<MaterialComponent>(entity, MaterialComponent { .material_id = 0 });
	}

	{
	Entity entity = _registry.CreateEntity();

	_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(-1.0f, 1.0f, 0.0f),
																		   .rotation = glm::vec3(90.0f, -90.0f, 0.0f),
																		   .scale = glm::vec3(0.05f) });

	_registry.AddComponent<MeshComponent>(entity, MeshComponent{ .mesh_id = 1 });
	_registry.AddComponent<MaterialComponent>(entity, MaterialComponent { .material_id = 1 });
	}
}

void Application::InitEditorUI()
{
	_editor_ui.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain, &_renderer, &_registry);
	_editor_ui.SetSelectedEntity(_entity);
}

void Application::UpdateScene()
{
	{
	auto& transform = _registry.GetComponent<TransformComponent>(static_cast<Entity>(0));
		transform.position.x += 0.0001f;
	}

	{
	auto& transform = _registry.GetComponent<TransformComponent>(static_cast<Entity>(1));
		transform.position.y += 0.0001f;
	}
}

void Application::RenderFrame()
{
	RenderBundle render_bundle = RenderExtraction::ExtractRenderBundle(_registry);

	_renderer.DrawFrame(_framebuffer_resized, _editor_ui.GetDrawData(), render_bundle);
	_framebuffer_resized = false;
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_framebuffer_resized = true;
}
