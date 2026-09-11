#pragma once

#include <OrcaEngine/Rendering/VulkanTypes.hpp>
#include <OrcaEngine/Rendering/RenderTypes.hpp>
#include <OrcaEngine/Rendering/Vertex.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>
#include <vector>
#include <chrono>
#include <unordered_map>

class VulkanContext;
class Swapchain;
struct ImDrawData;
struct TransformComponent;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "C:/Users/moise/Documents/VS_projects/OrcaEngine/models/iron_golem.obj";
const std::string TEXTURE_PATH = "C:/Users/moise/Documents/VS_projects/OrcaEngine/textures/iron_golem.png";

struct UniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

class Renderer {
public:
	Renderer();
	~Renderer();

	void Initialize(GLFWwindow* window, VulkanContext* vulkan_context, Swapchain* swapchain);
	void DestroySwapchainResources();
	void Shutdown();

	void RecreateSwapchainResources();
	void DrawFrame(bool framebuffer_resized, ImDrawData* imgui_draw_data, RenderBundle& render_bundle);

	//std::vector<Vertex> GetVertices() { return _vertices; }
	//std::vector<uint32_t> GetIndices() { return _indices; }
	uint32_t GetDrawCallCounter() { return _draw_call_counter; }

private:

	void RegisterMeshes();
	void RegisterTextures();
	void RegisterMaterials();

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateColorResources();
	void CreateDepthResources();
	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool HasStencilComponent(VkFormat format);

	void CreateTextureImages();
	void CreateTextureImage(TextureResource& texture);

	void CreateImage(uint32_t width, 
					 uint32_t height, 
				 	 uint32_t mip_levels, 
					 VkSampleCountFlagBits num_samples, 
					 VkFormat format, 
					 VkImageTiling tiling, 
					 VkImageUsageFlags usage, 
					 VkMemoryPropertyFlags properties, 
					 VkImage& image, 
					 VkDeviceMemory& image_memory);

	void TransitionImageLayout(VkImage image, 
							   VkFormat format, 
							   VkImageLayout old_layout, 
							   VkImageLayout new_layout, 
							   uint32_t mip_levels);

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void CreateTextureImageViews();
	void CreateTextureImageView(TextureResource& texture);

	void CreateTextureSamplers();
	void CreateTextureSampler(TextureResource& texture);

	void LoadModels();
	void LoadModel(MeshResource& mesh);

	void CreateVertexBuffers();
	void CreateVertexBuffer(MeshResource& mesh);

	void CreateIndexBuffers();
	void CreateIndexBuffer(MeshResource& mesh);

	void CreateUniformBuffers();
	void CreateBuffer(VkDeviceSize size, 
					  VkBufferUsageFlags usage, 
					  VkMemoryPropertyFlags properties, 
					  VkBuffer& buffer, 
					  VkDeviceMemory& buffer_memory);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer command_buffer);
	void CopyBuffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
	uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
	void CreateDescriptorPool();

	void CreateDescriptorSets();
	void CreateDescriptorSet(MaterialId material_id);

	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, ImDrawData* imgui_draw_data, RenderBundle& render_bundle);
	void CreateSyncObjects();
	void RecreateSwapchain();

	void UpdateUniformBuffer(uint32_t current_image);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void GenerateMipmaps(VkImage image, 
						 VkFormat image_format, 
						 int32_t tex_width, 
						 int32_t tex_height, 
						 uint32_t mip_levels);

	static std::vector<char> ReadFile(const std::string& filename);

	GLFWwindow* _window = nullptr;
	VulkanContext* _vulkan_context = nullptr;
	Swapchain* _swapchain = nullptr;

	std::unordered_map<MeshId, MeshResource> _meshes;
	std::unordered_map<TextureId, TextureResource> _textures;
	std::unordered_map<MaterialId, MaterialResource> _materials;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debug_messenger;
	VkDescriptorSetLayout _descriptor_set_layout;
	VkPipelineLayout _pipeline_layout;
	VkPipeline _graphics_pipeline;
	std::vector<VkFramebuffer> _swapchain_framebuffers;
	VkCommandPool _command_pool;
	std::vector<VkCommandBuffer> _command_buffers;
	std::vector<VkSemaphore> _image_available_semaphores;
	std::vector<VkSemaphore> _render_finished_semaphores;
	std::vector<VkFence> _in_flight_fences;
	uint32_t _current_frame = 0;
	
	std::vector<VkBuffer> _uniform_buffers;
	std::vector<VkDeviceMemory> _uniform_buffers_memory;
	std::vector<void*> _uniform_buffers_mapped;
	VkDescriptorPool _descriptor_pool;
	std::vector<std::vector<VkDescriptorSet>> _descriptor_sets;
	
	VkImage _depth_image;
	VkDeviceMemory _depth_image_memory;
	VkImageView _depth_image_view;
	
	VkSampleCountFlagBits _msaa_samples = VK_SAMPLE_COUNT_1_BIT;
	VkImage _color_image;
	VkDeviceMemory _color_image_memory;
	VkImageView _color_image_view;
	uint32_t _draw_call_counter = 0;
};