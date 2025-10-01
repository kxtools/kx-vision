#include "ObjectsTab.h"
#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"

namespace kx {
    namespace GUI {

        void RenderObjectsTab() {
            if (ImGui::BeginTabItem("Objects")) {
                auto& settings = kx::AppState::Get().GetSettings();
                
                ImGui::Checkbox("Enable Object ESP", &settings.objectESP.enabled);
                
                if (settings.objectESP.enabled) {
                    ImGui::Separator();
                    ImGui::Text("Object Type Filter");

                    // Navigation
                    ImGui::Checkbox("Waypoints", &settings.objectESP.showWaypoints);
                    ImGui::SameLine();
                    ImGui::Checkbox("Vistas", &settings.objectESP.showVistas);
                    ImGui::SameLine();
                    ImGui::Checkbox("Portals", &settings.objectESP.showPortals);

                    // Resources & Crafting
                    ImGui::Checkbox("Resource Nodes", &settings.objectESP.showResourceNodes);
                    ImGui::SameLine();
                    ImGui::Checkbox("Crafting Stations", &settings.objectESP.showCraftingStations);

                    // Interactive
                    ImGui::Checkbox("Interactables", &settings.objectESP.showInteractables); // Chests, etc.
                    ImGui::SameLine();
                    ImGui::Checkbox("Doors", &settings.objectESP.showDoors);
                    ImGui::SameLine();
                    ImGui::Checkbox("Props", &settings.objectESP.showProps); // anvils, jump pads
                    ImGui::SameLine();
                    ImGui::Checkbox("Bounty Boards", &settings.objectESP.showBountyBoards);
                    ImGui::SameLine();
                    ImGui::Checkbox("Rifts", &settings.objectESP.showRifts);

                    // Combat & WvW
                    ImGui::Checkbox("Attack Targets", &settings.objectESP.showAttackTargets);
                    ImGui::SameLine();
                    ImGui::Checkbox("Player Created", &settings.objectESP.showPlayerCreated); // Siege, banners
                    ImGui::SameLine();
                    ImGui::Checkbox("Destructible", &settings.objectESP.showDestructible); // Dummies, siege targets
                    
                    ImGui::Checkbox("Build Sites", &settings.objectESP.showBuildSites);
                    ImGui::SameLine();
                    ImGui::Checkbox("Control Points", &settings.objectESP.showPoints);

                    // Other
                    ImGui::Checkbox("Player Specific", &settings.objectESP.showPlayerSpecific);
                    ImGui::SameLine();
                    ImGui::Checkbox("Generic Triggers", &settings.objectESP.showGeneric);
                    ImGui::SameLine();
                    ImGui::Checkbox("Show Unknown", &settings.objectESP.showUnknown);

                    ImGui::Separator();
                    ImGui::Text("Quick Selection");
                    if (ImGui::Button("Select All")) {
                        // Enable all object types
                        settings.objectESP.showResourceNodes = true;
                        settings.objectESP.showWaypoints = true;
                        settings.objectESP.showVistas = true;
                        settings.objectESP.showCraftingStations = true;
                        settings.objectESP.showAttackTargets = true;
                        settings.objectESP.showPlayerCreated = true;
                        settings.objectESP.showInteractables = true;
                        settings.objectESP.showDoors = true;
                        settings.objectESP.showPortals = true;
                        settings.objectESP.showDestructible = true;
                        settings.objectESP.showPoints = true;
                        settings.objectESP.showPlayerSpecific = true;
                        settings.objectESP.showProps = true;
                        settings.objectESP.showBuildSites = true;
                        settings.objectESP.showBountyBoards = true;
                        settings.objectESP.showRifts = true;
                        settings.objectESP.showGeneric = true;
                        settings.objectESP.showUnknown = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear All")) {
                        // Disable all object types
                        settings.objectESP.showResourceNodes = false;
                        settings.objectESP.showWaypoints = false;
                        settings.objectESP.showVistas = false;
                        settings.objectESP.showCraftingStations = false;
                        settings.objectESP.showAttackTargets = false;
                        settings.objectESP.showPlayerCreated = false;
                        settings.objectESP.showInteractables = false;
                        settings.objectESP.showDoors = false;
                        settings.objectESP.showPortals = false;
                        settings.objectESP.showDestructible = false;
                        settings.objectESP.showPoints = false;
                        settings.objectESP.showPlayerSpecific = false;
                        settings.objectESP.showProps = false;
                        settings.objectESP.showBuildSites = false;
                        settings.objectESP.showBountyBoards = false;
                        settings.objectESP.showRifts = false;
                        settings.objectESP.showGeneric = false;
                        settings.objectESP.showUnknown = false;
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("Special Filters");
                    ImGui::Checkbox("Hide Depleted Resource Nodes", &settings.hideDepletedNodes);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Hide resource nodes that cannot be gathered (depleted).");
                    }

                    ImGui::Separator();
                    RenderCategoryStyleSettings("Object Style", settings.objectESP.renderBox, settings.objectESP.renderDistance, settings.objectESP.renderDot, nullptr, &settings.objectESP.renderDetails);
                }
                ImGui::EndTabItem();
            }
        }
    }
}