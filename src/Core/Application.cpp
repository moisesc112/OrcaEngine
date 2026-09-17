#include <OrcaEngine/Core/Application.hpp>
#include <OrcaEngine/Rendering/RenderExtraction.hpp>
#include <OrcaEngine/Scene/SceneSerializer.hpp>

#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/NameComponent.hpp>
#include <OrcaEngine/ECS/Components/LightComponent.hpp>

#include <glm/glm.hpp>

#include <chrono>

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
	auto last_time = std::chrono::high_resolution_clock::now();

	while (!_window.ShouldClose()) {
		auto current_time = std::chrono::high_resolution_clock::now();
		float delta_time = std::chrono::duration<float>(current_time - last_time).count();
		last_time = current_time;

		glfwPollEvents();

		_editor_ui.BeginFrame();
		_editor_ui.Draw();
		_editor_ui.EndFrame();

		UpdateCamera(delta_time);
		UpdateScene();
		RenderFrame();

		UpdateViewport();
	}
}

void Application::Shutdown() 
{
	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());

	_editor_ui.DestroyViewportTexture();
	_renderer.DestroyViewportResources();
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
	SceneSerializer::Deserialize(_registry, "scenes/default.json");
}

void Application::InitEditorUI()
{
	_editor_ui.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain, &_renderer, &_registry);
	//_editor_ui.SetSelectedEntity(_entity);
}

void Application::UpdateCamera(float delta_time)
{
	GLFWwindow* window = _window.GetHandle();

	bool is_right_mouse_pressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

	if (!_is_camera_look_active && _editor_ui.IsViewportHovered() && is_right_mouse_pressed) {
		_is_camera_look_active = true;

		glfwGetCursorPos(window, &_last_mouse_x, &_last_mouse_y);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	if (_is_camera_look_active && !is_right_mouse_pressed) {
		_is_camera_look_active = false;

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (_is_camera_look_active) {
		float distance = _camera_move_speed * delta_time;

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			_camera.MoveForward(distance);
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			_camera.MoveForward(-distance);
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			_camera.MoveRight(-distance);
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			_camera.MoveRight(distance);
		}
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			_camera.MoveUp(distance);
		}
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			_camera.MoveUp(-distance);
		}

		UpdateCameraRotation();
	}
}

void Application::UpdateCameraRotation()
{
	double mouse_x;
	double mouse_y;

	glfwGetCursorPos(_window.GetHandle(), &mouse_x, &mouse_y);

	float delta_x = static_cast<float>(mouse_x - _last_mouse_x);
	float delta_y = static_cast<float>(mouse_y - _last_mouse_y);

	_last_mouse_x = mouse_x;
	_last_mouse_y = mouse_y;

	_camera.AddYaw(-delta_x * _camera_mouse_sensitivity);
	_camera.AddPitch(-delta_y * _camera_mouse_sensitivity);
}

void Application::UpdateViewport()
{
	VkExtent2D new_extent = _editor_ui.GetViewportExtent();
	VkExtent2D current_extent = _renderer.GetViewportExtent();

	if (new_extent.width == 0 || new_extent.height == 0) {
		return;
	}

	if (current_extent.width == 0 || current_extent.height == 0) {
		_renderer.CreateViewportResources(new_extent);
		_editor_ui.SetViewportTexture(_renderer.GetViewportSampler(), _renderer.GetViewportImageView());

		return;
	}

	if (new_extent.width != current_extent.width || new_extent.height != current_extent.height) {
		vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());
		
		_editor_ui.DestroyViewportTexture();

		_renderer.RecreateViewportResources(new_extent);

		_editor_ui.SetViewportTexture(_renderer.GetViewportSampler(), _renderer.GetViewportImageView());
	}
}

void Application::UpdateScene()
{
	{
	auto& transform = _registry.GetComponent<TransformComponent>(static_cast<Entity>(0));
		//transform.position.x += 0.0001f;
	}

	{
	auto& transform = _registry.GetComponent<TransformComponent>(static_cast<Entity>(1));
		//transform.position.y += 0.0001f;
	}
}

void Application::RenderFrame()
{
	VkExtent2D viewport_extent = _renderer.GetViewportExtent();

	//if (viewport_extent.width == 0 || viewport_extent.height == 0) {
	//	return;
	//}

	RenderBundle render_bundle = RenderExtraction::ExtractRenderBundle(_registry);

	_renderer.DrawFrame(_framebuffer_resized, 
						_editor_ui.GetDrawData(), 
						render_bundle, 
						viewport_extent,
						_camera);

	_framebuffer_resized = false;
}

void Application::CreateDefaultScene()
{
	{
		Entity entity = _registry.CreateEntity();

		_registry.AddComponent<NameComponent>(entity, NameComponent{ .name = "Directional Light"});
		_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(0.0f),
																			   .rotation = glm::vec3(-45.0f, 30.0f, 0.0f),
																			   .scale = glm::vec3(1.0f) });

		_registry.AddComponent<LightComponent>(entity, LightComponent{ .color = glm::vec3(1.0f),
																	   .intensity = 1.0f });
	}

	{
		Entity entity = _registry.CreateEntity();

		_registry.AddComponent<NameComponent>(entity, NameComponent{ .name = "Viking Room" });
		_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(0.0f),
																			   .rotation = glm::vec3(0.0f),
																			   .scale = glm::vec3(1.0f) });

		_registry.AddComponent<MeshComponent>(entity, MeshComponent{ .mesh_id = 0 });
		_registry.AddComponent<MaterialComponent>(entity, MaterialComponent { .material_id = 0 });
	}

	{
		Entity entity = _registry.CreateEntity();

		_registry.AddComponent<NameComponent>(entity, NameComponent{ .name = "Iron Golem" });
		_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(-1.0f, 1.0f, 0.0f),
																			   .rotation = glm::vec3(90.0f, -90.0f, 0.0f),
																			   .scale = glm::vec3(0.05f) });

		_registry.AddComponent<MeshComponent>(entity, MeshComponent{ .mesh_id = 1 });
		_registry.AddComponent<MaterialComponent>(entity, MaterialComponent { .material_id = 1 });
	}

	{
		Entity entity = _registry.CreateEntity();

		_registry.AddComponent<NameComponent>(entity, NameComponent{ .name = "Grass Block" });
		_registry.AddComponent<TransformComponent>(entity, TransformComponent{ .position = glm::vec3(-1.0f, 1.0f, -2.5f),
																			   .rotation = glm::vec3(90.0f, 0.0f, 0.0f),
																			   .scale = glm::vec3(1.0f) });

		_registry.AddComponent<MeshComponent>(entity, MeshComponent{ .mesh_id = 2 });
		_registry.AddComponent<MaterialComponent>(entity, MaterialComponent { .material_id = 2 });
	}
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->_framebuffer_resized = true;
}
