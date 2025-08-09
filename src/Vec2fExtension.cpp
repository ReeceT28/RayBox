#include "Vec2fExtension.h"
#include <cmath>

// Operator overloads
sf::Vector2f operator-(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return sf::Vector2f(a.x - b.x, a.y - b.y);
}

sf::Vector2f operator+(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return sf::Vector2f(a.x + b.x, a.y + b.y);
}

sf::Vector2f operator*(const sf::Vector2f& a, float scalar) 
{
	return sf::Vector2f(a.x * scalar, a.y * scalar);
}

// 2D cross product
float cross(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return a.x * b.y - a.y * b.x;
}
// 2D dot product
float dotProduct(const sf::Vector2f& a, const sf::Vector2f& b)
{
	return a.x * b.x + a.y * b.y;
}
float magnitude(const sf::Vector2f& a)
{
	return std::sqrt(a.x * a.x + a.y * a.y);
}
sf::Vector2f normalize(const sf::Vector2f& a)
{
	return a * 1 / magnitude(a);
}
