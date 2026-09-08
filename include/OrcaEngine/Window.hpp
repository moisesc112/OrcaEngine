#pragma once

#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Window {
public:
	Window();
	~Window();

	void Initialize();
	void Shutdown();
	bool ShouldClose();

	GLFWwindow* GetHandle() { return _window; }
private:
	GLFWwindow* _window = nullptr;
};