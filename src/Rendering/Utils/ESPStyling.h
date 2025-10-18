#pragma once

#include "../../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include "ESPConstants.h"
#include "Generated/EnumsAndStructs.h"
#include "../Data/RenderableData.h" // Need this for RenderableEntity, RenderablePlayer, RenderableNpc
#include "../Data/ESPEntityTypes.h"

// Forward declaration to avoid circular dependency
namespace kx {
namespace Filtering {
    class EntityFilter;
}
}

namespace kx {

namespace ESPStyling {

    inline ImU32 GetRarityColor(Game::ItemRarity rarity) {
        switch (rarity) {
            case Game::ItemRarity::Junk:       return RarityColors::JUNK;
            case Game::ItemRarity::Common:     return RarityColors::COMMON;
            case Game::ItemRarity::Fine:       return RarityColors::FINE;
            case Game::ItemRarity::Masterwork: return RarityColors::MASTERWORK;
            case Game::ItemRarity::Rare:       return RarityColors::RARE;
            case Game::ItemRarity::Exotic:     return RarityColors::EXOTIC;
            case Game::ItemRarity::Ascended:   return RarityColors::ASCENDED;
            case Game::ItemRarity::Legendary:  return RarityColors::LEGENDARY;
            default:                           return RarityColors::DEFAULT;
        }
    }

    // Get color based on tactical role
    inline ImU32 GetTacticalColor(data::ApiAttribute attribute) {
        switch (attribute) {
            // Offensive Stats -> Red
            case data::ApiAttribute::Power:
            case data::ApiAttribute::Precision:
            case data::ApiAttribute::CritDamage:
            case data::ApiAttribute::ConditionDamage:
                return IM_COL32(255, 80, 80, 255); // Red

            // Defensive Stats -> Blue
            case data::ApiAttribute::Toughness:
            case data::ApiAttribute::Vitality:
                return IM_COL32(30, 144, 255, 255); // Blue

            // Support Stats -> Green
            case data::ApiAttribute::Healing:
            case data::ApiAttribute::BoonDuration:
            case data::ApiAttribute::ConditionDuration:
                return IM_COL32(100, 255, 100, 255); // Green

            default:
                return ESPColors::DEFAULT_TEXT;
        }
    }

    // Helper to determine if a gadget's combat UI (health, dps) should be hidden based on its type.
    inline bool ShouldHideCombatUIForGadget(Game::GadgetType type) {
        switch (type) {
            // These types often have unstable health values or health is not a meaningful metric,
            // so we hide the bar to prevent visual noise and flickering.
            case Game::GadgetType::Prop:
            case Game::GadgetType::Interact:
            case Game::GadgetType::ResourceNode:
            case Game::GadgetType::Waypoint:
            case Game::GadgetType::MapPortal:
            case Game::GadgetType::Generic:
            case Game::GadgetType::Generic2:
            case Game::GadgetType::Crafting:
                return true;
            default:
                return false;
        }
    }

    inline ImU32 GetEntityColor(const RenderableEntity& entity) {
        switch (entity.entityType) {
            case ESPEntityType::Player: {
                const auto* p = static_cast<const RenderablePlayer*>(&entity);
                switch (p->attitude) {
                    case Game::Attitude::Hostile:     return ESPColors::NPC_HOSTILE;
                    case Game::Attitude::Friendly:    return ESPColors::NPC_FRIENDLY;
                    case Game::Attitude::Neutral:     return ESPColors::NPC_NEUTRAL;
                    case Game::Attitude::Indifferent: return ESPColors::NPC_INDIFFERENT;
                    default:                          return ESPColors::NPC_UNKNOWN;
                }
            }
            case ESPEntityType::NPC: {
                const auto* n = static_cast<const RenderableNpc*>(&entity);
                 switch (n->attitude) {
                    case Game::Attitude::Hostile:     return ESPColors::NPC_HOSTILE;
                    case Game::Attitude::Friendly:    return ESPColors::NPC_FRIENDLY;
                    case Game::Attitude::Neutral:     return ESPColors::NPC_NEUTRAL;
                    case Game::Attitude::Indifferent: return ESPColors::NPC_INDIFFERENT;
                    default:                          return ESPColors::NPC_UNKNOWN;
                }
            }
            case ESPEntityType::Gadget:
                return ESPColors::GADGET;
        }
        return ESPColors::NPC_UNKNOWN; // Fallback
    }

} // namespace ESPStyling

} // namespace kx
