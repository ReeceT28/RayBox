#pragma once
#include <string>
#include "Components.h"
#include "RayCollisionBuffers.h"
#include <memory>
class Entity
{
	std::string m_tag = "";
	size_t m_ID = 0;
public:
	std::unique_ptr<CTransform> cTransform;
	std::unique_ptr<CShape> cShape;
	std::unique_ptr<CCustomShape> cCustomShape;
	std::unique_ptr<CPrism> cPrism;
	std::unique_ptr<CChildOf<Entity>> cChildOfEntity;
	std::unique_ptr<CChildOf<RayData>> cChildOfRay;
	std::unique_ptr<CMarker> cMarker;

	bool m_dead = false;
	bool m_isMirror = false;

	Entity(const std::string& tag, size_t ID)
		:m_tag(tag), m_ID(ID) {}

	const std::string& getTag();
	void setTag(const std::string& tag);

};

