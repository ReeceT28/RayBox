#include "PrismDemo.h"

PrismDemoShape::PrismDemoShape(const float width, const float alpha, const sf::Vector2f& position, RayData* incidentRay)
	: m_width(width), m_alpha(alpha), m_incidentRay(incidentRay)
{
	setPosition(position);
	regenerateGeometry();
	createIncidentRay(); // Create the incident ray with the initial angle
}
sf::Vector2f PrismDemoShape::getPoint(size_t index)const 
{
	return m_points[index];
}

std::size_t PrismDemoShape::getPointCount() const
{
	return m_points.size();
}
float PrismDemoShape::getAlpha() const
{
	return m_alpha;
}
float PrismDemoShape::getAlphaIncrement() const
{
	return m_alphaIncrement;
}

void PrismDemoShape::setAlpha(float alpha)
{
	m_alpha = alpha;
	regenerateGeometry();
}
void PrismDemoShape::setAlphaIncrement(float increment)
{
	m_alphaIncrement = increment;
}

void PrismDemoShape::regenerateGeometry()
{
    m_points.clear();

    float halfWidth = m_width / 2.0f;

    // Protect against invalid alpha (e.g., alpha = 0)
    if (std::abs(m_alpha) < 1e-5f) return;

    float height = halfWidth / std::tan(m_alpha / 2.0f);

    sf::Vector2f bottomLeft(-halfWidth, height);
    sf::Vector2f bottomRight(halfWidth, height);
    sf::Vector2f tip(0.0f, 0.0f);

    m_points = { tip, bottomRight, bottomLeft };
	createIncidentRay(); // Update the incident ray with the current alpha
    if (m_points.size() >= 3)
        sf::Shape::update();
}

void PrismDemoShape::updateAlpha() 
{
	if (m_alpha > pi/2.0f - 0.15f)
	{
		m_alphaDir = -1;
	}
	else if (m_alpha < 0.3)
	{
		m_alphaDir = 1;
	}
	m_alpha += m_alphaIncrement * m_alphaDir;
	regenerateGeometry();
}

void PrismDemoShape::createIncidentRay()
{

	// Normal of the left face (in SFML's clockwise system)
	float angle = m_alpha / 2.0f - pi - m_incidentAngle; // Normal is at half the angle of the prism base, adjusted for clockwise rotation
	// Final angle is normal + 40°, since rotation is clockwise

	// Ray direction
	sf::Vector2f dir = {
		std::cos(angle),
		std::sin(angle)
	};
	dir = -dir;

	// Origin slightly to the left of the prism base
	sf::Vector2f origin = (m_points[0] + m_points[2]) / 2.0f; // midpoint of left side
	origin -= dir * 100.0f;
	origin = getTransform().transformPoint(origin); // World coordinates

	m_incidentRay->originX = origin.x;
	m_incidentRay->originY = origin.y;
	m_incidentRay->dirX = dir.x;
	m_incidentRay->dirY = dir.y;
	m_incidentRay->finished = false;
	m_incidentRay->whiteLight = true;
}


void PrismDemoShape::setIncidentAngle(float angle)
{
	m_incidentAngle = angle;
	createIncidentRay(); // Update the incident ray with the new angle
}
float PrismDemoShape::getIncidentAngle() const
{
	return m_incidentAngle;
}

RayData* PrismDemoShape::getIncidentRay() const
{
	return m_incidentRay;
}
