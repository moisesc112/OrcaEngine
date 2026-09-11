#include <OrcaEngine/Core/Application.hpp>

#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>

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

		RenderBundle render_bundle = ExtractRenderBundle(_registry);

		_renderer.DrawFrame(_framebuffer_resized, _editor_ui.GetDrawData(), render_bundle);
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
	{
	Entity entity = _registry.CreateEntity();

	_registry.AddComponent<TransformComponent>(entity, TransformComponent{});
	_registry.AddComponent<MeshComponent>(entity, MeshComponent{ 0 });
	auto& transform = _registry.GetComponent<TransformComponent>(entity);
	//transform.position.x += 0.5f;

	}
	{
	Entity entity = _registry.CreateEntity();

	_registry.AddComponent<TransformComponent>(entity, TransformComponent{});
	_registry.AddComponent<MeshComponent>(entity, MeshComponent{ 0 });
	auto& transform = _registry.GetComponent<TransformComponent>(entity);
	//transform.position.x -= 0.5f;
	}
	//auto& transform = _registry.GetComponent<TransformComponent>(_entity);
}

void Application::InitEditorUI()
{
	_editor_ui.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain, &_renderer, &_registry);
	_editor_ui.SetSelectedEntity(_entity);
}

RenderBundle Application::ExtractRenderBundle(Registry& registry)
{
	std::vector<RenderItem> render_items;

	for (Entity entity : registry.View<MeshComponent, TransformComponent>()) {
		auto& mesh = registry.GetComponent<MeshComponent>(entity);
		auto& transform = registry.GetComponent<TransformComponent>(entity);
		if (entity.id == 0)
			transform.position.x += 0.0001f;
		else 
			transform.position.y += 0.0001f;
			
		render_items.push_back({mesh, transform});
		std::cout << "entity_id:  " << entity.id << std::endl;
	}
	std::cout << "render_items size:  " << render_items.size() << std::endl;

	RenderBundle render_bundle{render_items};

	return render_bundle;
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_framebuffer_resized = true;
}
