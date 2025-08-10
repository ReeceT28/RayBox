#include "Simulation.h"
#include "Entity.h"
#include "CustomPolygonPrism.h"
#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <iostream>



void Simulation::sGui()
{
	ImGuiWindowFlags window_flags = 0;
	static sf::Clock profileSaveTimer;
	static bool profileSaved = false;

	static bool no_titlebar = true;
	static bool menu_bar = true;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (menu_bar)           window_flags |= ImGuiWindowFlags_MenuBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;

	// Get SFML window size
	sf::Vector2u winSize = m_window.getSize();
	float fullWidth = static_cast<float>(winSize.x);
	float fixedHeight = 30.f;  // Set your desired fixed vertical height

	// Set ImGui window to top-left corner
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);

	// Stretch horizontally, fixed height
	ImGui::SetNextWindowSize(ImVec2(fullWidth, fixedHeight), ImGuiCond_Always);

	ImGui::Begin("Simulation Controls", NULL, window_flags);


	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Light Source"))
		{
			if (ImGui::Button("Single Ray"))
			{
				m_placingMarker = true;
				m_placingSingleRay = true;
			}
			if (ImGui::BeginMenu("Point Light"))
			{
				float contentWidth = ImGui::GetContentRegionAvail().x;

				auto centerNextLabeledItem = [&](const char* label)
					{
						float labelWidth = ImGui::CalcTextSize(label).x;
						float itemWidth = ImGui::CalcItemWidth(); // Just the input field
						float totalWidth = labelWidth + ImGui::GetStyle().ItemInnerSpacing.x + itemWidth;

						float contentWidth = ImGui::GetContentRegionAvail().x;
						float cursorX = (contentWidth - totalWidth) * 0.5f;

						if (cursorX > 0.0f)
							ImGui::SetCursorPosX(cursorX);
					};

				auto centerNextButton = [&](const char* label)
					{
						float buttonWidth = ImGui::CalcTextSize(label).x + ImGui::GetStyle().FramePadding.x * 2;
						float contentWidth = ImGui::GetContentRegionAvail().x;
						float cursorX = (contentWidth - buttonWidth) * 0.5f;

						if (cursorX > 0.0f)
							ImGui::SetCursorPosX(cursorX);
					};

				const float sliderWidth = 200.0f;

				centerNextLabeledItem("Resolution");
				if (ImGui::SliderInt("Resolution", &m_pointLightResolution, 4, 100000, "%d", ImGuiSliderFlags_AlwaysClamp))
				{
					m_stateChange = true;
					for (auto& e : m_entities.getEntities())
					{
						if (e->getTag() == "Marker" && e->cMarker)
						{
							bool isPoint = e->cMarker->getRole() == MarkerRole::PointLightMarker;
							bool isColour = e->cMarker->getRole() == MarkerRole::ColouredPointLightMarker;
							if (!isPoint && !isColour) continue;

							for (RayData* ray : e->cMarker->getRays()) sRemoveRay(ray);
							e->cMarker->clearRays();
							sf::Vector2f center = e->cShape->circle.getPosition();

							for (int i = 0; i < m_pointLightResolution; ++i)
							{
								float angle = static_cast<float>(i) * (2 * pi / m_pointLightResolution);
								sf::Vector2f dir = { std::cos(angle), std::sin(angle) };
								sf::Vector2f rayOrigin = center + dir;

								if (isPoint)
								{
									RayData* ray = sCreateWhiteRay(rayOrigin, dir);
									e->cMarker->addTargetRay(ray);
								}
								else
								{
									RayData* ray = sCreateColouredRay(rayOrigin, dir, m_wavelengthCreation);
									e->cMarker->addTargetRay(ray);

									const int minRes = 5;
									const int maxRes = 5000;
									const float minAlpha = 15.f;
									const float maxAlpha = 100.0f;

									float t = std::clamp((float)(m_pointLightResolution - minRes) / (maxRes - minRes), 0.0f, 1.0f);
									ray->color.a = (maxAlpha * (1.0f - t) + minAlpha * t);
								}
							}
						}
					}
				}

				centerNextLabeledItem("Wavelength");

				if (ImGui::SliderFloat("Wavelength", &m_wavelengthCreation, m_startWavelength, m_endWavelength))
				{
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

				centerNextLabeledItem("Wavelength Step");

				ImGui::SliderFloat("Wavelength Step", &m_wavelengthCreationStep, 0.0f, 10.0f, "%.1f");
				centerNextButton("Create Point Light");
				if (ImGui::Button("Create Point Light"))
				{
					m_placingMarker = true;
					m_placingPointLight = true;
				}
				centerNextButton("Coloured Point Light");

				if (ImGui::Button("Coloured Point Light"))
				{
					m_placingMarker = true;
					m_placingColouredPointLight = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Mirror"))
		{
			if(ImGui::BeginMenu("Circular Arc"))
			{
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create Circular Arc ").x;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Circular Arc", ImVec2(buttonWidth, 0)))
				{
					m_placingMarker = true;
					m_placingCircularArc = true;
				}
				ImGui::EndMenu();

			}
			if(ImGui::BeginMenu("Create Line Mirror"))
			{
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create Line Mirror ").x;
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Line Mirror", ImVec2(buttonWidth, 0)))
				{
					m_placingMarker = true;
					m_placingLineMirror = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		static bool polygonSettingsWindow = false;
		static bool sellmeierProfileWindow = false;
		static bool sellmeierReviewWindow = false;
		static float radius = 100.0f;
		static int sides = 3;
		static float rotation = 0.0f;
		static float posX = m_window.getSize().x / 2.0f;
		static float posY = m_window.getSize().y / 2.0f;

		if (ImGui::BeginMenu("Glass"))
		{
			if (ImGui::BeginMenu("Polygon prism"))
			{
				// Get available width in the menu
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Polygon Settings ").x;
				// Center Polygon Settings button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Polygon Settings", ImVec2(buttonWidth, 0)))
				{
					polygonSettingsWindow = true;
				}

				buttonWidth = ImGui::CalcTextSize(" Create Prism ").x; // 20 for padding, tweak if needed
				// Center Create Prism button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Prism", ImVec2(buttonWidth, 0)))
				{
					m_stateChange = true;
					sCreatePrism({ posX,posY }, radius, sides, sf::Color(50, 50, 50, 10), sf::Color::White, "Crown Glass", rotation);
				}


				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Sellmeier Profile"))
			{
				// Get available width in the menu
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create New Profile ").x;
				// Center Polygon Settings button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create New Profile", ImVec2(buttonWidth, 0)))
				{
					sellmeierProfileWindow = true; // Open the profile creation window

				}
				buttonWidth = ImGui::CalcTextSize(" Review Profiles ").x;
				// Center Create Prism button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Review Profiles", ImVec2(buttonWidth, 0)))
				{
					sellmeierReviewWindow = true;
				}

				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Lens"))
			{
				// Get available width in the menu
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create Lens ").x;
				// Center Create Lens button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Lens", ImVec2(buttonWidth, 0)))
				{
					m_stateChange = true;
					sCreateLens(sf::Vector2f(300.0f, 250.0f), 200.0f, 200.0f, 51.0f, 50.0f, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Demo Prism"))
			{
				// Get available width in the menu
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create Demo Prism ").x;
				// Center Create Demo Prism button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Demo Prism", ImVec2(buttonWidth, 0)))
				{
					sCreateDemoPrism(sf::Vector2f(300.0f, 100.0f), 300.0f, 60.0f / 180.0f * 3.14159265f, sf::Color(50, 50, 50, 50), sf::Color::White, "Crown Glass");
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Custom Polygon Prism"))
			{
				// Get available width in the menu
				float windowWidth = ImGui::GetWindowWidth();
				float buttonWidth = ImGui::CalcTextSize(" Create Custom Prism ").x;
				// Center Create Demo Prism button
				ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f);
				if (ImGui::Button("Create Custom Prism", ImVec2(buttonWidth, 0)))
				{
					m_placingMarker = true;
					m_placingCustomPrism = true;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (sellmeierReviewWindow)
		{
			ImGui::Begin("Sellmeier Profile Review", &sellmeierReviewWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("All available Sellmeier profiles:");
			int index = 0;
			for (const auto& profile : m_sellmeierManager.getProfiles())
			{
				std::string tag = profile->getTag();
				if (tag.empty()) tag = "Unnamed Profile";
				std::string label = tag + "##Profile" + std::to_string(index++);

				if (ImGui::TreeNode(label.c_str()))
				{
					std::vector<double> a = profile->getCoefficientsA();
					std::vector<double> b = profile->getCoefficientsB();
					ImGui::Text("Coefficients A: %.8f, %.8f, %.8f", a[0], a[1], a[2]);
					ImGui::Text("Coefficients B: %.8f, %.8f, %.8f", b[0], b[1], b[2]);
					ImGui::TreePop();
				}

				ImGui::Separator();
			}
			ImGui::End();

		}
		if (sellmeierProfileWindow)
		{
			ImGui::Begin("Sellmeier Profile Creation", &sellmeierProfileWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
			ImGui::Text("Change these values to create a new Sellmeier profile for a material!");
			static std::vector<double> aCoefficients = { 1.03961212, 0.231792344, 1.01146945 };
			static std::vector<double> bCoefficients = { 0.00600069867, 0.0200179144, 103.560653 };
			for (size_t i = 0; i < aCoefficients.size(); ++i)
			{
				ImGui::InputDouble(("a" + std::to_string(i + 1)).c_str(), &aCoefficients[i], 0.01f, 1.0f, "%.8f");
			}
			for (size_t i = 0; i < bCoefficients.size(); ++i)
			{
				ImGui::InputDouble(("b" + std::to_string(i + 1)).c_str(), &bCoefficients[i], 0.01f, 1.0f, "%.8f");
			}
			static char str1[128] = "";
			ImGui::InputTextWithHint("Name Of Profile", "Input Text Here", str1, IM_ARRAYSIZE(str1));
			if (ImGui::Button("Save Profile"))
			{
				m_sellmeierManager.addProfile({ aCoefficients[0], aCoefficients[1], aCoefficients[2] },
					{ bCoefficients[0], bCoefficients[1], bCoefficients[2] },
					str1);

				Sellmeier* sellmeierCrownGlass = m_sellmeierManager.getSellmeier(str1);
				sellmeierCrownGlass->calculateWavelengthToIndex(m_startWavelength, m_step, prismResolution);

				profileSaved = true;
				profileSaveTimer.restart();
			}

			// Show success message for 2 seconds
			if (profileSaved && profileSaveTimer.getElapsedTime().asSeconds() < 0.75f)
			{
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0.1f, 1.0f, 0.1f, 1.0f), "Profile Saved!");
			}
			else if (profileSaveTimer.getElapsedTime().asSeconds() >= 0.75f)
			{
				profileSaved = false; // Reset flag
			}
			ImGui::End();
		}
		if (polygonSettingsWindow)
		{
			ImGui::Begin("Polygon Prism Creation Settings", &polygonSettingsWindow, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);  // Pass pointer to allow closing
			if (ImGui::SliderFloat("Radius", &radius, 40.0f, 200.0f, "%.1f")) {}
			if (ImGui::SliderInt("Sides", &sides, 3, 10, "%d")) {}
			if (ImGui::SliderFloat("Rotation", &rotation, 0.0f, 360.0f, "%.1f")) {}
			if (ImGui::InputFloat("Position X", &posX) || ImGui::InputFloat("Position Y", &posY)) {}

			// Static to persist across frames
			static std::vector<std::string> profileTags;
			static std::vector<const char*> profileTagPtrs;
			static int selectedProfileIndex = 0;
			static size_t lastProfileCount = 0;

			// Only update if profiles have changed
			const auto& profiles = m_sellmeierManager.getProfiles();
			if (profiles.size() != lastProfileCount)
			{
				profileTags.clear();
				profileTagPtrs.clear();
				profileTags.reserve(profiles.size()); // Prevent reallocations
				profileTagPtrs.reserve(profiles.size());

				for (const auto& profile : profiles)
				{
					profileTags.emplace_back(profile->getTag());
					profileTagPtrs.push_back(profileTags.back().c_str());
				}

				// Clamp index to avoid out-of-range
				if (selectedProfileIndex >= static_cast<int>(profileTagPtrs.size()))
					selectedProfileIndex = 0;

				lastProfileCount = profiles.size();
			}

			ImGui::Combo("Select Material", &selectedProfileIndex,
				profileTagPtrs.data(), static_cast<int>(profileTagPtrs.size()));

			if (ImGui::Button("Create Prism"))
			{
				m_stateChange = true;
				sCreatePrism({ posX, posY }, radius, sides,
					sf::Color(50, 50, 50, 50),
					sf::Color::White,
					profileTags[selectedProfileIndex],
					rotation);
			}

			ImGui::End();
		}


		if (ImGui::BeginMenu("Blocker"))
		{
			ImGui::MenuItem("Add Blocker");
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Other"))
		{
			if (ImGui::BeginMenu("Blend Mode Settings"))
			{
				static int srcFactorIndex = 6;  // Default to SrcAlpha
				static int dstFactorIndex = 1;  // Default to One
				static int equationIndex = 0;   // Default to Add

				const char* factorNames[] = {
					"Zero", "One", "SrcColor", "OneMinusSrcColor", "DstColor", "OneMinusDstColor",
					"SrcAlpha", "OneMinusSrcAlpha", "DstAlpha", "OneMinusDstAlpha"
				};

				sf::BlendMode::Factor factorEnums[] = {
					sf::BlendMode::Factor::Zero,
					sf::BlendMode::Factor::One,
					sf::BlendMode::Factor::SrcColor,
					sf::BlendMode::Factor::OneMinusSrcColor,
					sf::BlendMode::Factor::DstColor,
					sf::BlendMode::Factor::OneMinusDstColor,
					sf::BlendMode::Factor::SrcAlpha,
					sf::BlendMode::Factor::OneMinusSrcAlpha,
					sf::BlendMode::Factor::DstAlpha,
					sf::BlendMode::Factor::OneMinusDstAlpha
				};

				const char* equationNames[] = {
					"Add", "Subtract", "ReverseSubtract", "Max", "Min"
				};

				sf::BlendMode::Equation equationEnums[] = {
					sf::BlendMode::Equation::Add,
					sf::BlendMode::Equation::Subtract,
					sf::BlendMode::Equation::ReverseSubtract,
					sf::BlendMode::Equation::Max,
					sf::BlendMode::Equation::Min
				};
				ImGui::Combo("Source Factor", &srcFactorIndex, factorNames, IM_ARRAYSIZE(factorNames));
				ImGui::Combo("Destination Factor", &dstFactorIndex, factorNames, IM_ARRAYSIZE(factorNames));
				ImGui::Combo("Blend Equation", &equationIndex, equationNames, IM_ARRAYSIZE(equationNames));

				if (ImGui::Button("Apply Blend Mode"))
				{
					m_blendMode = sf::BlendMode(
						factorEnums[srcFactorIndex],
						factorEnums[dstFactorIndex],
						equationEnums[equationIndex]
					);
				}

				ImGui::Text("Current: %s / %s / %s",
					factorNames[srcFactorIndex],
					factorNames[dstFactorIndex],
					equationNames[equationIndex]);

				ImGui::EndMenu();
			}

			ImGui::MenuItem("Settings");
			if (ImGui::BeginMenu("Screenshot"))
			{
				if (ImGui::Button("Take a Screenshot"))
				{
					std::string filename = "Screenshots/screenshot_" + std::to_string(time(nullptr)) + ".png";
					saveScreenshot(filename);
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}
		int lineSegmentCount = allRays.getVertexCount() / 2;

		// Format the text first
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "Line Segments: %d", lineSegmentCount);

		// Calculate text width
		float textWidth = ImGui::CalcTextSize(buffer).x;

		// Get window width
		float windowWidth = ImGui::GetWindowWidth();

		// Set cursor X position to right-align the text
		ImGui::SetCursorPosX(windowWidth - textWidth - 100.0f);

		// Render the text
		ImGui::Text("%s", buffer);
		float fpsTextWidth = ImGui::CalcTextSize("FPS: 000.0").x;
		float padding = 20.0f; // Add some padding from the right edge
		float availableWidth = ImGui::GetWindowWidth();
		ImGui::SetCursorPosX(availableWidth - fpsTextWidth - padding);
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

		ImGui::EndMenuBar();
	}


		if (m_selectedEntity && m_selectedEntity->getTag() != "Marker")
		{
			ImGui::Begin("Selected Entity Settings", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);

			if (ImGui::CollapsingHeader("Orientation Settings"))
			{
				if (m_selectedEntity->cShape)
				{
					float radius = m_selectedEntity->cShape->circle.getRadius();
					if (ImGui::SliderFloat("Radius", &radius, 40.0f, 200.0f, "%.1f"))
					{
						m_stateChange = true;
						m_selectedEntity->cShape->circle.setRadius(radius);
						m_selectedEntity->cShape->circle.setOrigin({ radius, radius });
						if (m_rotator) m_rotator->cShape->circle.setRadius(radius / 10.0f);
						updateRotatorPosition();
					}

					int sides = m_selectedEntity->cShape->circle.getPointCount();
					if (ImGui::SliderInt("Sides", &sides, 3, 50, "%d"))
					{
						m_stateChange = true;
						m_selectedEntity->cShape->circle.setPointCount(sides);
						updateRotatorPosition();
					}

					float rotation = m_selectedEntity->cShape->circle.getRotation().asDegrees();
					if (ImGui::SliderFloat("Rotation", &rotation, 0.0f, 360.0f, "%.1f"))
					{
						m_stateChange = true;
						m_selectedEntity->cShape->circle.setRotation(sf::Angle(sf::degrees(rotation)));
						updateRotatorPosition();
					}

					sf::Vector2f pos = m_selectedEntity->cShape->circle.getPosition();
					if (ImGui::InputFloat("Position X", &pos.x) || ImGui::InputFloat("Position Y", &pos.y))
					{
						m_stateChange = true;
						m_selectedEntity->cShape->circle.setPosition(pos);
						updateRotatorPosition();
					}

					sf::Vector2f scale = m_selectedEntity->cShape->circle.getScale();
					if (ImGui::SliderFloat("Scale X", &scale.x, 0.5f, 3.0f, "%.2f") ||
						ImGui::SliderFloat("Scale Y", &scale.y, 0.5f, 3.0f, "%.2f"))
					{
						m_stateChange = true;
						m_selectedEntity->cShape->circle.setScale(scale);
						updateRotatorPosition();
					}
				}

				if (m_selectedEntity->cCustomShape)
				{
					auto lensShape = dynamic_cast<MyLensShape*>(m_selectedEntity->cCustomShape->customShape.get());
					if (lensShape)
					{
						sf::Vector2f pos = lensShape->getPosition();
						if (ImGui::InputFloat("Position X", &pos.x) || ImGui::InputFloat("Position Y", &pos.y))
						{
							m_stateChange = true;
							lensShape->setPosition(pos);
							updateRotatorPosition();
						}
					}
					else if (auto prismShape = dynamic_cast<PrismDemoShape*>(m_selectedEntity->cCustomShape->customShape.get()))
					{
						float alpha = prismShape->getAlpha() * 180.0f / pi;
						float incidentAngle = prismShape->getIncidentAngle() * 180.0f / pi;
						float alphaIncrement = prismShape->getAlphaIncrement() * 180.0f / pi;

						if (ImGui::SliderFloat("Alpha", &alpha, 0.0f, 90.0f, "%.3f"))
						{
							m_stateChange = true;
							prismShape->setAlpha(alpha * pi / 180.0f);
						}
						if (ImGui::SliderFloat("Incident Angle", &incidentAngle, 0.0f, 90.0f, "%.1f"))
						{
							m_stateChange = true;
							prismShape->setIncidentAngle(incidentAngle * pi / 180.0f);
						}
						if (ImGui::SliderFloat("Alpha Increment", &alphaIncrement, 0.0f, 10.0f, "%.3f"))
						{
							m_stateChange = true;
							prismShape->setAlphaIncrement(alphaIncrement * pi / 180.0f);
						}
					}
					else if (auto customPrismShape = dynamic_cast<CustomPolygonPrism*>(m_selectedEntity->cCustomShape->customShape.get()))
					{
						sf::Vector2f pos = customPrismShape->getPosition();
						if (ImGui::InputFloat("Position X", &pos.x) || ImGui::InputFloat("Position Y", &pos.y))
						{
							m_stateChange = true;
							customPrismShape->setPosition(pos);
							customPrismShape->updateMarkerPositions();
						}
					}
				}

				if (m_selectedEntity->cPrism)
				{
					static std::vector<std::string> profileTags;
					static std::vector<const char*> profileTagPtrs;
					static size_t lastProfileCount = 0;

					const auto& profiles = m_sellmeierManager.getProfiles();
					if (profiles.size() != lastProfileCount)
					{
						profileTags.clear();
						profileTagPtrs.clear();
						profileTags.reserve(profiles.size());
						profileTagPtrs.reserve(profiles.size());

						for (const auto& profile : profiles)
						{
							profileTags.emplace_back(profile->getTag());
							profileTagPtrs.push_back(profileTags.back().c_str());
						}

						lastProfileCount = profiles.size();
					}

					std::string currentTag = m_selectedEntity->cPrism->getTag();
					int selectedIndex = 0;
					for (int i = 0; i < profileTags.size(); ++i)
					{
						if (profileTags[i] == currentTag)
						{
							selectedIndex = i;
							break;
						}
					}

					if (ImGui::Combo("Select Material", &selectedIndex, profileTagPtrs.data(), (int)profileTagPtrs.size()))
					{
						if (profileTags[selectedIndex] != m_selectedEntity->cPrism->getTag())
						{
							m_stateChange = true;
							m_selectedEntity->cPrism->setTag(profileTags[selectedIndex]);
						}
					}
				}

				if (ImGui::Button("Delete Entity"))
				{
					if (auto shape = dynamic_cast<PrismDemoShape*>(m_selectedEntity->cCustomShape ? m_selectedEntity->cCustomShape->customShape.get() : nullptr))
					{
						if (RayData* incidentRay = shape->getIncidentRay())
						{
							auto it = std::remove_if(m_lightSources.begin(), m_lightSources.end(),
								[incidentRay](const std::unique_ptr<RayData>& ray) {
									return ray.get() == incidentRay;
								});
							m_lightSources.erase(it, m_lightSources.end());
						}
					}

					if (auto lensShape = dynamic_cast<MyLensShape*>(m_selectedEntity->cCustomShape ? m_selectedEntity->cCustomShape->customShape.get() : nullptr))
					{
						lensShape->destroyMarkers();
					}
					if(auto customPrismShape = dynamic_cast<CustomPolygonPrism*>(m_selectedEntity->cCustomShape ? m_selectedEntity->cCustomShape->customShape.get() : nullptr))
					{
						customPrismShape->destroyMarkers();
					}

					m_selectedEntity->m_dead = true;
					m_selectedEntity = nullptr;
					m_stateChange = true;
				}
			}

			ImGui::End();
		}

	if (m_placingMarker)
	{
		ImGui::Begin("Placing Marker", &m_placingMarker, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::Text("Click to place a marker at the mouse position.");
		if (ImGui::Button("Cancel"))
		{
			// Reset placement flags
			m_placingMarker = false;
			m_placingSingleRay = false;
			m_placingCustomPrism = false;

			// Optionally clear preview from previous prism marker
			if (m_previousMarker && m_previousMarker->cMarker)
			{
				if (Entity* customPrism = m_previousMarker->cMarker->getTargetEntity())
				{
					if (auto* customShape = customPrism->cCustomShape.get())
					{
						if (auto* prismShape = dynamic_cast<CustomPolygonPrism*>(customShape->customShape.get()))
						{
							prismShape->clearPreview();
						}
					}
				}
			}

			// Reset previous marker
			m_previousMarker = nullptr;
		}

		ImGui::End();
	}

	ImGui::End();
}