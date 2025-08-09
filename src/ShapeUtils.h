#pragma once
#include <SFML/Graphics.hpp>

// Point-in-polygon test using ray casting
inline bool shapeContainsPoint(const sf::Shape& shape, const sf::Vector2f& worldPoint)
{
    sf::Vector2f localPoint = shape.getInverseTransform().transformPoint(worldPoint);
    size_t count = shape.getPointCount();
    int crossings = 0;

    for (size_t i = 0; i < count; i++)
    {
        sf::Vector2f a = shape.getPoint(i);
        sf::Vector2f b = shape.getPoint((i + 1) % count);

        if (((a.y > localPoint.y) != (b.y > localPoint.y)) &&
            (localPoint.x < (b.x - a.x) * (localPoint.y - a.y) / (b.y - a.y) + a.x))
        {
            crossings++;
        }
    }

    return (crossings % 2 == 1);
}
