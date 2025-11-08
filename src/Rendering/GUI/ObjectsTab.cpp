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
                    &settings.showRifts, &settings.showGeneric, &settings.showGeneric2, &settings.showUnknown,
                    &settings.showAttackTargetList
                };
                for (bool* filter : filters) {
                    *filter = value;
                }
            }
        } // anonymous namespace

        void RenderObjectsTab() {
            if (ImGui::BeginTabItem("Objects")) {
                auto& settings = AppState::Get().GetSettings();

                ImGui::Checkbox("Enable Object ESP", &settings.objectESP.enabled);

                if (settings.objectESP.enabled) {
                    if (ImGui::CollapsingHeader("Object Type Filters"))
                    {
                        ImGui::Indent();

                        const float column1 = 180.0f;
                        const float column2 = 360.0f;

                        CheckboxWithTooltip("Waypoints", "Objects", &settings.objectESP.showWaypoints, "Show map waypoints."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Vistas", "Objects", &settings.objectESP.showVistas, "Show vista locations."); ImGui::SameLine(column2);
                        CheckboxWithTooltip("Portals", "Objects", &settings.objectESP.showPortals, "Show map portals and other teleporters.");
                        CheckboxWithTooltip("Resource Nodes", "Objects", &settings.objectESP.showResourceNodes, "Show ore, wood, and plant gathering nodes."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Crafting Stations", "Objects", &settings.objectESP.showCraftingStations, "Show all crafting disciplines.");
                        CheckboxWithTooltip("Attack Targets", "Objects", &settings.objectESP.showAttackTargets, "Show world bosses, event structures, and siege targets."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Player Created", "Objects", &settings.objectESP.showPlayerCreated, "Show player-built siege, banners, and other objects."); ImGui::SameLine(column2);
                        CheckboxWithTooltip("Destructible", "Objects", &settings.objectESP.showDestructible, "Show destructible objects like training dummies or walls.");
                        CheckboxWithTooltip("Build Sites", "Objects", &settings.objectESP.showBuildSites, "Show WvW siege build sites."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Control Points", "Objects", &settings.objectESP.showPoints, "Show PvP capture points.");
                        CheckboxWithTooltip("Interactables", "Objects", &settings.objectESP.showInteractables, "Show chests, puzzles, and other general interactive objects."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Doors", "Objects", &settings.objectESP.showDoors, "Show interactive doors and gates."); ImGui::SameLine(column2);
                        CheckboxWithTooltip("Props", "Objects", &settings.objectESP.showProps, "Show miscellaneous props like anvils and jump pads.");
                        CheckboxWithTooltip("Bounty Boards", "Objects", &settings.objectESP.showBountyBoards, "Show bounty and mission boards."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Rifts", "Objects", &settings.objectESP.showRifts, "Show reality rifts from expansions."); ImGui::SameLine(column2);
                        CheckboxWithTooltip("Player Specific", "Objects", &settings.objectESP.showPlayerSpecific, "Show objects created for a specific player.");
                        CheckboxWithTooltip("Generic", "Objects", &settings.objectESP.showGeneric, "Show generic or invisible trigger objects (for debugging)."); ImGui::SameLine(column1);
                        CheckboxWithTooltip("Generic 2", "Objects", &settings.objectESP.showGeneric2, "Show generic or invisible trigger objects (for debugging).");
                        CheckboxWithTooltip("Unknown", "Objects", &settings.objectESP.showUnknown, "Show any object type not explicitly handled.");
                        CheckboxWithTooltip("Attack Target List", "Objects", &settings.objectESP.showAttackTargetList, "Show attackable world objects from the attack target list (walls, destructible objects).");

                        ImGui::Separator();

                        ImGui::Text("Quick Selection:");
                        if (ImGui::Button("Select All", ImVec2(100, 0))) { SetAllObjectFilters(settings.objectESP, true); }
                        ImGui::SameLine();
                        if (ImGui::Button("Clear All", ImVec2(100, 0))) { SetAllObjectFilters(settings.objectESP, false); }

                        ImGui::Unindent();
                    }

                    if (ImGui::CollapsingHeader("Special Filters")) {
                        CheckboxWithTooltip("Hide Depleted Nodes", "Objects", &settings.hideDepletedNodes, "Hide resource nodes that have already been gathered.");
                        CheckboxWithTooltip("Show Dead Gadgets", "Objects", &settings.objectESP.showDeadGadgets, "Show destroyed gadgets with health (e.g., siege, doors).");
                    }

                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Visual Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        RenderObjectStyleSettings(settings.objectESP);
                    }

                    if (ImGui::CollapsingHeader("Detailed Information")) {
                        ImGui::Checkbox("Show Details Panel##Object", &settings.objectESP.renderDetails);
                        if (settings.objectESP.renderDetails) {
                            ImGui::Indent();
                            CheckboxWithTooltip("Type##ObjectDetail", "ObjectDetails", &settings.objectESP.showDetailGadgetType, "Show the type of gadget (e.g., Resource Node, Waypoint)."); ImGui::SameLine();
                            CheckboxWithTooltip("HP##ObjectDetail", "ObjectDetails", &settings.objectESP.showDetailHealth, "Show current and maximum health if applicable."); ImGui::SameLine();
                            CheckboxWithTooltip("Pos##ObjectDetail", "ObjectDetails", &settings.objectESP.showDetailPosition, "Show the object's world coordinates.");
                            CheckboxWithTooltip("Node Type##ObjectDetail", "ObjectDetails", &settings.objectESP.showDetailResourceInfo, "Show resource node type."); ImGui::SameLine();
                            CheckboxWithTooltip("Status##ObjectDetail", "ObjectDetails", &settings.objectESP.showDetailGatherableStatus, "Show if a resource node is currently gatherable.");
                            ImGui::Unindent();
                        }
                    }
                }
                ImGui::EndTabItem();
            }
        }
    }
}
