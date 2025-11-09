#include "ESPContextFactory.h"

#include "../Utils/ESPConstants.h"
#include "../Core/ESPStageRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../Game/GameEnums.h"

namespace kx {

EntityRenderContext ESPContextFactory::CreateContextForPlayer(const RenderablePlayer* player,
                                                             const Settings& settings,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;
    
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
        settings.playerESP.renderBox,
        settings.playerESP.renderDistance,
        settings.playerESP.renderDot,
        !details.empty(),
        settings.playerESP.renderHealthBar,
        settings.playerESP.renderPlayerName,
        ESPEntityType::Player,
        player->attitude,
        screenWidth,
        screenHeight,
        player->playerName,
        player
    };
}

EntityRenderContext ESPContextFactory::CreateContextForNpc(const RenderableNpc* npc,
                                                          const Settings& settings,
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
    
    return EntityRenderContext{
        npc->position,
        npc->visualDistance,
        npc->gameplayDistance,
        color,
        details,
        healthPercent,
        settings.npcESP.renderBox,
        settings.npcESP.renderDistance,
        settings.npcESP.renderDot,
        settings.npcESP.renderDetails,
        settings.npcESP.renderHealthBar,
        true,
        ESPEntityType::NPC,
        npc->attitude,
        screenWidth,
        screenHeight,
        npc->name,
        nullptr
    };
}

EntityRenderContext ESPContextFactory::CreateContextForGadget(const RenderableGadget* gadget,
                                                             const Settings& settings,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    return EntityRenderContext{
        gadget->position,
        gadget->visualDistance,
        gadget->gameplayDistance,
        ESPColors::GADGET,
        details,
        -1.0f,
        settings.objectESP.renderBox,
        settings.objectESP.renderDistance,
        settings.objectESP.renderDot,
        settings.objectESP.renderDetails,
        true,
        false,
        ESPEntityType::Gadget,
        Game::Attitude::Neutral,
        screenWidth,
        screenHeight,
        gadget->name,
        nullptr
    };
}

} // namespace kx
