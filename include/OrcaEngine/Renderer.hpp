#pragma once

#include <OrcaEngine/VulkanTypes.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <array>
#include <vector>
#include <chrono>
#include <unordered_map>

class VulkanContext;
class Swapchain;
struct ImDrawData;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::string MODEL_PATH = "C:/Users/moise/Documents/VS_projects/OrcaEngine/models/viking_room.obj";
const std::string TEXTURE_PATH = "C:/Users/moise/Documents/VS_projects/OrcaEngine/textures/viking_room.png";

struct Vertex {
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 tex_coord;

	static VkVertexInputBindingDescription GetBindingDescription() {
		VkVertexInputBindingDescription binding_description{};
		binding_description.binding = 0;
		binding_description.stride = sizeof(Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return binding_description;
	}

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions{};
		attribute_descriptions[0].binding = 0;
		attribute_descriptions[0].location = 0;
		attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[0].offset = offsetof(Vertex, position);
				 
		attribute_descriptions[1].binding = 0;
		attribute_descriptions[1].location = 1;
		attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribute_descriptions[1].offset = offsetof(Vertex, color);
				 
		attribute_descriptions[2].binding = 0;
		attribute_descriptions[2].location = 2;
		attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

		return attribute_descriptions;
	}

	bool operator==(const Vertex& other) const {
		return position == other.position && color == other.color && tex_coord == other.tex_coord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
}

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
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
	void DrawFrame(bool framebuffer_resized, ImDrawData* imgui_draw_data);

private:

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	void CreateDescriptorSetLayout();
	void CreateGraphicsPipeline();
	void CreateCommandPool();
	void CreateColorResources();
	void CreateDepthResources();
	VkFormat FindDepthFormat();
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool HasStencilComponent(VkFormat format);
	void CreateTextureImage();

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
	void CreateTextureImageView();
	void CreateTextureSampler();
	void LoadModel();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
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
	void CreateCommandBuffers();
	void RecordCommandBuffer(VkCommandBuffer command_buffer, uint32_t image_index, ImDrawData* imgui_draw_data);
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
	VkBuffer _vertex_buffer;
	VkDeviceMemory _vertex_buffer_memory;
	VkBuffer _index_buffer;
	VkDeviceMemory _index_buffer_memory;
	std::vector<VkBuffer> _uniform_buffers;
	std::vector<VkDeviceMemory> _uniform_buffers_memory;
	std::vector<void*> _uniform_buffers_mapped;
	VkDescriptorPool _descriptor_pool;
	std::vector<VkDescriptorSet> _descriptor_sets;
	VkImage _texture_image;
	VkDeviceMemory _texture_image_memory;
	VkImageView _texture_image_view;
	VkSampler _texture_sampler;
	VkImage _depth_image;
	VkDeviceMemory _depth_image_memory;
	VkImageView _depth_image_view;
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;
	uint32_t _mip_levels;
	VkSampleCountFlagBits _msaa_samples = VK_SAMPLE_COUNT_1_BIT;
	VkImage _color_image;
	VkDeviceMemory _color_image_memory;
	VkImageView _color_image_view;
};