#pragma once

#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>

#include <glm/glm.hpp>

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