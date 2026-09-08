#include <OrcaEngine/EditorUI.hpp>
#include <OrcaEngine/Window.hpp>
#include <OrcaEngine/VulkanContext.hpp>
#include <OrcaEngine/Swapchain.hpp>
#include <OrcaEngine/Renderer.hpp>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <iostream>


EditorUI::EditorUI() {}

EditorUI::~EditorUI() {}

void EditorUI::Initialize(GLFWwindow* window, VulkanContext* vulkan_context, Swapchain* swapchain, Renderer* renderer)
{
    _vulkan_context = vulkan_context;
    _swapchain = swapchain;
    _renderer = renderer;

    IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

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

void EditorUI::Draw()
{
    DrawDebugPanel();
}

void EditorUI::EndFrame()
{
	ImGui::Render();
}

ImDrawData* EditorUI::GetDrawData()
{
    return ImGui::GetDrawData();
}

void EditorUI::DrawDebugPanel()
{
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(220.0f, 220.0f), ImGuiCond_Appearing);

    ImGui::Begin("Debug Options");

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.2f ms", 1000.0f / ImGui::GetIO().Framerate);

    ImGui::SeparatorText("Rendering");

    ImGui::Text("Vertices: %u", _renderer->GetVertices().size());
    ImGui::Text("Triangles: %u", _renderer->GetIndices().size() / 3);
    ImGui::Text("Draw Calls: %u", _renderer->GetDrawCallCounter());

    ImGui::SeparatorText("Vulkan");

    ImGui::Text("Resolution: %u x %u", _swapchain->GetExtent().width, _swapchain->GetExtent().height);
    ImGui::Text("MSAA: %ux", _vulkan_context->GetMsaaSamples());

    ImGui::End();
}

void EditorUI::check_vk_result(VkResult err) 
{
    if (err == 0) return;
    std::cerr << "[vulkan] Error: VkResult = " << err << std::endl;
    if (err < 0)
        abort();
}