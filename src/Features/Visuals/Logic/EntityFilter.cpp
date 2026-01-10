#include "EntityFilter.h"

#include "FilterSettings.h"
#include "../Settings/VisualsSettings.h"
#include "../../../Game/Data/EntityData.h"
#include "../../../Core/Services/Combat/CombatStateManager.h"
#include "../../../Game/Services/Camera/Camera.h"
#include "../../../Core/Services/Combat/CombatConstants.h"
#include "../../../Core/Settings.h"

namespace kx {

namespace { // Anonymous namespace for local helpers

    bool IsDeathAnimationPlaying(const GameEntity* entity, const CombatStateManager& stateManager, uint64_t now) {
        const EntityCombatState* state = stateManager.GetState(entity->GetCombatKey());
        if (!state || state->deathTimestamp == 0) {
            return false;
        }
        return (now - state->deathTimestamp) <= CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS;
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

        entity->visualDistance = glm::length(entity->position - cameraPos);
        entity->gameplayDistance = glm::length(entity->position - playerPos);

        float activeLimit = context.settings.distance.GetActiveDistanceLimit(entity->entityType, context.isInWvW);
        if (activeLimit > 0.0f && entity->gameplayDistance > activeLimit) {
            return false;
        }

        return true;
    }

} // anonymous namespace

void EntityFilter::FilterPooledData(const FrameGameData& extractedData, const FrameContext& context,
                                 FrameGameData& filteredData) {
    filteredData.Reset();
    
    const glm::vec3 playerPos = context.camera.GetPlayerPosition();
    const glm::vec3 cameraPos = context.camera.GetCameraPosition();
    
    // Filter players
    if (context.visualsSettings.playerESP.enabled) {
        filteredData.players.reserve(extractedData.players.size());
        for (PlayerEntity* player : extractedData.players) {
            // Call the common helper function first
            if (!PassesCommonFilters(player, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform player-specific filtering
            if (player->isLocalPlayer && !context.visualsSettings.playerESP.showLocalPlayer) continue;
            
            if (player->currentHealth <= 0.0f && !IsDeathAnimationPlaying(player, context.stateManager, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderPlayer(player->attitude, context.visualsSettings.playerESP)) continue;
            
            filteredData.players.push_back(player);
        }
    }
    
    // Filter NPCs
    if (context.visualsSettings.npcESP.enabled) {
        filteredData.npcs.reserve(extractedData.npcs.size());
        for (NpcEntity* npc : extractedData.npcs) {
            // Call the common helper function first
            if (!PassesCommonFilters(npc, cameraPos, playerPos, context)) {
                continue;
            }
            
            // Now, perform NPC-specific filtering
            if (npc->currentHealth <= 0.0f && !context.visualsSettings.npcESP.showDeadNpcs && !IsDeathAnimationPlaying(npc, context.stateManager, context.now)) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderNpc(npc->attitude, npc->rank, context.visualsSettings.npcESP)) continue;
            
            filteredData.npcs.push_back(npc);
        }
    }
    
    // Filter gadgets
    if (context.visualsSettings.objectESP.enabled) {
        filteredData.gadgets.reserve(extractedData.gadgets.size());
        for (GadgetEntity* gadget : extractedData.gadgets) {
            // Call the common helper function first
            if (!PassesCommonFilters(gadget, cameraPos, playerPos, context)) {
                continue;
            }

            // Now, perform gadget-specific filtering
            if (gadget->maxHealth > 0 && gadget->currentHealth <= 0.0f && !context.visualsSettings.objectESP.showDeadGadgets && !IsDeathAnimationPlaying(gadget, context.stateManager, context.now)) {
                continue;
            }

            if (context.visualsSettings.hideDepletedNodes && gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
                continue;
            }
            
            if (!Filtering::FilterSettings::ShouldRenderGadget(gadget->type, context.visualsSettings.objectESP)) continue;
            
            // Note: Max height check is handled in context factory to disable box rendering only
            // Entity is still rendered with other visualizations (circles, dots, details, etc.)
            
            filteredData.gadgets.push_back(gadget);
        }
    }
    
    // Filter attack targets
    if (context.visualsSettings.objectESP.enabled && context.visualsSettings.objectESP.showAttackTargetList) {
        filteredData.attackTargets.reserve(extractedData.attackTargets.size());
        for (AttackTargetEntity* attackTarget : extractedData.attackTargets) {
            // Call the common helper function first
            if (!PassesCommonFilters(attackTarget, cameraPos, playerPos, context)) {
                continue;
            }

            // Filter by combat state if enabled
            if (context.visualsSettings.objectESP.showAttackTargetListOnlyInCombat) {
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
    if (context.visualsSettings.objectESP.enabled && context.visualsSettings.objectESP.showItems) {
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
                    shouldShow = context.visualsSettings.objectESP.showItemJunk;
                    break;
                case Game::ItemRarity::Common:
                    shouldShow = context.visualsSettings.objectESP.showItemCommon;
                    break;
                case Game::ItemRarity::Fine:
                    shouldShow = context.visualsSettings.objectESP.showItemFine;
                    break;
                case Game::ItemRarity::Masterwork:
                    shouldShow = context.visualsSettings.objectESP.showItemMasterwork;
                    break;
                case Game::ItemRarity::Rare:
                    shouldShow = context.visualsSettings.objectESP.showItemRare;
                    break;
                case Game::ItemRarity::Exotic:
                    shouldShow = context.visualsSettings.objectESP.showItemExotic;
                    break;
                case Game::ItemRarity::Ascended:
                    shouldShow = context.visualsSettings.objectESP.showItemAscended;
                    break;
                case Game::ItemRarity::Legendary:
                    shouldShow = context.visualsSettings.objectESP.showItemLegendary;
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
