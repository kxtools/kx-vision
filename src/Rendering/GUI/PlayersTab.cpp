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

                    // --- PILLAR 1: FILTERING (Who to show) ---
                    if (ImGui::CollapsingHeader("Filtering", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::SeparatorText("Attitude Filter");
                        ImGui::Checkbox("Show Friendly", &settings.playerESP.showFriendly); ImGui::SameLine();
                        ImGui::Checkbox("Show Hostile", &settings.playerESP.showHostile); ImGui::SameLine();
                        ImGui::Checkbox("Show Neutral", &settings.playerESP.showNeutral); ImGui::SameLine();
                        ImGui::Checkbox("Show Indifferent", &settings.playerESP.showIndifferent);

                        ImGui::SeparatorText("Specific Players");
                        ImGui::Checkbox("Show Local Player", &settings.playerESP.showLocalPlayer);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show your own character in the ESP overlay.");
                        }
                    }

                    // --- PILLAR 2: INFORMATION DISPLAY (What to show) ---
                    if (ImGui::CollapsingHeader("Information Display", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::SeparatorText("Identity");
                        ImGui::Checkbox("Show Player Name##Player", &settings.playerESP.renderPlayerName);

                        ImGui::SeparatorText("Status Bars");
                        ImGui::Checkbox("Show Health Bar##Player", &settings.playerESP.renderHealthBar);
                        if (settings.playerESP.renderHealthBar) {
                            ImGui::SameLine();
                            ImGui::Checkbox("Show %##Player", &settings.playerESP.showHealthPercentage); ImGui::SameLine();
                            ImGui::Checkbox("Only show damaged##Player", &settings.playerESP.showOnlyDamaged);
                        }

                        ImGui::Checkbox("Show Energy Bar##Player", &settings.playerESP.renderEnergyBar);
                        if (settings.playerESP.renderEnergyBar) {
                            ImGui::Indent();
                            ImGui::PushItemWidth(250.0f);
                            const char* energyTypes[] = { "Dodge", "Special/Mount" };
                            int energyTypeInt = static_cast<int>(settings.playerESP.energyDisplayType);
                            if (ImGui::Combo("Source", &energyTypeInt, energyTypes, IM_ARRAYSIZE(energyTypes))) {
                                settings.playerESP.energyDisplayType = static_cast<EnergyDisplayType>(energyTypeInt);
                            }
                            ImGui::PopItemWidth();
                            ImGui::Unindent();
                        }

                        ImGui::SeparatorText("Analysis");
                        ImGui::Checkbox("Enable Gear Display", &settings.playerESP.enableGearDisplay);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Show an analysis of the player's equipped gear and stats.");
                        }
                        if (settings.playerESP.enableGearDisplay) {
                            ImGui::Indent();
                            ImGui::PushItemWidth(250.0f);
                            const char* gearModes[] = { "Compact (Stat Sets)", "Compact (Attributes)", "Detailed" };
                            int gearModeInt = static_cast<int>(settings.playerESP.gearDisplayMode);
                            if (ImGui::Combo("Display Mode", &gearModeInt, gearModes, IM_ARRAYSIZE(gearModes))) {
                                settings.playerESP.gearDisplayMode = static_cast<GearDisplayMode>(gearModeInt);
                            }
                            ImGui::PopItemWidth();
                            ImGui::Unindent();
                        }

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

                    // --- PILLAR 3: VISUAL STYLING (How it looks) ---
                    if (ImGui::CollapsingHeader("Visual Styling", ImGuiTreeNodeFlags_DefaultOpen)) {
                        // The most common, pure styling options are presented first.
                        ImGui::SeparatorText("Core Visuals");
                        ImGui::Checkbox("Show Box##Player", &settings.playerESP.renderBox); ImGui::SameLine();
                        ImGui::Checkbox("3D Wireframe##Player", &settings.playerESP.renderWireframe);
                        ImGui::Checkbox("Show Dot##Player", &settings.playerESP.renderDot); ImGui::SameLine();
                        ImGui::Checkbox("Show Distance##Player", &settings.playerESP.renderDistance);

                        ImGui::SeparatorText("Floating Combat Text");
                        ImGui::Checkbox("Show Damage Numbers##Player", &settings.playerESP.showDamageNumbers); ImGui::SameLine();
                        ImGui::Checkbox("Show Burst DPS##Player", &settings.playerESP.showBurstDps);

                        // We now use SeparatorText for clean, lightweight sub-grouping.
                        ImGui::SeparatorText("Combat Emphasis");
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

                        ImGui::SeparatorText("Movement Trails");
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
