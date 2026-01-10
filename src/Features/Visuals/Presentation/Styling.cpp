#include "Styling.h"
#include "../../../Game/Data/RenderableData.h"
#include "../../../Rendering/Shared/ColorConstants.h"
#include "../../../Rendering/Shared/ScalingConstants.h"
#include <algorithm>

namespace kx::Styling {

    ImU32 GetRarityColor(Game::ItemRarity rarity) {
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

    ImU32 GetTacticalColor(data::ApiAttribute attribute) {
        switch (attribute) {
            case data::ApiAttribute::Power:
            case data::ApiAttribute::Precision:
            case data::ApiAttribute::CritDamage:
            case data::ApiAttribute::ConditionDamage:
                return IM_COL32(255, 80, 80, 255);

            case data::ApiAttribute::Toughness:
            case data::ApiAttribute::Vitality:
                return IM_COL32(30, 144, 255, 255);

            case data::ApiAttribute::Healing:
            case data::ApiAttribute::BoonDuration:
            case data::ApiAttribute::ConditionDuration:
                return IM_COL32(100, 255, 100, 255);

            default:
                return ESPColors::DEFAULT_TEXT;
        }
    }

    bool ShouldHideCombatUIForGadget(Game::GadgetType type) {
        switch (type) {
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

    ImU32 GetEntityColor(const GameEntity& entity) {
        switch (entity.entityType) {
            case EntityTypes::Player: {
                const auto* p = static_cast<const PlayerEntity*>(&entity);
                switch (p->attitude) {
                    case Game::Attitude::Hostile:     return ESPColors::NPC_HOSTILE;
                    case Game::Attitude::Friendly:    return ESPColors::NPC_FRIENDLY;
                    case Game::Attitude::Neutral:     return ESPColors::NPC_NEUTRAL;
                    case Game::Attitude::Indifferent: return ESPColors::NPC_INDIFFERENT;
                    default:                          return ESPColors::NPC_UNKNOWN;
                }
            }
            case EntityTypes::NPC: {
                const auto* n = static_cast<const NpcEntity*>(&entity);
                 switch (n->attitude) {
                    case Game::Attitude::Hostile:     return ESPColors::NPC_HOSTILE;
                    case Game::Attitude::Friendly:    return ESPColors::NPC_FRIENDLY;
                    case Game::Attitude::Neutral:     return ESPColors::NPC_NEUTRAL;
                    case Game::Attitude::Indifferent: return ESPColors::NPC_INDIFFERENT;
                    default:                          return ESPColors::NPC_UNKNOWN;
                }
            }
            case EntityTypes::Gadget:
                return ESPColors::GADGET;
            case EntityTypes::AttackTarget:
                return ESPColors::GADGET;
            case EntityTypes::Item: {
                const auto* item = static_cast<const RenderableItem*>(&entity);
                return GetRarityColor(item->rarity);
            }
        }
        return ESPColors::NPC_UNKNOWN;
    }

    float GetRankMultiplier(Game::CharacterRank rank) {
        switch (rank) {
            case Game::CharacterRank::Veteran:    return RankMultipliers::VETERAN;
            case Game::CharacterRank::Elite:      return RankMultipliers::ELITE;
            case Game::CharacterRank::Champion:   return RankMultipliers::CHAMPION;
            case Game::CharacterRank::Legendary:  return RankMultipliers::LEGENDARY;
            default:                              return RankMultipliers::NORMAL;
        }
    }

    float GetGadgetHealthMultiplier(float maxHealth) {
        if (maxHealth <= 0.0f) return 1.0f;
        
        const float MIN = GadgetHealthScaling::MIN_MULTIPLIER;
        const float MAX = GadgetHealthScaling::MAX_MULTIPLIER;
        const float HP_CAP = GadgetHealthScaling::HP_TO_REACH_MAX;
        
        float progress = maxHealth / HP_CAP;
        progress = (std::min)(progress, 1.0f);
        
        return MIN + progress * (MAX - MIN);
    }

    float GetDamageNumberFontSizeMultiplier(float damageToDisplay) {
        if (damageToDisplay <= 0.0f) {
            return DamageNumberScaling::MIN_MULTIPLIER;
        }

        const float MIN = DamageNumberScaling::MIN_MULTIPLIER;
        const float MAX = DamageNumberScaling::MAX_MULTIPLIER;
        const float DAMAGE_CAP = DamageNumberScaling::DAMAGE_TO_REACH_MAX;
        
        float progress = damageToDisplay / DAMAGE_CAP;
        progress = (std::min)(progress, 1.0f);

        return MIN + progress * (MAX - MIN);
    }

} // namespace kx::Styling

