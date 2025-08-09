#include "RayManager.h"


const std::vector<std::unique_ptr<Ray>>& RayManager::getRays() const
{
    return m_rays;
}
void RayManager::createRay(const sf::Vector2f& origin, const sf::Vector2f& dir, const sf::Color& lineColor, const float waveLength) {
    std::unique_ptr<Ray> r = std::make_unique<Ray>(origin, dir,lineColor,waveLength);
    m_raysToAdd.push_back(std::move(r));
}

// Add a point to an existing ray by index
void RayManager::addPointToRay(size_t rayIndex, const sf::Vector2f& point) {
    if (rayIndex >= m_rays.size()) {
        // Out of bounds
        throw std::out_of_range("Ray index out of range");
    }
    m_rays[rayIndex]->points.append(sf::Vertex{ point,m_rays[rayIndex]->lineColor });
   
}

const Ray RayManager::getRay(size_t rayIndex) const {
    if (rayIndex >= m_rays.size()) {
        throw std::out_of_range("Ray index out of range");
    }
    return *m_rays[rayIndex];
}

size_t RayManager::getRayCount() const
{
    return m_rays.size();
}

void RayManager::update()
{
    for (auto& r : m_raysToAdd )
    {
        m_rays.push_back(std::move(r));
    }
    m_raysToAdd.clear();

    auto toRemove = std::remove_if(m_rays.begin(), m_rays.end(), [](const std::unique_ptr<Ray>& r) {
        return r->destroy;
        });
    m_rays.erase(toRemove, m_rays.end());
}
