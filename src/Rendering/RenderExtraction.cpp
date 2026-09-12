#include <OrcaEngine/Rendering/RenderExtraction.hpp>
#include <OrcaEngine/ECS/Components/MaterialComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>

#include <glm/glm.hpp>

#include <vector>

namespace RenderExtraction{

    RenderBundle ExtractRenderBundle(Registry& registry)
    {
        std::vector<RenderItem> render_items;

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

        RenderBundle render_bundle{render_items};
        
        return render_bundle;
    }

}
