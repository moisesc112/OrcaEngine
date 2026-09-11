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
#include <chrono>
#include <unordered_map>

class Application {
public:
	Application();
	~Application();

	void Initialize();
	void Run();

private:
	bool _framebuffer_resized = false;

	void Shutdown();

	void InitWindow();
	void InitContext();
	void InitSwapchain();
	void InitRenderer();
	void InitEditorUI();
	void InitRegistry();

	RenderBundle ExtractRenderBundle(Registry& registry);

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);


	Window _window;
	VulkanContext _vulkan_context;
	Swapchain _swapchain;
	Renderer _renderer;
	EditorUI _editor_ui;
	Registry _registry;
	Entity _entity{};
};