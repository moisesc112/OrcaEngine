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
    void DestroyViewportTexture();

    void BeginFrame();
    void Draw();
    void EndFrame();

    ImDrawData* GetDrawData();

    void SetSelectedEntity(Entity entity);
    void SetViewportTexture(VkSampler sampler, VkImageView image_view);

    VkExtent2D& GetViewportExtent() { return _viewport_extent; }

private:
    void DrawDebugPanel();
    void DrawScenePanel();
    void DrawInspectorPanel();
    void DrawViewport();

    static void check_vk_result(VkResult err);

    VulkanContext* _vulkan_context = nullptr;
    Swapchain* _swapchain = nullptr;
    Renderer* _renderer = nullptr;
    Registry* _registry = nullptr;

    VkDescriptorSet _viewport_descriptor_set = VK_NULL_HANDLE;
    VkExtent2D _viewport_extent = { 0, 0 };

    std::optional<Entity> _selected_entity;
};