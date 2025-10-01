#include "ESPContextFactory.h"

#include "ESPConstants.h"
#include "ESPStageRenderer.h"
#include "ESPStyling.h"
#include "../Game/GameEnums.h"

namespace kx {

// Context struct definition (shared with ESPStageRenderer.cpp)
struct EntityRenderContext {
    // Entity data
    const glm::vec3& position;    // World position for real-time screen projection
    float visualDistance;
    float gameplayDistance;
    unsigned int color;
    const std::vector<ColoredDetail>& details;
    float healthPercent;

    // Style and settings
    bool renderBox;
    bool renderDistance;
    bool renderDot;
    bool renderDetails;
    bool renderHealthBar;
    bool renderPlayerName;  // Separate player name rendering from details
    ESPEntityType entityType;
    
    // Screen dimensions for bounds checking
    float screenWidth;
    float screenHeight;
    
    // Player-specific data
    const std::string& playerName;
    const RenderablePlayer* player; // Pointer to the full player object for summary rendering
};

EntityRenderContext ESPContextFactory::CreateContextForPlayer(const RenderablePlayer* player,
                                                             const Settings& settings,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;
    
    return EntityRenderContext{
        player->position,
        player->visualDistance,
        player->gameplayDistance,
        ESPColors::PLAYER,
        details,
        healthPercent,
        settings.playerESP.renderBox,
        settings.playerESP.renderDistance,
        settings.playerESP.renderDot,
        !details.empty(),
        settings.playerESP.renderHealthBar,
        settings.playerESP.renderPlayerName,
        ESPEntityType::Player,
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
    
    static const std::string emptyPlayerName = "";
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
        false,
        ESPEntityType::NPC,
        screenWidth,
        screenHeight,
        emptyPlayerName,
        nullptr
    };
}

EntityRenderContext ESPContextFactory::CreateContextForGadget(const RenderableGadget* gadget,
                                                             const Settings& settings,
                                                             const std::vector<ColoredDetail>& details,
                                                             float screenWidth,
                                                             float screenHeight) {
    static const std::string emptyPlayerName = "";
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
        false,
        false,
        ESPEntityType::Gadget,
        screenWidth,
        screenHeight,
        emptyPlayerName,
        nullptr
    };
}

} // namespace kx
