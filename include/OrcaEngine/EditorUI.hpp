#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>

class Window;
class VulkanContext;
class Swapchain;
class Renderer;

class EditorUI {
public:
    EditorUI();
    ~EditorUI();

    void Initialize(GLFWwindow* window, VulkanContext* vulkan_context, Swapchain* swapchain, Renderer* renderer);
    void Shutdown();

    void BeginFrame();
    void Draw();
    void EndFrame();
private:
    void DrawDebugPanel();

    static void check_vk_result(VkResult err);

    VulkanContext* _vulkan_context = nullptr;
    Swapchain* _swapchain = nullptr;
    Renderer* _renderer = nullptr;
};