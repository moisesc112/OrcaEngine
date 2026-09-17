#include <OrcaEngine/Scene/SceneSerializer.hpp>
#include <OrcaEngine/ECS/Registry.hpp>

#include <json.hpp>

void SceneSerializer::Serialize(const Registry& registry, const std::string& filepath)
{
    nlohmann::json scene;
}

void SceneSerializer::Deserialize(Registry& registry, const std::string& filepath)
{
    
}