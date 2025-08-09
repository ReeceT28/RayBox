#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Util.h"
#include "Entity.h"
#include "earcut.hpp"


class MyLensShape : public sf::Shape {
public:
    MyLensShape(const float width,const float height,const float leftInset,const float rightInset, Entity* , Entity* , Entity*,const sf::Vector2f & pos, const unsigned int resolution = 500);

    virtual std::size_t getPointCount() const override;
    virtual sf::Vector2f getPoint(std::size_t index) const override;

    void setLeftInset(float);
    void setRightInset(float);
    void setInsets(float, float);
	void setWidth(float newWidth);
	void setHeight(float newHeight);
	void setMarkerEntities(Entity* leftMarker, Entity* rightMarker, Entity* widthAndHeightMarker);
    void updateMarkers();
    void destroyMarkers();

    float getLeftInset() const;
    float getRightInset() const;
    float getWidth() const;
    float getHeight() const;
    
private:
    void update();
    void triangulate();
    void sampleArc(std::vector<sf::Vector2f>& out,
        const sf::Vector2f& center,
        const sf::Vector2f& from,
        const sf::Vector2f& to,
        float radius,
        unsigned int resolution,
        float angle, float inset, bool right);
    Entity* m_leftMarker;
    Entity* m_rightMarker;
    Entity* m_widthAndHeightMarker;
    float leftInset, rightInset, m_width, m_height;
    unsigned int m_resolution;
    std::vector<sf::Vector2f> m_points;
    std::vector<uint32_t> m_indices;

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

};
