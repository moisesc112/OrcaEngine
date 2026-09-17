#pragma once

#include <string>

class Registry;

namespace SceneSerializer {
    void Serialize(Registry& registry, const std::string& filepath);
    void Deserialize(Registry& registry, const std::string& filepath);
};