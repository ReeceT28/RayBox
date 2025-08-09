#pragma once
#include <vector>
#include "Entity.h"
#include "Util.h"
#include <SFML/Graphics.hpp>

class CircularArcShape : public sf::Shape {
public:
    CircularArcShape(unsigned int resolution = 200);

    virtual std::size_t getPointCount() const override;
    virtual sf::Vector2f getPoint(std::size_t index) const override;

    void addMarker(Entity* marker);
    void updateShape();
    bool isComplete();
    Entity* getMarker(const size_t index) const;
    void setMarkerPos(const size_t index, const sf::Vector2f& position);
    void setMarkerPos(Entity* marker, const sf::Vector2f& position);

private:
    float m_radius = 0.f;
    float m_arcAngle = 0.f;
    sf::Vector2f m_arcCenter;
    unsigned int m_resolution;

    std::vector<sf::Vector2f> m_points;
    std::vector<Entity*> m_markers;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
