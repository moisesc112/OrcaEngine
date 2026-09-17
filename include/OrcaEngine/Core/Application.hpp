#pragma once

#include <OrcaEngine/Core/Window.hpp>
#include <OrcaEngine/Rendering/VulkanContext.hpp>
#include <OrcaEngine/Rendering/Swapchain.hpp>
#include <OrcaEngine/Rendering/Renderer.hpp>
#include <OrcaEngine/Editor/EditorUI.hpp>
#include <OrcaEngine/ECS/Registry.hpp>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <optional>
#include <set>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <unordered_map>

class Application {
public:
	Application();
	~Application();

	void Initialize();
	void Run();

private:
	bool _framebuffer_resized = false;

	bool _is_camera_look_active = false;

	double _last_mouse_x = 0.0;
	double _last_mouse_y = 0.0;

	float _camera_move_speed = 6.0f;
	float _camera_mouse_sensitivity = 0.1f;

	void Shutdown();

	void InitWindow();
	void InitContext();
	void InitSwapchain();
	void InitRenderer();
	void InitEditorUI();
	void InitRegistry();

	void UpdateCamera(float delta_time);
	void UpdateCameraRotation();
	void UpdateViewport();
	void UpdateScene(float delta_time);
	void RenderFrame();

	void CreateDefaultScene();

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

	Window _window;
	VulkanContext _vulkan_context;
	Swapchain _swapchain;
	Renderer _renderer;
	EditorUI _editor_ui;
	Registry _registry;
	Camera _camera;
	Entity _entity{};

	bool _light_animation_enabled = false;
};