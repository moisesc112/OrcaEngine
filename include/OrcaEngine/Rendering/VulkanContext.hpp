#pragma once

#include <OrcaEngine/Rendering/VulkanTypes.hpp>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <optional>
#include <vector>

const std::vector<const char*> g_validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> g_device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool g_enable_validation_layers = false;
#else
const bool g_enable_validation_layers = true;
#endif

class VulkanContext {
public:
	VulkanContext();
	VulkanContext(GLFWwindow* window);
	~VulkanContext();

	void Initialize(GLFWwindow* window);
	void Shutdown();

	VkInstance GetInstance() { return _instance; }
	VkPhysicalDevice GetPhysicalDevice() { return _physical_device; }
	VkDevice GetLogicalDevice() { return _device; }
	VkSurfaceKHR GetSurface() { return _surface; }
	VkQueue GetGraphicsQueue() { return _graphics_queue; }
	VkQueue GetPresentQueue() { return _present_queue; }
	VkSampleCountFlagBits GetMsaaSamples() { return _msaa_samples; }
	uint32_t GetGraphicsQueueFamilyIndex() { return _graphics_queue_family_index; }
	uint32_t GetPresentQueueFamilyIndex() { return _present_queue_family_index; }

private:
	void CreateInstance();
	bool CheckValidationLayerSupport();
	std::vector<const char*> GetRequiredExtensions();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& create_info);
	void SetupDebugMessenger();
	void CreateSurface();
	void PickPhysicalDevice();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	void CreateLogicalDevice();
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSampleCountFlagBits GetMaxUsableSampleCount();

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
										  const VkDebugUtilsMessengerCreateInfoEXT* p_createinfo,
										  const VkAllocationCallbacks* p_allocator, 
										  VkDebugUtilsMessengerEXT* p_debug_messenger);

	void DestroyDebugUtilsMessengerEXT(VkInstance instance, 
									   VkDebugUtilsMessengerEXT debug_messenger, 
									   const VkAllocationCallbacks* p_allocator);

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
		void* p_user_data);


	GLFWwindow* _window = nullptr;

	VkInstance _instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
	VkSurfaceKHR _surface = VK_NULL_HANDLE;

	VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;

	VkQueue _graphics_queue = VK_NULL_HANDLE;
	VkQueue _present_queue = VK_NULL_HANDLE;

	VkSampleCountFlagBits _msaa_samples = VK_SAMPLE_COUNT_1_BIT;

	uint32_t _graphics_queue_family_index = 0;
	uint32_t _present_queue_family_index = 0;
};