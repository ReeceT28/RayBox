#include "EntityManager.h"
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
Entity& EntityManager::addEntity(const std::string& tag)
{
    std::unique_ptr<Entity> e = std::make_unique<Entity>(tag, m_totalEntities++);
    m_entitiesToAdd.push_back(std::move(e));
    return *(m_entitiesToAdd.back());
}

void EntityManager::update()
{
    for (auto& e : m_entitiesToAdd)
    {
        m_entities.push_back(std::move(e));
    }
    m_entitiesToAdd.clear();
    auto toRemove = std::remove_if(m_entities.begin(), m_entities.end(), [](const std::unique_ptr<Entity>& e) {
        return e->m_dead;
        });
    m_entities.erase(toRemove, m_entities.end());
}
const std::vector<std::unique_ptr<Entity>>& EntityManager::getEntities() const
{
    return m_entities;
}
std::vector<Entity*> EntityManager::getEntitiesWithTag(const std::string& tag) const
{
    std::vector<Entity*> taggedEntities;

    for (const auto& entity : m_entities)
    {
        if (entity->getTag() == tag)
        {
            taggedEntities.push_back(entity.get());
        }
    }

    return taggedEntities;
}

