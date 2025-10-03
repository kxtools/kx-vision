#include "ObjectsTab.h"
#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include <vector>

namespace kx {
    namespace GUI {

        namespace {
            /**
             * @brief Helper function to set all object type filters to a specific value
             * @param settings Reference to ObjectEspSettings
             * @param value The boolean value to set for all filters
             */
            void SetAllObjectFilters(ObjectEspSettings& settings, bool value) {
                // Array of pointers to all filter boolean members
                std::vector<bool*> filters = {
                    &settings.showResourceNodes,
                    &settings.showWaypoints,
                    &settings.showVistas,
                    &settings.showCraftingStations,
                    &settings.showAttackTargets,
                    &settings.showPlayerCreated,
                    &settings.showInteractables,
                    &settings.showDoors,
                    &settings.showPortals,
                    &settings.showDestructible,
                    &settings.showPoints,
                    &settings.showPlayerSpecific,
                    &settings.showProps,
                    &settings.showBuildSites,
                    &settings.showBountyBoards,
                    &settings.showRifts,
                    &settings.showGeneric,
                    &settings.showUnknown
                };

                // Set all filters to the specified value
                for (bool* filter : filters) {
                    *filter = value;
                }
            }
        }

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
                        SetAllObjectFilters(settings.objectESP, true);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear All")) {
                        SetAllObjectFilters(settings.objectESP, false);
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("Special Filters");
                    ImGui::Checkbox("Hide Depleted Resource Nodes", &settings.hideDepletedNodes);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Hide resource nodes that cannot be gathered (depleted).");
                    }

                    ImGui::Separator();
                    
                    // Custom rendering of style settings with object-specific tooltips
                    if (ImGui::CollapsingHeader("Object Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Visual Elements");
                        
                        ImGui::Checkbox("Show Circle##ObjectStyle", &settings.objectESP.renderBox);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Render a circle around objects (not a box - objects are points in space).");
                        }
                        ImGui::SameLine();
                        ImGui::Checkbox("Show Distance##ObjectStyle", &settings.objectESP.renderDistance);
                        ImGui::SameLine();
                        ImGui::Checkbox("Show Dot##ObjectStyle", &settings.objectESP.renderDot);
                        
                        ImGui::Checkbox("Show Details##ObjectStyle", &settings.objectESP.renderDetails);
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}