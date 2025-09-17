#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "ESPFilter.h"
#include "ESPFeatureRenderer.h"
#include "../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

// Context struct for unified entity rendering
struct EntityRenderContext {
    // Entity data
    const glm::vec3& position;    // World position for real-time screen projection
    float distance;
    unsigned int color;
    const std::vector<std::string>& details;
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
    
    // Player name (for players only)
    const std::string& playerName;
};

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const PooledFrameRenderData& frameData, Camera& camera) {
    // Simple rendering - no filtering logic, just draw everything that was passed in
    RenderPooledPlayers(drawList, screenWidth, screenHeight, frameData.players, camera);
    RenderPooledNpcs(drawList, screenWidth, screenHeight, frameData.npcs, camera);
    RenderPooledGadgets(drawList, screenWidth, screenHeight, frameData.gadgets, camera);
}

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera) {
    // Calculate screen position every frame for smooth movement
    glm::vec2 screenPos;
    if (!ESPMath::WorldToScreen(context.position, camera, context.screenWidth, context.screenHeight, screenPos)) {
        // Entity is off-screen, skip it
        return;
    }
    
    // Screen bounds culling with small margin for partially visible entities
    const float margin = 50.0f;
    if (screenPos.x < -margin || screenPos.x > context.screenWidth + margin || 
        screenPos.y < -margin || screenPos.y > context.screenHeight + margin) {
        return; // Entity is off-screen
    }

    // Calculate distance-based fade alpha
    const auto& settings = AppState::Get().GetSettings();
    float distanceFadeAlpha = CalculateEntityDistanceFadeAlpha(context.distance, 
                                                              settings.espUseDistanceLimit, 
                                                              settings.espRenderDistanceLimit);
    
    // Early exit if entity is fully transparent
    if (distanceFadeAlpha <= 0.0f) {
        return;
    }
    
    // Apply distance fade to entity color
    unsigned int fadedEntityColor = ESPFeatureRenderer::ApplyAlphaToColor(context.color, distanceFadeAlpha);

    // For players and NPCs, prioritize natural health bars over artificial boxes
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    
    // Show standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        ESPFeatureRenderer::RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, fadedEntityColor);
    }

    // Calculate bounding box for entity based on type and distance-based scaling
    float boxHeight, boxWidth;
    
    // Calculate distance-based scaling using settings
    float rawScale = settings.espScaleFactor / (context.distance + 10.0f);
    float clampedScale = std::clamp(rawScale, settings.espMinScale, settings.espMaxScale);
    
    switch (context.entityType) {
        case ESPEntityType::Player:
            // Players: tall rectangle (humanoid) with distance scaling
            boxHeight = BoxDimensions::PLAYER_HEIGHT * clampedScale;
            boxWidth = BoxDimensions::PLAYER_WIDTH * clampedScale;
            // Ensure minimum size for visibility
            if (boxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
                boxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
                boxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
            }
            break;
        case ESPEntityType::NPC:
            // NPCs: square box with distance scaling
            boxHeight = BoxDimensions::NPC_HEIGHT * clampedScale;
            boxWidth = BoxDimensions::NPC_WIDTH * clampedScale;
            // Ensure minimum size for visibility
            if (boxHeight < MinimumSizes::NPC_MIN_HEIGHT) {
                boxHeight = MinimumSizes::NPC_MIN_HEIGHT;
                boxWidth = MinimumSizes::NPC_MIN_WIDTH;
            }
            break;
        case ESPEntityType::Gadget:
            // Gadgets: very small square with half scaling for smaller appearance
            {
                float gadgetScale = clampedScale * 0.5f; // Use half scale for gadgets
                boxHeight = BoxDimensions::GADGET_HEIGHT * gadgetScale;
                boxWidth = BoxDimensions::GADGET_WIDTH * gadgetScale;
                // Ensure minimum size for visibility
                if (boxHeight < MinimumSizes::GADGET_MIN_HEIGHT) {
                    boxHeight = MinimumSizes::GADGET_MIN_HEIGHT;
                    boxWidth = MinimumSizes::GADGET_MIN_WIDTH;
                }
            }
            break;
        default:
            // Fallback to player dimensions with scaling
            boxHeight = BoxDimensions::PLAYER_HEIGHT * clampedScale;
            boxWidth = BoxDimensions::PLAYER_WIDTH * clampedScale;
            // Apply player minimum size
            if (boxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
                boxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
                boxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
            }
            break;
    }
    
    ImVec2 boxMin(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    ImVec2 boxMax(screenPos.x + boxWidth / 2, screenPos.y);
    ImVec2 center(screenPos.x, screenPos.y - boxHeight / 2);

    // Render old-style health bar only if requested and no standalone health bar was shown
    if (context.renderHealthBar && context.healthPercent >= 0.0f && !isLivingEntity) {
        ESPFeatureRenderer::RenderAttachedHealthBar(drawList, boxMin, boxMax, context.healthPercent, distanceFadeAlpha);
    }

    // Render bounding box (should be disabled by default for living entities)
    if (context.renderBox) {
        ESPFeatureRenderer::RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor);
    }

    // Render distance text
    if (context.renderDistance) {
        ESPFeatureRenderer::RenderDistanceText(drawList, center, boxMin, context.distance, distanceFadeAlpha);
    }

    // Render center dot
    if (context.renderDot) {
        if (context.entityType == ESPEntityType::Gadget) {
            // Always render natural white dot for gadgets with distance fade
            ESPFeatureRenderer::RenderNaturalWhiteDot(drawList, screenPos, distanceFadeAlpha);
        } else {
            // Use colored dots for players and NPCs with distance fade
            ESPFeatureRenderer::RenderColoredDot(drawList, screenPos, fadedEntityColor);
        }
    }

    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName && !context.playerName.empty()) {
        ESPFeatureRenderer::RenderPlayerName(drawList, screenPos, context.playerName, fadedEntityColor);
    }

    // Render details text (for all entities when enabled, but not player names for players)
    if (context.renderDetails && !context.details.empty()) {
        ESPFeatureRenderer::RenderDetailsText(drawList, center, boxMax, context.details, distanceFadeAlpha);
    }
}

void ESPStageRenderer::RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                          const std::vector<RenderablePlayer*>& players, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* player : players) {
        if (!player) continue; // Safety check
        
        // All filtering has been done - just render everything
        unsigned int color = ESPColors::PLAYER;

        float healthPercent = -1.0f;
        if (player->maxHealth > 0) {
            healthPercent = player->currentHealth / player->maxHealth;
        }

        std::vector<std::string> details;
        details.reserve(7); // Pre-allocate to avoid reallocations
        if (settings.playerESP.renderDetails) {
            if (!player->playerName.empty()) {
                details.emplace_back("Player: " + player->playerName);
            }
            
            if (player->level > 0) {
                std::string levelText = "Level: " + std::to_string(player->level);
                if (player->scaledLevel != player->level && player->scaledLevel > 0) {
                    levelText += " (" + std::to_string(player->scaledLevel) + ")";
                }
                details.emplace_back(levelText);
            }
            
            if (player->profession != Game::Profession::None) {
                details.emplace_back("Prof: " + ESPFormatting::ProfessionToString(player->profession));
            }
            
            if (player->race != Game::Race::None) {
                details.emplace_back("Race: " + ESPFormatting::RaceToString(player->race));
            }
            
            if (player->maxHealth > 0) {
                details.emplace_back("HP: " + std::to_string(static_cast<int>(player->currentHealth)) + "/" + std::to_string(static_cast<int>(player->maxHealth)));
            }
            
            if (player->maxEnergy > 0) {
                const int energyPercent = static_cast<int>((player->currentEnergy / player->maxEnergy) * 100.0f);
                details.emplace_back("Energy: " + std::to_string(static_cast<int>(player->currentEnergy)) + "/" + std::to_string(static_cast<int>(player->maxEnergy)) + " (" + std::to_string(energyPercent) + "%)");
            }

#ifdef _DEBUG
            char addrStr[32];
            snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", player->address);
            details.emplace_back(addrStr);
#endif
        }

        EntityRenderContext context{
            player->position,  // Use position instead of screenPos for real-time projection
            player->distance,
            color,
            details,
            healthPercent,
            settings.playerESP.renderBox,
            settings.playerESP.renderDistance,
            settings.playerESP.renderDot,
            settings.playerESP.renderDetails,
            settings.playerESP.renderHealthBar,
            settings.playerESP.renderPlayerName,  // Use new player name setting
            ESPEntityType::Player,
            screenWidth,
            screenHeight,
            player->playerName  // Pass player name to context
        };
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                       const std::vector<RenderableNpc*>& npcs, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* npc : npcs) {
        if (!npc) continue; // Safety check
        
        // Use attitude-based coloring for NPCs (minimalistic GW2-style)
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
        
        float healthPercent = -1.0f;
        if (npc->maxHealth > 0) {
            healthPercent = npc->currentHealth / npc->maxHealth;
        }

        std::vector<std::string> details;
        details.reserve(6); // Pre-allocate to avoid reallocations
        if (settings.npcESP.renderDetails) {
            if (!npc->name.empty()) {
                details.emplace_back("NPC: " + npc->name);
            }
            
            if (npc->level > 0) {
                details.emplace_back("Level: " + std::to_string(npc->level));
            }
            
            if (npc->maxHealth > 0) {
                details.emplace_back("HP: " + std::to_string(static_cast<int>(npc->currentHealth)) + "/" + std::to_string(static_cast<int>(npc->maxHealth)));
            }
            
            details.emplace_back("Attitude: " + ESPFormatting::AttitudeToString(npc->attitude));
            details.emplace_back("Rank: " + ESPFormatting::RankToString(npc->rank));

#ifdef _DEBUG
            char addrStr[32];
            snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", npc->address);
            details.emplace_back(addrStr);
#endif
        }

        static const std::string emptyPlayerName = "";
        EntityRenderContext context{
            npc->position,  // Use position instead of screenPos for real-time projection
            npc->distance,
            color,
            details,
            healthPercent,
            settings.npcESP.renderBox,
            settings.npcESP.renderDistance,
            settings.npcESP.renderDot,
            settings.npcESP.renderDetails,
            settings.npcESP.renderHealthBar,
            false,  // NPCs don't have player names
            ESPEntityType::NPC,
            screenWidth,
            screenHeight,
            emptyPlayerName  // Empty string for NPCs (no player name)
        };
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                          const std::vector<RenderableGadget*>& gadgets, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* gadget : gadgets) {
        if (!gadget) continue; // Safety check
        
        unsigned int color = ESPColors::GADGET;

        std::vector<std::string> details;
        details.reserve(3); // Pre-allocate for type and gatherable status
        if (settings.objectESP.renderDetails) {
            details.emplace_back("Type: " + ESPFormatting::GadgetTypeToString(gadget->type));
            if (gadget->isGatherable) {
                details.emplace_back("Status: Gatherable");
            }

#ifdef _DEBUG
            char addrStr[32];
            snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", gadget->address);
            details.emplace_back(addrStr);
#endif
        }

        static const std::string emptyPlayerName = "";
        EntityRenderContext context{
            gadget->position,  // Use position instead of screenPos for real-time projection
            gadget->distance,
            color,
            details,
            -1.0f,  // healthPercent
            settings.objectESP.renderBox,
            settings.objectESP.renderDistance,
            settings.objectESP.renderDot,
            settings.objectESP.renderDetails,
            false,  // renderHealthBar - gadgets don't have health bars
            false,  // renderPlayerName - gadgets don't have player names
            ESPEntityType::Gadget,
            screenWidth,
            screenHeight,
            emptyPlayerName  // Empty string for gadgets (no player name)
        };
        RenderEntity(drawList, context, camera);
    }
}

float ESPStageRenderer::CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    // Use the same logic as ESPFilter for consistency
    return ESPFilter::CalculateDistanceFadeAlpha(distance, useDistanceLimit, distanceLimit);
}

} // namespace kx