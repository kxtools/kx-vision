#include "PlayersTab.h"
#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"

namespace kx {
    namespace GUI {

        void RenderPlayersTab() {
            if (ImGui::BeginTabItem("Players")) {
                auto& settings = AppState::Get().GetSettings();
                
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

                    if (ImGui::CollapsingHeader("Combat Emphasis"))
                    {
                        ImGui::PushItemWidth(250.0f);
                        ImGui::SliderFloat("Hostile Player Boost", &settings.playerESP.hostileBoostMultiplier, 1.0f, 3.0f, "%.1fx");
                        ImGui::PopItemWidth();
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip(
                                "Size multiplier for hostile player text and health bars.\n\n"
                                "1.0x: No boost (uniform with other players)\n"
                                "2.0x: Default (double size for combat awareness)\n"
                                "3.0x: Maximum emphasis (triple size)\n\n"
                                "Tip: Set to 1.0x for cleaner visuals, or increase for better combat clarity."
                            );
                        }
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

                    if (ImGui::CollapsingHeader("Movement Trails")) {
                        ImGui::Checkbox("Enable Trails", &settings.playerESP.trails.enabled);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show smooth movement trails behind players for tactical awareness.");
                        }

                        if (settings.playerESP.trails.enabled) {
                            ImGui::Indent();

                            const char* displayModes[] = { "Hostile Only", "All Players" };
                            int displayModeInt = static_cast<int>(settings.playerESP.trails.displayMode);
                            ImGui::PushItemWidth(250.0f);
                            if (ImGui::Combo("Display Mode", &displayModeInt, displayModes, IM_ARRAYSIZE(displayModes))) {
                                settings.playerESP.trails.displayMode = static_cast<TrailDisplayMode>(displayModeInt);
                            }
                            ImGui::PopItemWidth();

                            const char* teleportModes[] = { "Tactical (Break on Teleport)", "Analysis (Connect with Dotted Line)" };
                            int teleportModeInt = static_cast<int>(settings.playerESP.trails.teleportMode);
                            ImGui::PushItemWidth(250.0f);
                            if (ImGui::Combo("Teleport Behavior", &teleportModeInt, teleportModes, IM_ARRAYSIZE(teleportModes))) {
                                settings.playerESP.trails.teleportMode = static_cast<TrailTeleportMode>(teleportModeInt);
                            }
                            ImGui::PopItemWidth();
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Tactical: breaks trails for clean visualization.\nAnalysis: draws dotted lines for cheat detection and portal tracking.");
                            }

                            ImGui::PushItemWidth(250.0f);
                            ImGui::SliderInt("Max Trail Points", &settings.playerESP.trails.maxPoints, 15, 60);
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Maximum number of position history points to store per player.");
                            }
                            ImGui::PopItemWidth();

                            ImGui::PushItemWidth(250.0f);
                            ImGui::SliderFloat("Max Duration (s)", &settings.playerESP.trails.maxDuration, 0.5f, 3.0f, "%.1f");
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Maximum time (in seconds) to keep trail history. 1.0s recommended for fast PvP.");
                            }
                            ImGui::PopItemWidth();

                            ImGui::PushItemWidth(250.0f);
                            ImGui::SliderFloat("Line Thickness", &settings.playerESP.trails.thickness, 1.0f, 5.0f, "%.1f");
                            ImGui::PopItemWidth();

                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}
