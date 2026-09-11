#pragma once

#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>

#include <vulkan/vulkan_raii.hpp>

#include <glm/glm.hpp>
#include <cstdint>

using MeshId = std::uint32_t;

struct RenderItem {
	MeshComponent mesh;
	TransformComponent transform;
};

struct RenderBundle {
    std::vector<RenderItem> render_items;
};

struct PushConstantData {
    alignas(16) glm::mat4 model_matrix;
};

struct MeshResource {
    std::string model_path;
    std::string texture_path;

    VkImage texture_image = VK_NULL_HANDLE;
    VkDeviceMemory texture_image_memory = VK_NULL_HANDLE;
    VkImageView texture_image_view = VK_NULL_HANDLE;
    VkSampler texture_sampler = VK_NULL_HANDLE;
};