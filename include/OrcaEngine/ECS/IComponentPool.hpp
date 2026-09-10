#pragma once

#include <string>
#include <unordered_map>
#include <memory>

struct Entity;

class IComponentPool {
public:
    virtual ~IComponentPool() = default;

    virtual void RemoveEntity(Entity entity) = 0;
private:
    std::unordered_map<std::string, std::unique_ptr<IComponentPool>> _component_pools;
};