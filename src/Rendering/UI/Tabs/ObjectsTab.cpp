#include "ObjectsTab.h"
#include <string>
#include <vector>

#include "AppState.h"
#include "ImGui/imgui.h"
#include "Settings.h"
#include "UI/GuiHelpers.h"

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

            // The "Select All / Clear All" helper - excludes Attack Target List and Items (have their own sections)
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

        void RenderObjectTypeFilters(ObjectEspSettings& settings) {
            if (ImGui::CollapsingHeader("Object Type Filters")) {
                ImGui::Indent();

                const float column1 = 180.0f;
                const float column2 = 360.0f;

                CheckboxWithTooltip("Waypoints", "Objects", &settings.showWaypoints, "Show map waypoints."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Vistas", "Objects", &settings.showVistas, "Show vista locations."); ImGui::SameLine(column2);
                CheckboxWithTooltip("Portals", "Objects", &settings.showPortals, "Show map portals and other teleporters.");
                CheckboxWithTooltip("Resource Nodes", "Objects", &settings.showResourceNodes, "Show ore, wood, and plant gathering nodes."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Crafting Stations", "Objects", &settings.showCraftingStations, "Show all crafting disciplines.");
                CheckboxWithTooltip("Attack Targets", "Objects", &settings.showAttackTargets, "Show world bosses, event structures, and siege targets."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Player Created", "Objects", &settings.showPlayerCreated, "Show player-built siege, banners, and other objects."); ImGui::SameLine(column2);
                CheckboxWithTooltip("Destructible", "Objects", &settings.showDestructible, "Show destructible objects like training dummies or walls.");
                CheckboxWithTooltip("Build Sites", "Objects", &settings.showBuildSites, "Show WvW siege build sites."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Control Points", "Objects", &settings.showPoints, "Show PvP capture points.");
                CheckboxWithTooltip("Interactables", "Objects", &settings.showInteractables, "Show chests, puzzles, and other general interactive objects."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Doors", "Objects", &settings.showDoors, "Show interactive doors and gates."); ImGui::SameLine(column2);
                CheckboxWithTooltip("Props", "Objects", &settings.showProps, "Show miscellaneous props like anvils and jump pads.");
                CheckboxWithTooltip("Bounty Boards", "Objects", &settings.showBountyBoards, "Show bounty and mission boards."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Rifts", "Objects", &settings.showRifts, "Show reality rifts from expansions."); ImGui::SameLine(column2);
                CheckboxWithTooltip("Player Specific", "Objects", &settings.showPlayerSpecific, "Show objects created for a specific player.");
                CheckboxWithTooltip("Generic", "Objects", &settings.showGeneric, "Show generic or invisible trigger objects (for debugging)."); ImGui::SameLine(column1);
                CheckboxWithTooltip("Generic 2", "Objects", &settings.showGeneric2, "Show generic or invisible trigger objects (for debugging)."); ImGui::SameLine(column2);
                CheckboxWithTooltip("Unknown", "Objects", &settings.showUnknown, "Show any object type not explicitly handled.");

                ImGui::Separator();

                ImGui::Text("Quick Selection:");
                if (ImGui::Button("Select All", ImVec2(100, 0))) { SetAllObjectFilters(settings, true); }
                ImGui::SameLine();
                if (ImGui::Button("Clear All", ImVec2(100, 0))) { SetAllObjectFilters(settings, false); }

                ImGui::Unindent();
            }
        }

        void RenderSpecialFilters(Settings& settings) {
            if (ImGui::CollapsingHeader("Special Filters")) {
                CheckboxWithTooltip("Hide Depleted Nodes", "Objects", &settings.hideDepletedNodes, "Hide resource nodes that have already been gathered.");
                CheckboxWithTooltip("Show Dead Gadgets", "Objects", &settings.objectESP.showDeadGadgets, "Show destroyed gadgets with health (e.g., siege, doors).");
            }
        }

        void RenderAttackTargetListSettings(ObjectEspSettings& settings) {
            if (ImGui::CollapsingHeader("Attack Target List")) {
                CheckboxWithTooltip("Show Attack Target List", "AttackTargetList", &settings.showAttackTargetList, 
                    "Show attackable world objects from the attack target list (walls, destructible objects).\n"
                    "Note: This is separate from 'Attack Targets' above, which shows GadgetType::AttackTarget objects.");
                CheckboxWithTooltip("Only Show In Combat", "AttackTargetList", &settings.showAttackTargetListOnlyInCombat, 
                    "Only display attack targets that are currently in combat state.\n"
                    "Filters out idle/inactive targets.");
            }
        }

        void RenderItemListSettings(ObjectEspSettings& settings) {
            if (ImGui::CollapsingHeader("Dropped Items")) {
                ImGui::Indent();
                
                // Main Toggle inside the header
                CheckboxWithTooltip("Show Dropped Items", "Items", &settings.showItems, "Enable rendering of items dropped on the ground.");

                // Rarity Filters (only show if main toggle is on)
                if (settings.showItems) {
                    ImGui::Spacing();
                    ImGui::SeparatorText("Rarity Filters");

                    const float column1 = 180.0f;

                    // Layout rarities in two columns
                    CheckboxWithTooltip("Legendary", "Rarity", &settings.showItemLegendary, "Purple"); ImGui::SameLine(column1);
                    CheckboxWithTooltip("Ascended", "Rarity", &settings.showItemAscended, "Pink");
                    
                    CheckboxWithTooltip("Exotic", "Rarity", &settings.showItemExotic, "Orange"); ImGui::SameLine(column1);
                    CheckboxWithTooltip("Rare", "Rarity", &settings.showItemRare, "Yellow");
                    
                    CheckboxWithTooltip("Masterwork", "Rarity", &settings.showItemMasterwork, "Green"); ImGui::SameLine(column1);
                    CheckboxWithTooltip("Fine", "Rarity", &settings.showItemFine, "Blue");
                    
                    CheckboxWithTooltip("Common", "Rarity", &settings.showItemCommon, "White"); ImGui::SameLine(column1);
                    CheckboxWithTooltip("Junk", "Rarity", &settings.showItemJunk, "Gray");
                }

                ImGui::Unindent();
            }
        }

        void RenderDetailedInformationSettings(ObjectEspSettings& settings) {
            if (ImGui::CollapsingHeader("Detailed Information")) {
                ImGui::Checkbox("Show Details Panel##Object", &settings.renderDetails);
                if (settings.renderDetails) {
                    ImGui::Indent();
                    CheckboxWithTooltip("Type##ObjectDetail", "ObjectDetails", &settings.showDetailGadgetType, "Show the type of gadget (e.g., Resource Node, Waypoint)."); ImGui::SameLine();
                    CheckboxWithTooltip("HP##ObjectDetail", "ObjectDetails", &settings.showDetailHealth, "Show current and maximum health if applicable."); ImGui::SameLine();
                    CheckboxWithTooltip("Pos##ObjectDetail", "ObjectDetails", &settings.showDetailPosition, "Show the object's world coordinates.");
                    CheckboxWithTooltip("Node Type##ObjectDetail", "ObjectDetails", &settings.showDetailResourceInfo, "Show resource node type."); ImGui::SameLine();
                    CheckboxWithTooltip("Status##ObjectDetail", "ObjectDetails", &settings.showDetailGatherableStatus, "Show if a resource node is currently gatherable.");
                    ImGui::Unindent();
                }
            }
        }

        void RenderObjectsTab() {
            if (ImGui::BeginTabItem("Objects")) {
                auto& settings = AppState::Get().GetSettings();

                ImGui::Checkbox("Enable Object ESP", &settings.objectESP.enabled);

                if (settings.objectESP.enabled) {
                    RenderObjectTypeFilters(settings.objectESP);
                    RenderSpecialFilters(settings);
                    RenderItemListSettings(settings.objectESP);
                    RenderAttackTargetListSettings(settings.objectESP);

                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Visual Style", ImGuiTreeNodeFlags_DefaultOpen)) {
                        RenderObjectStyleSettings(settings.objectESP);
                    }

                    RenderDetailedInformationSettings(settings.objectESP);
                }
                ImGui::EndTabItem();
            }
        }
    }
}
