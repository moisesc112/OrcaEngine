#include <OrcaEngine/Scene/SceneSerializer.hpp>
#include <OrcaEngine/ECS/Registry.hpp>
#include <OrcaEngine/ECS/Components/NameComponent.hpp>
#include <OrcaEngine/ECS/Components/TransformComponent.hpp>
#include <OrcaEngine/ECS/Components/MeshComponent.hpp>
#include <OrcaEngine/ECS/Components/MaterialComponent.hpp>
#include <OrcaEngine/ECS/Components/LightComponent.hpp>

#include <json.hpp>

#include <fstream>
#include <stdexcept>

void SceneSerializer::Serialize(Registry& registry, const std::string& filepath)
{
    nlohmann::json scene;
    scene["entities"] = nlohmann::json::array();

    for (Entity entity : registry.GetAliveEntities()) {
        nlohmann::json entity_json;

        if (registry.HasComponent<NameComponent>(entity)) {
            const auto& name = registry.GetComponent<NameComponent>(entity);
            entity_json["name"] = name.name;
        }

        if (registry.HasComponent<TransformComponent>(entity)) {
            const auto& transform = registry.GetComponent<TransformComponent>(entity);
            entity_json["transform"] = {{"position", {transform.position.x, transform.position.y, transform.position.z}},
                                        {"rotation", {transform.rotation.x, transform.rotation.y, transform.rotation.z}},
                                        {"scale", {transform.scale.x, transform.scale.y, transform.scale.z}}};
        }

        if (registry.HasComponent<MeshComponent>(entity)) {
            const auto& mesh = registry.GetComponent<MeshComponent>(entity);
            entity_json["mesh"] = mesh.mesh_id;
        }

        if (registry.HasComponent<MaterialComponent>(entity)) {
            const auto& material = registry.GetComponent<MaterialComponent>(entity);
            entity_json["material"] = material.material_id;
        }

        if (registry.HasComponent<LightComponent>(entity)) {
            const auto& light = registry.GetComponent<LightComponent>(entity);
            entity_json["light"] = {{"color", {light.color.x, light.color.y, light.color.z}}, 
                                    {"intensity", light.intensity}};
        }

        scene["entities"].push_back(entity_json);
    }

    std::ofstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("failed to save scene file!");
    }

    file << scene.dump(4);
}

void SceneSerializer::Deserialize(Registry& registry, const std::string& filepath)
{
    std::ifstream file(filepath);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open scene file!");
    }

    nlohmann::json scene;
    file >> scene;

    registry.Clear();

    for (const auto& entity_json : scene["entities"]) {
        Entity entity = registry.CreateEntity();

        if (entity_json.contains("name")) {
            const auto& name_json = entity_json["name"];

            NameComponent name{};
            name.name = name_json.get<std::string>();

            registry.AddComponent<NameComponent>(entity, name);
        }

        if (entity_json.contains("transform")) {
            const auto& transform_json = entity_json["transform"];

            const auto& position = transform_json["position"];
            const auto& rotation = transform_json["rotation"];
            const auto& scale = transform_json["scale"];

            TransformComponent transform{};

            transform.position = {position[0].get<float>(), 
                                  position[1].get<float>(), 
                                  position[2].get<float>()};

            transform.rotation = {rotation[0].get<float>(), 
                                  rotation[1].get<float>(), 
                                  rotation[2].get<float>()};

            transform.scale = {scale[0].get<float>(), 
                               scale[1].get<float>(), 
                               scale[2].get<float>()};

            registry.AddComponent<TransformComponent>(entity, transform);
        }

        if (entity_json.contains("mesh")) {
            const auto& mesh_json = entity_json["mesh"];

            MeshComponent mesh{};
            mesh.mesh_id = mesh_json.get<std::uint32_t>();

            registry.AddComponent<MeshComponent>(entity, mesh);
        }

        if (entity_json.contains("material")) {
            const auto& material_json = entity_json["material"];

            MaterialComponent material{};
            material.material_id = material_json.get<std::uint32_t>();

            registry.AddComponent<MaterialComponent>(entity, material);
        }

        if (entity_json.contains("light")) {
            const auto& light_json = entity_json["light"];
            const auto& color = light_json["color"];

            LightComponent light{};
            light.color = {color[0].get<float>(), 
                           color[1].get<float>(), 
                           color[2].get<float>()};
            
            light.intensity = light_json["intensity"].get<float>();

            registry.AddComponent<LightComponent>(entity, light);
        }
    }
}