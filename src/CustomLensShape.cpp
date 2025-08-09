#include "customLensShape.h"
#include <cmath>
#include <iostream>

MyLensShape::MyLensShape(const float width,const float height,const float leftInset,const float rightInset, Entity* leftInsetMarker,
    Entity* rightInsetMarker, Entity* widthAndHeightMarker,const sf::Vector2f& pos, const unsigned int resolution)
    : m_width(width), m_height(height), m_resolution(resolution),
    leftInset(leftInset), rightInset(rightInset)
{
	setPosition(pos);
    update();
    setMarkerEntities(leftInsetMarker, rightInsetMarker, widthAndHeightMarker);
}

std::size_t MyLensShape::getPointCount() const
{
    return m_points.size();
}

sf::Vector2f MyLensShape::getPoint(std::size_t index) const
{
    return m_points[index];
}
void MyLensShape::destroyMarkers()
{
    m_leftMarker->m_dead=true;
	m_rightMarker->m_dead = true;
    m_widthAndHeightMarker->m_dead = true;
}

float MyLensShape::getLeftInset() const
{
    return leftInset;
}
float MyLensShape::getRightInset() const
{
    return rightInset;
}
float MyLensShape::getWidth() const
{
    return m_width;
}
float MyLensShape::getHeight() const
{
    return m_height;
}

void MyLensShape::setLeftInset(float newLeftInset)
{
    leftInset = newLeftInset;
    update();
}

void MyLensShape::setRightInset(float newRightInset)
{
    rightInset = newRightInset;
    update();
}

void MyLensShape::setInsets(float newLeftInset, float newRightInset) 
{
    leftInset = newLeftInset;
    rightInset = newRightInset;
    update();
}
void MyLensShape::setWidth(float newWidth)
{
    m_width = newWidth;
    update();
}
void MyLensShape::setHeight(float newHeight)
{
    m_height = newHeight;
    update();
}

// Private methods

void MyLensShape::sampleArc(std::vector<sf::Vector2f>& out,
    const sf::Vector2f& center,
    const sf::Vector2f& from,
    const sf::Vector2f& to,
    float radius,
    unsigned int resolution,
    float angle,
    float inset,
    bool right) {
    sf::Vector2f vStart = from - center;
	sf::Vector2f vEnd = to - center;
    float startAngle = std::atan2(vStart.y, vStart.x);
    float direction = 1.0f;
    float angleStep = direction * angle / static_cast<float>(resolution);
    unsigned int start = 0;
	unsigned int end = resolution;
    for (unsigned int i = start; i <= end; ++i) 
    {
        float theta = startAngle + angleStep * static_cast<float>(i);
        sf::Vector2f point = center + sf::Vector2f(std::cos(theta), std::sin(theta)) * radius;
        out.push_back(point);
    }
}

void MyLensShape::triangulate()
{
    using Coord = double;
    using Point = std::array<Coord, 2>;

    std::vector<std::vector<Point>> polygon;
    std::vector<Point> ring;

    // Convert m_points to Earcut input format
    for (const sf::Vector2f& p : m_points)
    {
        ring.push_back({ static_cast<Coord>(p.x), static_cast<Coord>(p.y) });
    }
    polygon.push_back(ring);

    // Triangulates shape using earcut 
    m_indices = mapbox::earcut<uint32_t>(polygon);
}

void MyLensShape::update()
{
    m_points.clear();
    float h = m_height / 2.0f;
    sf::Vector2f topLeft(0.f, 0.f);
    sf::Vector2f topRight(m_width, 0.f);
    sf::Vector2f bottomLeft(0.f, m_height);
    sf::Vector2f bottomRight(m_width, m_height);
    sf::Vector2f leftMid(leftInset, h);
    sf::Vector2f rightMid(m_width - rightInset, h);
    ArcData rightCircle;
    ArcData leftCircle;
    bool rightCircleValid = true;
    bool leftCircleValid = true;
    try 
    {
        rightCircle = arcFromThreePoints(topRight, rightMid, bottomRight);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Right arc error: " << e.what() << std::endl;
        rightCircleValid = false;
    }

    try
    {
        leftCircle = arcFromThreePoints(bottomLeft, leftMid, topLeft);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Left arc error: " << e.what() << std::endl;
        leftCircleValid = false;
    }
    m_points.push_back(topLeft);
    m_points.push_back(topRight);
    if (rightCircleValid)
    {
        sampleArc(m_points, rightCircle.center, topRight, bottomRight, rightCircle.radius, m_resolution, rightCircle.angle, rightInset, true);
    }
    m_points.push_back(bottomRight);
    m_points.push_back(bottomLeft);
    if (leftCircleValid)
    {
        sampleArc(m_points, leftCircle.center, bottomLeft, topLeft, leftCircle.radius, m_resolution, leftCircle.angle, leftInset, false);
    }
    setOrigin(sf::Vector2f(m_width / 2.0f, m_height / 2.0f));
    updateMarkers();
    triangulate();
    sf::Shape::update();
}



void MyLensShape::updateMarkers()
{
    sf::Vector2f topLeftPos = getPosition() - getOrigin();
    if (m_leftMarker) 
    {
        m_leftMarker->cShape->circle.setPosition(topLeftPos + sf::Vector2f(leftInset, m_height / 2.0f));
    }
    if (m_rightMarker)
    {
        m_rightMarker->cShape->circle.setPosition(topLeftPos + sf::Vector2f(m_width - rightInset, m_height / 2.0f));
    }
    if (m_widthAndHeightMarker)
    {
        m_widthAndHeightMarker->cShape->circle.setPosition(topLeftPos + sf::Vector2f(0.0f, 0.0f));
    }
}

void MyLensShape::setMarkerEntities(Entity* leftMarker, Entity* rightMarker, Entity* widthAndHeightMarker)
{
    m_leftMarker = leftMarker;
    m_rightMarker = rightMarker;
    m_widthAndHeightMarker = widthAndHeightMarker;
    updateMarkers();
}

void MyLensShape::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // Apply transform of the shape
    states.transform *= getTransform();

    // Create vertex array for triangles
    sf::VertexArray va(sf::PrimitiveType::Triangles, m_indices.size());

    for (size_t i = 0; i < m_indices.size(); ++i)
    {
        uint32_t index = m_indices[i];
        sf::Vector2f pos = m_points[index];
        va[i].position = pos;

        // Set color or other attributes here
        va[i].color = getFillColor();
    }

    // Draw the vertex array
    target.draw(va, states);

    // Optionally draw outline with sf::LinesStrip or sf::LineLoop:
    if (getOutlineThickness() != 0)
    {
        sf::VertexArray outline(sf::PrimitiveType::LineStrip, m_points.size() + 1);
        for (size_t i = 0; i < m_points.size(); ++i)
        {
            outline[i].position = m_points[i];
            outline[i].color = getOutlineColor();
        }
        outline[m_points.size()].position = m_points[0];
        outline[m_points.size()].color = getOutlineColor();

        target.draw(outline, states);
    }
}
