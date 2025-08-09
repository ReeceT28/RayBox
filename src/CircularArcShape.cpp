#include "CirularArcShape.h"
#include "Vec2fExtension.h"
#include <cmath>
#include <iostream>

CircularArcShape::CircularArcShape(unsigned int resolution)
    : m_resolution(resolution < 2 ? 2 : resolution), m_markers(3, nullptr) {
}

void CircularArcShape::addMarker(Entity* marker)
{
    for (size_t i = 0; i < m_markers.size(); ++i)
    {
        if (m_markers[i] == nullptr)
        {
            m_markers[i] = marker;
            if (isComplete()) updateShape();
            return;
        }
    }

    std::cerr << "Max of 3 markers already added.\n";
}

void CircularArcShape::updateShape()
{
    if (!isComplete()) return;
    sf::Vector2f p1 = m_markers[0]->cShape->circle.getPosition();
    sf::Vector2f p2 = m_markers[1]->cShape->circle.getPosition();
    sf::Vector2f p3 = m_markers[2]->cShape->circle.getPosition();

    ArcData arc;
    try
    {
        arc = arcFromThreePoints(p1, p3, p2);
    }
    catch (const std::exception& e)
    {
        std::cerr << "arcFromThreePoints error: " << e.what() << '\n';
        return;
    }

    m_radius = arc.radius;
    m_arcAngle = arc.angle;
    m_arcCenter = arc.center;

    m_points.clear();

    // Start angle from center to p1
    float angleStart = std::atan2(p1.y - m_arcCenter.y, p1.x - m_arcCenter.x);

    // angleStep is total arc angle divided by (resolution - 1)
    float angleStep = m_arcAngle / static_cast<float>(m_resolution - 1);

    for (unsigned int i = 0; i < m_resolution; ++i)
    {
        float angle = angleStart + i * angleStep;
        float x = std::cos(angle) * m_radius;
        float y = std::sin(angle) * m_radius;
        m_points.emplace_back(m_arcCenter + sf::Vector2f(x, y));
    }

    update(); // refresh SFML shape or equivalent
}


std::size_t CircularArcShape::getPointCount() const
{
    return m_points.size();
}

sf::Vector2f CircularArcShape::getPoint(std::size_t index) const
{
    if (index >= m_points.size())
    {
        std::cerr << "CircularArcShape: index out of bounds.\n";
        return sf::Vector2f();
    }

    return m_points[index];
}

void CircularArcShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();

    sf::VertexArray arc(sf::PrimitiveType::LineStrip, m_points.size());
    for (std::size_t i = 0; i < m_points.size(); ++i)
    {
        arc[i].position = m_points[i];
        arc[i].color = getOutlineColor();
    }

    target.draw(arc, states);
}

bool CircularArcShape::isComplete()
{
    return std::all_of(m_markers.begin(), m_markers.end(), [](Entity* e) { return e != nullptr; });
}

Entity* CircularArcShape::getMarker(const size_t index) const
{
    return m_markers[index];
}

void CircularArcShape::setMarkerPos(const size_t index, const sf::Vector2f& position)
{
    if (m_markers[index] == nullptr) return;
    m_markers[index]->cShape->circle.setPosition(position);
    updateShape();
}

void CircularArcShape::setMarkerPos(Entity* marker, const sf::Vector2f& position)
{
    marker->cShape->circle.setPosition(position);
    updateShape();
}
