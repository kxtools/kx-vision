#pragma once

#include "../Game/GameEnums.h"
#include "../Core/AppState.h"

namespace kx {
namespace Filtering {

/**
 * @brief Enhanced filtering utilities using the new game enums
 */
class EntityFilter {
public:
    /**
     * @brief Check if an NPC should be rendered based on attitude
     */
    static bool ShouldRenderNpc(Game::Attitude attitude, const NpcEspSettings& settings) {
        switch (attitude) {
            case Game::Attitude::Friendly:
                return settings.showFriendly;
            case Game::Attitude::Hostile:
                return settings.showHostile;
            case Game::Attitude::Neutral:
                return settings.showNeutral;
            case Game::Attitude::Indifferent:
                return settings.showIndifferent;
            default:
                return true; // Show unknown attitudes by default
        }
    }

    /**
     * @brief Check if a gadget should be rendered based on type
     */
    static bool ShouldRenderGadget(Game::GadgetType type, const ObjectEspSettings& settings) {
        switch (type) {
            case Game::GadgetType::ResourceNode:
                return settings.showResourceNodes;
            case Game::GadgetType::Waypoint:
                return settings.showWaypoints;
            case Game::GadgetType::Vista:
                return settings.showVistas;
            case Game::GadgetType::Crafting:
                return settings.showCraftingStations;
            case Game::GadgetType::AttackTarget:
                return settings.showAttackTargets;
            case Game::GadgetType::PlayerCreated:
                return settings.showPlayerCreated;
            case Game::GadgetType::Interact:
                return settings.showInteractables;
            case Game::GadgetType::Door:
                return settings.showDoors;
            case Game::GadgetType::MapPortal:
                return settings.showPortals;
            case Game::GadgetType::Destructible:
                return settings.showDestructible;
            case Game::GadgetType::Point:
                return settings.showPoints;
            case Game::GadgetType::PlayerSpecific:
                return settings.showPlayerSpecific;
            case Game::GadgetType::Prop:
                return settings.showProps;
            case Game::GadgetType::BuildSite:
                return settings.showBuildSites;
            case Game::GadgetType::BountyBoard:
                return settings.showBountyBoards;
            case Game::GadgetType::Rift:
                return settings.showRifts;
            case Game::GadgetType::Generic:
                return settings.showGeneric;
            default:
                return settings.showUnknown;
        }
    }

    /**
     * @brief Get priority level for rendering order (higher = render first/more prominent)
     */
    static int GetRenderPriority(Game::GadgetType type) {
        switch (type) {
            case Game::GadgetType::AttackTarget:
                return 100; // Highest priority - bosses, etc.
            case Game::GadgetType::ResourceNode:
                return 80;  // High priority - gathering
            case Game::GadgetType::Vista:
                return 70;  // Medium-high - exploration
            case Game::GadgetType::Waypoint:
                return 60;  // Medium - travel
            case Game::GadgetType::Interact:
                return 50;  // Medium - general interaction
            case Game::GadgetType::Crafting:
                return 40;  // Medium-low - crafting
            case Game::GadgetType::PlayerCreated:
                return 30;  // Low - player objects
            case Game::GadgetType::Door:
                return 20;  // Lower - environment
            default:
                return 10;  // Lowest - everything else
        }
    }

    /**
     * @brief Check if a profession indicates a support role
     */
    static bool IsSupportProfession(Game::Profession profession) {
        switch (profession) {
            case Game::Profession::Guardian:
            case Game::Profession::Engineer: // Has healing/support builds
            case Game::Profession::Ranger:   // Druid healing
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Check if a profession is primarily DPS focused
     */
    static bool IsDpsProfession(Game::Profession profession) {
        switch (profession) {
            case Game::Profession::Thief:
            case Game::Profession::Elementalist:
            case Game::Profession::Necromancer:
                return true;
            default:
                return false;
        }
    }

    /**
     * @brief Get threat level based on attitude and profession
     */
    static int GetThreatLevel(Game::Attitude attitude, Game::Profession profession) {
        int baseThreat = 0;
        
        switch (attitude) {
            case Game::Attitude::Hostile:
                baseThreat = 100;
                break;
            case Game::Attitude::Indifferent:
                baseThreat = 50;
                break;
            case Game::Attitude::Neutral:
                baseThreat = 25;
                break;
            case Game::Attitude::Friendly:
                baseThreat = 0;
                break;
        }

        // Modify based on profession capabilities
        if (IsDpsProfession(profession)) {
            baseThreat += 20;
        } else if (IsSupportProfession(profession)) {
            baseThreat += 10; // Support can be dangerous too
        }

        return baseThreat;
    }

    /**
     * @brief Advanced filtering: Check if entity should be visible based on context
     */
    static bool ShouldShowInContext(Game::GadgetType type, bool inCombat, bool nearWaypoint) {
        // Hide non-essential objects during combat
        if (inCombat) {
            switch (type) {
                case Game::GadgetType::AttackTarget:
                case Game::GadgetType::ResourceNode:
                    return true;
                case Game::GadgetType::Vista:
                case Game::GadgetType::Crafting:
                case Game::GadgetType::Door:
                    return false;
                default:
                    return Game::EnumHelpers::IsImportantGadgetType(type);
            }
        }

        // Near waypoints, prioritize travel-related objects
        if (nearWaypoint) {
            switch (type) {
                case Game::GadgetType::Waypoint:
                case Game::GadgetType::MapPortal:
                    return true;
                default:
                    return !Game::EnumHelpers::IsImportantGadgetType(type);
            }
        }

        return true; // Show everything in normal context
    }
};

} // namespace Filtering
} // namespace kx
