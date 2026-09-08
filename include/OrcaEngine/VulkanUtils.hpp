#pragma once

#include <vulkan/vulkan_raii.hpp>

namespace VulkanUtils {
	VkImageView CreateImageView(VkDevice device, 
								VkImage image, 
								VkFormat format, 
								VkImageAspectFlags aspect_flags, 
								uint32_t mip_levels);
}