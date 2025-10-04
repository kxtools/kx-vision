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
     * @brief Check if a player should be rendered based on attitude
     */
    static bool ShouldRenderPlayer(Game::Attitude attitude, const PlayerEspSettings& settings) {
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

};

} // namespace Filtering
} // namespace kx
