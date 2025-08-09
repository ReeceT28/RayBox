#pragma once
#include <SFML/Graphics.hpp>
#include "Ray.h"
#include <memory>
#include <vector>

class RayManager {
private:
    // Using sf::LinesStrip so points are connected in order
    std::vector<std::unique_ptr<Ray>> m_rays;
    std::vector<std::unique_ptr<Ray>> m_raysToAdd;
public:
    void createRay(const sf::Vector2f& origin, const sf::Vector2f& dir, const sf::Color& lineColor, const float waveLength);
    void addPointToRay(size_t rayIndex, const sf::Vector2f& point);
    void update();
    const Ray getRay(size_t rayIndex) const;
    size_t getRayCount() const;
    const std::vector<std::unique_ptr<Ray>>& getRays() const;
};
