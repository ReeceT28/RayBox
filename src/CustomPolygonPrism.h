#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Util.h"
#include "Entity.h"
#include "earcut.hpp"

class CustomPolygonPrism : public sf::Shape {
public:
    CustomPolygonPrism(Entity* initialMarker);
    virtual std::size_t getPointCount() const override;
    virtual sf::Vector2f getPoint(std::size_t index) const override;
    void addMarker(Entity* marker);
    void addPoint(sf::Vector2f);
    void updateShapeData();
    void setPreviewPoint(const sf::Vector2f& point);
    void clearPreview();
    void setShapeComplete();
    bool getShapeComplete() const;
    bool containsPoint(sf::Vector2f& point) const;
    void updateMarkerPositions();
    void destroyMarkers();
private:
    std::vector<sf::Vector2f> m_points;
    std::vector<Entity*> m_markers;
    sf::VertexArray m_triangulatedShape;

    // Header or private section of your class
    bool m_isShapeComplete = false;
    bool m_hasPreview = false;
    sf::Vector2f m_previewPoint;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
};
