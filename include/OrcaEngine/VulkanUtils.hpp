#pragma once

#include <vulkan/vulkan_raii.hpp>

VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);