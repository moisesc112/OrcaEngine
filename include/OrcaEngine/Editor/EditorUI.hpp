#pragma once

#include <OrcaEngine/ECS/Entity.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <optional>

class Window;
class VulkanContext;
class Swapchain;
class Renderer;
class Registry;

struct GLFWwindow;
struct ImDrawData;

class EditorUI {
public:
    EditorUI();
    ~EditorUI();

    void Initialize(GLFWwindow* window, 
                    VulkanContext* vulkan_context, 
                    Swapchain* swapchain, 
                    Renderer* renderer,
                    Registry* registry);

    void Shutdown();

    void BeginFrame();
    void Draw();
    void EndFrame();

    ImDrawData* GetDrawData();

    void SetSelectedEntity(Entity entity);

private:
    void DrawDebugPanel();
    void DrawScenePanel();
    void DrawInspectorPanel();

    static void check_vk_result(VkResult err);

    VulkanContext* _vulkan_context = nullptr;
    Swapchain* _swapchain = nullptr;
    Renderer* _renderer = nullptr;
    Registry* _registry = nullptr;

    std::optional<Entity> _selected_entity;
};