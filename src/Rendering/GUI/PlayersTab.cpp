#include "PlayersTab.h"
#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"

namespace kx {
    namespace GUI {

        void RenderPlayersTab() {
            if (ImGui::BeginTabItem("Players")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable Player ESP", &settings.playerESP.enabled);
                
                if (settings.playerESP.enabled) {
                    ImGui::Separator();
                    if (ImGui::CollapsingHeader("Attitude Filter"))
                    {
                        ImGui::Checkbox("Show Friendly", &settings.playerESP.showFriendly); ImGui::SameLine();
                        ImGui::Checkbox("Show Hostile", &settings.playerESP.showHostile); ImGui::SameLine();
                        ImGui::Checkbox("Show Neutral", &settings.playerESP.showNeutral); ImGui::SameLine();
                        ImGui::Checkbox("Show Indifferent", &settings.playerESP.showIndifferent);
                    }

                    if (ImGui::CollapsingHeader("Player Filter Options", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Checkbox("Show Local Player", &settings.playerESP.showLocalPlayer);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show your own character in the ESP overlay.");
                        }

                        const char* energyTypes[] = { "Dodge", "Special/Mount" };
                        int energyTypeInt = static_cast<int>(settings.playerESP.energyDisplayType);
                        ImGui::PushItemWidth(250.0f);
                        if (ImGui::Combo("Energy Bar Source", &energyTypeInt, energyTypes, IM_ARRAYSIZE(energyTypes))) {
                            settings.playerESP.energyDisplayType = static_cast<EnergyDisplayType>(energyTypeInt);
                        }
                        ImGui::PopItemWidth();

                        const char* gearModes[] = { "Off", "Compact (Top 3 Stat Sets)", "Compact (Top 3 Attributes)", "Detailed" };
                        ImGui::PushItemWidth(250.0f);
                        int gearModeInt = static_cast<int>(settings.playerESP.gearDisplayMode);
                        if (ImGui::Combo("Gear Display", &gearModeInt, gearModes, IM_ARRAYSIZE(gearModes))) {
                            settings.playerESP.gearDisplayMode = static_cast<GearDisplayMode>(gearModeInt);
                        }
                        ImGui::PopItemWidth();
                    }

                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Visual Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        RenderPlayerStyleSettings(settings.playerESP);
                    }

                    if (ImGui::CollapsingHeader("Detailed Information")) {
                        ImGui::Checkbox("Show Details Panel", &settings.playerESP.renderDetails);
                        if (settings.playerESP.renderDetails) {
                            ImGui::Indent();
                            ImGui::Checkbox("Level##PlayerDetail", &settings.playerESP.showDetailLevel); ImGui::SameLine();
                            ImGui::Checkbox("Profession##PlayerDetail", &settings.playerESP.showDetailProfession); ImGui::SameLine();
                            ImGui::Checkbox("Attitude##PlayerDetail", &settings.playerESP.showDetailAttitude); ImGui::SameLine();
                            ImGui::Checkbox("Race##PlayerDetail", &settings.playerESP.showDetailRace);
                            ImGui::Checkbox("HP##PlayerDetail", &settings.playerESP.showDetailHp); ImGui::SameLine();
                            ImGui::Checkbox("Energy##PlayerDetail", &settings.playerESP.showDetailEnergy); ImGui::SameLine();
                            ImGui::Checkbox("Position##PlayerDetail", &settings.playerESP.showDetailPosition);
                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}
