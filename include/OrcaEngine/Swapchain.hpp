#pragma once

#include <OrcaEngine/VulkanTypes.hpp>

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

	void init(GLFWwindow* window, VulkanContext& vulkanContext);
	void cleanupSwapChain();
	void recreate();

	VkSwapchainKHR getSwapchain() { return swapChain; }
	std::vector<VkImage> getImages() { return swapChainImages; }
	VkFormat getFormat() { return swapChainImageFormat; }
	VkExtent2D getExtent() { return swapChainExtent; }
	std::vector<VkImageView> getImageViews() { return swapChainImageViews; }
private:

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice& device, VkSurfaceKHR& surface);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	GLFWwindow* _window = nullptr;
	VulkanContext* _vulkanContext = VK_NULL_HANDLE;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;


};
