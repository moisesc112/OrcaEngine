#pragma once

#include <OrcaEngine/ECS/EntityManager.hpp>
#include <OrcaEngine/ECS/ComponentPool.hpp>
#include <OrcaEngine/ECS/IComponentPool.hpp>

#include <unordered_map>
#include <stdexcept>
#include <typeinfo>
#include <memory>
#include <string>

class Registry {
public:
    Entity CreateEntity() {
        return _entity_manager.CreateEntity();
    }

    void DestroyEntity(Entity entity) {
        for (auto& [name, pool] : _component_pools) {
            pool->RemoveEntity(entity);
        }

        _entity_manager.DestroyEntity(entity);
    }

    template<typename T>
    void AddComponent(Entity entity, const T& component)
    {
        GetOrCreatePool<T>().AddComponent(entity, component);
    }

    template<typename T>
    bool HasComponent(Entity entity) const
    {
        auto* pool = GetPool<T>();
        
        if (pool == nullptr) {
            return false;
        }

        return pool->HasComponent(entity);
    }

    template<typename T>
    T& GetComponent(Entity entity)
    {
        auto* pool = GetPool<T>();

        if (!pool) {
            throw std::runtime_error("Component pool does not exist for this type.");
        }

        if (!pool->HasComponent(entity)) {
            throw std::runtime_error("Entity does not have this component.");
        }

        return pool->GetComponent(entity);
    }

    template<typename T>
    void RemoveComponent(Entity entity)
    {
        auto* pool = GetPool<T>();

        if (!pool) {
            return;
        }

        pool->RemoveComponent(entity);
    }

    template<typename... Components>
    std::vector<Entity> View() 
    {
        std::vector<Entity> entities;

        for (Entity entity : _entity_manager.GetAliveEntities()) {
            if ((HasComponent<Components>(entity) && ...)) {
                entities.push_back(entity);
            }
        }

        return entities;
    }

private:
    template<typename T>
    ComponentPool<T>& GetOrCreatePool() 
    {
        std::string name(typeid(T).name());

        auto it = _component_pools.find(name);

        if (it == _component_pools.end()) {
            auto pool = std::make_unique<ComponentPool<T>>();
            auto* pool_ptr = pool.get();

            _component_pools.emplace(name, std::move(pool));

            return *pool_ptr;
        }

        return *static_cast<ComponentPool<T>*>(it->second.get());
    }

    template<typename T>
    ComponentPool<T>* GetPool()
    {
        std::string name(typeid(T).name());

        auto it = _component_pools.find(name);

        if (it == _component_pools.end()) {
            return nullptr;
        }

        return static_cast<ComponentPool<T>*>(it->second.get());
    }

    template<typename T>
    const ComponentPool<T>* GetPool() const
    {
        std::string name(typeid(T).name());

        auto it = _component_pools.find(name);

        if (it == _component_pools.end()) {
            return nullptr;
        }

        return static_cast<const ComponentPool<T>*>(it->second.get());
    }


private:
    EntityManager _entity_manager;

    std::unordered_map<std::string, std::unique_ptr<IComponentPool>> _component_pools;
};