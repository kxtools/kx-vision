#include "ObjectsTab.h"
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
                    
                    // Resource and Collection Objects
                    ImGui::Checkbox("Resource Nodes", &settings.objectESP.showResourceNodes);
                    ImGui::SameLine();
                    ImGui::Checkbox("Crafting Stations", &settings.objectESP.showCraftingStations);
                    
                    // Navigation and Points of Interest
                    ImGui::Checkbox("Waypoints", &settings.objectESP.showWaypoints);
                    ImGui::SameLine();
                    ImGui::Checkbox("Vistas", &settings.objectESP.showVistas);
                    ImGui::SameLine();
                    ImGui::Checkbox("Portals", &settings.objectESP.showPortals);
                    
                    // Interactive Objects
                    ImGui::Checkbox("Interactables", &settings.objectESP.showInteractables);
                    ImGui::SameLine();
                    ImGui::Checkbox("Doors", &settings.objectESP.showDoors);
                    
                    // Combat and Player-related
                    ImGui::Checkbox("Attack Targets", &settings.objectESP.showAttackTargets);
                    ImGui::SameLine();
                    ImGui::Checkbox("Player Created", &settings.objectESP.showPlayerCreated);
                    
                    ImGui::Separator();
                    ImGui::Text("Quick Selection");
                    if (ImGui::Button("Select Important")) {
                        // Set individual checkboxes based on important gadget types
                        settings.objectESP.showResourceNodes = true;
                        settings.objectESP.showWaypoints = true;
                        settings.objectESP.showVistas = true;
                        settings.objectESP.showAttackTargets = true;
                        settings.objectESP.showInteractables = true;
                        settings.objectESP.showCraftingStations = false;
                        settings.objectESP.showPlayerCreated = false;
                        settings.objectESP.showDoors = false;
                        settings.objectESP.showPortals = false;
                    }
                    ImGui::SameLine();
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
                    }
                    
                    ImGui::Separator();
                    ImGui::Text("Special Filters");
                    ImGui::Checkbox("Hide Depleted Resource Nodes", &settings.hideDepletedNodes);
                    if (ImGui::IsItemHovered()) {
                        ImGui::SetTooltip("Hide resource nodes that cannot be gathered (depleted).");
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}
