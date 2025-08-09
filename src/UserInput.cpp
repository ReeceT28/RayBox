#include "Simulation.h"
#include "Entity.h"
#include "CustomPolygonPrism.h"
#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "ShapeUtils.h"

void Simulation::sUserInput()
{
	sf::Vector2f mouseWorldPos = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window));
	// Check for mouse events
	while (const std::optional event = m_window.pollEvent())
	{
		ImGui::SFML::ProcessEvent(m_window, *event);
		// If true the click was on an ImGui window so ignore the click
		if (ImGui::GetIO().WantCaptureMouse)
			continue;

		if (event->is<sf::Event::Closed>())
		{
			m_window.close();
			m_running = false;
		}
		if (const auto* resized = event->getIf<sf::Event::Resized>())
		{
			m_view.setSize(sf::Vector2f(static_cast<float>(resized->size.x), static_cast<float>(resized->size.y)));
			m_view.setCenter(m_view.getSize() / 2.f);
			m_window.setView(m_view);
		}

		if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
		{
			if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
				m_window.close();
			else if (keyPressed->scancode == sf::Keyboard::Scancode::F11)
			{
				toggleFullscreen();
			}
		}



		handleMouseScrollWheel(event);
		handleMousePressLeft(event, mouseWorldPos);
		handleMousePressRight(event, mouseWorldPos);
		handleMouseButtonReleased(event);
		handleMouseMoved(event);
	}

	handleDragging(mouseWorldPos);

	if (m_isRotating && m_selectedEntity && m_rotator)
	{
		m_stateChange = true; 
		auto& shape = m_selectedEntity->cShape->circle;
		sRotateToMouse(mouseWorldPos, shape);
	}

	// Live preview for next polygon point
	if (m_placingCustomPrism && m_previousMarker)
	{
		if (m_previousMarker->cMarker && m_previousMarker->cMarker->getRole() == MarkerRole::polygonPrismMarker)
		{
			Entity* customPrism = m_previousMarker->cMarker->getTargetEntity();
			CustomPolygonPrism* customPrismShape = dynamic_cast<CustomPolygonPrism*>(customPrism->cCustomShape->customShape.get());
			if (customPrismShape)
			{
				customPrismShape->setPreviewPoint(mouseWorldPos); 
			}
		}
	}

	if (m_placingCircularArc && m_circularArcInProgress)
	{
		auto arcShape = dynamic_cast<CircularArcShape*>(m_circularArcInProgress->cCustomShape->customShape.get());
		if (arcShape->getMarker(2))
		{
			m_stateChange = true;
			arcShape->setMarkerPos(2,mouseWorldPos);
		}
	}

	

}

void Simulation::handleMouseScrollWheel(const std::optional<sf::Event> event)
{
	if (const auto* mouseWheelScrolled = event->getIf<sf::Event::MouseWheelScrolled>())
	{
		float zoomFactor = 1.1f; // Adjust this value for zoom sensitivity

		switch (mouseWheelScrolled->wheel)
		{
		case sf::Mouse::Wheel::Vertical:
		{
			sf::Vector2i mousePos = mouseWheelScrolled->position;
			sf::Vector2f mousePosCoords = m_window.mapPixelToCoords(mousePos, m_view);

			if (mouseWheelScrolled->delta > 0.0f)
			{
				m_view.zoom(1.0f / zoomFactor); // Zoom in
			}
			else
			{
				m_view.zoom(zoomFactor); // Zoom out
			}

			sf::Vector2f afterZoom = m_window.mapPixelToCoords(mousePos, m_view);
			sf::Vector2f offset = mousePosCoords - afterZoom;
			m_view.move(offset);
			m_window.setView(m_view);
			break;
		}
		case sf::Mouse::Wheel::Horizontal:
			break;
		}

	}
}

void Simulation::handleMousePressLeft(const std::optional<sf::Event> event, sf::Vector2f mouseWorldPos)
{
	if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>())
	{
		if (mouse->button == sf::Mouse::Button::Left)
		{
			m_benchmarkClock.restart(); // Reset the benchmark clock
			bool hitEntity = false;
			for (auto& e : m_entities.getEntities())
			{
				if (e->cShape)
				{
					auto& shape = e->cShape->circle;
					float distance = magnitude(shape.getPosition() - mouseWorldPos);
					float radius = shape.getRadius();
					if (e->getTag() == "Marker")
					{
						radius = radius * 3; // Make the hitbox a bit bigger as markers are small
					}
					if (distance <= radius)
					{
						hitEntity = true;

						if (e->getTag() == "Prism")
						{
							m_selectedEntity = e.get();
							m_isDragging = true;
							m_dragOffset = e->cShape->circle.getPosition() - mouseWorldPos;
							// Destroy old rotator
							if (m_rotator != nullptr)
							{
								m_rotator->m_dead = true;
								m_rotator = nullptr;
							}

							// Create rotator about the currently selected entity
							m_rotator = &m_entities.addEntity("Rotator");

							// Create the small triangle arrow shape
							m_rotator->cShape = std::make_unique<CShape>(
								shape.getRadius() / 10.0f,   // smaller radius
								3,                          // triangle
								sf::Color(0, 0, 0, 0),     // transparent fill
								sf::Color(255, 255, 255, 255), // white border
								1.0f
							);

							sf::Vector2f point0 = shape.getPoint(0);
							sf::Transform shapeTransform = shape.getTransform();
							point0 = shapeTransform.transformPoint(point0);

							m_rotator->cShape->circle.setRotation(shape.getRotation());
							m_rotator->cShape->circle.setPosition(point0 + normalize(point0 - shape.getPosition()) * 25);


						}
						else if (e->getTag() == "Rotator")
						{
							if (m_selectedEntity && m_rotator)
							{
								// Rotate the rotator to face the mouse position
								sf::Vector2f shapeToMouse = mouseWorldPos - m_rotator->cShape->circle.getPosition();
								sf::Angle angle = sf::radians(std::atan2(shapeToMouse.y, shapeToMouse.x));
								m_selectedEntity->cShape->circle.setRotation(angle);
								m_isRotating = true;

							}
						}
						else if (e->getTag() == "Marker")
						{
							// We don't want a rotator for markers
							if (m_rotator != nullptr)
							{
								m_rotator->m_dead = true;
								m_rotator = nullptr;
							}
							m_isDragging = true;
							m_selectedEntity = e.get();
							m_dragOffset = e->cShape->circle.getPosition() - mouseWorldPos;
						}

						break; // Only handle one entity per click
					}
				}
				else // Custom shape
				{
					// if it is a custom polygon prism AND it isn't complete dont allow dragging
					if (dynamic_cast<CustomPolygonPrism*>(e->cCustomShape->customShape.get()) && !dynamic_cast<CustomPolygonPrism*>(e->cCustomShape->customShape.get())->getShapeComplete())
					{
						continue;
					}
					if (shapeContainsPoint(*e->cCustomShape->customShape, mouseWorldPos))
					{
						m_dragOffset = e->cCustomShape->customShape->getPosition() - mouseWorldPos;
						hitEntity = true;
						if (m_rotator != nullptr)
						{
							// Destroy old rotator
							m_rotator->m_dead = true;
							m_rotator = nullptr;
						}
						m_isDragging = true;
						m_selectedEntity = e.get();
					}
				}
			}
			if (!hitEntity)
			{
				m_selectedEntity = nullptr;
				if (m_rotator != nullptr)
				{
					m_rotator->m_dead = true;
					m_rotator = nullptr;

				}
			}

			if (m_placingMarker)
			{
				m_stateChange = true;
				if (m_placingSingleRay)
				{
					if (m_previousMarker == nullptr)
					{
						// Create first marker (origin)
						Entity* marker1 = sCreateMarker(mouseWorldPos, 2.5f, 4,
							sf::Color(255, 0, 0, 122),
							sf::Color(255, 255, 255, 255),
							45.0f);
						marker1->cMarker = std::make_unique<CMarker>(RayTag(), MarkerRole::singleRayMarker1, nullptr);

						m_previousMarker = marker1;
					}
					else
					{
						sf::Vector2f origin = m_previousMarker->cShape->circle.getPosition();
						sf::Vector2f direction = normalize(mouseWorldPos - origin);

						// Create the ray
						RayData* ray = sCreateWhiteRay(origin, direction);

						// Update first marker role and ray link
						m_previousMarker->cMarker = std::make_unique<CMarker>(RayTag(), MarkerRole::singleRayMarker1, ray);


						// Create second marker (direction)
						Entity* marker2 = sCreateMarker(mouseWorldPos, 2.5f, 4,
							sf::Color(255, 0, 0, 122),
							sf::Color(255, 255, 255, 255),
							45.0f);
						marker2->cMarker = std::make_unique<CMarker>(RayTag(), MarkerRole::singleRayMarker2, ray);

						// Reset for next pair
						m_previousMarker = nullptr;
					}
				}
				else if (m_placingPointLight)
				{
					Entity* pointLightMarker = sCreateMarker(mouseWorldPos, 2.5f, 4,
						sf::Color(255, 0, 0, 122),
						sf::Color(255, 255, 255, 255),
						45.0f);
					pointLightMarker->cMarker = std::make_unique<CMarker>(RayTag(), MarkerRole::PointLightMarker, nullptr);
					for (int i = 0; i < m_pointLightResolution; i++)
					{
						float angle = static_cast<float>(i) * (2 * pi / m_pointLightResolution);
						sf::Vector2f dir = { std::cos(angle), std::sin(angle) };
						sf::Vector2f rayOrigin = mouseWorldPos;
						RayData* ray = sCreateWhiteRay(rayOrigin, dir);
						pointLightMarker->cMarker->addTargetRay(ray);
					}
					m_placingPointLight = false;
					m_stateChange = true; 
				}
				else if (m_placingColouredPointLight)
				{
					Entity* pointLightMarker = sCreateMarker(mouseWorldPos, 2.5f, 4,
						sf::Color(255, 0, 0, 122),
						sf::Color(255, 255, 255, 255),
						45.0f);
					pointLightMarker->cMarker = std::make_unique<CMarker>(RayTag(), MarkerRole::ColouredPointLightMarker, nullptr);
					for (int i = 0; i < m_pointLightResolution; i++)
					{
						float angle = static_cast<float>(i) * (2 * pi / m_pointLightResolution);
						sf::Vector2f dir = { std::cos(angle), std::sin(angle) };
						sf::Vector2f rayOrigin = mouseWorldPos;
						RayData* ray = sCreateColouredRay(rayOrigin, dir, m_wavelengthCreation);
						pointLightMarker->cMarker->addTargetRay(ray);
					}
					m_placingColouredPointLight = false;
					m_stateChange = true;

				}
				else if (m_placingCustomPrism)
				{
					if (m_previousMarker != nullptr)
					{   // Subsequent points
						if (m_previousMarker->cMarker && m_previousMarker->cMarker->getRole() == MarkerRole::polygonPrismMarker)
						{
							Entity* customPrism = m_previousMarker->cMarker->getTargetEntity();
							CustomPolygonPrism* customPrismShape = dynamic_cast<CustomPolygonPrism*>(customPrism->cCustomShape->customShape.get());
							if (magnitude(customPrismShape->getPoint(0) - mouseWorldPos) < 10.0f )
							{
								customPrismShape->setShapeComplete();
								m_previousMarker = nullptr;
							}
							else
							{
								// Create and add new marker
								Entity* marker = sCreateMarker(mouseWorldPos, 2.5f , 4,
									sf::Color(255, 0, 0, 122),
									sf::Color(255, 255, 255, 255),
									45.0f);

								marker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::polygonPrismMarker, customPrism);


								// Add to shape
								customPrismShape->addMarker(marker);
								customPrismShape->addPoint(mouseWorldPos);         // Don't triangulate yet
								customPrismShape->clearPreview();                  // Remove preview after placing

								m_previousMarker = marker;
							}
						}
					}
					else
					{   // First point
						Entity* marker = sCreateCustomPolygonPrism(mouseWorldPos, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass");
						marker->cShape->circle.setRadius(2.5f);
						marker->cShape->circle.setOutlineThickness(1.0f );
						marker->cShape->circle.setOrigin({ 2.5f, 2.5f });

						m_previousMarker = marker;
					}
				}
				else if (m_placingCircularArc)
				{
					if (m_previousMarker)
					{
						auto arcShape = dynamic_cast<CircularArcShape*>(m_circularArcInProgress->cCustomShape->customShape.get());
						if (!arcShape)
						{
							std::cerr << "Error: cCustomShape is not a CircularArcShape\n";
							return;
						}

						if (arcShape->isComplete())
						{
							// Third click
							m_previousMarker = nullptr;
							m_circularArcInProgress = nullptr;
						}
						else
						{
							// Second click
							Entity* marker = sCreateMarker(mouseWorldPos, 2.5f, 4,
								sf::Color(255, 0, 0, 122),
								sf::Color(255, 255, 255, 255),
								45.0f);
							marker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::CircularArcMarker, m_circularArcInProgress);
							arcShape->addMarker(marker);

							m_previousMarker = marker;
							Entity* markerPreview = sCreateMarker(mouseWorldPos, 2.5f, 4,
								sf::Color(255, 0, 0, 122),
								sf::Color(255, 255, 255, 255),
								45.0f);
							markerPreview->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::CircularArcMarker, m_circularArcInProgress);
							arcShape->addMarker(markerPreview);
						}
					}
					else
					{
						// First point — create shape and marker
						Entity* marker = sCreateMarker(mouseWorldPos, 2.5f, 4,
							sf::Color(255, 0, 0, 122),
							sf::Color(255, 255, 255, 255),
							45.0f);

						auto arcShape = std::make_unique<CircularArcShape>();
						arcShape->addMarker(marker);
						arcShape->setOutlineColor(sf::Color::White);

						Entity* arc = &m_entities.addEntity("CircularArc");
						marker->cMarker = std::make_unique<CMarker>(EntityTag(), MarkerRole::CircularArcMarker, arc);
						arc->cCustomShape = std::make_unique<CCustomShape>(std::move(arcShape));
						arc->m_isMirror = true;
						m_previousMarker = marker;
						m_circularArcInProgress = arc;
					}
				}


			}
		}
	}
}

void Simulation::handleMousePressRight(const std::optional<sf::Event> event, sf::Vector2f mouseWorldPos)
{
	if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>())
	{
		if(mousePressed->button == sf::Mouse::Button::Right)
		{
			m_isPanning = true;
			m_lastMousePos = mousePressed->position;
		}
	}
}

void Simulation::handleMouseButtonReleased(const std::optional <sf::Event> event)
{

	if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonReleased>())
	{
		switch (mousePressed->button)
		{
		case sf::Mouse::Button::Left:
			m_isDragging = false;
			m_isRotating = false;
			break;
		case sf::Mouse::Button::Right:
			m_isPanning = false;
			break;
		}
	}
}

void Simulation::handleMouseMoved(const std::optional <sf::Event> event)
{
	if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>())
	{
		if (m_isPanning)
		{
			sf::Vector2i currentMousePos = sf::Mouse::getPosition(m_window);
			sf::Vector2i pixelDelta = currentMousePos - m_lastMousePos;

			sf::Vector2f viewSize = m_view.getSize();
			sf::Vector2u windowSize = m_window.getSize();

			// Avoid divide-by-zero
			if (windowSize.x == 0 || windowSize.y == 0)
				return;

			// World units per pixel (scaled by zoom)
			sf::Vector2f scale(
				viewSize.x / static_cast<float>(windowSize.x),
				viewSize.y / static_cast<float>(windowSize.y)
			);


			// Final delta scaled correctly with zoom
			sf::Vector2f worldDelta(
				static_cast<float>(pixelDelta.x) * scale.x,
				static_cast<float>(pixelDelta.y) * scale.y
			);

			m_view.move(-worldDelta);
			m_window.setView(m_view);

			m_lastMousePos = currentMousePos;
		}
	}
}

// I probably should fix this to make more use of flags like m_isDraggable but they have wildly different behaviour apart from just updating position anyways so wouldn't save that much
void Simulation::handleDragging(sf::Vector2f mouseWorldPos)
{
	if (m_isDragging)
	{
		m_stateChange = true;  // Flag indicating state has changed
		if (m_selectedEntity->getTag() == "Prism")
		{
			sf::Vector2f deltaPosition = (mouseWorldPos + m_dragOffset) - (m_selectedEntity->cShape->circle.getPosition());
			m_selectedEntity->cShape->circle.setPosition(mouseWorldPos + m_dragOffset);
			m_rotator->cShape->circle.setPosition(m_rotator->cShape->circle.getPosition() + deltaPosition);
		}
		else if (m_selectedEntity->getTag() == "DemoPrism")
		{
			// Just update position
			PrismDemoShape* prismShape = dynamic_cast<PrismDemoShape*>(m_selectedEntity->cCustomShape->customShape.get());
			prismShape->setPosition(mouseWorldPos + m_dragOffset);
		}
		else if (m_selectedEntity->getTag() == "CustomPolyPrism")
		{
			CustomPolygonPrism* customPrism = dynamic_cast<CustomPolygonPrism*>(m_selectedEntity->cCustomShape->customShape.get());
			customPrism->setPosition(mouseWorldPos + m_dragOffset);
			customPrism->updateMarkerPositions();
		}
		else if (m_selectedEntity->getTag() == "Lens")
		{ 
			// Just update position
			m_selectedEntity->cCustomShape->customShape->setPosition(mouseWorldPos + m_dragOffset);
			MyLensShape* lensShape = dynamic_cast<MyLensShape*>(m_selectedEntity->cCustomShape->customShape.get());
			lensShape->updateMarkers();
		}
		else if (m_selectedEntity->getTag() == "Marker")  // Just update position
		{
			// Just a check to make sure no errors
			if (m_selectedEntity->cMarker)
			{
				MarkerRole markerRole = m_selectedEntity->cMarker->getRole();
				if (markerRole == MarkerRole::LeftInset)
				{
					sf::Vector2f currentPos = m_selectedEntity->cShape->circle.getPosition();
					float newX = (mouseWorldPos + m_dragOffset).x;
					m_selectedEntity->cShape->circle.setPosition(sf::Vector2f(newX, currentPos.y));

					Entity* lens = m_selectedEntity->cMarker->getTargetEntity();
					MyLensShape* lensShape = dynamic_cast<MyLensShape*>(lens->cCustomShape->customShape.get());
					if (lensShape)  // Check cast succeeded
					{
						float deltaLeftInset = newX - currentPos.x;
						lensShape->setLeftInset(lensShape->getLeftInset() + deltaLeftInset);
					}
				}
				else if (markerRole == MarkerRole::RightInset)
				{
					sf::Vector2f currentPos = m_selectedEntity->cShape->circle.getPosition();
					float newX = (mouseWorldPos + m_dragOffset).x;
					m_selectedEntity->cShape->circle.setPosition(sf::Vector2f(newX, currentPos.y));

					Entity* lens = m_selectedEntity->cMarker->getTargetEntity();
					MyLensShape* lensShape = dynamic_cast<MyLensShape*>(lens->cCustomShape->customShape.get());
					if (lensShape)  // Check cast succeeded
					{
						float deltaRightInset = currentPos.x - newX;
						lensShape->setRightInset(lensShape->getRightInset() + deltaRightInset);
					}
				}
				else if (markerRole == MarkerRole::WidthAndHeight)
				{
					sf::Vector2f currentPos = m_selectedEntity->cShape->circle.getPosition();
					sf::Vector2f newPos = mouseWorldPos + m_dragOffset;
					m_selectedEntity->cShape->circle.setPosition(newPos);
					Entity* lens = m_selectedEntity->cMarker->getTargetEntity();
					MyLensShape* lensShape = dynamic_cast<MyLensShape*>(lens->cCustomShape->customShape.get());
					if (lensShape)  // Check cast succeeded
					{
						float deltaWidth = currentPos.x - newPos.x;
						float deltaHeight = currentPos.y - newPos.y;
						lensShape->setWidth(lensShape->getWidth() + deltaWidth);
						lensShape->setHeight(lensShape->getHeight() + deltaHeight);
						lensShape->setPosition(lensShape->getPosition() - sf::Vector2f(deltaWidth, deltaHeight));
						lensShape->updateMarkers();
					}
				}
				else if (markerRole == MarkerRole::singleRayMarker1)
				{
					m_selectedEntity->cShape->circle.setPosition(mouseWorldPos + m_dragOffset);
					sf::Vector2f marker1Pos = m_selectedEntity->cShape->circle.getPosition();
					sf::Vector2f marker2Pos = sf::Vector2f(0.0f, 0.0f);
					for (auto& e : m_entities.getEntities())
					{
						if (e->cMarker && e->cMarker->getRole() == MarkerRole::singleRayMarker2 && (e->cMarker->getTargetRay() == m_selectedEntity->cMarker->getTargetRay()))
						{
							marker2Pos = e.get()->cShape->circle.getPosition();
							break;
						}
					}
					if (marker2Pos == sf::Vector2f(0.0f, 0.0f))
					{
						std::cerr << "Error: marker2 not found!" << std::endl;
					}
					else
					{
						m_stateChange = true;
						sf::Vector2f newDirection = normalize(marker2Pos - marker1Pos);
						RayData* ray = m_selectedEntity->cMarker->getTargetRay();
						if (ray)
						{
							ray->originX = marker1Pos.x;
							ray->originY = marker1Pos.y;
							ray->dirX = newDirection.x;
							ray->dirY = newDirection.y;
						}
						else
						{
							std::cerr << "Error: marker has no target ray!" << std::endl;
						}
					}
				}
				else if (markerRole == MarkerRole::singleRayMarker2)
				{
					m_selectedEntity->cShape->circle.setPosition(mouseWorldPos + m_dragOffset);
					sf::Vector2f marker2Pos = m_selectedEntity->cShape->circle.getPosition();
					sf::Vector2f marker1Pos = sf::Vector2f(0.0f, 0.0f);
					for (auto& e : m_entities.getEntities())
					{
						if (e->cMarker && e->cMarker->getRole() == MarkerRole::singleRayMarker1 && (e->cMarker->getTargetRay() == m_selectedEntity->cMarker->getTargetRay()))
						{
							marker1Pos = e.get()->cShape->circle.getPosition();
							break;
						}
					}
					if (marker1Pos == sf::Vector2f(0.0f, 0.0f))
					{
						std::cerr << "Error: marker1 not found!" << std::endl;
					}
					else
					{
						sf::Vector2f newDirection = normalize(marker2Pos - marker1Pos);
						RayData* ray = m_selectedEntity->cMarker->getTargetRay();
						ray->originX = marker1Pos.x;
						ray->originY = marker1Pos.y;
						ray->dirX = newDirection.x;
						ray->dirY = newDirection.y;
					}
				}
				else if (markerRole == MarkerRole::CircularArcMarker)
				{
					Entity* arcEntity = m_selectedEntity->cMarker->getTargetEntity();
					CircularArcShape* arcShape = dynamic_cast<CircularArcShape*>(arcEntity->cCustomShape->customShape.get());
					arcShape->setMarkerPos(m_selectedEntity, mouseWorldPos);
				}
			}
		}
	}
}
