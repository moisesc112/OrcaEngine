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

	void init();
	void run();

private:
	void cleanup();

	void initWindow();
	void initContext();
	void initSwapchain();
	void initRenderer();

	Window _window;
	VulkanContext _vulkanContext;
	Swapchain _swapChain;
	Renderer _renderer;
};