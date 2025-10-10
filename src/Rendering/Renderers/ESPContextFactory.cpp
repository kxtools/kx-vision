#include "ESPContextFactory.h"

#include <Windows.h> // For GetTickCount64
#include "../Combat/CombatStateManager.h" // For CombatStateManager
#include "../Utils/ESPConstants.h" // For CombatEffects
#include "../Core/ESPStageRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"

namespace kx {

EntityRenderContext ESPContextFactory::CreateContextForPlayer(const RenderablePlayer* player,
                                                             const Settings& settings,
                                                             const CombatStateManager& stateManager,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;
    
    float energyPercent = -1.0f;
    if (settings.playerESP.energyDisplayType == EnergyDisplayType::Dodge) {
        if (player->maxEnergy > 0) {
            energyPercent = player->currentEnergy / player->maxEnergy;
        }
    } else { // Special
        if (player->maxSpecialEnergy > 0) {
            energyPercent = player->currentSpecialEnergy / player->maxSpecialEnergy;
        }
    }
    
    // Use attitude-based coloring for players (same as NPCs for semantic consistency)
    unsigned int color;
    switch (player->attitude) {
        case Game::Attitude::Hostile:
            color = ESPColors::NPC_HOSTILE;
            break;
        case Game::Attitude::Friendly:
            color = ESPColors::NPC_FRIENDLY;
            break;
        case Game::Attitude::Neutral:
            color = ESPColors::NPC_NEUTRAL;
            break;
        case Game::Attitude::Indifferent:
            color = ESPColors::NPC_INDIFFERENT;
            break;
        default:
            color = ESPColors::NPC_UNKNOWN;
            break;
    }
    
    return EntityRenderContext{
        player->position,
        player->visualDistance,
        player->gameplayDistance,
        color,
        details,
        healthPercent,
        energyPercent,
        settings.playerESP.renderBox,
        settings.playerESP.renderDistance,
        settings.playerESP.renderDot,
        !details.empty(),
        settings.playerESP.renderHealthBar,
        settings.playerESP.renderEnergyBar,
        settings.playerESP.renderPlayerName,
        ESPEntityType::Player,
        player->attitude,
        Game::CharacterRank::Ambient, // Players are considered Ambient for rank purposes
        screenWidth,
        screenHeight,
        player, // entity pointer
        player->playerName,
        player
    };
}

EntityRenderContext ESPContextFactory::CreateContextForNpc(const RenderableNpc* npc,
                                                          const Settings& settings,
                                                          const CombatStateManager& stateManager,
                                                          const std::vector<ColoredDetail>& details,
                                                          float screenWidth,
                                                          float screenHeight) {
    float healthPercent = (npc->maxHealth > 0) ? (npc->currentHealth / npc->maxHealth) : -1.0f;
    
    // Use attitude-based coloring for NPCs
    unsigned int color;
    switch (npc->attitude) {
        case Game::Attitude::Hostile:
            color = ESPColors::NPC_HOSTILE;
            break;
        case Game::Attitude::Friendly:
            color = ESPColors::NPC_FRIENDLY;
            break;
        case Game::Attitude::Neutral:
            color = ESPColors::NPC_NEUTRAL;
            break;
        case Game::Attitude::Indifferent:
            color = ESPColors::NPC_INDIFFERENT;
            break;
        default:
            color = ESPColors::NPC_UNKNOWN;
            break;
    }
    
    static const std::string emptyPlayerName = "";
    return EntityRenderContext{
        npc->position,
        npc->visualDistance,
        npc->gameplayDistance,
        color,
        details,
        healthPercent,
        -1.0f, // No energy for NPCs
        settings.npcESP.renderBox,
        settings.npcESP.renderDistance,
        settings.npcESP.renderDot,
        settings.npcESP.renderDetails,
        settings.npcESP.renderHealthBar,
        false, // No energy bar for NPCs
        false,
        ESPEntityType::NPC,
        npc->attitude,
        npc->rank,
        screenWidth,
        screenHeight,
        npc, // entity pointer
        emptyPlayerName,
        nullptr
    };
}

namespace {
    // Helper to determine if a gadget's health bar should be hidden based on its type.
    bool ShouldHideHealthBarForGadgetType(Game::GadgetType type) {
        switch (type) {
            // These types often have unstable health values or health is not a meaningful metric,
            // so we hide the bar to prevent visual noise and flickering.
            case Game::GadgetType::Prop:
            case Game::GadgetType::Interact:
            case Game::GadgetType::ResourceNode:
            case Game::GadgetType::Waypoint:
            case Game::GadgetType::MapPortal:
                return true;
            default:
                return false;
        }
    }
} // anonymous namespace

EntityRenderContext ESPContextFactory::CreateContextForGadget(const RenderableGadget* gadget,
                                                             const Settings& settings,
                                                             const CombatStateManager& stateManager,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    static const std::string emptyPlayerName = "";

    bool renderHealthBar = settings.objectESP.renderHealthBar;

    // Do not render health bar for certain gadget types to avoid flickering, or if the base setting is off.
    if (renderHealthBar) {
        if (ShouldHideHealthBarForGadgetType(gadget->type)) {
            renderHealthBar = false;
        }
        else if (gadget->maxHealth > 0) {
            // "Only show damaged" filter: Hide bar if gadget is at full health.
            if (settings.objectESP.showOnlyDamagedGadgets && gadget->currentHealth >= gadget->maxHealth) {
                renderHealthBar = false;
            }
            // "Don't show bar on already-dead gadgets" filter: Hide bar if gadget is dead and animation is over.
            else if (gadget->currentHealth <= 0.0f) {
                const EntityCombatState* state = stateManager.GetState(gadget->address);
                if (!state || state->deathTimestamp == 0 || (GetTickCount64() - state->deathTimestamp) > CombatEffects::DEATH_ANIMATION_TOTAL_DURATION_MS) {
                    renderHealthBar = false;
                }
            }
        }
    }

    return EntityRenderContext{
        gadget->position,
        gadget->visualDistance,
        gadget->gameplayDistance,
        ESPColors::GADGET,
        details,
        gadget->maxHealth > 0 ? (gadget->currentHealth / gadget->maxHealth) : -1.0f,
        -1.0f, // No energy for gadgets
        (settings.objectESP.renderCircle || settings.objectESP.renderSphere),
        settings.objectESP.renderDistance,
        settings.objectESP.renderDot,
        settings.objectESP.renderDetails,
        renderHealthBar,
        false, // No energy bar for gadgets
        false,
        ESPEntityType::Gadget,
        Game::Attitude::Neutral,
        Game::CharacterRank::Normal, // Gadgets don't have ranks
        screenWidth,
        screenHeight,
        gadget, // entity pointer
        emptyPlayerName,
        nullptr
    };
}

} // namespace kx