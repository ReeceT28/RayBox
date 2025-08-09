#include "Util.h"
#include "Vec2fExtension.h"
#include <iostream>
#include <cmath>
#include <stdexcept>

ArcData arcFromThreePoints(const sf::Vector2f& A, const sf::Vector2f& B, const sf::Vector2f& C)
{
    const float EPSILON = 1e-6f;
    const float TWO_PI = 2.f * 3.14159265f;

    // Vectors along segments
    sf::Vector2f AB = B - A;
    sf::Vector2f BC = C - B;

    // Midpoints of segments
    sf::Vector2f midAB = (A + B) * 0.5f;
    sf::Vector2f midBC = (B + C) * 0.5f;

    // Perpendicular vectors to segments (rotated 90 degrees CCW)
    sf::Vector2f perpAB(-AB.y, AB.x);
    sf::Vector2f perpBC(-BC.y, BC.x);

    // Check for colinearity
    float det = cross(AB, BC);
    if (std::abs(det) < EPSILON)
        throw std::runtime_error("Points are colinear; no unique circle");

    // Find intersection of perpendicular bisectors to get center
    sf::Vector2f delta = midBC - midAB;
    float denom = cross(perpAB, perpBC);
    if (std::abs(denom) < EPSILON)
        throw std::runtime_error("Perpendicular bisectors are parallel");

    float t = cross(delta, perpBC) / denom;
    sf::Vector2f center = midAB + t * perpAB;

    // Radius
    float radius = std::hypot(center.x - A.x, center.y - A.y);

    // Compute angles from center to points A, B, C normalized to [0, 2PI)
    auto angleFromCenter = [&](const sf::Vector2f& p) {
        float a = std::atan2(p.y - center.y, p.x - center.x);
        if (a < 0) a += TWO_PI;
        return a;
        };

    float angleA = angleFromCenter(A);
    float angleB = angleFromCenter(B);
    float angleC = angleFromCenter(C);

    // Calculate forward sweep from A to C
    float sweep = angleC - angleA;
    if (sweep < 0) sweep += TWO_PI;

    // Check if B lies between A and C on forward sweep
    bool bBetween = false;
    if (angleA <= angleC)
        bBetween = (angleB > angleA && angleB < angleC);
    else // wrapped around 0
        bBetween = (angleB > angleA || angleB < angleC);

    // If B is NOT between A and C, flip sweep to go the other way around circle
    if (!bBetween)
        sweep = sweep - TWO_PI; // negative sweep

    return { radius, sweep, center };
}
