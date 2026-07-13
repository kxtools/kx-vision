#include "EntityFilter.h"
#include "../Core/VisualFrameTypes.h"
#include "../../../Game/Data/FrameData.h"

#include "FilterSettings.h"
#include "../Settings/VisualsSettings.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Game/Services/Camera/Camera.h"
#include "../Combat/VisualCombatConstants.h"
#include "../../../Game/Services/Combat/CombatObservation.h"

namespace kx {

namespace { // Anonymous namespace for local helpers

    bool IsDeathAnimationPlaying(const GameEntity* entity, const FrameContext& context, uint64_t now) {
        const auto it = context.combatObservations.find(entity->GetCombatKey());
        const CombatObservation* state = it != context.combatObservations.end() ? &it->second : nullptr;
        if (!state || state->deathTimestamp == 0) {
            return false;
        }
        return CombatElapsedMs(now, state->deathTimestamp) <= VisualCombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS;
    }

    /**
     * @brief Performs common filtering logic applicable to all entity types.
     * @return True if the entity passes common filters, false otherwise.
     */
    bool PassesCommonFilters(
        GameEntity* entity,
        const glm::vec3& cameraPos,
        const glm::vec3& playerPos,
        const FrameContext& context
    ) {
        if (!entity || !entity->isValid) {
            return false;
        }

        // Guardrail: published snapshot entities are immutable on render thread. Keep
        // camera/player-derived metrics local; writing entity distance fields creates a race.
        const float gameplayDistance = glm::length(entity->position - playerPos);

        float activeLimit = context.settings.distance.GetActiveDistanceLimit(entity->entityType, context.isInWvW);
        if (activeLimit > 0.0f && gameplayDistance > activeLimit) {
            return false;
        }

        return true;
    }

} // anonymous namespace

void EntityFilter::FilterPooledData(const FrameGameData& extractedData, const FrameContext& context,
                                 const VisualsConfiguration& visualsConfig, FrameGameData& filteredData) {
    filteredData.Reset();
    
    const glm::vec3 playerPos = context.camera.GetPlayerPosition();
    const glm::vec3 cameraPos = context.camera.GetCameraPosition();
    
    // Filter players
    if (visualsConfig.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (PlayerEntity* player : extractedData.players) {
            // Call the common helper function first
            if (!PassesCommonFilters(player, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform player-specific filtering
            if (player->isLocalPlayer && !visualsConfig.playerESP.showLocalPlayer) continue;
            
            if (player->currentHealth <= 0.0f && !IsDeathAnimationPlaying(player, context, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderPlayer(player->attitude, visualsConfig.playerESP)) continue;
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs
    if (visualsConfig.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (NpcEntity* npc : extractedData.npcs) {
            // Call the common helper function first
            if (!PassesCommonFilters(npc, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform NPC-specific filtering
            if (npc->currentHealth <= 0.0f && !visualsConfig.npcESP.showDeadNpcs && !IsDeathAnimationPlaying(npc, context, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderNpc(npc->attitude, npc->rank, visualsConfig.npcESP)) continue;
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets
    if (visualsConfig.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (GadgetEntity* gadget : extractedData.gadgets) {
            // Call the common helper function first
            if (!PassesCommonFilters(gadget, cameraPos, playerPos, context)) {
                continue;
            }

            // Now, perform gadget-specific filtering
            if (gadget->maxHealth > 0 && gadget->currentHealth <= 0.0f && !visualsConfig.objectESP.showDeadGadgets && !IsDeathAnimationPlaying(gadget, context, context.now)) {
                continue;
            }

            if (visualsConfig.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderGadget(gadget->type, visualsConfig.objectESP)) continue;
            
            // Note: Max height check is handled in context factory to disable box rendering only
            // Entity is still rendered with other visualizations (circles, dots, details, etc.)
            
            filteredData.gadgets.push_back(gadget);
        }
    }
    
    // Filter attack targets
    if (visualsConfig.objectESP.enabled && visualsConfig.objectESP.showAttackTargetList) {
        filteredData.attackTargets.reserve(extractedData.attackTargets.size());
        for (AttackTargetEntity* attackTarget : extractedData.attackTargets) {
            // Call the common helper function first
            if (!PassesCommonFilters(attackTarget, cameraPos, playerPos, context)) {
                continue;
            }

            // Filter by combat state if enabled
            if (visualsConfig.objectESP.showAttackTargetListOnlyInCombat) {
                if (attackTarget->combatState != Game::AttackTargetCombatState::InCombat) {
                    continue;
                }
            }

            // Note: Max height check is handled in context factory to disable box rendering only
            // Entity is still rendered with other visualizations (circles, dots, details, etc.)
            
            filteredData.attackTargets.push_back(attackTarget);
        }
    }
    
    // Filter items
    if (visualsConfig.objectESP.enabled && visualsConfig.objectESP.showItems) {
        filteredData.items.reserve(extractedData.items.size());
        for (ItemEntity* item : extractedData.items) {
            // Call the common helper function first
            if (!PassesCommonFilters(item, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Filter by rarity
            bool shouldShow = false;
            switch (item->rarity) {
                case Game::ItemRarity::Junk:
                    shouldShow = visualsConfig.objectESP.showItemJunk;
                    break;
                case Game::ItemRarity::Common:
                    shouldShow = visualsConfig.objectESP.showItemCommon;
                    break;
                case Game::ItemRarity::Fine:
                    shouldShow = visualsConfig.objectESP.showItemFine;
                    break;
                case Game::ItemRarity::Masterwork:
                    shouldShow = visualsConfig.objectESP.showItemMasterwork;
                    break;
                case Game::ItemRarity::Rare:
                    shouldShow = visualsConfig.objectESP.showItemRare;
                    break;
                case Game::ItemRarity::Exotic:
                    shouldShow = visualsConfig.objectESP.showItemExotic;
                    break;
                case Game::ItemRarity::Ascended:
                    shouldShow = visualsConfig.objectESP.showItemAscended;
                    break;
                case Game::ItemRarity::Legendary:
                    shouldShow = visualsConfig.objectESP.showItemLegendary;
                    break;
                case Game::ItemRarity::None:
                default:
                    shouldShow = true; // Show items with no rarity
                    break;
            }
            
            if (!shouldShow) {
                continue;
            }
            
            filteredData.items.push_back(item);
        }
    }
}

} // namespace kx
