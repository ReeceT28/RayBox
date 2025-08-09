#pragma once

#pragma once
#include <SFML/System/Vector2.hpp>

// Operator overloads for Vector2f
sf::Vector2f operator-(const sf::Vector2f& a, const sf::Vector2f& b);
sf::Vector2f operator+(const sf::Vector2f& a, const sf::Vector2f& b);
sf::Vector2f operator*(const sf::Vector2f& a, float scalar);

// Extension function for Vector2f
float cross(const sf::Vector2f& a, const sf::Vector2f& b);
float dotProduct(const sf::Vector2f& a, const sf::Vector2f& b);
float magnitude(const sf::Vector2f& a);
sf::Vector2f normalize(const sf::Vector2f& a);
