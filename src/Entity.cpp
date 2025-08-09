#include "Entity.h"

const std::string& Entity::getTag()
{
	return m_tag;
}

void Entity::setTag(const std::string& tag)
{
	m_tag = tag;
}
