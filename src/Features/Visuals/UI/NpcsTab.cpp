#include "NpcsTab.h"

#include "../Settings/VisualsSettings.h"
#include "GuiHelpers.h"
#include "../../../../libs/ImGui/imgui.h"

namespace kx {
    namespace GUI {

        void RenderNPCsTab(VisualsConfiguration& config) {
            if (ImGui::BeginTabItem("NPCs")) {
                auto& settings = config.npcESP;
                
                ImGui::Checkbox("Enable NPC ESP", &settings.enabled);
                
                if (settings.enabled) {
                    ImGui::Separator();
                    if (ImGui::CollapsingHeader("Attitude Filter"))
                    {
                        ImGui::Checkbox("Show Friendly##NPC", &settings.showFriendly); ImGui::SameLine();
                        ImGui::Checkbox("Show Hostile##NPC", &settings.showHostile); ImGui::SameLine();
                        ImGui::Checkbox("Show Neutral##NPC", &settings.showNeutral); ImGui::SameLine();
                        ImGui::Checkbox("Show Indifferent##NPC", &settings.showIndifferent);
                    }

                    if (ImGui::CollapsingHeader("Rank Filter"))
                    {
                        const float column1 = 180.0f;
                        const float column2 = 360.0f;
                        ImGui::Checkbox("Show Legendary##NPC", &settings.showLegendary); ImGui::SameLine(column1);
                        ImGui::Checkbox("Show Champion##NPC", &settings.showChampion); ImGui::SameLine(column2);
                        ImGui::Checkbox("Show Elite##NPC", &settings.showElite);
                        ImGui::Checkbox("Show Veteran##NPC", &settings.showVeteran); ImGui::SameLine(column1);
                        ImGui::Checkbox("Show Ambient##NPC", &settings.showAmbient); ImGui::SameLine(column2);
                        ImGui::Checkbox("Show Normal##NPC", &settings.showNormal);
                    }

                    if (ImGui::CollapsingHeader("Health Filter")) {
                        ImGui::Checkbox("Show Dead NPCs", &settings.showDeadNpcs);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show NPCs with 0 HP (defeated enemies, corpses).\nUseful for loot opportunities and understanding combat situations.");
                        }
                    }

                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Visual Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        RenderNpcStyleSettings(settings);
                    }

                    if (ImGui::CollapsingHeader("Detailed Information")) {
                        ImGui::Checkbox("Show Details Panel##NPC", &settings.renderDetails);
                        if (settings.renderDetails) {
                            ImGui::Indent();
                            ImGui::Checkbox("Level##NpcDetail", &settings.showDetailLevel); ImGui::SameLine();
                            ImGui::Checkbox("HP##NpcDetail", &settings.showDetailHp); ImGui::SameLine();
                            ImGui::Checkbox("Attitude##NpcDetail", &settings.showDetailAttitude); ImGui::SameLine();
                            ImGui::Checkbox("Rank##NpcDetail", &settings.showDetailRank); ImGui::SameLine();
                            ImGui::Checkbox("Position##NpcDetail", &settings.showDetailPosition);
                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}

