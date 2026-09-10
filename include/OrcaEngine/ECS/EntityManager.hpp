#pragma once

#include <OrcaEngine/ECS/Entity.hpp>

#include <vector>

class EntityManager {
public:
    EntityManager();
    ~EntityManager();

    Entity CreateEntity();
    void DestroyEntity(Entity entity);

    bool IsEntityAlive(Entity entity) const;

private:
    std::vector<EntityID> _entity_ids;
    std::vector<uint32_t> _generations;
};