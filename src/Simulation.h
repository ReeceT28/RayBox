#pragma once
#include <SFML/Graphics.hpp>
#include <optional>
#include <map>
#include "EntityManager.h"
#include "RayManager.h"
#include "Vec2fExtension.h"
#include "Sellmeier.h"
#include "Components.h"
#include "customLensShape.h"
#include "opencl.hpp"
#include "RayCollisionBuffers.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "SellmeierManager.h"
#include "PrismDemo.h"
#include "CustomPolygonPrism.h"
#include "CirularArcShape.h"

class Simulation {
	// Window stuff
	sf::RenderWindow m_window;
	sf::View m_view;
	sf::BlendMode m_blendMode;
	sf::VideoMode m_windowedMode;
	// Clock for things
	sf::Clock m_deltaClock;
	sf::Clock m_benchmarkClock;

	// OpenCL stuff
	Device m_device;
	RayCollisionBuffers m_buffers;
	std::vector<std::unique_ptr<RayData>> m_lightSources;

	// VertexArray for every single ray in the simulation
	sf::VertexArray allRays = sf::VertexArray(sf::PrimitiveType::Lines);

	// Manage these things
	EntityManager m_entities;
	SellmeierManager m_sellmeierManager;

	// Stored for a variety of things, just search the variable in UserInput.cpp if want to see
	sf::Vector2i m_lastMousePos;

	// Essential constants
	int m_pointLightResolution = 10; // Number of rays created when a point light is created
	int prismResolution = 3000;        // Number of rays created when a white ray hits a prism and reflection doesn't occur 
	float m_wavelengthCreation = 550.0f;
	float m_startWavelength = 380.0f; // Start wavelength in nm
	float m_endWavelength = 730.0f;   // End wavelength in nm
	float m_step = (m_endWavelength - m_startWavelength) / (prismResolution - 1); // Subtract 1 to ensure correct number of steps
	float m_wavelengthCreationStep = 0.0f; // Step size for wavelength slider


	// Stores all the wavelengths and their colours
	std::unordered_map<float,sf::Color> wavelengthColors;

	// Need to store for certain logic
	Entity* m_previousMarker = nullptr;
	Entity* m_selectedEntity = nullptr;
	Entity* m_rotator = nullptr;
	Entity* m_circularArcInProgress = nullptr;
	sf::Vector2f m_dragOffset;

	// Bool flags for a variety of things
	bool m_running = true;
	bool m_isPanning = false;
	bool m_isDragging = false;
	bool m_isRotating = false;
	bool m_stateChange = false;
	bool m_placingMarker = false;
	bool m_placingPointLight = false;
	bool m_placingColouredPointLight = false;
	bool m_placingCustomPrism = false;
	bool m_placingSingleRay = false; 
	bool m_placingCircularArc = false;
	bool m_placingLineMirror = false;
	bool m_isFullscreen = false;
	bool m_wavelengthIncreasing = true; // For the wavelength creation slider

	// ╔═════════════════════════════╗
	// ║  MISCELLANEUOUS FUNCTIONS   ║
	// ╚═════════════════════════════╝
	
	// Updates m_wavelengthCreation based on m_wavelengthCreationStep.
	void sUpdateWavelengthCreation();

	// Removes a ray from m_lightSources.
	void sRemoveRay(RayData* ray);

	// Toggles fullscreen mode.
	void toggleFullscreen();

	// Main logic of the simulation, sends data to the GPU, processes ray-entity intersections, and updates the rays.
	void sCollisionv2();

	// Handles rendering of the simulation, draws all entities and rays.
	void sRender();

	// Handles state changes, such as when a new entity is created or an entity is moved. It simply restarts m_buffers and populates it with light sources.
	void sHandleStateChange();

	// Updates the alpha value of a demoPrism shape if they exist.
	void sUpdateAlpha();

	// Updates rotator position based on the selected entity.
	void updateRotatorPosition();

	// Updates the positions of lens markers.
	void sUpdateLensMarkerPositions(Entity* marker1, Entity* marker2, Entity* marker3);

	// Rotates a shape to face the mouse cursor.
	void sRotateToMouse(sf::Vector2f&, sf::CircleShape&);

	// Initializes the simulation, creates the window, sets up ImGui
	void init();

	// Saves a screenshot, scale factor is used to increase the resolution of the screenshot.
	void saveScreenshot(const std::string& filename, unsigned int scaleFactor = 10);

	// Renders a screenshot of the current state of the simulation.
	void sRenderScreenShot(sf::RenderTarget& target);

	// Convert wavelength to RGB color
	sf::Color wavelengthToRGB(double wavelength);

	// Clamps a value between min and max, default is 0.0 to 1.0
	double clamp(double value, double min = 0.0, double max = 1.0);

	// ╔════════════════════════════════════╗
	// ║  ENTITY / RAY CREATION FUNCTIONS   ║
	// ╚════════════════════════════════════╝
	// These are all in the name...
	Entity* sCreatePrism(const sf::Vector2f& position, const int radius, const int sides,const sf::Color& insideColor,const sf::Color& borderColor, const std::string& tag, const float rotationDeg);
	Entity* sCreateCustomPolygonPrism(const sf::Vector2f& position, const sf::Color& fillColour, const sf::Color& borderColour, const std::string material);
	void sCreateDemoPrism(const sf::Vector2f& position, const float width, const float angle, const sf::Color& fillColor, const sf::Color& borderColor, const std::string& material);
	void sCreateLens(const sf::Vector2f&, const float, const float, const float, const float, const sf::Color&, const sf::Color&, const std::string&);
	Entity* sCreateCircularArc(Entity* m1, Entity* m2, Entity* m3, const sf::Color& borderColour);
	Entity* sCreateMarker(const sf::Vector2f& position, const float radius, const int sides, const sf::Color& insideColor, const sf::Color& borderColor, const float rotationDeg);
	RayData* sCreateWhiteRay(sf::Vector2f origin, sf::Vector2f dir);
	RayData* sCreateColouredRay(sf::Vector2f origin, sf::Vector2f dir, float wavelength);


	// ╔═════════════════════════════╗
	// ║  "UserInput.cpp" functions  ║
	// ╚═════════════════════════════╝
	void sUserInput();
	void handleMouseScrollWheel(const std::optional<sf::Event> event);
	void handleMousePressLeft(const std::optional<sf::Event> event, sf::Vector2f mouseWorldPos);
	void handleMousePressRight(const std::optional<sf::Event> event, sf::Vector2f mouseWorldPos);
	void handleMouseButtonReleased(const std::optional<sf::Event> event);
	void handleMouseMoved(const std::optional<sf::Event> event);
	void handleDragging(sf::Vector2f mouseWorldPos);

	// ╔═════════════════════════════════╗
	// ║   "SimulationUI.cpp" functions  ║
	// ╚═════════════════════════════════╝
	void sGui();
public:
	// Handles initialization and main loop of the simulation.
	void run();
	Simulation()
		: m_device(select_device_with_most_flops()), m_windowedMode({ 1200,800 }),
		m_buffers(m_device)
		{}

};



