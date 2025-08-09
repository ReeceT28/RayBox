#pragma once
#include<SFML/Graphics.hpp>
#include <iostream>
class Ray
{
public:
	sf::VertexArray points;
	sf::Vector2f dir;
	sf::Color lineColor;
	float waveLengthNm;
	Ray(const sf::Vector2f origin, const sf::Vector2f dir, const sf::Color lineColor, const float waveLengthNm) :
		points(sf::PrimitiveType::LineStrip),  dir(dir), lineColor(lineColor), waveLengthNm(waveLengthNm) {
		points.append(sf::Vertex{ origin,lineColor });
		if (lineColor == sf::Color(255, 255, 255, 255))
		{
			whiteLight = true;
		}
	}
	bool finishedProcessing = false;
	bool whiteLight = false;
	bool lightSource = false;
	bool destroy = false;
};
