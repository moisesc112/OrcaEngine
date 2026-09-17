#include <OrcaEngine/Rendering/Renderer.hpp>
#include <OrcaEngine/Rendering/VulkanContext.hpp>
#include <OrcaEngine/Rendering/Swapchain.hpp>
#include <OrcaEngine/Rendering/VulkanUtils.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <fstream>
#include <chrono>

Renderer::Renderer() {}

Renderer::~Renderer() {}

void Renderer::Initialize(GLFWwindow* window, VulkanContext* vulkan_context, Swapchain* swapchain)
{
	_window = window;
	_vulkan_context = vulkan_context;
	_swapchain = swapchain;

	RegisterMeshes();
	RegisterTextures();
	RegisterMaterials();

	CreateShadowDescriptorSetLayout();
	CreateDescriptorSetLayout();

	CreateShadowPipeline();
	CreateGraphicsPipeline();

	CreateCommandPool();

	CreateShadowResources();
	CreateColorResources();
	CreateDepthResources();

	CreateTextureImages();
	CreateTextureImageViews();
	CreateTextureSamplers();

	LoadModels();
	CreateVertexBuffers();
	CreateIndexBuffers();

	CreateUniformBuffers();
	CreateShadowUniformBuffers();
	
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateShadowDescriptorSets();

	CreateCommandBuffers();
	CreateSyncObjects();
}

void Renderer::DestroySwapchainResources() 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	vkDestroyImageView(device, _color_image_view, nullptr);
	vkDestroyImage(device, _color_image, nullptr);
	vkFreeMemory(device, _color_image_memory, nullptr);

	vkDestroyImageView(device, _depth_image_view, nullptr);
	vkDestroyImage(device, _depth_image, nullptr);
	vkFreeMemory(device, _depth_image_memory, nullptr);
}

void Renderer::DestroyViewportResources()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	vkDestroyImageView(device, _viewport_color_image_view, nullptr);
	vkDestroyImage(device, _viewport_color_image, nullptr);
	vkFreeMemory(device, _viewport_color_image_memory, nullptr);

	vkDestroyImageView(device, _viewport_depth_image_view, nullptr);
	vkDestroyImage(device, _viewport_depth_image, nullptr);
	vkFreeMemory(device, _viewport_depth_image_memory, nullptr);

	vkDestroySampler(device, _viewport_sampler, nullptr);
	vkDestroyImageView(device, _viewport_image_view, nullptr);
	
	vkDestroyImage(device, _viewport_image, nullptr);
	vkFreeMemory(device, _viewport_image_memory, nullptr);
}

void Renderer::DestroyShadowResources()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	vkDestroySampler(device, _shadow_sampler, nullptr);
	vkDestroyImageView(device, _shadow_image_view, nullptr);
	vkDestroyImage(device, _shadow_image, nullptr);
	vkFreeMemory(device, _shadow_image_memory, nullptr);
	_shadow_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkDestroyPipeline(device, _shadow_graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, _shadow_pipeline_layout, nullptr);
	vkDestroyDescriptorSetLayout(device, _shadow_descriptor_set_layout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(device, _shadow_uniform_buffers[i], nullptr);
		vkFreeMemory(device, _shadow_uniform_buffers_memory[i], nullptr);
	}
}

void Renderer::Shutdown()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	vkDestroyDescriptorPool(device, _descriptor_pool, nullptr);

	DestroyShadowResources();

	for (auto& [texture_id, texture] : _textures) {
		vkDestroySampler(device, texture.texture_sampler, nullptr);
		vkDestroyImageView(device, texture.texture_image_view, nullptr);

		vkDestroyImage(device, texture.texture_image, nullptr);
		vkFreeMemory(device, texture.texture_image_memory, nullptr);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroyBuffer(device, _uniform_buffers[i], nullptr);
		vkFreeMemory(device, _uniform_buffers_memory[i], nullptr);
	}

	vkDestroyDescriptorSetLayout(device, _descriptor_set_layout, nullptr);

	for (auto& [mesh_id, mesh] : _meshes) {
		vkDestroyBuffer(device, mesh.index_buffer, nullptr);
		vkFreeMemory(device, mesh.index_buffer_memory, nullptr);

		vkDestroyBuffer(device, mesh.vertex_buffer, nullptr);
		vkFreeMemory(device, mesh.vertex_buffer_memory, nullptr);
	}

	vkDestroyPipeline(device, _graphics_pipeline, nullptr);
	vkDestroyPipelineLayout(device, _pipeline_layout, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, _image_available_semaphores[i], nullptr);
		vkDestroyFence(device, _in_flight_fences[i], nullptr);
	}

	for (size_t i = 0; i < _render_finished_semaphores.size(); i++) {
		vkDestroySemaphore(device, _render_finished_semaphores[i], nullptr);
	}

	vkDestroyCommandPool(device, _command_pool, nullptr);
}

void Renderer::RegisterMeshes()
{
	_meshes[0] = { .model_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/models/viking_room.obj" };
	_meshes[1] = { .model_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/models/iron_golem.obj" };
	_meshes[2] = { .model_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/models/grass_block.obj" };
}

void Renderer::RegisterTextures()
{
	_textures[0] = { .texture_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/textures/viking_room.png" };
	_textures[1] = { .texture_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/textures/iron_golem.png" };
	_textures[2] = { .texture_path = "C:/Users/moise/Documents/VS_projects/OrcaEngine/textures/grass_block.png" };
}

void Renderer::RegisterMaterials()
{
	_materials[0] = { .texture_id = 0 };
	_materials[1] = { .texture_id = 1};
	_materials[2] = { .texture_id = 2};
}

QueueFamilyIndices Renderer::FindQueueFamilies(VkPhysicalDevice device) 
{
	QueueFamilyIndices indices;

	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

	int i = 0;
	for (const auto& queue_family : queue_families) {
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _vulkan_context->GetSurface(), &present_support);

		if (present_support) {
			indices.present_family = i;
		}

		if (indices.IsComplete()) {
			break;
		}

		i++;
	}

	return indices;
}

void Renderer::RecreateSwapchainResources()
{
	CreateColorResources();
	CreateDepthResources();
}

void Renderer::DrawFrame(bool framebuffer_resized, 
						 ImDrawData* imgui_draw_data, 
						 RenderBundle& render_bundle, 
						 VkExtent2D& viewport_extent,
						 Camera& camera) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();
	const VkSwapchainKHR swapchain = _swapchain->GetSwapchain();

	vkWaitForFences(device, 1, &_in_flight_fences[_current_frame], VK_TRUE, UINT64_MAX);

	uint32_t image_index;
	VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, _image_available_semaphores[_current_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		RecreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	vkResetFences(device, 1, &_in_flight_fences[_current_frame]);

	vkResetCommandBuffer(_command_buffers[_current_frame], 0);
	RecordCommandBuffer(_command_buffers[_current_frame], image_index, imgui_draw_data, render_bundle, viewport_extent);

	if (viewport_extent.width > 0 &&
		viewport_extent.height > 0 &&
		_viewport_image != VK_NULL_HANDLE)
	{
		UpdateShadowUniformBuffer(_current_frame, render_bundle);
		UpdateUniformBuffer(_current_frame, camera, render_bundle);
	}
	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { _image_available_semaphores[_current_frame] };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &_command_buffers[_current_frame];

	VkSemaphore signal_semaphores[] = { _render_finished_semaphores[image_index] };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(_vulkan_context->GetGraphicsQueue(), 1, &submit_info, _in_flight_fences[_current_frame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swap_chains[] = { swapchain };
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swap_chains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;

	result = vkQueuePresentKHR(_vulkan_context->GetPresentQueue(), &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
		RecreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	_current_frame = (_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::CreateDescriptorSetLayout() 
{
	VkDescriptorSetLayoutBinding ubo_layout_binding{};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding sampler_layout_binding{};
	sampler_layout_binding.binding = 1;
	sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	sampler_layout_binding.descriptorCount = 1;
	sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sampler_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding shadow_sampler_layout_binding{};
	shadow_sampler_layout_binding.binding = 2;
	shadow_sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	shadow_sampler_layout_binding.descriptorCount = 1;
	shadow_sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	shadow_sampler_layout_binding.pImmutableSamplers = nullptr;

	std::array<VkDescriptorSetLayoutBinding, 3> bindings = { ubo_layout_binding, sampler_layout_binding, shadow_sampler_layout_binding };
	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
	layout_info.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(_vulkan_context->GetLogicalDevice(), &layout_info, nullptr, &_descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout");
	}
}

void Renderer::CreateShadowDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding ubo_layout_binding{};
	ubo_layout_binding.binding = 0;
	ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ubo_layout_binding.descriptorCount = 1;
	ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	ubo_layout_binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layout_info{};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &ubo_layout_binding;

	if (vkCreateDescriptorSetLayout(_vulkan_context->GetLogicalDevice(), &layout_info, nullptr, &_shadow_descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shadow descriptor set layout");
	}
}

void Renderer::CreateShadowPipeline()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	auto shadow_vert_shader_code = Renderer::ReadFile("C:/Users/moise/Documents/VS_projects/OrcaEngine/shaders/shadow_vert.spv");
	
	VkShaderModule shadow_vert_shader_module = CreateShaderModule(shadow_vert_shader_code);

	VkPipelineShaderStageCreateInfo shadow_vert_shader_stage_info{};
	shadow_vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shadow_vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	shadow_vert_shader_stage_info.module = shadow_vert_shader_module;
	shadow_vert_shader_stage_info.pName = "main";

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto binding_description = Vertex::GetBindingDescription();
	auto attribute_descriptions = Vertex::GetAttributeDescriptions();

	VkVertexInputAttributeDescription position_attribute{};
	position_attribute.binding = 0;
	position_attribute.location = 0;
	position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
	position_attribute.offset = offsetof(Vertex, position);

	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = 1;
	vertex_input_info.pVertexAttributeDescriptions = &position_attribute;

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates = dynamic_states.data();

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = 0.0f;
	depth_stencil.maxDepthBounds = 1.0f;
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {};
	depth_stencil.back = {};

	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &_shadow_descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &_shadow_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineRenderingCreateInfo render_info{};
	render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	render_info.colorAttachmentCount = 0;
	render_info.pColorAttachmentFormats = nullptr;
	render_info.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = &render_info;
	pipeline_info.stageCount = 1;
	pipeline_info.pStages = &shadow_vert_shader_stage_info;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = nullptr;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = _shadow_pipeline_layout;
	pipeline_info.renderPass = VK_NULL_HANDLE;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_shadow_graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, shadow_vert_shader_module, nullptr);
}	

void Renderer::CreateGraphicsPipeline() 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	auto vert_shader_code = Renderer::ReadFile("C:/Users/moise/Documents/VS_projects/OrcaEngine/shaders/vert.spv");
	auto frag_shader_code = Renderer::ReadFile("C:/Users/moise/Documents/VS_projects/OrcaEngine/shaders/frag.spv");

	VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code);
	VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code);

	VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	auto binding_description = Vertex::GetBindingDescription();
	auto attribute_descriptions = Vertex::GetAttributeDescriptions();

	vertex_input_info.vertexBindingDescriptionCount = 1;
	vertex_input_info.pVertexBindingDescriptions = &binding_description;
	vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_swapchain->GetExtent().width;
	viewport.height = (float)_swapchain->GetExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapchain->GetExtent();

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = _vulkan_context->GetMsaaSamples();
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo depth_stencil{};
	depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil.depthTestEnable = VK_TRUE;
	depth_stencil.depthWriteEnable = VK_TRUE;
	depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depth_stencil.depthBoundsTestEnable = VK_FALSE;
	depth_stencil.minDepthBounds = 0.0f;
	depth_stencil.maxDepthBounds = 1.0f;
	depth_stencil.stencilTestEnable = VK_FALSE;
	depth_stencil.front = {};
	depth_stencil.back = {};

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamic_states = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates = dynamic_states.data();

	VkPushConstantRange push_constant_range{};
	push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	push_constant_range.offset = 0;
	push_constant_range.size = sizeof(PushConstantData);

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 1;
	pipeline_layout_info.pSetLayouts = &_descriptor_set_layout;
	pipeline_layout_info.pushConstantRangeCount = 1;
	pipeline_layout_info.pPushConstantRanges = &push_constant_range;

	if (vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &_pipeline_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	const VkFormat swapchain_format = _swapchain->GetFormat();

	VkPipelineRenderingCreateInfo render_info{};
	render_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	render_info.colorAttachmentCount = 1;
	render_info.pColorAttachmentFormats = &swapchain_format;
	render_info.depthAttachmentFormat = FindDepthFormat();

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.pNext = &render_info;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pDepthStencilState = &depth_stencil;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = _pipeline_layout;
	//pipelineInfo.renderPass = renderPass;
	pipeline_info.renderPass = VK_NULL_HANDLE;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &_graphics_pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline");
	}

	vkDestroyShaderModule(device, vert_shader_module, nullptr);
	vkDestroyShaderModule(device, frag_shader_module, nullptr);
}

void Renderer::CreateCommandPool() 
{
	QueueFamilyIndices queue_family_indices = FindQueueFamilies(_vulkan_context->GetPhysicalDevice());

	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

	if (vkCreateCommandPool(_vulkan_context->GetLogicalDevice(), &pool_info, nullptr, &_command_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void Renderer::CreateShadowResources()
{
	VkDevice device = _vulkan_context->GetLogicalDevice();
	VkFormat shadow_format = VK_FORMAT_D32_SFLOAT;

	CreateImage(_shadow_map_width, 
				_shadow_map_height, 
				1, 
				VK_SAMPLE_COUNT_1_BIT, 
				shadow_format,
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
				VK_IMAGE_USAGE_SAMPLED_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				_shadow_image,
				_shadow_image_memory);

	_shadow_image_view = VulkanUtils::CreateImageView(device, 
													  _shadow_image, 
													  shadow_format, 
													  VK_IMAGE_ASPECT_DEPTH_BIT, 
													  1);

	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampler_info.anisotropyEnable = VK_FALSE;
	sampler_info.maxAnisotropy = 1.0f;
	sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;
	
	if (vkCreateSampler(device, &sampler_info, nullptr, &_shadow_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shadow sampler!");
	}

	_shadow_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Renderer::CreateColorResources() 
{
	const VkFormat color_format = _swapchain->GetFormat();
	const VkExtent2D swapchain_extent = _swapchain->GetExtent();

	CreateImage(swapchain_extent.width, swapchain_extent.height, 1, _vulkan_context->GetMsaaSamples(), color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _color_image, _color_image_memory);
	_color_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), _color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	TransitionImageLayout(_color_image, color_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);
}

void Renderer::CreateDepthResources() 
{
	const VkFormat depth_format = FindDepthFormat();
	const VkExtent2D swapchain_extent = _swapchain->GetExtent();

	CreateImage(swapchain_extent.width, swapchain_extent.height, 1, _vulkan_context->GetMsaaSamples(), depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _depth_image, _depth_image_memory);
	_depth_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), _depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	TransitionImageLayout(_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 1);
}

VkFormat Renderer::FindDepthFormat() 
{
	return FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkFormat Renderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) 
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_vulkan_context->GetPhysicalDevice(), format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

bool Renderer::HasStencilComponent(VkFormat format) 
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::CreateTextureImages()
{
	for (auto& [texture_id, texture] : _textures) {
		CreateTextureImage(texture);
	}
}

void Renderer::CreateTextureImage(TextureResource& texture) {
	int tex_width, tex_height, tex_channels;
	stbi_uc* pixels = stbi_load(texture.texture_path.c_str(), 
								&tex_width, 
								&tex_height,
								&tex_channels, 
								STBI_rgb_alpha);
								
	VkDeviceSize image_size = tex_width * tex_height * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	texture.mip_levels = static_cast<uint32_t>
				  (std::floor(std::log2(std::max(tex_width, tex_height)))) 
				  + 1;

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	CreateBuffer(image_size, 
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
				staging_buffer, 
				staging_buffer_memory);

	const VkDevice device = _vulkan_context->GetLogicalDevice();

	void* data;
	vkMapMemory(device, staging_buffer_memory, 0, image_size, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(image_size));
	vkUnmapMemory(device, staging_buffer_memory);

	stbi_image_free(pixels);

	CreateImage(tex_width, 
				tex_height, 
				texture.mip_levels, 
				VK_SAMPLE_COUNT_1_BIT, 
				VK_FORMAT_R8G8B8A8_SRGB, 
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
				VK_IMAGE_USAGE_SAMPLED_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				texture.texture_image, 
				texture.texture_image_memory);

	TransitionImageLayout(texture.texture_image, 
						  VK_FORMAT_R8G8B8A8_SRGB, 
						  VK_IMAGE_LAYOUT_UNDEFINED, 
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
						  texture.mip_levels);

	CopyBufferToImage(staging_buffer, 
					  texture.texture_image, 
					  static_cast<uint32_t>(tex_width), 
					  static_cast<uint32_t>(tex_height));
	//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps

	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);

	GenerateMipmaps(texture.texture_image, 
					VK_FORMAT_R8G8B8A8_SRGB, 
					tex_width, 
					tex_height, 
					texture.mip_levels);
}

void Renderer::CreateImage(uint32_t width, 
						   uint32_t height, 
						   uint32_t mip_levels, 
						   VkSampleCountFlagBits num_samples, 
						   VkFormat format, 
						   VkImageTiling tiling,
						   VkImageUsageFlags usage,
						   VkMemoryPropertyFlags properties, 
						   VkImage& image,
						   VkDeviceMemory& image_memory) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	VkImageCreateInfo image_info{};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.extent.width = width;
	image_info.extent.height = height;
	image_info.extent.depth = 1;
	image_info.mipLevels = mip_levels;
	image_info.arrayLayers = 1;
	image_info.format = format;
	image_info.tiling = tiling;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.samples = num_samples;
	image_info.flags = 0;

	if (vkCreateImage(device, &image_info, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetImageMemoryRequirements(device, image, &mem_requirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, image_memory, 0);
}

void Renderer::TransitionImageLayout(VkImage image, 
									 VkFormat format, 
									 VkImageLayout old_layout, 
									 VkImageLayout new_layout, 
									 uint32_t mip_levels) 
{
	VkCommandBuffer command_buffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = old_layout;
	barrier.newLayout = new_layout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags source_stage;
	VkPipelineStageFlags destination_stage;

	if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destination_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(command_buffer);
}

void Renderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) 
{
	VkCommandBuffer command_buffer = BeginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	EndSingleTimeCommands(command_buffer);
}

void Renderer::CreateTextureImageViews() 
{
	for (auto& [texture_id, texture] : _textures) {
		CreateTextureImageView(texture);
	}
}

void Renderer::CreateTextureImageView(TextureResource& texture) 
{
	texture.texture_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), 
													   texture.texture_image, 
													   VK_FORMAT_R8G8B8A8_SRGB, 
													   VK_IMAGE_ASPECT_COLOR_BIT, 
													   texture.mip_levels);
}

void Renderer::CreateTextureSamplers()
{
	for (auto& [texture_id, texture] : _textures) {
		CreateTextureSampler(texture);
	}
}

void Renderer::CreateTextureSampler(TextureResource& texture) 
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_vulkan_context->GetPhysicalDevice(), &properties);

	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler_info.anisotropyEnable = VK_TRUE;
	sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = VK_LOD_CLAMP_NONE;

	if (vkCreateSampler(_vulkan_context->GetLogicalDevice(), &sampler_info, nullptr, &texture.texture_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

void Renderer::LoadModels() 
{
	for (auto& [mesh_id, mesh] : _meshes) {
		LoadModel(mesh);
	}
}

void Renderer::LoadModel(MeshResource& mesh) 
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, mesh.model_path.c_str())) {
		throw std::runtime_error(err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.tex_coord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
				mesh.vertices.push_back(vertex);
			}

			mesh.indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void Renderer::CreateVertexBuffers()
{
	for (auto& [mesh_id, mesh] : _meshes) {
		CreateVertexBuffer(mesh);
	}
}

void Renderer::CreateVertexBuffer(MeshResource& mesh) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	VkDeviceSize buffer_size = sizeof(mesh.vertices[0]) * mesh.vertices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void* data;
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, mesh.vertices.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer_memory);

	CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.vertex_buffer, mesh.vertex_buffer_memory);

	CopyBuffer(staging_buffer, mesh.vertex_buffer, buffer_size);

	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void Renderer::CreateIndexBuffers()
{
	for (auto& [mesh_id, mesh] : _meshes) {
		CreateIndexBuffer(mesh);
	}
}


void Renderer::CreateIndexBuffer(MeshResource& mesh) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	VkDeviceSize buffer_size = sizeof(mesh.indices[0]) * mesh.indices.size();

	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void* data;
	vkMapMemory(device, staging_buffer_memory, 0, buffer_size, 0, &data);
	memcpy(data, mesh.indices.data(), (size_t)buffer_size);
	vkUnmapMemory(device, staging_buffer_memory);

	CreateBuffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mesh.index_buffer, mesh.index_buffer_memory);

	CopyBuffer(staging_buffer, mesh.index_buffer, buffer_size);

	vkDestroyBuffer(device, staging_buffer, nullptr);
	vkFreeMemory(device, staging_buffer_memory, nullptr);
}

void Renderer::CreateShadowUniformBuffers()
{
	VkDeviceSize buffer_size = sizeof(ShadowUniformBufferObject);

	_shadow_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
	_shadow_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
	_shadow_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		CreateBuffer(buffer_size, 
					 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
					 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
					 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
					 _shadow_uniform_buffers[i], 
					 _shadow_uniform_buffers_memory[i]);

		vkMapMemory(_vulkan_context->GetLogicalDevice(), _shadow_uniform_buffers_memory[i], 0, buffer_size, 0, &_shadow_uniform_buffers_mapped[i]);
	}
}

void Renderer::CreateUniformBuffers() 
{
	VkDeviceSize buffer_size = sizeof(UniformBufferObject);

	_uniform_buffers.resize(MAX_FRAMES_IN_FLIGHT);
	_uniform_buffers_memory.resize(MAX_FRAMES_IN_FLIGHT);
	_uniform_buffers_mapped.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		CreateBuffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, _uniform_buffers[i], _uniform_buffers_memory[i]);

		vkMapMemory(_vulkan_context->GetLogicalDevice(), _uniform_buffers_memory[i], 0, buffer_size, 0, &_uniform_buffers_mapped[i]);
	}
}

void Renderer::CreateBuffer(VkDeviceSize size, 
							VkBufferUsageFlags usage, 
							VkMemoryPropertyFlags properties, 
							VkBuffer& buffer, 
							VkDeviceMemory& buffer_memory) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	VkBufferCreateInfo buffer_info{};
	buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_info.size = size;
	buffer_info.usage = usage;
	buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements mem_requirements;
	vkGetBufferMemoryRequirements(device, buffer, &mem_requirements);

	VkMemoryAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = mem_requirements.size;
	alloc_info.memoryTypeIndex = FindMemoryType(mem_requirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &alloc_info, nullptr, &buffer_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

VkCommandBuffer Renderer::BeginSingleTimeCommands() 
{
	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandPool = _command_pool;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(_vulkan_context->GetLogicalDevice(), &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	return command_buffer;
}

void Renderer::EndSingleTimeCommands(VkCommandBuffer command_buffer) 
{
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	const VkQueue graphics_queue = _vulkan_context->GetGraphicsQueue();

	vkQueueSubmit(graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	vkFreeCommandBuffers(_vulkan_context->GetLogicalDevice(), _command_pool, 1, &command_buffer);
}

void Renderer::CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size) 
{
	VkCommandBuffer command_buffer = BeginSingleTimeCommands();

	VkBufferCopy copy_region{};
	copy_region.srcOffset = 0;
	copy_region.dstOffset = 0;
	copy_region.size = size;
	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

	EndSingleTimeCommands(command_buffer);
}

uint32_t Renderer::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) 
{
	VkPhysicalDeviceMemoryProperties mem_properties;
	vkGetPhysicalDeviceMemoryProperties(_vulkan_context->GetPhysicalDevice(), &mem_properties);

	for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
		if (type_filter & (1 << i) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void Renderer::CreateDescriptorPool() 
{
	uint32_t material_descriptor_count = MAX_FRAMES_IN_FLIGHT * static_cast<uint32_t>(_materials.size());
	uint32_t shadow_descriptor_count = MAX_FRAMES_IN_FLIGHT;
	uint32_t total_descriptor_count = material_descriptor_count + shadow_descriptor_count;

	std::array<VkDescriptorPoolSize, 2> pool_sizes{};
	pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_sizes[0].descriptorCount = static_cast<uint32_t>(total_descriptor_count);
	pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	pool_sizes[1].descriptorCount = static_cast<uint32_t>(material_descriptor_count * 2);

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
	pool_info.pPoolSizes = pool_sizes.data();
	pool_info.maxSets = static_cast<uint32_t>(total_descriptor_count);

	if (vkCreateDescriptorPool(_vulkan_context->GetLogicalDevice(), &pool_info, nullptr, &_descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

void Renderer::CreateShadowDescriptorSets()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	std::vector<VkDescriptorSetLayout> shadow_layouts(MAX_FRAMES_IN_FLIGHT, _shadow_descriptor_set_layout);
	VkDescriptorSetAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = _descriptor_pool;
	alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	alloc_info.pSetLayouts = shadow_layouts.data();

	_shadow_descriptor_sets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &alloc_info, _shadow_descriptor_sets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate shadow descriptor sets");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = _shadow_uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(ShadowUniformBufferObject);

		VkWriteDescriptorSet descriptor_write{};
		descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_write.dstSet = _shadow_descriptor_sets[i];
		descriptor_write.dstBinding = 0;
		descriptor_write.dstArrayElement = 0;
		descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_write.descriptorCount = 1;
		descriptor_write.pBufferInfo = &buffer_info;

		vkUpdateDescriptorSets(device, 1, &descriptor_write, 0, nullptr);
	}
}

void Renderer::CreateDescriptorSets()
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	_descriptor_sets.resize(_materials.size());

	for (auto& [material_id, material] : _materials) {
		
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _descriptor_set_layout);
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = _descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		alloc_info.pSetLayouts = layouts.data();

		_descriptor_sets[material_id].resize(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(device, &alloc_info, _descriptor_sets[material_id].data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		CreateDescriptorSet(material_id);
	}
}

void Renderer::CreateDescriptorSet(MaterialId material_id) 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();

	const MaterialResource& material = _materials.at(material_id);
	const TextureResource& texture = _textures.at(material.texture_id);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VkDescriptorBufferInfo buffer_info{};
		buffer_info.buffer = _uniform_buffers[i];
		buffer_info.offset = 0;
		buffer_info.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo image_info{};
		image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		image_info.imageView = texture.texture_image_view;
		image_info.sampler = texture.texture_sampler;

		VkDescriptorImageInfo shadow_image_info{};
		shadow_image_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		shadow_image_info.imageView = _shadow_image_view;
		shadow_image_info.sampler = _shadow_sampler;

		std::array<VkWriteDescriptorSet, 3> descriptor_writes{};

		descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[0].dstSet = _descriptor_sets[material_id][i];
		descriptor_writes[0].dstBinding = 0;
		descriptor_writes[0].dstArrayElement = 0;
		descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes[0].descriptorCount = 1;
		descriptor_writes[0].pBufferInfo = &buffer_info;

		descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[1].dstSet = _descriptor_sets[material_id][i];
		descriptor_writes[1].dstBinding = 1;
		descriptor_writes[1].dstArrayElement = 0;
		descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[1].descriptorCount = 1;
		descriptor_writes[1].pImageInfo = &image_info;

		descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[2].dstSet = _descriptor_sets[material_id][i];
		descriptor_writes[2].dstBinding = 2;
		descriptor_writes[2].dstArrayElement = 0;
		descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes[2].descriptorCount = 1;
		descriptor_writes[2].pImageInfo = &shadow_image_info;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
	}
}

void Renderer::CreateCommandBuffers() 
{
	_command_buffers.resize(MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo alloc_info{};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = _command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)_command_buffers.size();

	if (vkAllocateCommandBuffers(_vulkan_context->GetLogicalDevice(), &alloc_info, _command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}
}

void Renderer::RecordShadowPass(VkCommandBuffer command_buffer, RenderBundle& render_bundle)
{
	VkImageMemoryBarrier2 shadow_to_depth_barrier{};
	shadow_to_depth_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

	if (_shadow_image_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
		shadow_to_depth_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		shadow_to_depth_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	}
	else if (_shadow_image_layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL) {
		shadow_to_depth_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		shadow_to_depth_barrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	}

	shadow_to_depth_barrier.dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
	shadow_to_depth_barrier.dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	shadow_to_depth_barrier.oldLayout = _shadow_image_layout;
	shadow_to_depth_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	shadow_to_depth_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	shadow_to_depth_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	shadow_to_depth_barrier.image = _shadow_image;
	shadow_to_depth_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	shadow_to_depth_barrier.subresourceRange.baseMipLevel = 0;
	shadow_to_depth_barrier.subresourceRange.levelCount = 1;
	shadow_to_depth_barrier.subresourceRange.baseArrayLayer = 0;
	shadow_to_depth_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo shadow_to_depth_dependency{};
	shadow_to_depth_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	shadow_to_depth_dependency.imageMemoryBarrierCount = 1;
	shadow_to_depth_dependency.pImageMemoryBarriers = &shadow_to_depth_barrier;

	vkCmdPipelineBarrier2(command_buffer, &shadow_to_depth_dependency);

	_shadow_image_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

	VkRenderingAttachmentInfo depth_attachment_info{};
	depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment_info.imageView = _shadow_image_view;
	depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment_info.clearValue.depthStencil = { 1.0f, 0 };

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea.offset = { 0, 0 };
	rendering_info.renderArea.extent = { _shadow_map_width, _shadow_map_height };
	rendering_info.layerCount = 1;
	rendering_info.colorAttachmentCount = 0;
	rendering_info.pColorAttachments = nullptr;
	rendering_info.pDepthAttachment = &depth_attachment_info;

	vkCmdBeginRendering(command_buffer, &rendering_info);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _shadow_graphics_pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(_shadow_map_width);
	viewport.height = static_cast<float>(_shadow_map_height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { _shadow_map_width, _shadow_map_height };
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	VkDescriptorSet shadow_descriptor_set = _shadow_descriptor_sets[_current_frame];

	vkCmdBindDescriptorSets(command_buffer, 
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							_shadow_pipeline_layout,
							0,
							1,
							&shadow_descriptor_set,
							0,
							nullptr);

	for (const RenderItem& render_item : render_bundle.render_items) {

		auto& mesh = _meshes.at(render_item.mesh_id);
		auto& material = _materials.at(render_item.material_id);

		VkBuffer vertex_buffers[] = { mesh.vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		PushConstantData push_constants{};
		push_constants.model_matrix = render_item.model_matrix;

		vkCmdPushConstants(command_buffer, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantData), &push_constants);

		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
	}

	//vkCmdEndRenderPass(commandBuffer);
	vkCmdEndRendering(command_buffer);

	VkImageMemoryBarrier2 shadow_to_shader_barrier{};
	shadow_to_shader_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	shadow_to_shader_barrier.srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
	shadow_to_shader_barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	shadow_to_shader_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	shadow_to_shader_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	shadow_to_shader_barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	shadow_to_shader_barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
	shadow_to_shader_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	shadow_to_shader_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	shadow_to_shader_barrier.image = _shadow_image;
	shadow_to_shader_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	shadow_to_shader_barrier.subresourceRange.baseMipLevel = 0;
	shadow_to_shader_barrier.subresourceRange.levelCount = 1;
	shadow_to_shader_barrier.subresourceRange.baseArrayLayer = 0;
	shadow_to_shader_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo shadow_to_shader_dependency{};
	shadow_to_shader_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	shadow_to_shader_dependency.imageMemoryBarrierCount = 1;
	shadow_to_shader_dependency.pImageMemoryBarriers = &shadow_to_shader_barrier;

	vkCmdPipelineBarrier2(command_buffer, &shadow_to_shader_dependency);

	_shadow_image_layout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
}

void Renderer::RecordScenePass(VkCommandBuffer command_buffer, RenderBundle& render_bundle, VkExtent2D& viewport_extent)
{
	VkImageMemoryBarrier2 viewport_to_color_barrier{};
	viewport_to_color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;

	if (_viewport_image_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
		viewport_to_color_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
		viewport_to_color_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	}
	else if (_viewport_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		viewport_to_color_barrier.srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
		viewport_to_color_barrier.srcAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	}

	viewport_to_color_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	viewport_to_color_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	viewport_to_color_barrier.oldLayout = _viewport_image_layout;
	viewport_to_color_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	viewport_to_color_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	viewport_to_color_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	viewport_to_color_barrier.image = _viewport_image;
	viewport_to_color_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewport_to_color_barrier.subresourceRange.baseMipLevel = 0;
	viewport_to_color_barrier.subresourceRange.levelCount = 1;
	viewport_to_color_barrier.subresourceRange.baseArrayLayer = 0;
	viewport_to_color_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo viewport_to_color_dependency{};
	viewport_to_color_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	viewport_to_color_dependency.imageMemoryBarrierCount = 1;
	viewport_to_color_dependency.pImageMemoryBarriers = &viewport_to_color_barrier;

	vkCmdPipelineBarrier2(command_buffer, &viewport_to_color_dependency);

	_viewport_image_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkRenderingAttachmentInfo color_attachment_info{};
	color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	color_attachment_info.imageView = _viewport_color_image_view;
	color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment_info.clearValue.color = { 0.01f, 0.01f, 0.01f, 1 };
	color_attachment_info.resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT;
	color_attachment_info.resolveImageView = _viewport_image_view;
	color_attachment_info.resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkRenderingAttachmentInfo depth_attachment_info{};
	depth_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depth_attachment_info.imageView = _viewport_depth_image_view;
	depth_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	depth_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment_info.clearValue.depthStencil = { 1.0f, 0 };

	VkRenderingInfo rendering_info{};
	rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	rendering_info.renderArea.offset = { 0, 0 };
	rendering_info.renderArea.extent = viewport_extent;
	rendering_info.layerCount = 1;
	rendering_info.colorAttachmentCount = 1;
	rendering_info.pColorAttachments = &color_attachment_info;
	rendering_info.pDepthAttachment = &depth_attachment_info;

	vkCmdBeginRendering(command_buffer, &rendering_info);

	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphics_pipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(viewport_extent.width);
	viewport.height = static_cast<float>(viewport_extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = viewport_extent;
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	for (const RenderItem& render_item : render_bundle.render_items) {

		auto& mesh = _meshes.at(render_item.mesh_id);
		auto& material = _materials.at(render_item.material_id);

		VkBuffer vertex_buffers[] = { mesh.vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		PushConstantData push_constants{};
		push_constants.model_matrix = render_item.model_matrix;

		vkCmdPushConstants(command_buffer, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstantData), &push_constants);

		vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

		vkCmdBindIndexBuffer(command_buffer, mesh.index_buffer, 0, VK_INDEX_TYPE_UINT32);

		VkDescriptorSet descriptor_set = _descriptor_sets[render_item.material_id][_current_frame];

		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

		vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
		_draw_call_counter++;
	}

	//vkCmdEndRenderPass(commandBuffer);
	vkCmdEndRendering(command_buffer);

	VkImageMemoryBarrier2 viewport_to_shader_barrier{};
	viewport_to_shader_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	viewport_to_shader_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	viewport_to_shader_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	viewport_to_shader_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
	viewport_to_shader_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
	viewport_to_shader_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	viewport_to_shader_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	viewport_to_shader_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	viewport_to_shader_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	viewport_to_shader_barrier.image = _viewport_image;
	viewport_to_shader_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewport_to_shader_barrier.subresourceRange.baseMipLevel = 0;
	viewport_to_shader_barrier.subresourceRange.levelCount = 1;
	viewport_to_shader_barrier.subresourceRange.baseArrayLayer = 0;
	viewport_to_shader_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo viewport_to_shader_dependency{};
	viewport_to_shader_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	viewport_to_shader_dependency.imageMemoryBarrierCount = 1;
	viewport_to_shader_dependency.pImageMemoryBarriers = &viewport_to_shader_barrier;

	vkCmdPipelineBarrier2(command_buffer, &viewport_to_shader_dependency);

	_viewport_image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

void Renderer::RecordEditorPass(VkCommandBuffer command_buffer, uint32_t image_index, ImDrawData* imgui_draw_data)
{
	VkImageMemoryBarrier2 swapchain_to_color_barrier{};
	swapchain_to_color_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	swapchain_to_color_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
	swapchain_to_color_barrier.srcAccessMask = VK_ACCESS_2_NONE;
	swapchain_to_color_barrier.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	swapchain_to_color_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	swapchain_to_color_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchain_to_color_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	swapchain_to_color_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchain_to_color_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	swapchain_to_color_barrier.image = _swapchain->GetImages()[image_index];
	swapchain_to_color_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	swapchain_to_color_barrier.subresourceRange.baseMipLevel = 0;
	swapchain_to_color_barrier.subresourceRange.levelCount = 1;
	swapchain_to_color_barrier.subresourceRange.baseArrayLayer = 0;
	swapchain_to_color_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo swapchain_to_color_dependency{};
	swapchain_to_color_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	swapchain_to_color_dependency.imageMemoryBarrierCount = 1;
	swapchain_to_color_dependency.pImageMemoryBarriers = &swapchain_to_color_barrier;

	vkCmdPipelineBarrier2(command_buffer, &swapchain_to_color_dependency);

	VkRenderingAttachmentInfo imgui_color_attachment_info{};
	imgui_color_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	imgui_color_attachment_info.imageView = _swapchain->GetImageViews()[image_index];
	imgui_color_attachment_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imgui_color_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	imgui_color_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	VkRenderingInfo imgui_rendering_info{};
	imgui_rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	imgui_rendering_info.renderArea.offset = { 0, 0 };
	imgui_rendering_info.renderArea.extent = _swapchain->GetExtent();
	imgui_rendering_info.layerCount = 1;
	imgui_rendering_info.colorAttachmentCount = 1;
	imgui_rendering_info.pColorAttachments = &imgui_color_attachment_info;

	vkCmdBeginRendering(command_buffer, &imgui_rendering_info);

	ImGui_ImplVulkan_RenderDrawData(imgui_draw_data, command_buffer);

	vkCmdEndRendering(command_buffer);

	VkImageMemoryBarrier2 to_present_barrier{};
	to_present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	to_present_barrier.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
	to_present_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
	to_present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
	to_present_barrier.dstAccessMask = VK_ACCESS_2_NONE;
	to_present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	to_present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	to_present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	to_present_barrier.image = _swapchain->GetImages()[image_index];
	to_present_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	to_present_barrier.subresourceRange.baseMipLevel = 0;
	to_present_barrier.subresourceRange.levelCount = 1;
	to_present_barrier.subresourceRange.baseArrayLayer = 0;
	to_present_barrier.subresourceRange.layerCount = 1;

	VkDependencyInfo to_present_dependency{};
	to_present_dependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	to_present_dependency.imageMemoryBarrierCount = 1;
	to_present_dependency.pImageMemoryBarriers = &to_present_barrier;

	vkCmdPipelineBarrier2(command_buffer, &to_present_dependency);
}

void Renderer::RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, ImDrawData* imgui_draw_data, RenderBundle& render_bundle, VkExtent2D& viewport_extent) 
{
	_draw_call_counter = 0;

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = 0;
	begin_info.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	if (viewport_extent.width > 0 &&
		viewport_extent.height > 0 &&
		_viewport_image != VK_NULL_HANDLE) 
	{
		RecordShadowPass(command_buffer, render_bundle);
		RecordScenePass(command_buffer, render_bundle, viewport_extent);
	}

	RecordEditorPass(command_buffer, image_index, imgui_draw_data);

	if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to record command buffer!");
	}
}

void Renderer::CreateSyncObjects() 
{
	const VkDevice device = _vulkan_context->GetLogicalDevice();
	const auto& swapchain_images = _swapchain->GetImages();

	_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_render_finished_semaphores.resize(swapchain_images.size());
	_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphore_info, nullptr, &_image_available_semaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fence_info, nullptr, &_in_flight_fences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create semaphores!");
		}
	}

	for (size_t i = 0; i < swapchain_images.size(); i++) {
		if (vkCreateSemaphore(device, &semaphore_info, nullptr, &_render_finished_semaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render finished semaphore!");
		}
	}
}

void Renderer::RecreateSwapchain()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(_window, &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(_window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(_vulkan_context->GetLogicalDevice());

	DestroySwapchainResources();

	_swapchain->Recreate();

	RecreateSwapchainResources();
}

void Renderer::UpdateShadowUniformBuffer(uint32_t current_image, RenderBundle& render_bundle)
{
	ShadowUniformBufferObject shadow_ubo{};
	shadow_ubo.light_view_projection = render_bundle.directional_light.light_view_projection;

	memcpy(_shadow_uniform_buffers_mapped[current_image], &shadow_ubo, sizeof(shadow_ubo));
}

void Renderer::UpdateUniformBuffer(uint32_t current_image, Camera& camera, RenderBundle& render_bundle) 
{
	static auto start_time = std::chrono::high_resolution_clock::now();

	auto current_time = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

	float aspect_ratio = _viewport_extent.width / (float)_viewport_extent.height;

	UniformBufferObject ubo{};
	ubo.view = camera.GetViewMatrix();
	ubo.proj = camera.GetProjectionMatrix(aspect_ratio);

	ubo.light_direction = render_bundle.directional_light.direction;
	ubo.light_color = render_bundle.directional_light.color;
	ubo.light_intensity = render_bundle.directional_light.intensity;
	ubo.light_view_projection = render_bundle.directional_light.light_view_projection;

	ubo.camera_position = camera.GetPosition();
	
	memcpy(_uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

VkShaderModule Renderer::CreateShaderModule(const std::vector<char>& code) 
{
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = code.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(_vulkan_context->GetLogicalDevice(), &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shader_module;
}

void Renderer::GenerateMipmaps(VkImage image, 
							   VkFormat image_format, 
							   int32_t tex_width, 
							   int32_t tex_height, 
							   uint32_t mip_levels) 
{
	VkFormatProperties format_properties;
	vkGetPhysicalDeviceFormatProperties(_vulkan_context->GetPhysicalDevice(), image_format, &format_properties);

	if (!(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	VkCommandBuffer command_buffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mip_width = tex_width;
	int32_t mip_height = tex_height;

	for (uint32_t i = 1; i < mip_levels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mip_width, mip_height, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mip_width > 1 ? mip_width / 2 : 1, mip_height > 1 ? mip_height / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(command_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

		if (mip_width > 1) mip_width /= 2;
		if (mip_height > 1) mip_height /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mip_levels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleTimeCommands(command_buffer);
}

void Renderer::CreateViewportResources(VkExtent2D& viewport_extent)
{
	const VkFormat color_format = _swapchain->GetFormat();
	const VkFormat depth_format = FindDepthFormat();
	const VkExtent2D swapchain_extent = _swapchain->GetExtent();

	_viewport_extent = viewport_extent;
	_viewport_image_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	CreateImage(viewport_extent.width, viewport_extent.height, 1, _vulkan_context->GetMsaaSamples(), color_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _viewport_color_image, _viewport_color_image_memory);
	_viewport_color_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), _viewport_color_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	TransitionImageLayout(_viewport_color_image, color_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 1);

	CreateImage(viewport_extent.width, viewport_extent.height, 1, _vulkan_context->GetMsaaSamples(), depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _viewport_depth_image, _viewport_depth_image_memory);
	_viewport_depth_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), _viewport_depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	TransitionImageLayout(_viewport_depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, 1);

	CreateImage(viewport_extent.width, 
				viewport_extent.height, 
				1, 
				VK_SAMPLE_COUNT_1_BIT, 
				color_format, 
				VK_IMAGE_TILING_OPTIMAL, 
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
				VK_IMAGE_USAGE_SAMPLED_BIT, 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				_viewport_image, 
				_viewport_image_memory);

	_viewport_image_view = VulkanUtils::CreateImageView(_vulkan_context->GetLogicalDevice(), _viewport_image, color_format, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(_vulkan_context->GetPhysicalDevice(), &properties);

	VkSamplerCreateInfo sampler_info{};
	sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler_info.magFilter = VK_FILTER_LINEAR;
	sampler_info.minFilter = VK_FILTER_LINEAR;
	sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler_info.anisotropyEnable = VK_FALSE;
	sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	sampler_info.unnormalizedCoordinates = VK_FALSE;
	sampler_info.compareEnable = VK_FALSE;
	sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
	sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler_info.mipLodBias = 0.0f;
	sampler_info.minLod = 0.0f;
	sampler_info.maxLod = 0.0f;

	if (vkCreateSampler(_vulkan_context->GetLogicalDevice(), &sampler_info, nullptr, &_viewport_sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create viewport sampler!");
	}
}

void Renderer::RecreateViewportResources(VkExtent2D& viewport_extent)
{
	DestroyViewportResources();
	CreateViewportResources(viewport_extent);
}

std::vector<char> Renderer::ReadFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t file_size = (size_t)file.tellg();
	std::vector<char> buffer(file_size);

	file.seekg(0);
	file.read(buffer.data(), file_size);
	file.close();

	return buffer;
}
