#pragma once

#include <OrcaEngine/Rendering/VulkanTypes.hpp>

#include <vulkan/vulkan_raii.hpp>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vector>
#include <optional>

class VulkanContext;

class Swapchain {
public:
	Swapchain();
	~Swapchain();

	void Initialize(GLFWwindow* window, VulkanContext& vulkan_context);
	void Shutdown();
	void Recreate();

	VkSwapchainKHR GetSwapchain() { return _swapchain; }
	std::vector<VkImage> GetImages() { return _swapchain_images; }
	VkFormat GetFormat() { return _swapchain_image_format; }
	VkExtent2D GetExtent() { return _swapchain_extent; }
	std::vector<VkImageView> GetImageViews() { return _swapchain_image_views; }
private:

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice& device, VkSurfaceKHR& surface);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void CreateSwapChain();
	void CreateImageViews();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	GLFWwindow* _window = nullptr;
	VulkanContext* _vulkan_context = VK_NULL_HANDLE;

	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchain_images;
	VkFormat _swapchain_image_format;
	VkExtent2D _swapchain_extent;
	std::vector<VkImageView> _swapchain_image_views;


};
