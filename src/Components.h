#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include "Sellmeier.h"
#include "Entity.h"
#include <map>
#include "RayCollisionBuffers.h"
class Entity;


class CTransform
{
public:
	sf::Vector2f pos = { 0.0,0.0 };
	sf::Vector2f velocity = { 0.0,0.0 };
	float angle = 0;
	CTransform(const sf::Vector2f& p, const sf::Vector2f& v, const float a)
		:pos(p), velocity(v), angle(a) {}
};

class CShape
{
public:
	sf::CircleShape circle;
	CShape(float radius, int vertices, const sf::Color& fill, const sf::Color& outline, float thickness)
		: circle(radius, vertices)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		sf::Vector2f origin(radius, radius); // set origin to centre
		circle.setOrigin(origin); // set origin to centre
	}

};


class CCustomShape {
public:
	std::unique_ptr<sf::Shape> customShape;

	CCustomShape(std::unique_ptr<sf::Shape> shape)
		: customShape(std::move(shape))
	{}
};


class CPrism
{
	std::string profileTag;
public:


	CPrism(const std::string profileTag) 
		: profileTag(profileTag) {}
	std::string getTag() const
	{
		return profileTag;
	}
	void setTag(const std::string& tag)
	{
		profileTag = tag;
	}

};


template<typename T>
class CChildOf {
	T* parent;
public:
	CChildOf(T* parentEntity) : parent(parentEntity) {}
	T* getParent() const
	{
		return parent;
	}
	void setParent(T* newParent)
	{
		parent = newParent;
	}
};

enum class MarkerRole {
	// Lens controls
	LeftInset,
	RightInset,
	WidthAndHeight,
	// Single light ray markers
	singleRayMarker1,
	singleRayMarker2,
    // Custom Polygon Prism
	polygonPrismMarker,
	// Circular Arc Marker
	CircularArcMarker,
	// Point light marker
	PointLightMarker,
	ColouredPointLightMarker,
	// Line segment mirror marker
	LineMirrorMarker
};

struct EntityTag {};
struct RayTag {};

class CMarker {
	MarkerRole role;
	Entity* targetEntity = nullptr; // The entity this marker modifies
	RayData* targetRay = nullptr; // The ray this marker modifies, if applicable
	std::vector<RayData*> targetRays;  // NEW: Store associated rays
public:
	// Constructor for Entity*
	explicit CMarker(EntityTag,MarkerRole rolee, Entity* targEntity)
		:role(rolee), targetEntity(targEntity) {
	}
	// Constructor for RayData*
	explicit CMarker(RayTag, MarkerRole role, RayData* ray)
		: role(role), targetRay(ray) {
	}
	MarkerRole getRole() const
	{
		return role;
	}
	Entity* getTargetEntity() const
	{
		return targetEntity;
	}

	RayData* getTargetRay() const 
	{
		return targetRay;
	}

	RayData* getTargetRayAtIndex(size_t index) const 
	{
		if (index < targetRays.size())
		{
			return targetRays[index];
		}
		return nullptr; 
	}

	void addTargetRay(RayData* ray) 
	{
		targetRays.push_back(ray);
	}

	std::vector<RayData*>& getRays()
	{
		return targetRays;
	}

	void clearRays()
	{
		targetRays.clear();
	}
};

