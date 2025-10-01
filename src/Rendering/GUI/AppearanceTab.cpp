#include "AppearanceTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include "../../Core/Settings.h"

namespace kx {
    namespace GUI {

        // --- Helper Functions for Rendering UI Sections ---

        // Renders the checkboxes for a specific ESP category (Player, NPC, Object).
        static void RenderCategoryStyleSettings(const char* categoryName, bool& renderBox, bool& renderDistance, bool& renderDot, bool* renderHealthBar = nullptr, bool* renderDetails = nullptr, bool* renderPlayerName = nullptr) {
            if (ImGui::CollapsingHeader(categoryName, ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Text("Visual Elements");

                std::string boxLabel = "Show Box##" + std::string(categoryName);
                std::string distLabel = "Show Distance##" + std::string(categoryName);
                std::string dotLabel = "Show Dot##" + std::string(categoryName);

                ImGui::Checkbox(boxLabel.c_str(), &renderBox);
                ImGui::SameLine();
                ImGui::Checkbox(distLabel.c_str(), &renderDistance);
                ImGui::SameLine();
                ImGui::Checkbox(dotLabel.c_str(), &renderDot);

                if (renderHealthBar) {
                    std::string healthLabel = "Show Health Bar##" + std::string(categoryName);
                    ImGui::Checkbox(healthLabel.c_str(), renderHealthBar);
                    if (renderDetails) ImGui::SameLine();
                }

                if (renderDetails) {
                    std::string detailsLabel = "Show Details##" + std::string(categoryName);
                    ImGui::Checkbox(detailsLabel.c_str(), renderDetails);
                }

                if (renderPlayerName) {
                    std::string nameLabel = "Show Player Name##" + std::string(categoryName);
                    ImGui::Checkbox(nameLabel.c_str(), renderPlayerName);
                }
            }
        }

        // Renders the global settings like distance limit.
        static void RenderGlobalSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Global Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Use Distance Limit", &settings.espUseDistanceLimit);
                if (settings.espUseDistanceLimit) {
                    ImGui::SliderFloat("Render Distance Limit", &settings.espRenderDistanceLimit, 10.0f, 2000.0f, "%.0fm");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Entities beyond this distance will not be rendered based on gameplay distance (player-to-target).");
                    }
                }
            }
        }

        // Renders all sliders related to the ESP scaling curve.
        static void RenderScalingSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("ESP Scaling Configuration")) { // Collapsed by default

                // --- PRIMARY, ALWAYS-VISIBLE CONTROLS ---
                ImGui::SliderFloat("Scaling Start Distance", &settings.espScalingStartDistance, 0.0f, 150.0f, "%.1fm");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The distance at which elements begin to scale down.\nSet to 0.0 for a continuous curve.");
                }

                ImGui::SliderFloat("Distance Factor", &settings.espDistanceFactor, 50.0f, 500.0f, "%.0f");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The main control for the curve's steepness.\nHigher values = gentler slope (slower scaling).");
                }

                ImGui::Separator();

                // --- ADVANCED OPTIONS TOGGLE ---
                static bool showAdvancedScaling = false;
                ImGui::Checkbox("Show Advanced Scaling Options", &showAdvancedScaling);

                if (showAdvancedScaling) {
                    ImGui::Indent(); // Indent the advanced options for clarity

                    ImGui::SliderFloat("Scaling Exponent", &settings.espScalingExponent, 0.5f, 2.5f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Controls the mathematical shape of the curve.\nDefault is 1.2.");
                    }

                    ImGui::SliderFloat("Min Scale", &settings.espMinScale, 0.1f, 1.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The minimum size an element can shrink to (as a percentage).\nPrevents elements from becoming invisibly small.");
                    }

                    ImGui::SliderFloat("Max Scale", &settings.espMaxScale, 0.5f, 2.0f, "%.2f");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The maximum size an element can magnify to up close.\nSet to 1.0 to disable any magnification.");
                    }

                    ImGui::SliderFloat("Min Font Size", &settings.espMinFontSize, 6.0f, 12.0f, "%.1fpx");
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("The absolute minimum pixel size for fonts at maximum distance.");
                    }

                    ImGui::Unindent();
                }
            }
        }

        // Renders all sliders related to the base size of ESP elements.
        static void RenderBaseSizeSettings(Settings& settings) {
            if (ImGui::CollapsingHeader("Element Base Sizes")) { // Collapsed by default
                ImGui::SliderFloat("Font Size", &settings.espBaseFontSize, 6.0f, 30.0f, "%.1fpx");
                ImGui::SliderFloat("Dot Radius", &settings.espBaseDotRadius, 1.0f, 10.0f, "%.1fpx");
                ImGui::SliderFloat("Box Thickness", &settings.espBaseBoxThickness, 0.5f, 5.0f, "%.1fpx");

                ImGui::Separator();

                ImGui::SliderFloat("Health Bar Width", &settings.espBaseHealthBarWidth, 20.0f, 100.0f, "%.0fpx");
                ImGui::SliderFloat("Health Bar Height", &settings.espBaseHealthBarHeight, 2.0f, 20.0f, "%.1fpx");

                ImGui::Separator();

                ImGui::SliderFloat("Base Box Height", &settings.espBaseBoxHeight, 20.0f, 150.0f, "%.0fpx");
                ImGui::SliderFloat("Base Box Width", &settings.espBaseBoxWidth, 10.0f, 100.0f, "%.0fpx");
            }
        }

        // --- Main Tab Rendering Function ---

        void RenderAppearanceTab() {
            if (ImGui::BeginTabItem("Appearance")) {
                auto& settings = kx::AppState::Get().GetSettings();

                ImGui::Text("Visual Style Settings");
                ImGui::Separator();

                RenderGlobalSettings(settings);
                RenderScalingSettings(settings);
                RenderBaseSizeSettings(settings);

                ImGui::Separator();

                RenderCategoryStyleSettings("Player Style", settings.playerESP.renderBox, settings.playerESP.renderDistance, settings.playerESP.renderDot, &settings.playerESP.renderHealthBar, &settings.playerESP.renderDetails, &settings.playerESP.renderPlayerName);
                RenderCategoryStyleSettings("NPC Style", settings.npcESP.renderBox, settings.npcESP.renderDistance, settings.npcESP.renderDot, &settings.npcESP.renderHealthBar, &settings.npcESP.renderDetails);
                RenderCategoryStyleSettings("Object Style", settings.objectESP.renderBox, settings.objectESP.renderDistance, settings.objectESP.renderDot, nullptr, &settings.objectESP.renderDetails);

                ImGui::EndTabItem();
            }
        }

    } // namespace GUI
} // namespace kx