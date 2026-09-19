#include <OrcaEngine/Editor/EditorUI.hpp>
#include <OrcaEngine/Core/Window.hpp>
#include <OrcaEngine/Core/Logger.hpp>
#include <OrcaEngine/Rendering/VulkanContext.hpp>
#include <OrcaEngine/Rendering/Swapchain.hpp>
#include <OrcaEngine/Rendering/Renderer.hpp>
#include <OrcaEngine/ECS/Registry.hpp>
#include <OrcaEngine/Rendering/ShadowSettings.hpp>

#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/NameComponent.hpp>
#include <OrcaEngine/ECS/Components/LightComponent.hpp>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <iostream>
#include <filesystem>


EditorUI::EditorUI() {}

EditorUI::~EditorUI() {}

void EditorUI::Initialize(GLFWwindow* window, 
                          VulkanContext* vulkan_context, 
                          Swapchain* swapchain, 
                          Renderer* renderer,
                          Registry* registry)
{
    _vulkan_context = vulkan_context;
    _swapchain = swapchain;
    _renderer = renderer;
    _registry = registry;

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	const VkFormat swapchain_image_format = swapchain->GetFormat();

	ImGui_ImplGlfw_InitForVulkan(window, true);
	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = vulkan_context->GetInstance();
	init_info.PhysicalDevice = vulkan_context->GetPhysicalDevice();
	init_info.Device = vulkan_context->GetLogicalDevice();
	init_info.QueueFamily = vulkan_context->GetGraphicsQueueFamilyIndex();
	init_info.Queue = vulkan_context->GetGraphicsQueue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1000;
	init_info.MinImageCount = 2;
	init_info.ImageCount = static_cast<uint32_t>(swapchain->GetImages().size());
	init_info.Allocator = nullptr;
	init_info.UseDynamicRendering = true;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain_image_format;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = EditorUI::check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);
}

void EditorUI::DestroyViewportTexture()
{
    if (_viewport_descriptor_set == VK_NULL_HANDLE) {
        return;
    }

    ImGui_ImplVulkan_RemoveTexture(_viewport_descriptor_set);
    _viewport_descriptor_set = VK_NULL_HANDLE;
}

void EditorUI::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void EditorUI::BeginFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
} 

void EditorUI::Draw(bool& light_animation_enabled)
{
    DrawDockSpace();
    DrawMenuBar();
    DrawOpenScenePopup();
    DrawScenePanel();
    DrawViewport();
    DrawInspectorPanel();
    DrawDebugPanel(light_animation_enabled);
    DrawConsolePanel();
}

void EditorUI::EndFrame()
{
	ImGui::Render();
}

ImDrawData* EditorUI::GetDrawData()
{
    return ImGui::GetDrawData();
}

void EditorUI::DrawDockSpace()
{
    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
}

void EditorUI::DrawMenuBar()
{
    bool open_scene_popup = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open Scene")) {
                open_scene_popup = true;
            }

            if (ImGui::MenuItem("Save Scene")) {
                _save_scene_requested = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (open_scene_popup) {
        ImGui::OpenPopup("Open Scene");
    }
}

void EditorUI::DrawOpenScenePopup()
{
    if (ImGui::BeginPopupModal("Open Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::filesystem::path scenes_directory = "scenes";

        if (std::filesystem::exists(scenes_directory)) {
            for (const auto& entry : std::filesystem::directory_iterator(scenes_directory)) {
                if (!entry.is_regular_file()) {
                    continue;
                }

                if (entry.path().extension() != ".json") {
                    continue;
                }

                std::string filename = entry.path().filename().string();

                if (ImGui::Selectable(filename.c_str())) {
                    _scene_filepath = entry.path().string();

                    _open_scene_requested = true;

                    ImGui::CloseCurrentPopup();
                }
            }
        }
        else {
            ImGui::TextDisabled("Scenes directory not found");
        }

        ImGui::Separator();

        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void EditorUI::DrawDebugPanel(bool& light_animation_enabled)
{
    //ImGui::SetNextWindowPos(ImVec2(10.0f, 300.0f), ImGuiCond_Appearing);
    //ImGui::SetNextWindowSize(ImVec2(220.0f, 250.0f), ImGuiCond_Appearing);

    ImGui::Begin("Debug Options");

    if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Frame Time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);

        //ImGui::Text("Vertices: %u", _renderer->GetVertices().size());
        //ImGui::Text("Triangles: %u", _renderer->GetIndices().size() / 3);
        ImGui::Text("Draw Calls: %u", _renderer->GetDrawCallCounter());
        ImGui::Text("Resolution: %u x %u", _swapchain->GetExtent().width, _swapchain->GetExtent().height);
        ImGui::Text("MSAA: %ux", _vulkan_context->GetMsaaSamples());
    }

    if (ImGui::CollapsingHeader("Vulkan", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto physical_device_properties = _vulkan_context->GetPhysicalDeviceProperties();
        ImGui::Text("GPU: %s", physical_device_properties.deviceName);
        ImGui::Text("Engine API: Vulkan 1.3");
        ImGui::Text("Driver API: Vulkan %u.%u.%u",
                    VK_API_VERSION_MAJOR(physical_device_properties.apiVersion),
                    VK_API_VERSION_MINOR(physical_device_properties.apiVersion),
                    VK_API_VERSION_PATCH(physical_device_properties.apiVersion));

        ImGui::Text("Swapchain Images: %zu", _swapchain->GetImages().size());
    }
   
    if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
        ShadowSettings& shadow_settings = _renderer->GetShadowSettings();

        ImGui::Text("Enable Shadows");
        ImGui::SameLine(110.0f);
        ImGui::Checkbox("##Enable Shadows", &shadow_settings.enabled);

        const char* shadow_filters[] = { "Hard", "PCF" };
        int current_filter = static_cast<int>(shadow_settings.filter);

        ImGui::PushItemWidth(80.0f);

        ImGui::Text("Shadow Filter");
        ImGui::SameLine(110.0f);
        ImGui::Combo("##Shadow Filter", &current_filter, shadow_filters, IM_ARRAYSIZE(shadow_filters));
        shadow_settings.filter = static_cast<ShadowFilter>(current_filter);

        ImGui::Text("Constant Bias");
        ImGui::SameLine(110.0f);
        ImGui::DragFloat("##Constant Bias", &shadow_settings.constant_bias, 0.00001f, 0.0f, 1.0f, "%.5f");

        ImGui::Text("Slope Bias");
        ImGui::SameLine(110.0f);
        ImGui::DragFloat("##Slope Bias", &shadow_settings.slope_bias, 0.00001f, 0.0f, 1.0f, "%.5f");

        ImGui::PopItemWidth();
    }

    if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {

        LightComponent* light = nullptr;
        for (Entity entity : _registry->View<LightComponent>()) {
			light = &_registry->GetComponent<LightComponent>(entity);
            break;
		}

        if (light) {

            ImGui::Text("Animate Light");
            ImGui::SameLine(110.0f);
            ImGui::Checkbox("##Animate Light", &light_animation_enabled);

            ImGui::PushItemWidth(80.0f);

            ImGui::Text("Color");
            ImGui::SameLine(110.0f);
            ImGui::ColorEdit3("##Color", &light->color.x);

            ImGui::Text("Intensity");
            ImGui::SameLine(110.0f);
            ImGui::DragFloat("##Intensity", &light->intensity, 0.05f, 0.0f, 10.0f);

           ImGui::PopItemWidth();
        }


    }

    ImGui::End();
}

void EditorUI::DrawScenePanel()
{
    //ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Appearing);
    //ImGui::SetNextWindowSize(ImVec2(220.0f, 250.0f), ImGuiCond_Appearing);
    ImGui::Begin("Scene");

    for (Entity entity : _registry->GetAliveEntities()) {

        std::string entity_name = "Entity: " + std::to_string(entity.id);

        if (_registry->HasComponent<NameComponent>(entity)) {
            entity_name = _registry->GetComponent<NameComponent>(entity).name;
        }

        bool is_selected = entity == _selected_entity;

        if (ImGui::Selectable(entity_name.c_str(), is_selected)) {
            SetSelectedEntity(entity);
        }
    }

    ImGui::End();
}

void EditorUI::DrawInspectorPanel()
{
    //ImGui::SetNextWindowPos(ImVec2(950.0f, 10.0f), ImGuiCond_Appearing);
    //ImGui::SetNextWindowSize(ImVec2(220.0f, 250.0f), ImGuiCond_Appearing);
    ImGui::Begin("Inspector");

    if (_selected_entity && _registry->IsAlive(*_selected_entity)) {
        std::string entity_name = _registry->GetComponent<NameComponent>(*_selected_entity).name;
        ImGui::Text(entity_name.c_str());

        if (_registry->HasComponent<TransformComponent>(*_selected_entity)) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                auto& entity_transform = _registry->GetComponent<TransformComponent>(*_selected_entity);
                ImGui::Text("Position");
                ImGui::SameLine(90.0f);
                ImGui::DragFloat3("##Position", &entity_transform.position.x, 0.05f);

                ImGui::Text("Rotation");
                ImGui::SameLine(90.0f);
                ImGui::DragFloat3("##Rotation", &entity_transform.rotation.x, 1.0f);

                ImGui::Text("Scale");
                ImGui::SameLine(90.0f);
                ImGui::DragFloat3(
                    "##Scale",
                    &entity_transform.scale.x,
                    0.01f,
                    0.01f,
                    100.0f
                );
            }
        }

        if (_registry->HasComponent<MaterialComponent>(*_selected_entity)) {
            ImGui::Separator();
            if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {

                auto& material_component = _registry->GetComponent<MaterialComponent>(*_selected_entity);

                MaterialResource& material = _renderer->GetMaterial(material_component.material_id);
            
                ImGui::Text("Use Texture");
                ImGui::SameLine(90.0f);
                ImGui::Checkbox("##Use Texture", &material.use_texture);

                ImGui::Text("Color");
                ImGui::SameLine(90.0f);
                ImGui::ColorEdit3("##Color", &material.color.x);
            }
        }  
    }
    else {
        _selected_entity.reset();
    }

    ImGui::End();
}

void EditorUI::DrawViewport()
{   
    //ImGui::SetNextWindowPos(ImVec2(250.0f, 10.0f), ImGuiCond_Appearing);
    //ImGui::SetNextWindowSize(ImVec2(680.0f, 550.0f), ImGuiCond_Appearing);

    ImGui::Begin("Viewport");

    _is_viewport_hovered = ImGui::IsWindowHovered();
    _is_viewport_focused = ImGui::IsWindowFocused();

    ImVec2 viewport_size = ImGui::GetContentRegionAvail();

    _viewport_extent.width = static_cast<std::uint32_t>(std::max(viewport_size.x, 0.0f));
    _viewport_extent.height = static_cast<std::uint32_t>(std::max(viewport_size.y, 0.0f));

    if (_viewport_descriptor_set != VK_NULL_HANDLE) {
        ImGui::Image(_viewport_descriptor_set, viewport_size);
    }

    ImGui::End();
}

void EditorUI::DrawConsolePanel()
{
    ImGui::Begin("Console");

    if (ImGui::Button("Clear")) {
        Logger::Clear();
    }
    
    ImGui::Separator();

    for (const LogMessage& log : Logger::GetMessages()) {
        switch (log.level) {
            case LogLevel::Info:
                ImGui::Text("[Info] %s", log.message.c_str());
                break;
            
            case LogLevel::Warning:
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[Warning] %s", log.message.c_str());
                break;
            
            case LogLevel::Error:
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[Error] %s", log.message.c_str());
                break;
        }
    }
    
    ImGui::End();
}

void EditorUI::SetSelectedEntity(Entity entity)
{
    _selected_entity = entity;
}

void EditorUI::SetViewportTexture(VkSampler sampler, VkImageView image_view)
{
    _viewport_descriptor_set = ImGui_ImplVulkan_AddTexture(sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void EditorUI::ClearSceneRequests()
{
    _save_scene_requested = false;
    _open_scene_requested = false;
}


void EditorUI::check_vk_result(VkResult err) 
{
    if (err == 0) return;
    std::cerr << "[vulkan] Error: VkResult = " << err << std::endl;
    if (err < 0)
        abort();
}