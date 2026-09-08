#include <OrcaEngine/VulkanUtils.hpp>

VkImageView VulkanUtils::CreateImageView(VkDevice device, 
										 VkImage image, 
										 VkFormat format, 
										 VkImageAspectFlags aspect_flags, 
										 uint32_t mip_levels) 
{
	VkImageViewCreateInfo view_info{};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange.aspectMask = aspect_flags;
	view_info.subresourceRange.baseMipLevel = 0;
	view_info.subresourceRange.levelCount = mip_levels;
	view_info.subresourceRange.baseArrayLayer = 0;
	view_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
	if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	return image_view;
}