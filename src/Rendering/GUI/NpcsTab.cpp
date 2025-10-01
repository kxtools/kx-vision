#include "NpcsTab.h"
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

        void RenderNPCsTab() {
            if (ImGui::BeginTabItem("NPCs")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable NPC ESP", &settings.npcESP.enabled);
                
                if (settings.npcESP.enabled) {
                    ImGui::Separator();
                    ImGui::Text("Attitude Filter");
                    
                    ImGui::Checkbox("Show Friendly", &settings.npcESP.showFriendly);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Hostile", &settings.npcESP.showHostile);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Neutral", &settings.npcESP.showNeutral);
                    ImGui::Checkbox("Show Indifferent", &settings.npcESP.showIndifferent);
                    
                    ImGui::Separator();
                    ImGui::Text("Health Filter");
                    ImGui::Checkbox("Show Dead NPCs", &settings.npcESP.showDeadNpcs);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Show NPCs with 0 HP (defeated enemies, corpses).\nUseful for loot opportunities and understanding combat situations.");
                    }

                    ImGui::Separator();
                    RenderCategoryStyleSettings("NPC Style", settings.npcESP.renderBox, settings.npcESP.renderDistance, settings.npcESP.renderDot, &settings.npcESP.renderHealthBar, &settings.npcESP.renderDetails);
                }
                ImGui::EndTabItem();
            }
        }
    }
}
