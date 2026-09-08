#include <OrcaEngine/Application.hpp>
#include <OrcaEngine/VulkanUtils.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

Application::Application() {}

Application::~Application()
{
	Shutdown();
}

void Application::Initialize()
{
	InitWindow();
	InitContext();
	InitSwapchain();
	InitRenderer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	const VkFormat swapchain_image_format = _swapchain.GetFormat();

	ImGui_ImplGlfw_InitForVulkan(_window.GetHandle(), true);
	ImGui_ImplVulkan_InitInfo init_info{};
	init_info.ApiVersion = VK_API_VERSION_1_3;
	init_info.Instance = _vulkan_context.GetInstance();
	init_info.PhysicalDevice = _vulkan_context.GetPhysicalDevice();
	init_info.Device = _vulkan_context.GetLogicalDevice();
	init_info.QueueFamily = _vulkan_context.GetGraphicsQueueFamilyIndex();
	init_info.Queue = _vulkan_context.GetGraphicsQueue();
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = VK_NULL_HANDLE;
	init_info.DescriptorPoolSize = 1000;
	init_info.MinImageCount = 2;
	init_info.ImageCount = static_cast<uint32_t>(_swapchain.GetImages().size());
	init_info.Allocator = nullptr;
	init_info.UseDynamicRendering = true;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain_image_format;
	init_info.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
	init_info.PipelineInfoMain.RenderPass = VK_NULL_HANDLE;
	init_info.PipelineInfoMain.Subpass = 0;
	init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = check_vk_result;
	ImGui_ImplVulkan_Init(&init_info);
}

void Application::Run() 
{
	while (!_window.ShouldClose()) {
		glfwPollEvents();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();
		ImGui::Render();
		
		_renderer.DrawFrame(framebuffer_resized, ImGui::GetDrawData());
		framebuffer_resized = false;
	}
	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());
}

void Application::Shutdown() 
{
	vkDeviceWaitIdle(_vulkan_context.GetLogicalDevice());

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	_renderer.DestroySwapchainResources();
	_swapchain.Shutdown();
	_renderer.Shutdown();
	_vulkan_context.Shutdown();
	_window.Shutdown();
}

void Application::InitWindow() 
{
	_window.Initialize();
	glfwSetWindowUserPointer(_window.GetHandle(), this);
	glfwSetFramebufferSizeCallback(_window.GetHandle(), Application::FramebufferResizeCallback);
}


void Application::InitContext() 
{
	_vulkan_context.Initialize(_window.GetHandle());
}

void Application::InitSwapchain()
{
	_swapchain.Initialize(_window.GetHandle(), _vulkan_context);
}

void Application::InitRenderer()
{
	_renderer.Initialize(_window.GetHandle(), &_vulkan_context, &_swapchain);
}

void Application::FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	app->framebuffer_resized = true;
}
