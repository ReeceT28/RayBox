#pragma once

#include "Entity.h"
#include <vector>
#include <memory>
#include <string>

class EntityManager
{
    std::vector<std::unique_ptr<Entity>> m_entities;
    std::vector<std::unique_ptr<Entity>> m_entitiesToAdd;
    size_t m_totalEntities = 0;

public:
    Entity& addEntity(const std::string& tag);
    void update();
    const std::vector<std::unique_ptr<Entity>>& getEntities() const;
    std::vector<Entity*> getEntitiesWithTag(const std::string& tag) const;


};
