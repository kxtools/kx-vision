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
                ImGui::SliderFloat("Scaling Start Distance", &settings.espScalingStartDistance, 0.0f, 150.0f, "%.1fm"); // Increased range
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The distance at which elements begin to scale down.\nInside this range, elements remain at 100% size.");
                }

                ImGui::SliderFloat("Distance Factor", &settings.espDistanceFactor, 10.0f, 200.0f, "%.0f"); // Tightened range
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Controls the curve's steepness after the start distance.\nHigher values = gentler slope (slower scaling).\nLower values = steeper cliff (faster scaling).");
                }

                ImGui::SliderFloat("Scaling Exponent", &settings.espScalingExponent, 0.5f, 2.5f, "%.2f");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Controls the curve shape. > 1.0 makes scaling more aggressive.\n< 1.0 makes it less aggressive.");
                }

                ImGui::Separator();

                ImGui::SliderFloat("Min Scale", &settings.espMinScale, 0.1f, 1.0f, "%.2f");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Minimum scale factor for very distant entities.\nPrevents elements from becoming invisibly small.");
                }

                ImGui::SliderFloat("Max Scale", &settings.espMaxScale, 0.5f, 2.0f, "%.2f"); // Tightened range
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Maximum scale factor for very close entities.\nSet to 1.0 to prevent any magnification.");
                }

                ImGui::SliderFloat("Min Font Size", &settings.espMinFontSize, 1.0f, 10.0f, "%.1fpx");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("The absolute minimum pixel size for fonts at maximum distance.\nPrevents text from becoming completely unreadable.");
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