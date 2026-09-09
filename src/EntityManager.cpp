#include <OrcaEngine/EntityManager.hpp>

EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

Entity EntityManager::CreateEntity()
{
    EntityID new_id;

    if (!_entity_ids.empty()) {
        new_id = _entity_ids.back();
        _entity_ids.pop_back();
    }
    else {
        new_id = static_cast<EntityID>(_generations.size());
        _generations.push_back(0);
    }

    return Entity{new_id, _generations[new_id]};
}

void EntityManager::DestroyEntity(Entity entity)
{
    if (!IsEntityAlive(entity)) {
        return;
    }

    _generations[entity.id]++;
    _entity_ids.push_back(entity.id);
}

bool EntityManager::IsEntityAlive(Entity entity) const
{
    return entity.id < _generations.size() && 
           _generations[entity.id] == entity.generation;
}