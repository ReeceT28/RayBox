#include "Simulation.h"
#include<optional>
#include<limits>
#include<iostream>
#include <tuple>
#include <cmath>
#include "customLensShape.h"
#include <algorithm>
#include "Components.h"
#include "SellmeierManager.h"

void Simulation::init()
{
	// ╔════════════════════════════════════╗
	// ║   WINDOW AND VIEW INITIALIZATION   ║
	// ╚════════════════════════════════════╝
	std::cout << "Simulation initialising\n";
	m_window.create(sf::VideoMode({ 1200, 800 }), "SIMULATION");
	m_window.setFramerateLimit(60);
	m_view = m_window.getDefaultView();
	// Initialize ImGui with the SFML window
	ImGui::SFML::Init(m_window);
	m_blendMode = sf::BlendMode(
		sf::BlendMode::Factor::SrcAlpha,
		sf::BlendMode::Factor::DstAlpha,
		sf::BlendMode::Equation::Max);
	// ╔════════════════════════════════╗
	// ║   SELLMEIER PROFILE CREATION   ║
	// ╚════════════════════════════════╝
	m_sellmeierManager.addProfile(
		{ 1.03961212, 0.231792344, 1.01146945 },
		{ 0.00600069867, 0.0200179144, 103.560653 },
		"Crown Glass");
	m_sellmeierManager.addProfile(
		{ 1.43134930, 0.65054713 , 5.3414021 },
		{ 0.0052799261, 0.0142382647 , 325.017834 },
		"Sapphire");
	m_sellmeierManager.addProfile(
		{ 0.6961663 , 0.4079426  , 0.8974794 },
		{ 0.004679148 , 0.01351206 , 97.934 },
		"Fused Silica");
	m_sellmeierManager.addProfile(
		{ 0.48755108 ,0.39875031 ,2.3120353 },
		{ 0.001882178,  0.008951888 , 566.13559 },
		"Magnesium Fluoride");
	m_sellmeierManager.addProfile(
		{ 0.568908 , 0.173945 , 0.020397 },
		{ 0.005101,  0.019199 , 36.54681 },
		"Water");
	// ╔═════════════════════════════════╗
	// ║   MAPS WAVELENGTHS TO COLOURS   ║
	// ╚═════════════════════════════════╝
	float step = (m_endWavelength - m_startWavelength) / (prismResolution - 1); 
	for (int i = 0; i < prismResolution; ++i)
	{
		float wavelength = m_startWavelength + i * step;
		sf::Color color = wavelengthToRGB(wavelength);
		wavelengthColors[wavelength] = color;
	}

	// ╔═════════════════════════════════════════════════╗
	// ║   EXAMPLE SCENES (LONG AND SHORT SIGHTEDNESS)   ║
	// ╚═════════════════════════════════════════════════╝
	if (false)
	{
		sCreatePrism({ 300.0f, 100.0f }, 150.0f, 300, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass", 0.0f);
		sCreateWhiteRay({ 0.0f, 100.0f }, { 1.0f,0.0f });
		sCreateWhiteRay({ 0.0f, 50.0f }, { 1.0f,0.0f });
		sCreateWhiteRay({ 0.0f, 150.0f }, { 1.0f,0.0f });
		m_stateChange = true;
		sCreatePrism({ 350.0f, 500.0f }, 150.0f, 300, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass", 0.0f);
		sCreateLens({ 170.0f,500.0f }, 15.0f, 150.0f, -7.0f, -7.0f, sf::Color(50, 50, 50, 50), sf::Color::White, "Magnesium Fluoride");
		sCreateWhiteRay({ 0.0f, 500.0f }, { 1.0f,0.0f });
		sCreateWhiteRay({ 0.0f, 450.0f }, { 1.0f,0.0f });
		sCreateWhiteRay({ 0.0f, 550.0f }, { 1.0f,0.0f });
	}
	if (false)
	{
			Entity* prism = sCreatePrism({ 300.0f, 900.0f }, 150.0f, 300, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass", 0.0f);
			prism->cShape->circle.setScale({ 1.3f, 1.0f });
			sCreateWhiteRay({ 0.0f, 900.0f }, { 1.0f,0.0f });
			sCreateWhiteRay({ 0.0f, 850.0f }, { 1.0f,0.0f });
			sCreateWhiteRay({ 0.0f, 950.0f }, { 1.0f,0.0f });
			m_stateChange = true;
			prism = sCreatePrism({ 400.0f, 1300.0f }, 150.0f, 300, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass", 0.0f);
			prism->cShape->circle.setScale({ 1.3f, 1.0f });
			sCreateLens({ 170.0f,1300.0f }, 15.0f, 150.0f, 2.0f, 2.0f, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass");
			sCreateWhiteRay({ 0.0f, 1300.0f }, { 1.0f,0.0f });
			sCreateWhiteRay({ 0.0f, 1350.0f }, { 1.0f,0.0f });
			sCreateWhiteRay({ 0.0f, 1250.0f }, { 1.0f,0.0f });
	}

	std::cout << "Simulation succesfully initialized with " << prismResolution << " ray resolution." << '\n';
}

void Simulation::run()
{
	// Initialise simulation
	init();
	// ╔═══════════════╗
	// ║   MAIN LOOP   ║
	// ╚═══════════════╝
	while (m_running)
	{
		m_entities.update();
		sUpdateWavelengthCreation();
		sUpdateAlpha();
		sUserInput();
		sHandleStateChange();
		sCollisionv2();
		ImGui::SFML::Update(m_window, m_deltaClock.restart());
		sGui();
		sRender();
	}
	// ---- Shut down ImGui ----
	ImGui::SFML::Shutdown(); 
}

void Simulation::sHandleStateChange()
{
	if (m_stateChange)
	{
		// ---- Reset buffers and clear all rays ----
		allRays.clear();
		m_buffers.currRayCount = 0;
		// ---- Create rays for all light sources ----
		for (const auto& lightSource : m_lightSources)
		{
			m_buffers.createRay(RayData(
				lightSource->originX, lightSource->originY,
				lightSource->dirX, lightSource->dirY,
				false, lightSource->whiteLight, 1.0f,
				lightSource->color, lightSource->wavelength
			));
		}
		// ---- Commit the new rays to the buffer ----
		m_buffers.commitNewRays();
		// ---- Reset flag ----
		m_stateChange = false;
	}
	return;
}

void Simulation::sRender()
{
	m_window.clear();
	// ---- Draw all rays ----
	m_window.draw(allRays, m_blendMode);
	// ---- Draw all entities ----
	for (auto& e : m_entities.getEntities())
	{
		if (e->cShape)
		{
			m_window.draw(e->cShape->circle);
		}
		else if(e->cCustomShape)
		{
			m_window.draw(*e->cCustomShape->customShape.get());
		}
		else
		{
			std::cerr << "Entity without shape or custom shape found in sRender().\n";
		}
	}
	// ---- Render ImGui ----
	ImGui::SFML::Render(m_window);
	m_window.display();
}

void Simulation::sUpdateAlpha()
{
	for (auto& e : m_entities.getEntities())
	{
		if (e->cCustomShape)
		{
			// If the shape is a prism demo shape then update the alpha value 
			if (auto* demoShape = dynamic_cast<PrismDemoShape*>(e->cCustomShape->customShape.get()))
			{
				demoShape->updateAlpha();
				m_stateChange = true;
			}
		}
	}
}

void Simulation::sCreateLens(const sf::Vector2f& lensPos, const float width, const float height, const float leftInset, const float rightInset,
	const sf::Color& fillColor, const sf::Color& outlineColor,const std::string& tag)
{
	// Create all necessary markers for the lens
	Entity* leftInsetMarker = sCreateMarker(lensPos + sf::Vector2f(leftInset, height / 2.0f), 2.5f, 4,
		sf::Color(255, 0, 0, 122),
		sf::Color(255, 255, 255, 255),
		45.0f);
	Entity* rightInsetMarker = sCreateMarker(lensPos + sf::Vector2f(width - rightInset, height/2.0f), 2.5f, 4,
		sf::Color(255, 0, 0, 122),
		sf::Color(255, 255, 255, 255),
		45.0f);
	Entity* wAndHMarker = sCreateMarker(lensPos + sf::Vector2f(width / 2.0f, 0.0f), 2.5f, 4,
		sf::Color(255, 0, 0, 122),
		sf::Color(255, 255, 255, 255),
		45.0f);
	// Initialise a MyLensShape
	MyLensShape lens(width, height, leftInset, rightInset, leftInsetMarker, rightInsetMarker, wAndHMarker, lensPos);
	// Set properties of the lens shape
	lens.setOutlineColor(outlineColor);
	lens.setOutlineThickness(1.f);
	lens.setFillColor(fillColor);
	// Add entity to m_entities
	Entity* lensEntity = &m_entities.addEntity("Lens");
	// Give all the markers marker Components with the correct Roles
	leftInsetMarker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::LeftInset, lensEntity);
	rightInsetMarker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::RightInset, lensEntity);
	wAndHMarker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::WidthAndHeight, lensEntity);
	// Give lens entity a prism and custom shape component 
	lensEntity->cCustomShape = std::make_unique<CCustomShape>(std::make_unique<MyLensShape>(lens));
	lensEntity->cPrism = std::make_unique<CPrism>(tag);
}

void Simulation::sCreateDemoPrism(const sf::Vector2f& position, const float width, const float angle, const sf::Color& fillColor, const sf::Color& borderColor, const std::string& material)
{
	// Just some random data as it gets updated immediately 
	RayData* ray = sCreateWhiteRay({ 0.0f, 0.0f }, { 1.0f, 0.0f }); 
	// ---- Create the demo prism and set its properties ----
	std::unique_ptr<PrismDemoShape> prismShape = std::make_unique<PrismDemoShape>(width, angle, position,ray);
	prismShape->setFillColor(fillColor);
	prismShape->setOutlineColor(borderColor);
	prismShape->setOutlineThickness(1.0f);
	// ---- Create the Entity and assign the custom shape and prism components ----
	Entity* prismEntity = &m_entities.addEntity("DemoPrism");
	prismEntity->cCustomShape = std::make_unique<CCustomShape>(std::move(prismShape));
	prismEntity->cPrism = std::make_unique<CPrism>(material);
}

Entity* Simulation::sCreatePrism(const sf::Vector2f& position, const int radius, const int sides, const sf::Color& insideColor, const sf::Color& borderColor, const std::string& tag, const float rotationDeg)
{
	Entity* e = &m_entities.addEntity("Prism");
	e->cShape = std::make_unique<CShape>(radius, sides, insideColor, borderColor, 1.0f);
	e->cShape->circle.setPosition(position);
	e->cPrism = std::make_unique<CPrism>(tag);
	e->cShape->circle.setRotation(sf::Angle(sf::degrees(rotationDeg)));
	return e;
}

Entity* Simulation::sCreateCustomPolygonPrism(const sf::Vector2f& position, const sf::Color& fillColour, const sf::Color& borderColour, const std::string material)
{
	Entity* customPrism = &m_entities.addEntity("CustomPolyPrism");
	Entity* marker = sCreateMarker(position, 2.5f, 4,
		sf::Color(255, 0, 0, 122),
		sf::Color(255, 255, 255, 255),
		45.0f);
	marker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::polygonPrismMarker, customPrism);
	customPrism->cCustomShape = std::make_unique<CCustomShape>(std::make_unique<CustomPolygonPrism>(marker));
	CustomPolygonPrism* customPrismShape = dynamic_cast<CustomPolygonPrism*>(customPrism->cCustomShape->customShape.get());
	customPrismShape->setFillColor(fillColour);
	customPrismShape->setOutlineColor(borderColour);
	customPrismShape->setOutlineThickness(1.0f);
	customPrism->cPrism = std::make_unique<CPrism>(material);
	return marker;
}

Entity* Simulation::sCreateMarker(const sf::Vector2f& position, const float radius, const int sides,
	const sf::Color& insideColor, const sf::Color& borderColor,
	const float rotationDeg)
{
	Entity* marker = &m_entities.addEntity("Marker");
	marker->cShape = std::make_unique<CShape>(radius, sides, insideColor, borderColor, 1.0f);
	marker->cShape->circle.setPosition(position);
	marker->cShape->circle.setRotation(sf::Angle(sf::degrees(rotationDeg)));
	return marker;
}

Entity* Simulation::sCreateCircularArc(Entity* m1, Entity* m2, Entity* m3, const sf::Color& borderColour)
{
	auto arcShape = std::make_unique<CircularArcShape>();

	arcShape->addMarker(m1);
	arcShape->addMarker(m2);
	arcShape->addMarker(m3);

	arcShape->setOutlineColor(borderColour);

	Entity* e = &m_entities.addEntity("CircularArc");
	e->cCustomShape = std::make_unique<CCustomShape>(std::move(arcShape));
	e->m_isMirror = true;

	return e;
}


RayData* Simulation::sCreateWhiteRay(sf::Vector2f origin, sf::Vector2f dir)
{
	m_lightSources.emplace_back(std::make_unique<RayData>(
		origin.x, origin.y, dir.x, dir.y,
		false, true, 1.0f,
		sf::Color(255, 255, 255, 255),
		0.0f));
	return m_lightSources.back().get();
}

RayData* Simulation::sCreateColouredRay(sf::Vector2f origin, sf::Vector2f dir, float wavelength)
{
	// std::cout << "Creating coloured ray with wavelength: " << wavelength << " nm\n";
	sf::Color color = wavelengthToRGB(wavelength);
	m_lightSources.emplace_back(std::make_unique<RayData>(
		origin.x, origin.y, dir.x, dir.y,
		false, false, 1.0f,
		color,
		wavelength));
	return m_lightSources.back().get();
}

void Simulation::sUpdateLensMarkerPositions(Entity* leftInsetMarker, Entity* rightInsetMarker, Entity* widthAndHeightMarker)
{
	if (!leftInsetMarker || !rightInsetMarker || !widthAndHeightMarker) return;
	// Assume all markers have the same Lens target Entity in CMarker
	Entity* lens = leftInsetMarker->cMarker->getTargetEntity();
	MyLensShape* lensShape = dynamic_cast<MyLensShape*>(lens->cCustomShape->customShape.get());
	sf::Vector2f lensPos = lensShape->getPosition();
	float lensHeight = lensShape->getHeight();
	float lensWidth = lensShape->getWidth();
	float leftInset = lensShape->getLeftInset();
	float rightInset = lensShape->getRightInset();
	leftInsetMarker->cShape->circle.setPosition(sf::Vector2f(lensPos.x + leftInset, lensHeight/2.0f ));
	rightInsetMarker->cShape->circle.setPosition(sf::Vector2f(lensWidth - rightInset, lensHeight/2.0f));
	widthAndHeightMarker->cShape->circle.setPosition(lensPos);
}

void Simulation::sRotateToMouse(sf::Vector2f & mouseWorldPos, sf::CircleShape& shape)
{

	// Origin is the center of the shape so returns (radius,radius)
	sf::Vector2f origin = shape.getOrigin();
	sf::Vector2f point0Local = shape.getPoint(0);
	// Vector from origin to point(0)
	sf::Vector2f point0Vector = point0Local - origin;
	// Calculate Angle that the point(0) vector makes with the x-axis
	float baseAngle = std::atan2(point0Vector.y, point0Vector.x);

	// Find vector from shape center to mouse
	sf::Vector2f shapePos = shape.getPosition();
	sf::Vector2f dirToMouse = mouseWorldPos - shapePos;
	// This is the desired angle that the point makes with the x-axis
	float desiredAngle = std::atan2(dirToMouse.y, dirToMouse.x);

	// Calculate needed rotation, we minus the base angle from the desired angle because we want point(0) to points towards the mouse
	float rotation = desiredAngle - baseAngle;

	// Apply rotation
	shape.setRotation(sf::radians(rotation));
	m_rotator->cShape->circle.setRotation(sf::radians(rotation)); // Optional

	// How far to place rotator from centre 
	float offsetDistance = shape.getRadius() + 25.f;
	// Normalised offset direction from centre
	sf::Vector2f offset(std::cos(desiredAngle), std::sin(desiredAngle));
	m_rotator->cShape->circle.setPosition(shapePos + offset * offsetDistance);

}

void Simulation::updateRotatorPosition()
{
	if (!m_selectedEntity || !m_rotator) return;

	auto& shape = m_selectedEntity->cShape->circle;
	auto& rotatorShape = m_rotator->cShape->circle;
	rotatorShape.setOrigin(sf::Vector2f(rotatorShape.getRadius(), rotatorShape.getRadius())); // Keep origin at center

	sf::Vector2f point0 = shape.getPoint(0);
	sf::Transform shapeTransform = shape.getTransform();
	point0 = shapeTransform.transformPoint(point0);

	rotatorShape.setPosition(point0 + normalize(point0 - shape.getPosition()) * 25.f);
	rotatorShape.setRotation(shape.getRotation());
}

void Simulation::sRenderScreenShot(sf::RenderTarget& target)
{
	target.clear();

	target.draw(allRays, m_blendMode);

	for (auto& e : m_entities.getEntities())
	{
		if (e->cShape)
		{
			target.draw(e->cShape->circle);
		}
		else // customShape
		{
			target.draw(*e->cCustomShape->customShape.get());
		}
	}

}

void Simulation::saveScreenshot(const std::string& filename, unsigned int scaleFactor)
{
	sf::Vector2u windowSize = m_window.getSize();
	sf::Vector2u screenshotSize = windowSize * scaleFactor;

	sf::RenderTexture renderTexture(screenshotSize);

	// Use the current view without changing size
	sf::View currentView = m_window.getView();
	// Make sure the viewport covers the entire render texture
	currentView.setViewport(sf::FloatRect({ 0.f, 0.f }, { 1.f, 1.f }));

	renderTexture.setView(currentView);

	renderTexture.clear(sf::Color::Black);
	sRenderScreenShot(renderTexture);
	renderTexture.display();

	sf::Image screenshot = renderTexture.getTexture().copyToImage();
	if (screenshot.saveToFile(filename)) {
		std::cout << "Screenshot saved to " << filename << "\n";
	}
	else {
		std::cerr << "Failed to save screenshot to " << filename << "\n";
	}
}

// Writing this made me realise if possible (as I don't know) that if we could have much more of the logic on GPU to reduce writes it would speed things up a lot 
// If anyone is reading this and wants to make something similar you should probably just use openGL for the whole thing with compute shaders instead of openCL
void Simulation::sCollisionv2()
{
	sf::Clock preProcessingClock;

	sf::Vector2f viewSize   = m_view.getSize();
	sf::Vector2f viewCenter = m_view.getCenter();

	float left = viewCenter.x - viewSize.x / 2.f;
	float top  = viewCenter.y - viewSize.y / 2.f;

	// Bounds in world coordinates  
	m_buffers.screenBounds[0] = (uint)left;         // left (x)

	m_buffers.screenBounds[1] = (uint)top;          // top (y)
	m_buffers.screenBounds[2] = (uint)viewSize.x;   // width
	m_buffers.screenBounds[3] = (uint)viewSize.y;   // height

	// Can do all entity data preparation outside of loop as it can't change in this function 
	// We do not know the number of vertices in each entity, so we will create a vector to hold all vertices and their counts

	std::vector<float> edgeA_X;
	std::vector<float> edgeA_Y;
	std::vector<float> edgeB_X;
	std::vector<float> edgeB_Y;
	std::vector<int> edgeEntityIndices;
	std::vector<int> sellmeierIndices;

	std::vector<Entity*> prismEntities;
	sf::Clock entityEdgeClock;
	
	int sellmeierIndex = 0;
	for (auto& s : m_sellmeierManager.getProfiles())
	{
		for (int i = 0; i < 3; i++)
		{
			m_buffers.sellmeierCoefficientsA[sellmeierIndex] = (s->getCoefficientsA()[i]);
			m_buffers.sellmeierCoefficientsB[sellmeierIndex] = (s->getCoefficientsB()[i]);
			sellmeierIndex++;
		}
	}
	m_buffers.sellmeierCoefficientsA.write_to_device(0,sellmeierIndex);
	m_buffers.sellmeierCoefficientsB.write_to_device(0, sellmeierIndex);

	for (auto& e : m_entities.getEntities())
	{
		if (e->cPrism != nullptr || e->m_isMirror)
		{
			prismEntities.push_back(e.get());

			if (e->cPrism != nullptr)
			{
				std::string sellmeierTag = e->cPrism->getTag();
				int index = 0;
				for (auto& s : m_sellmeierManager.getProfiles())
				{
					if (s->getTag() == sellmeierTag)
					{
						sellmeierIndices.push_back(index); // One per prism entity
						break;
					}
					index++;
				}
			}
			else
			{
				sellmeierIndices.push_back(-1);
			}

			std::vector<sf::Vector2f> worldPolygon;

			if (e->cShape != nullptr)
			{
				auto& shape = e->cShape->circle;
				sf::Transform globalTransform = shape.getTransform();

				for (size_t i = 0; i < shape.getPointCount(); ++i)
				{
					sf::Vector2f localPoint = shape.getPoint(i);
					sf::Vector2f worldPoint = globalTransform.transformPoint(localPoint);
					worldPolygon.push_back(worldPoint);
				}
			}
			else if (e->cCustomShape)
			{
				CustomPolygonPrism* customPrismShape = dynamic_cast<CustomPolygonPrism*>(e->cCustomShape->customShape.get());
				if (customPrismShape && !customPrismShape->getShapeComplete())
					continue;

				auto& customShape = e->cCustomShape->customShape;
				sf::Transform globalTransform = customShape->getTransform();

				for (size_t i = 0; i < customShape->getPointCount(); ++i)
				{
					sf::Vector2f localPoint = customShape->getPoint(i);
					sf::Vector2f worldPoint = globalTransform.transformPoint(localPoint);
					worldPolygon.push_back(worldPoint);
				}

				// Ensure clockwise winding
				auto signedArea = [](const std::vector<sf::Vector2f>& pts) -> float {
					float area = 0.f;
					size_t n = pts.size();
					for (size_t i = 0; i < n; ++i)
					{
						const auto& p1 = pts[i];
						const auto& p2 = pts[(i + 1) % n];
						area += (p2.x - p1.x) * (p2.y + p1.y);
					}
					return area;
					};
				if (signedArea(worldPolygon) > 0)
				{
					std::reverse(worldPolygon.begin(), worldPolygon.end());
				}
			}

			// Generate edges
			int edgeCount = worldPolygon.size();
			bool isCircularArc = (e->getTag() == "CircularArc");
			int limit = isCircularArc ? edgeCount - 1 : edgeCount;

			for (int i = 0; i < limit; ++i)
			{
				sf::Vector2f a = worldPolygon[i];
				sf::Vector2f b = worldPolygon[(i + 1) % worldPolygon.size()];

				edgeA_X.push_back(a.x);
				edgeA_Y.push_back(a.y);
				edgeB_X.push_back(b.x);
				edgeB_Y.push_back(b.y);
				edgeEntityIndices.push_back(prismEntities.size() - 1); // index into prismEntities
			}
		}
	}


	for (int i = 0; i < edgeA_X.size(); ++i)
	{
		m_buffers.edgeA_X[i] = edgeA_X[i];
		m_buffers.edgeA_Y[i] = edgeA_Y[i];
		m_buffers.edgeB_X[i] = edgeB_X[i];
		m_buffers.edgeB_Y[i] = edgeB_Y[i];
		m_buffers.edgeEntityIndices[i] = edgeEntityIndices[i];
	}

	for(int i=0 ; i< prismEntities.size(); ++i)
	{
		m_buffers.entitySellmeierProfiles[i] = sellmeierIndices[i];
	}
	m_buffers.entitySellmeierProfiles.write_to_device(0, prismEntities.size());
	const int edgeCount = edgeA_X.size();

	m_buffers.edgeA_X.write_to_device(0,edgeCount);
	m_buffers.edgeA_Y.write_to_device(0,edgeCount);
	m_buffers.edgeB_X.write_to_device(0,edgeCount);
	m_buffers.edgeB_Y.write_to_device(0,edgeCount);
	m_buffers.edgeEntityIndices.write_to_device(0,edgeCount);
	m_buffers.screenBounds.write_to_device(); // Entire thing can be written



	int maxCount = 15; // max amount of operations in 1 frame e.g. reflection/ refraction
	int count = 0;
	
	int N = m_buffers.currRayCount; // Current number of rays to process

//	std::cout << "PRE-PROCESSING: " << preProcessingClock.getElapsedTime().asMilliseconds() << " ms\n";

	while (count < maxCount )
	{
	
		if (N == 0) break;

		std::vector<float> prevRayX(N);
		std::vector<float> prevRayY(N);

		// Store current ray origins before kernel modifies them
		for (int i = 0; i < N; ++i)
		{
			// Since we add the ray direction to the buffer ray origin to avoid repeated collisions offset by radDir to get back to the original collision point 
			// Otherwise we would see gaps when zooming in 
			prevRayX[i] = m_buffers.rayOriginsX[i] - m_buffers.rayDirsX[i];
			prevRayY[i] = m_buffers.rayOriginsY[i] - m_buffers.rayDirsY[i];
		}
		sf::Clock GPUClock;
		
		Kernel ray_edge_intersection(
			m_device, N, "ray_fresnel_mode",
			m_buffers.rayOriginsX, m_buffers.rayOriginsY,
			m_buffers.rayDirsX, m_buffers.rayDirsY,
			m_buffers.reflectedRayDirsX, m_buffers.reflectedRayDirsY,
			m_buffers.edgeA_X, m_buffers.edgeA_Y,
			m_buffers.edgeB_X, m_buffers.edgeB_Y,
			m_buffers.edgeEntityIndices, edgeCount,
			m_buffers.collisionPointsX, m_buffers.collisionPointsY, m_buffers.entityIndexHit,
			m_buffers.refracIndices, m_buffers.whiteLight,
			m_buffers.finishedProcessing, m_buffers.screenBounds, m_buffers.transmissionCoefficients, m_buffers.sellmeierCoefficientsA, m_buffers.sellmeierCoefficientsB,
			m_buffers.entitySellmeierProfiles, m_buffers.wavelengths
		);

		// WRITE ALL DATA
		m_buffers.rayOriginsX.write_to_device(0,N);
		m_buffers.rayOriginsY.write_to_device(0,N);
		m_buffers.rayDirsX.write_to_device(0,N);
		m_buffers.rayDirsY.write_to_device(0,N);
		m_buffers.refracIndices.write_to_device(0,N);
		m_buffers.wavelengths.write_to_device(0, N);
		m_buffers.whiteLight.write_to_device(0, N);

		// Run the kernel to process ray-entity intersections
		ray_edge_intersection.run();

		// Read back results
		// Might remove these false don't think they make a difference
		m_buffers.rayDirsX.read_from_device(0,N,false);
		m_buffers.rayDirsY.read_from_device(0,N,false);
		m_buffers.collisionPointsX.read_from_device(0,N,false);
		m_buffers.collisionPointsY.read_from_device(0,N,false);
		// We don't use entityIndexHit yet but we might in the future e.g. if we want to only update certain rays if the movement of an entity doesn't effect them
		m_buffers.entityIndexHit.read_from_device(0,N,false);
		m_buffers.transmissionCoefficients.read_from_device(0, N, false);
		m_buffers.reflectedRayDirsX.read_from_device(0, N, false);
		m_buffers.reflectedRayDirsY.read_from_device(0, N, false);
		m_buffers.finishedProcessing.read_from_device(0,N);
		

		// std::cout << "GPU processing time: " << GPUClock.getElapsedTime().asMilliseconds() << " ms\n";
		auto computeBrightness = [](const sf::Color& c) -> float {
			return 0.2126f * c.r + 0.7152f * c.g + 0.0722f * c.b;
			};


		auto scaleColor = [](const sf::Color& c, float factor) -> sf::Color {
			return sf::Color(
				static_cast<int>(std::clamp(c.r * factor, 0.f, 255.f)),
				static_cast<int>(std::clamp(c.g * factor, 0.f, 255.f)),
				static_cast<int>(std::clamp(c.b * factor, 0.f, 255.f)),
				static_cast<int>(std::clamp(c.a * factor, 15.f, 255.f))
			);
			};
		
		sf::Clock postProcessingClock;
		for (int i = 0; i < N; i++)
		{
			// Calculate new dimmed colour
			float transmission = m_buffers.transmissionCoefficients[i];
			sf::Color rayColour = m_buffers.rayColours[i];
				// Add a line from the previous point to the new point
			allRays.append(sf::Vertex{ sf::Vector2f(prevRayX[i], prevRayY[i]), rayColour});
			allRays.append(sf::Vertex{ sf::Vector2f(m_buffers.collisionPointsX[i], m_buffers.collisionPointsY[i]), rayColour });

			if (!m_buffers.finishedProcessing[i])
			{   // If not finished processing, update the data about it
				int entityIndex = m_buffers.entityIndexHit[i];


				sf::Color transmittedColour = scaleColor(rayColour, transmission);
				sf::Color reflectedColour = scaleColor(rayColour, 1.0f - transmission);

				if (entityIndex < 0 || entityIndex >= prismEntities.size())
				{
					std::cerr << "Invalid entityIndex: " << entityIndex << "\n";
					continue;
				}
				if (prismEntities[entityIndex] == nullptr)
				{
					std::cerr << "Null entity hit? At index: " << entityIndex << '\n';
					continue;
				}

				float reflectedRayDirX = m_buffers.reflectedRayDirsX[i];
				float reflectedRayDirY = m_buffers.reflectedRayDirsY[i];



				float brightnessThreshold = 0.5f; // You can tweak this


				if (prismEntities[entityIndex]->m_isMirror || transmission == 0.0f) 
				{
					// If we have TIR / reflected ray, scale the brightness down a bit to prevent infinite reflections
					// If we have a reflected ray, use that instead
					float reflectedRayX = m_buffers.collisionPointsX[i] + reflectedRayDirX;
					float reflectedRayY = m_buffers.collisionPointsY[i] + reflectedRayDirY;
					// 0.85 is nice
					float loss = 1.0f;
					sf::Color newColour = sf::Color(rayColour.r *loss, rayColour.g *loss, rayColour.b*loss, rayColour.a*loss);
					if (computeBrightness(newColour) < brightnessThreshold)
					{
						continue;
					}
					RayData reflectedRayData;
					reflectedRayData.originX = reflectedRayX;
					reflectedRayData.originY = reflectedRayY;
					reflectedRayData.dirX = reflectedRayDirX;
					reflectedRayData.dirY = reflectedRayDirY;
					reflectedRayData.refracIndex = 1.0f;
					reflectedRayData.whiteLight = m_buffers.whiteLight[i];
					reflectedRayData.color = newColour;
					reflectedRayData.wavelength = m_buffers.wavelengths[i];
					m_buffers.createRay(reflectedRayData);
					continue; 
				}
				std::string entityTag = prismEntities[entityIndex]->cPrism->getTag();
				Sellmeier* wavelengthToIndex = m_sellmeierManager.getSellmeier(entityTag);
				float refracIndex = m_buffers.refracIndices[i];

				// Create reflected ray if we need to 

				if (reflectedRayDirX != 0.0f && reflectedRayDirY != 0.0f)
				{
					if (computeBrightness(reflectedColour) > brightnessThreshold)
					{

						// If we have a reflected ray, use that instead
						float reflectedRayX = m_buffers.collisionPointsX[i] + reflectedRayDirX;
						float reflectedRayY = m_buffers.collisionPointsY[i] + reflectedRayDirY;
						RayData reflectedRayData;
						reflectedRayData.originX = reflectedRayX;
						reflectedRayData.originY = reflectedRayY;
						reflectedRayData.dirX = reflectedRayDirX;
						reflectedRayData.dirY = reflectedRayDirY;
						reflectedRayData.refracIndex = refracIndex;
						reflectedRayData.whiteLight = m_buffers.whiteLight[i];
						reflectedRayData.color = reflectedColour;
						reflectedRayData.wavelength = m_buffers.wavelengths[i];
						m_buffers.createRay(reflectedRayData);
					}
					else
					{
					}
				}
				//Print reflected and transmitted ray data

				// Now handle transmitted ray 
				// Add a slight offset to new ray Dir to avoid repeated collisions

				if (computeBrightness(transmittedColour) < brightnessThreshold) continue;


				float newRayX = m_buffers.collisionPointsX[i] + m_buffers.rayDirsX[i];
				float newRayY = m_buffers.collisionPointsY[i] + m_buffers.rayDirsY[i];
				
				RayData transmittedRayData;
				transmittedRayData.originX = newRayX;
				transmittedRayData.originY = newRayY;
				transmittedRayData.dirX = m_buffers.rayDirsX[i];
				transmittedRayData.dirY = m_buffers.rayDirsY[i];
				transmittedRayData.refracIndex = refracIndex;
				transmittedRayData.whiteLight = m_buffers.whiteLight[i];
				transmittedRayData.color = transmittedColour;
				transmittedRayData.wavelength = m_buffers.wavelengths[i];

				m_buffers.createRay(transmittedRayData);
			}
			else
			{
				int idx = m_buffers.entityIndexHit[i];
				if (m_buffers.whiteLight[i] &&
					idx >= 0 &&
					idx < static_cast<int>(prismEntities.size()) &&
					(prismEntities[idx]->getTag() == "Prism" || prismEntities[idx]->getTag() == "Lens" || prismEntities[idx]->getTag() == "DemoPrism" || prismEntities[idx]->getTag() == "CustomPolyPrism"))
				{
					int entityIndex = m_buffers.entityIndexHit[i];
					// Create new rays from the hit point
					float dirX = m_buffers.rayDirsX[i];
					float dirY = m_buffers.rayDirsY[i];
					float originX = m_buffers.collisionPointsX[i] - dirX;
					float originY = m_buffers.collisionPointsY[i] - dirY;
					for (const auto& [wavelength, colour] : wavelengthColors) 
					{

						RayData rayData;
						rayData.originX = originX;
						rayData.originY = originY;
						rayData.dirX = dirX;
						rayData.dirY = dirY;
						rayData.whiteLight = false;
						rayData.color = colour;
						rayData.wavelength = wavelength; 
						m_buffers.createRay(rayData);
					}
				}
			}
		}

		// Final count of rays for next iteration
		m_buffers.currRayCount = 0;
		m_buffers.commitNewRays();
		N = m_buffers.currRayCount;
	//	std::cout << "POST-PROCESSING: " << postProcessingClock.getElapsedTime().asMilliseconds() << " ms\n";
	//	std::cout << "Processed " << N << " rays in this iteration.\n";
	

		//std::cout << "Post processing time: " << postProcessingClock.getElapsedTime().asMilliseconds() << " ms\n";
		count++;
	}
	//std::cout << "Collision processing time: " << collisionClock.getElapsedTime().asMilliseconds() << " ms\n";
}


// Clamp a value between min and max	
double Simulation::clamp(double value, double min, double max) {
	return std::max(min, std::min(max, value));
}

// Wavelength in nm to RGB - Found this online and just copied it

sf::Color Simulation::wavelengthToRGB(double wavelength) {
	static const int LEN_MIN = 380;
	static const int LEN_MAX = 780;
	static const int LEN_STEP = 5;

	static const std::vector<double> X = {
		0.000160, 0.000662, 0.002362, 0.007242, 0.019110, 0.043400, 0.084736, 0.140638, 0.204492, 0.264737,
		0.314679, 0.357719, 0.383734, 0.386726, 0.370702, 0.342957, 0.302273, 0.254085, 0.195618, 0.132349,
		0.080507, 0.041072, 0.016172, 0.005132, 0.003816, 0.015444, 0.037465, 0.071358, 0.117749, 0.172953,
		0.236491, 0.304213, 0.376772, 0.451584, 0.529826, 0.616053, 0.705224, 0.793832, 0.878655, 0.951162,
		1.014160, 1.074300, 1.118520, 1.134300, 1.123990, 1.089100, 1.030480, 0.950740, 0.856297, 0.754930,
		0.647467, 0.535110, 0.431567, 0.343690, 0.268329, 0.204300, 0.152568, 0.112210, 0.081261, 0.057930,
		0.040851, 0.028623, 0.019941, 0.013842, 0.009577, 0.006605, 0.004553, 0.003145, 0.002175, 0.001506,
		0.001045, 0.000727, 0.000508, 0.000356, 0.000251, 0.000178, 0.000126, 0.000090, 0.000065, 0.000046,
		0.000033
	};

	static const std::vector<double> Y = {
		0.000017, 0.000072, 0.000253, 0.000769, 0.002004, 0.004509, 0.008756, 0.014456, 0.021391, 0.029497,
		0.038676, 0.049602, 0.062077, 0.074704, 0.089456, 0.106256, 0.128201, 0.152761, 0.185190, 0.219940,
		0.253589, 0.297665, 0.339133, 0.395379, 0.460777, 0.531360, 0.606741, 0.685660, 0.761757, 0.823330,
		0.875211, 0.923810, 0.961988, 0.982200, 0.991761, 0.999110, 0.997340, 0.982380, 0.955552, 0.915175,
		0.868934, 0.825623, 0.777405, 0.720353, 0.658341, 0.593878, 0.527963, 0.461834, 0.398057, 0.339554,
		0.283493, 0.228254, 0.179828, 0.140211, 0.107633, 0.081187, 0.060281, 0.044096, 0.031800, 0.022602,
		0.015905, 0.011130, 0.007749, 0.005375, 0.003718, 0.002565, 0.001768, 0.001222, 0.000846, 0.000586,
		0.000407, 0.000284, 0.000199, 0.000140, 0.000098, 0.000070, 0.000050, 0.000036, 0.000025, 0.000018,
		0.000013
	};

	static const std::vector<double> Z = {
		0.000705, 0.002928, 0.010482, 0.032344, 0.086011, 0.197120, 0.389366, 0.656760, 0.972542, 1.282500,
		1.553480, 1.798500, 1.967280, 2.027300, 1.994800, 1.900700, 1.745370, 1.554900, 1.317560, 1.030200,
		0.772125, 0.570060, 0.415254, 0.302356, 0.218502, 0.159249, 0.112044, 0.082248, 0.060709, 0.043050,
		0.030451, 0.020584, 0.013676, 0.007918, 0.003988, 0.001091, 0.000000, 0.000000, 0.000000, 0.000000,
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
		0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000, 0.000000,
		0.000000
	};

	static const double M[9] = {
		 3.2404542, -1.5371385, -0.4985314,
		-0.9692660,  1.8760108,  0.0415560,
		 0.0556434, -0.2040259,  1.0572252
	};

	auto interpolate = [](const std::vector<double>& arr, int index, double offset) -> double {
		if (offset == 0)
			return arr[index];
		double x0 = index * LEN_STEP;
		double x1 = x0 + LEN_STEP;
		double y0 = arr[index];
		double y1 = arr[index + 1];
		return y0 + offset * (y1 - y0) / (x1 - x0);
		};

	auto gammaCorrect = [](double c) -> double {
		if (c <= 0.0031308)
			return 12.92 * c;
		double a = 0.055;
		return (1 + a) * std::pow(c, 1.0 / 2.4) - a;
		};

	auto clip = [](double c) -> double {
		if (c < 0) return 0;
		if (c > 1) return 1;
		return c;
		};

	if (wavelength < LEN_MIN || wavelength > LEN_MAX)
		return sf::Color(0, 0, 0);

	wavelength -= LEN_MIN;
	int index = static_cast<int>(std::floor(wavelength / LEN_STEP));
	double offset = wavelength - LEN_STEP * index;

	double x = interpolate(X, index, offset);
	double y = interpolate(Y, index, offset);
	double z = interpolate(Z, index, offset);

	double r = M[0] * x + M[1] * y + M[2] * z;
	double g = M[3] * x + M[4] * y + M[5] * z;
	double b = M[6] * x + M[7] * y + M[8] * z;

	r = clip(gammaCorrect(r));
	g = clip(gammaCorrect(g));
	b = clip(gammaCorrect(b));

	return sf::Color(
		(255 * r),
		(255 * g),
		(255 * b),
		255
	);
}

void Simulation::toggleFullscreen() {
	m_isFullscreen = !m_isFullscreen;
	m_window.close();  // Close current window

	if (m_isFullscreen) {
		m_window.create(sf::VideoMode::getDesktopMode(), "SIMULATION", sf::State::Fullscreen);
	}
	else {
		m_window.create(m_windowedMode, "SIMULATION", sf::Style::Resize | sf::Style::Close);
	}

	sf::Vector2u size = m_window.getSize();
	m_view.setSize({ static_cast<float>(size.x), static_cast<float>(size.y) });
	m_view.setCenter(m_view.getSize() / 2.f);
	m_window.setView(m_view);

	m_window.setFramerateLimit(60);
	ImGui::SFML::Init(m_window);  // Re-init ImGui with new window
}

void Simulation::sRemoveRay(RayData* ray)
{
	if (!ray) return;
	// Find the index of the ray in the m_lightSources vector
	auto it = std::find_if(m_lightSources.begin(), m_lightSources.end(),
		[ray](const std::unique_ptr<RayData>& r) {
			return r.get() == ray;
		});
	if (it != m_lightSources.end()) {
		// std::cout << " REMOVING RAY \n";
		m_lightSources.erase(it);  // Remove the ray from the vector
	}
}

void Simulation::sUpdateWavelengthCreation()
{
	if (m_wavelengthCreationStep == 0.0f)
	{
		return;
	}
	if (m_wavelengthIncreasing)
	{
		m_wavelengthCreation += m_wavelengthCreationStep;
		if (m_wavelengthCreation >= m_endWavelength)
		{
			m_wavelengthCreation = m_endWavelength;
			m_wavelengthIncreasing = false;
		}
	}
	else
	{
		m_wavelengthCreation -= m_wavelengthCreationStep;
		if (m_wavelengthCreation <= m_startWavelength)
		{
			m_wavelengthCreation = m_startWavelength;
			m_wavelengthIncreasing = true;
		}
	}
	m_stateChange = true;
	sf::Color color = wavelengthToRGB(m_wavelengthCreation);
	for (auto& e : m_entities.getEntities())
	{
		if (e->getTag() == "Marker" && e->cMarker &&
			e->cMarker->getRole() == MarkerRole::ColouredPointLightMarker)
		{
			for (RayData* ray : e->cMarker->getRays())
			{
				color.a = ray->color.a; // maintain alpha
				ray->color = color;
			}
		}
	}
}