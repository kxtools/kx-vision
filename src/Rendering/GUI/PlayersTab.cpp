#include "PlayersTab.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include <string>

namespace kx {
    namespace GUI {
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

        void RenderPlayersTab() {
            if (ImGui::BeginTabItem("Players")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable Player ESP", &settings.playerESP.enabled);
                
                if (settings.playerESP.enabled) {
                    ImGui::Separator();
                    ImGui::Text("Player Filter Options");
                    ImGui::Checkbox("Show Local Player", &settings.playerESP.showLocalPlayer);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Show your own character in the ESP overlay.");
                    }

                    const char* gearModes[] = { "Off", "Compact (Stat Names)", "Compact (Top 3 Attributes)", "Detailed" };
                    ImGui::PushItemWidth(250.0f);
                    int gearModeInt = static_cast<int>(settings.playerESP.gearDisplayMode);
                    if (ImGui::Combo("Gear Display", &gearModeInt, gearModes, IM_ARRAYSIZE(gearModes))) {
                        settings.playerESP.gearDisplayMode = static_cast<GearDisplayMode>(gearModeInt);
                    }
                    ImGui::PopItemWidth();

                    ImGui::Separator();
                    RenderCategoryStyleSettings("Player Style", settings.playerESP.renderBox, settings.playerESP.renderDistance, settings.playerESP.renderDot, &settings.playerESP.renderHealthBar, &settings.playerESP.renderDetails, &settings.playerESP.renderPlayerName);
                }
                ImGui::EndTabItem();
            }
        }
    }
}