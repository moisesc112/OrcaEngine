#pragma once

#include <OrcaEngine/ECS/Entity.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <optional>
#include <string>

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
    void DestroyViewportTexture();

    void BeginFrame();
    void Draw(bool& light_animation_enabled);
    void EndFrame();

    ImDrawData* GetDrawData();

    void SetSelectedEntity(Entity entity);
    void SetViewportTexture(VkSampler sampler, VkImageView image_view);

    VkExtent2D& GetViewportExtent() { return _viewport_extent; }

    bool IsViewportHovered() { return _is_viewport_hovered; }
    bool IsViewportFocused() { return _is_viewport_focused; }

    bool IsSaveSceneRequested() { return _save_scene_requested; }
    bool IsOpenSceneRequested() { return _open_scene_requested; }

    std::string& GetSceneFilepath() { return _scene_filepath; }

    void ClearSceneRequests();

private:
    void DrawDockSpace();
    void DrawMenuBar();
    void DrawOpenScenePopup();
    void DrawDebugPanel(bool& light_animation_enabled);
    void DrawScenePanel();
    void DrawInspectorPanel();
    void DrawViewport();
    void DrawConsolePanel();

    static void check_vk_result(VkResult err);

    VulkanContext* _vulkan_context = nullptr;
    Swapchain* _swapchain = nullptr;
    Renderer* _renderer = nullptr;
    Registry* _registry = nullptr;

    bool _is_viewport_hovered = false;
    bool _is_viewport_focused = false;

    VkDescriptorSet _viewport_descriptor_set = VK_NULL_HANDLE;
    VkExtent2D _viewport_extent = { 0, 0 };

    bool _save_scene_requested = false;
    bool _open_scene_requested = false;
    std::string _scene_filepath = "scenes/default.json";

    std::optional<Entity> _selected_entity;
};