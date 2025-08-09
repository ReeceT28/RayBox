#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "CustomPolygonPrism.h"
#include "Entity.h"

CustomPolygonPrism::CustomPolygonPrism(Entity* initialMarker)
{
    setPosition({ 0, 0 });  
    addMarker(initialMarker);
    addPoint(initialMarker->cShape->circle.getPosition());
}

std::size_t CustomPolygonPrism::getPointCount() const
{
    return m_points.size();
}

sf::Vector2f CustomPolygonPrism::getPoint(std::size_t index) const
{
    if (index >= m_points.size()) 
    {
        throw std::out_of_range("CustomPolygonPrism::getPoint index out of range");
    }
    return m_points[index];
}

void CustomPolygonPrism::addMarker(Entity* marker)
{
    m_markers.push_back(marker);
}

void CustomPolygonPrism::addPoint(sf::Vector2f point)
{
    m_points.push_back(point);
    updateShapeData();
}

void CustomPolygonPrism::updateShapeData()
{
    using Coord = float;
    using Point = std::array<Coord, 2>;
    using Polygon = std::vector<std::vector<Point>>;
    // Convert m_points to Earcut input format
    std::vector<Point> outerRing;
    for (const sf::Vector2f& pt : m_points)
    {
        outerRing.push_back({ pt.x, pt.y });
    }
    Polygon polygon = { outerRing };
    // Triangulates shape using earcut 
    std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygon);
    // m_triangulated shape stores a bunch of triangles which we can draw
    m_triangulatedShape.clear();
    m_triangulatedShape.setPrimitiveType(sf::PrimitiveType::Triangles);

    for (uint32_t i : indices) 
    {
        const sf::Vector2f& pt = m_points[i];
        m_triangulatedShape.append(sf::Vertex(pt, getFillColor()));  // Set each triangle to correct fill colour
    
    }
    sf::Shape::update();
}
void CustomPolygonPrism::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // Apply the transform of the shape
    states.transform *= getTransform();

    // Draw the triangulated shape if the shape is marked as completed
    if (m_isShapeComplete)
        target.draw(m_triangulatedShape, states);

    // We add an extra point if preview which is the point of the mouse which we pass in 
    size_t basePointCount = m_points.size();
    size_t outlineCount = basePointCount + (m_hasPreview ? 1 : 0) + (m_isShapeComplete ? 1 : 0);

    sf::VertexArray outline(sf::PrimitiveType::LineStrip, outlineCount);

    size_t i = 0;

    // Add actual points
    for (; i < basePointCount; i++)
    {
        outline[i].position = m_points[i];
        outline[i].color = sf::Color::White;
    }

    // Add preview point if active
    if (m_hasPreview)
    {
        outline[i].position = m_previewPoint;
        outline[i].color = sf::Color(180, 180, 180);  // Light gray for preview
        i++;
    }

    // Close loop if shape is complete
    if (m_isShapeComplete && !m_points.empty())
    {
        outline[i].position = m_points[0];
        outline[i].color = sf::Color::White;
    }

    target.draw(outline, states);
}

void CustomPolygonPrism::setPreviewPoint(const sf::Vector2f& point)
{
    m_previewPoint = point;
    m_hasPreview = true;
}

void CustomPolygonPrism::clearPreview()
{
    m_hasPreview = false;
}

void CustomPolygonPrism::setShapeComplete()
{
    m_isShapeComplete = true;
    clearPreview();
}

bool CustomPolygonPrism::getShapeComplete() const
{
    return m_isShapeComplete;
}


bool CustomPolygonPrism::containsPoint(sf::Vector2f& point) const 
{
    // Transform point into local coordinates
    sf::Vector2f localPoint = getInverseTransform().transformPoint(point);

    int crossings = 0;
    size_t count = m_points.size();
    for (size_t i = 0; i < count; ++i) 
    {
        sf::Vector2f a = m_points[i];
        sf::Vector2f b = m_points[(i + 1) % count];

        if (((a.y > localPoint.y) != (b.y > localPoint.y)) &&
            (localPoint.x < (b.x - a.x) * (localPoint.y - a.y) / (b.y - a.y) + a.x))
            crossings++;
    }

    return (crossings % 2 == 1);  // Inside if odd crossings
}

void CustomPolygonPrism::updateMarkerPositions()
{
    // For each marker, update its position to match the corresponding point (offset by shape position)
    for (size_t i = 0; i < m_markers.size() && i < m_points.size(); ++i)
    {
        m_markers[i]->cShape->circle.setPosition(getPosition() + m_points[i]);
    }
}

void CustomPolygonPrism::destroyMarkers()
{
    for (Entity* marker : m_markers)
    {
        marker->m_dead = true;  // Mark each marker for deletion
    }
    m_markers.clear();  // Clear the markers vector
}



