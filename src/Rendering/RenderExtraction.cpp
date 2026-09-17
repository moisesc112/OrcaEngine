#include <OrcaEngine/Rendering/RenderExtraction.hpp>
#include <OrcaEngine/ECS/Components/MaterialComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/LightComponent.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace RenderExtraction{

    RenderBundle ExtractRenderBundle(Registry& registry)
    {
        std::vector<RenderItem> render_items;
        DirectionalLight directional_light;

        for (Entity entity : registry.View<MeshComponent, TransformComponent, MaterialComponent>()) {
            auto& mesh = registry.GetComponent<MeshComponent>(entity);
            auto& transform = registry.GetComponent<TransformComponent>(entity);
            auto& material = registry.GetComponent<MaterialComponent>(entity);

            glm::mat4 model_matrix(1.0f);

            model_matrix = glm::translate(model_matrix, transform.position);

            model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model_matrix = glm::rotate(model_matrix, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            model_matrix = glm::scale(model_matrix, transform.scale);

            render_items.push_back({mesh.mesh_id, material.material_id, model_matrix});
        }

        for (Entity entity : registry.View<LightComponent, TransformComponent>()) {
            auto& light = registry.GetComponent<LightComponent>(entity);
            auto& transform = registry.GetComponent<TransformComponent>(entity);

            directional_light.direction = transform.GetForwardDirection();
            directional_light.color = light.color;
            directional_light.intensity = light.intensity;
            directional_light.light_view_projection = CalculateLightViewProjection(directional_light.direction);
        }

        RenderBundle render_bundle{render_items, directional_light};
        
        return render_bundle;
    }

    glm::mat4 CalculateLightViewProjection(glm::vec3& light_direction)
    {
        glm::vec3 light_position = -light_direction * 10.0f;
        glm::vec3 scene_focus(0.0f);
        glm::vec3 world_up(0.0f, 0.0f, 1.0f);

        float ortho_extents = 10.0f;
        float near_plane = 0.1f;
        float far_plane = 30.0f;

        glm::mat4 light_view = glm::lookAt(light_position, scene_focus, world_up);

        glm::mat4 light_projection = glm::ortho(-ortho_extents,
                                                ortho_extents,
                                                -ortho_extents,
                                                ortho_extents,
                                                near_plane,
                                                far_plane);

        light_projection[1][1] *= 1.0f;

        return light_projection * light_view;
    }

}
