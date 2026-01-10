#include "NpcsTab.h"

#include "../../../Core/AppState.h"
#include "GuiHelpers.h"
#include "../../../../libs/ImGui/imgui.h"

namespace kx {
    namespace GUI {

        void RenderNPCsTab() {
            if (ImGui::BeginTabItem("NPCs")) {
                auto& settings = AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable NPC ESP", &settings.npcESP.enabled);
                
                if (settings.npcESP.enabled) {
                    ImGui::Separator();
                    if (ImGui::CollapsingHeader("Attitude Filter"))
                    {
                        ImGui::Checkbox("Show Friendly##NPC", &settings.npcESP.showFriendly); ImGui::SameLine();
                        ImGui::Checkbox("Show Hostile##NPC", &settings.npcESP.showHostile); ImGui::SameLine();
                        ImGui::Checkbox("Show Neutral##NPC", &settings.npcESP.showNeutral); ImGui::SameLine();
                        ImGui::Checkbox("Show Indifferent##NPC", &settings.npcESP.showIndifferent);
                    }

                    if (ImGui::CollapsingHeader("Rank Filter"))
                    {
                        const float column1 = 180.0f;
                        const float column2 = 360.0f;
                        ImGui::Checkbox("Show Legendary##NPC", &settings.npcESP.showLegendary); ImGui::SameLine(column1);
                        ImGui::Checkbox("Show Champion##NPC", &settings.npcESP.showChampion); ImGui::SameLine(column2);
                        ImGui::Checkbox("Show Elite##NPC", &settings.npcESP.showElite);
                        ImGui::Checkbox("Show Veteran##NPC", &settings.npcESP.showVeteran); ImGui::SameLine(column1);
                        ImGui::Checkbox("Show Ambient##NPC", &settings.npcESP.showAmbient); ImGui::SameLine(column2);
                        ImGui::Checkbox("Show Normal##NPC", &settings.npcESP.showNormal);
                    }

                    if (ImGui::CollapsingHeader("Health Filter")) {
                        ImGui::Checkbox("Show Dead NPCs", &settings.npcESP.showDeadNpcs);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show NPCs with 0 HP (defeated enemies, corpses).\nUseful for loot opportunities and understanding combat situations.");
                        }
                    }

                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Visual Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        RenderNpcStyleSettings(settings.npcESP);
                    }

                    if (ImGui::CollapsingHeader("Detailed Information")) {
                        ImGui::Checkbox("Show Details Panel##NPC", &settings.npcESP.renderDetails);
                        if (settings.npcESP.renderDetails) {
                            ImGui::Indent();
                            ImGui::Checkbox("Level##NpcDetail", &settings.npcESP.showDetailLevel); ImGui::SameLine();
                            ImGui::Checkbox("HP##NpcDetail", &settings.npcESP.showDetailHp); ImGui::SameLine();
                            ImGui::Checkbox("Attitude##NpcDetail", &settings.npcESP.showDetailAttitude); ImGui::SameLine();
                            ImGui::Checkbox("Rank##NpcDetail", &settings.npcESP.showDetailRank); ImGui::SameLine();
                            ImGui::Checkbox("Position##NpcDetail", &settings.npcESP.showDetailPosition);
                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}
