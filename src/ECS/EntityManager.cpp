#include <OrcaEngine/ECS/EntityManager.hpp>

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

    Entity new_entity{new_id, _generations[new_id]};
    _alive_entities.push_back(new_entity);

    return new_entity;
}

void EntityManager::DestroyEntity(Entity entity)
{
    if (!IsEntityAlive(entity)) {
        return;
    }

    _generations[entity.id]++;
    _entity_ids.push_back(entity.id);

    auto it = std::find(_alive_entities.begin(), _alive_entities.end(), entity);
    if (it != _alive_entities.end()) {
        _alive_entities.erase(it);
    }
}

bool EntityManager::IsEntityAlive(Entity entity) const
{
    return entity.id < _generations.size() && 
           _generations[entity.id] == entity.generation;
}

std::vector<Entity>& EntityManager::GetAliveEntities()
{
    return _alive_entities;
}