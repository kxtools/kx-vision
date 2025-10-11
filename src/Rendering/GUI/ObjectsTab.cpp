#include "ObjectsTab.h"
#include "GuiHelpers.h"
#include "../../../libs/ImGui/imgui.h"
#include "../../Core/AppState.h"
#include <string>
#include <vector>

namespace kx {
    namespace GUI {

        namespace {
            // Local helper to render a checkbox with a unique ID and a tooltip.
            void CheckboxWithTooltip(const char* label, const char* categoryId, bool* value, const char* tooltip) {
                std::string unique_label = std::string(label) + "##" + categoryId;
                ImGui::Checkbox(unique_label.c_str(), value);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip(tooltip);
                }
            }

            // The "Select All / Clear All" helper remains the same.
            void SetAllObjectFilters(ObjectEspSettings& settings, bool value) {
                std::vector<bool*> filters = {
                    &settings.showResourceNodes, &settings.showWaypoints, &settings.showVistas,
                    &settings.showCraftingStations, &settings.showAttackTargets, &settings.showPlayerCreated,
                    &settings.showInteractables, &settings.showDoors, &settings.showPortals,
                    &settings.showDestructible, &settings.showPoints, &settings.showPlayerSpecific,
                    &settings.showProps, &settings.showBuildSites, &settings.showBountyBoards,
                    &settings.showRifts, &settings.showGeneric, &settings.showGeneric2, &settings.showUnknown
                };
                for (bool* filter : filters) {
                    *filter = value;
                }
            }
        } // anonymous namespace

        void RenderObjectsTab() {
            if (ImGui::BeginTabItem("Objects")) {
                auto& settings = kx::AppState::Get().GetSettings();

                ImGui::Checkbox("Enable Object ESP", &settings.objectESP.enabled);

                if (settings.objectESP.enabled) {
                    // This entire section is now collapsible by default.
                    if (ImGui::CollapsingHeader("Object Type Filters"))
                    {
                        ImGui::Indent();

                        const float column1 = 180.0f;
                        const float column2 = 360.0f;

                        // --- Filter Checkboxes (organized into groups) ---
                        CheckboxWithTooltip("Waypoints", "Objects", &settings.objectESP.showWaypoints, "Show map waypoints.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Vistas", "Objects", &settings.objectESP.showVistas, "Show vista locations.");
                        ImGui::SameLine(column2);
                        CheckboxWithTooltip("Portals", "Objects", &settings.objectESP.showPortals, "Show map portals and other teleporters.");
                        // ... (all other checkbox groups remain the same) ...
                        CheckboxWithTooltip("Resource Nodes", "Objects", &settings.objectESP.showResourceNodes, "Show ore, wood, and plant gathering nodes.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Crafting Stations", "Objects", &settings.objectESP.showCraftingStations, "Show all crafting disciplines.");
                        CheckboxWithTooltip("Attack Targets", "Objects", &settings.objectESP.showAttackTargets, "Show world bosses, event structures, and siege targets.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Player Created", "Objects", &settings.objectESP.showPlayerCreated, "Show player-built siege, banners, and other objects.");
                        ImGui::SameLine(column2);
                        CheckboxWithTooltip("Destructible", "Objects", &settings.objectESP.showDestructible, "Show destructible objects like training dummies or walls.");
                        CheckboxWithTooltip("Build Sites", "Objects", &settings.objectESP.showBuildSites, "Show WvW siege build sites.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Control Points", "Objects", &settings.objectESP.showPoints, "Show PvP capture points.");
                        CheckboxWithTooltip("Interactables", "Objects", &settings.objectESP.showInteractables, "Show chests, puzzles, and other general interactive objects.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Doors", "Objects", &settings.objectESP.showDoors, "Show interactive doors and gates.");
                        ImGui::SameLine(column2);
                        CheckboxWithTooltip("Props", "Objects", &settings.objectESP.showProps, "Show miscellaneous props like anvils and jump pads.");
                        CheckboxWithTooltip("Bounty Boards", "Objects", &settings.objectESP.showBountyBoards, "Show bounty and mission boards.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Rifts", "Objects", &settings.objectESP.showRifts, "Show reality rifts from expansions.");
                        ImGui::SameLine(column2);
                        CheckboxWithTooltip("Player Specific", "Objects", &settings.objectESP.showPlayerSpecific, "Show objects created for a specific player.");
                        CheckboxWithTooltip("Generic", "Objects", &settings.objectESP.showGeneric, "Show generic or invisible trigger objects (for debugging).");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Generic 2", "Objects", &settings.objectESP.showGeneric2, "Show generic or invisible trigger objects (for debugging).");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Unknown", "Objects", &settings.objectESP.showUnknown, "Show any object type not explicitly handled.");

                        // Add a separator for better visual structure before the buttons.
                        ImGui::Separator();

                        // --- Quick Selection Buttons (MOVED INSIDE) ---
                        ImGui::Text("Quick Selection:");
                        if (ImGui::Button("Select All", ImVec2(100, 0))) {
                            SetAllObjectFilters(settings.objectESP, true);
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Clear All", ImVec2(100, 0))) {
                            SetAllObjectFilters(settings.objectESP, false);
                        }

                        ImGui::Unindent(); // Match the indent at the start of the block.
                    }

                    ImGui::SeparatorText("Special Filters");
                    CheckboxWithTooltip("Hide Depleted Nodes", "Objects", &settings.hideDepletedNodes, "Hide resource nodes that have already been gathered.");
                    CheckboxWithTooltip("Show Dead Gadgets", "Objects", &settings.objectESP.showDeadGadgets, "Show destroyed gadgets with health (e.g., siege, doors).");

                    // Object Style section remains the same, open by default.
                    if (ImGui::CollapsingHeader("Object Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        const float column1 = 180.0f;
                        const float column2 = 360.0f;

                        ImGui::SeparatorText("Core Visuals");
                        CheckboxWithTooltip("2D Circle", "ObjectStyle", &settings.objectESP.renderCircle, "Render a 2D circle at the object's location.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("3D Sphere", "ObjectStyle", &settings.objectESP.renderSphere, "Render a 3D sphere for the object.");
                        ImGui::SameLine(column2);
                        CheckboxWithTooltip("Show Distance", "ObjectStyle", &settings.objectESP.renderDistance, "Show the distance to the object.");

                        CheckboxWithTooltip("Show Dot", "ObjectStyle", &settings.objectESP.renderDot, "Render a dot at the object's exact location.");

                        ImGui::SeparatorText("Informational Overlays");
                        CheckboxWithTooltip("Show Health Bar", "ObjectStyle", &settings.objectESP.renderHealthBar, "Show health bars for destructible objects and gadgets.");
                        ImGui::SameLine(column1);
                        CheckboxWithTooltip("Show Details", "ObjectStyle", &settings.objectESP.renderDetails, "Show detailed information like the object type.");

                        if (settings.objectESP.renderHealthBar) {
                            ImGui::Indent();
                            CheckboxWithTooltip("Only show damaged", "ObjectStyle", &settings.objectESP.showOnlyDamagedGadgets, "Only show gadgets that are not at 100%% health and not dead.");
                            ImGui::Unindent();
                        }

                        if (settings.objectESP.renderDetails) {
                            if (ImGui::CollapsingHeader("Object Details Filters")) {
                                CheckboxWithTooltip("Type", "ObjectDetails", &settings.objectESP.showDetailGadgetType, "Show the type of gadget (e.g., Resource Node, Waypoint).");
                                ImGui::SameLine(column1);
                                CheckboxWithTooltip("HP", "ObjectDetails", &settings.objectESP.showDetailHealth, "Show current and maximum health if applicable.");
                                ImGui::SameLine(column2);
                                CheckboxWithTooltip("Pos", "ObjectDetails", &settings.objectESP.showDetailPosition, "Show the object's world coordinates.");
                                CheckboxWithTooltip("Node Type", "ObjectDetails", &settings.objectESP.showDetailResourceInfo, "Show resource node type.");
                                ImGui::SameLine(column1);
                                CheckboxWithTooltip("Status", "ObjectDetails", &settings.objectESP.showDetailGatherableStatus, "Show if a resource node is currently gatherable.");
							}
						}
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}