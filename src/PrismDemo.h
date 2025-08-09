#pragma once
#include "Util.h"
#include "Entity.h"
#include "RayCollisionBuffers.h"
class PrismDemoShape : public sf::Shape {
public:
    PrismDemoShape(const float width, const float alpha, const sf::Vector2f& position, RayData* incidentRay);
	virtual std::size_t getPointCount() const override;
	virtual sf::Vector2f getPoint(std::size_t index) const override;
	void setAlpha(float alpha);
	void setAlphaIncrement(float increment);
	void setIncidentAngle(float angle);
	float getAlpha() const;
	float getAlphaIncrement() const; 
	float getIncidentAngle() const;
	RayData* getIncidentRay() const;
	void createIncidentRay();
	void updateAlpha();
private:
	void regenerateGeometry(); 
	RayData* m_incidentRay = nullptr;
	std::vector<sf::Vector2f> m_points;
	float m_width;
	int m_alphaDir = 1; // 1 for increasing, -1 for decreasing
	float m_alphaIncrement = 0.01f; // Increment for alpha adjustment
	float m_alpha; // Radians
	float m_incidentAngle = 45.0f/180.0f * pi; // Angle of the incident ray in radians
};
