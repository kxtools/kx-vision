#pragma once

#include "../../Game/GameEnums.h"
#include "../Core/AppState.h"

namespace kx {
namespace Filtering {

/**
 * @brief Enhanced filtering utilities using the new game enums
 */
class EntityFilter {
    static bool ShouldRenderByAttitude(Game::Attitude attitude, const AttitudeSettings& settings) {
        switch (attitude) {
            case Game::Attitude::Friendly:    return settings.showFriendly;
            case Game::Attitude::Hostile:     return settings.showHostile;
            case Game::Attitude::Neutral:     return settings.showNeutral;
            case Game::Attitude::Indifferent: return settings.showIndifferent;
            default:                          return true;
        }
    }

public:
    /**
     * @brief Check if a player should be rendered based on attitude
     */
    static bool ShouldRenderPlayer(Game::Attitude attitude, const PlayerEspSettings& settings) {
        // Just call the helper directly
        return ShouldRenderByAttitude(attitude, settings);
    }

    /**
     * @brief Check if an NPC should be rendered based on attitude and rank
     */
    static bool ShouldRenderNpc(Game::Attitude attitude, Game::CharacterRank rank, const NpcEspSettings& settings) {
        // Attitude filter first
        if (!ShouldRenderByAttitude(attitude, settings)) {
            return false;
        }

        // Rank filter (this part remains unchanged)
        switch (rank) {
            case Game::CharacterRank::Legendary:
                return settings.showLegendary;
            case Game::CharacterRank::Champion:
                return settings.showChampion;
            case Game::CharacterRank::Elite:
                return settings.showElite;
            case Game::CharacterRank::Veteran:
                return settings.showVeteran;
            case Game::CharacterRank::Ambient:
                return settings.showAmbient;
            case Game::CharacterRank::Normal:
				return settings.showNormal;
            default:
                return true;
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
            case Game::GadgetType::Generic2:
                return settings.showGeneric2;
            default:
                return settings.showUnknown;
        }
    }

};

} // namespace Filtering
} // namespace kx
