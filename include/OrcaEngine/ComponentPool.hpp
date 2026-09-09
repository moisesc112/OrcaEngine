#pragma once

#include <OrcaEngine/Entity.hpp>
#include <OrcaEngine/IComponentPool.hpp>

#include <vector>
#include <optional>

template<typename T>
class ComponentPool : public IComponentPool {
public:
    void AddComponent(Entity entity, const T& component)
    {
        if (entity.id >= _components.size()) {
            _components.resize(entity.id + 1);
        }

        _components[entity.id] = component;
    }

    bool HasComponent(Entity entity) const 
    {
        return entity.id < _components.size() &&
               _components[entity.id].has_value();
    }

    T& GetComponent(Entity entity)
    {
        return _components[entity.id].value();
    }

    void RemoveComponent(Entity entity)
    {
        if (entity.id < _components.size()) {
            _components[entity.id].reset();
        }
    }

    void RemoveEntity(Entity entity) override
    {
        RemoveComponent(entity);
    }


private:
    std::vector<std::optional<T>> _components;
};