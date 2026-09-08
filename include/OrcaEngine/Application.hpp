#pragma once

#include <OrcaEngine/Window.hpp>
#include <OrcaEngine/VulkanContext.hpp>
#include <OrcaEngine/Swapchain.hpp>
#include <OrcaEngine/Renderer.hpp>

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
	bool framebuffer_resized = false;

	void Shutdown();

	void InitWindow();
	void InitContext();
	void InitSwapchain();
	void InitRenderer();

	static void check_vk_result(VkResult err) {
		if (err == 0) return;
		std::cerr << "[vulkan] Error: VkResult = " << err << std::endl;
		if (err < 0)
			abort();
	}

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);


	Window _window;
	VulkanContext _vulkan_context;
	Swapchain _swapchain;
	Renderer _renderer;
};