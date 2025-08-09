#pragma once
#include<algorithm>
#include <SFML/Graphics.hpp>
struct ArcData {
    float radius;
    float angle; // in radians
    sf::Vector2f center;
};

ArcData arcFromThreePoints(const sf::Vector2f &, const sf::Vector2f &, const sf::Vector2f &);