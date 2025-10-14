#pragma once

#include "../Game/GameEnums.h"
#include "../../libs/ImGui/imgui.h"
#include "ESPConstants.h"
#include "Generated/EnumsAndStructs.h"
#include "../Data/RenderableData.h" // Need this for RenderableEntity, RenderablePlayer, RenderableNpc

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
            case Game::ItemRarity::Junk:       return kx::RarityColors::JUNK;
            case Game::ItemRarity::Common:     return kx::RarityColors::COMMON;
            case Game::ItemRarity::Fine:       return kx::RarityColors::FINE;
            case Game::ItemRarity::Masterwork: return kx::RarityColors::MASTERWORK;
            case Game::ItemRarity::Rare:       return kx::RarityColors::RARE;
            case Game::ItemRarity::Exotic:     return kx::RarityColors::EXOTIC;
            case Game::ItemRarity::Ascended:   return kx::RarityColors::ASCENDED;
            case Game::ItemRarity::Legendary:  return kx::RarityColors::LEGENDARY;
            default:                           return kx::RarityColors::DEFAULT;
        }
    }

    // Get color based on tactical role
    inline ImU32 GetTacticalColor(kx::data::ApiAttribute attribute) {
        switch (attribute) {
            // Offensive Stats -> Red
            case kx::data::ApiAttribute::Power:
            case kx::data::ApiAttribute::Precision:
            case kx::data::ApiAttribute::CritDamage:
            case kx::data::ApiAttribute::ConditionDamage:
                return IM_COL32(255, 80, 80, 255); // Red

            // Defensive Stats -> Blue
            case kx::data::ApiAttribute::Toughness:
            case kx::data::ApiAttribute::Vitality:
                return IM_COL32(30, 144, 255, 255); // Blue

            // Support Stats -> Green
            case kx::data::ApiAttribute::Healing:
            case kx::data::ApiAttribute::BoonDuration:
            case kx::data::ApiAttribute::ConditionDuration:
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
