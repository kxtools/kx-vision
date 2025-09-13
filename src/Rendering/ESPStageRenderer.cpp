#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "ESPFilter.h"
#include "../../libs/ImGui/imgui.h"
#include <cmath>
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
    unsigned int fadedEntityColor = ApplyAlphaToColor(context.color, distanceFadeAlpha);

    // For players and NPCs, prioritize natural health bars over artificial boxes
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    
    // Show standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, fadedEntityColor);
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
        RenderHealthBar(drawList, boxMin, boxMax, context.healthPercent, distanceFadeAlpha);
    }

    // Render bounding box (should be disabled by default for living entities)
    if (context.renderBox) {
        RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor);
    }

    // Render distance text
    if (context.renderDistance) {
        RenderDistanceText(drawList, center, boxMin, context.distance, distanceFadeAlpha);
    }

    // Render center dot
    if (context.renderDot) {
        if (context.entityType == ESPEntityType::Gadget) {
            // Always render natural white dot for gadgets with distance fade
            RenderNaturalWhiteDot(drawList, screenPos, distanceFadeAlpha);
        } else {
            // Use colored dots for players and NPCs with distance fade
            RenderCenterDot(drawList, screenPos, fadedEntityColor);
        }
    }

    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName && !context.playerName.empty()) {
        RenderPlayerName(drawList, screenPos, context.playerName, fadedEntityColor);
    }

    // Render details text (for all entities when enabled, but not player names for players)
    if (context.renderDetails && !context.details.empty()) {
        RenderDetailsText(drawList, center, boxMax, context.details, distanceFadeAlpha);
    }
}

void ESPStageRenderer::RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent, float fadeAlpha) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    const float barWidth = 4.0f;
    const float barHeight = boxMax.y - boxMin.y;
    
    ImVec2 barMin(boxMin.x - barWidth - 2.0f, boxMin.y);
    ImVec2 barMax(boxMin.x - 2.0f, boxMax.y);
    
    // Background with fade alpha
    unsigned int bgAlpha = static_cast<unsigned int>(150 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha));
    
    // Health bar - fill from bottom to top with fade alpha
    ImVec2 healthBarMin(barMin.x, barMax.y - (barHeight * healthPercent));
    ImVec2 healthBarMax(barMax.x, barMax.y);
    unsigned int healthAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    unsigned int healthColor = IM_COL32(
        static_cast<int>(255 * (1.0f - healthPercent)),
        static_cast<int>(255 * healthPercent),
        0, healthAlpha
    );
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor);
    
    // Border with fade alpha
    unsigned int borderAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32(255, 255, 255, borderAlpha));
}

void ESPStageRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos, float healthPercent, unsigned int entityColor) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Health bar dimensions - more natural looking
    const float barWidth = 40.0f;
    const float barHeight = 6.0f;
    const float yOffset = 15.0f; // Distance below the center point
    
    // Position the health bar below the entity center
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);
    
    // Background with subtle transparency and distance fade
    unsigned int bgAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, bgAlpha), 1.0f);
    
    // Health fill - horizontal bar
    float healthWidth = barWidth * healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);
    
    // Health color: green -> yellow -> red based on percentage with distance fade
    unsigned int healthAlpha = static_cast<unsigned int>(160 * fadeAlpha);
    unsigned int healthColor;
    if (healthPercent > 0.66f) {
        // Green to yellow transition
        float t = (1.0f - healthPercent) / 0.34f;
        healthColor = IM_COL32(static_cast<int>(255 * t), 255, 0, healthAlpha);
    } else if (healthPercent > 0.33f) {
        // Yellow to orange transition
        healthColor = IM_COL32(255, static_cast<int>(255 * (healthPercent - 0.33f) / 0.33f), 0, healthAlpha);
    } else {
        // Red
        healthColor = IM_COL32(255, 0, 0, healthAlpha);
    }
    
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, 1.0f);
    
    // Subtle border using entity color for identification with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(80 * fadeAlpha);
    drawList->AddRect(barMin, barMax, IM_COL32((entityColor >> 16) & 0xFF, (entityColor >> 8) & 0xFF, entityColor & 0xFF, borderAlpha), 1.0f, 0, 1.0f);
}

void ESPStageRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, const std::string& playerName, unsigned int entityColor) {
    if (playerName.empty()) return;

    // Extract alpha from entity color for distance fading
    float fadeAlpha = ((entityColor >> 24) & 0xFF) / 255.0f;

    // Calculate text size and position
    ImVec2 textSize = ImGui::CalcTextSize(playerName.c_str());
    
    // Position name just below the feet position (below health bar area)
    const float nameOffset = 25.0f; // Below feet with more padding to avoid health bar overlap
    ImVec2 textPos(feetPos.x - textSize.x / 2, feetPos.y + nameOffset);
    
    // Extract RGB from entity color for a natural look
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    
    // Subtle background with rounded corners (like game UI) and distance fade
    ImVec2 bgMin(textPos.x - 4, textPos.y - 2);
    ImVec2 bgMax(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);
    unsigned int bgAlpha = static_cast<unsigned int>(100 * fadeAlpha);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, bgAlpha), 3.0f);
    
    // Subtle border using entity color with distance fade
    unsigned int borderAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddRect(bgMin, bgMax, IM_COL32(r, g, b, borderAlpha), 3.0f, 0, 1.0f);
    
    // Player name text in a clean, readable color with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(220 * fadeAlpha);
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), playerName.c_str()); // Shadow
    drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), playerName.c_str()); // Main text
}

void ESPStageRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color) {
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, 2.0f);
    
    // Corner indicators for better visibility
    const float cornerSize = 8.0f;
    const float thickness = 2.0f;
    
    // Top-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x + cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMin.y), ImVec2(boxMin.x, boxMin.y + cornerSize), color, thickness);
    
    // Top-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x - cornerSize, boxMin.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMin.y), ImVec2(boxMax.x, boxMin.y + cornerSize), color, thickness);
    
    // Bottom-left corner
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x + cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMin.x, boxMax.y), ImVec2(boxMin.x, boxMax.y - cornerSize), color, thickness);
    
    // Bottom-right corner
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x - cornerSize, boxMax.y), color, thickness);
    drawList->AddLine(ImVec2(boxMax.x, boxMax.y), ImVec2(boxMax.x, boxMax.y - cornerSize), color, thickness);
}

void ESPStageRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance, float fadeAlpha) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImVec2 textSize = ImGui::CalcTextSize(distText);
    ImVec2 textPos(center.x - textSize.x / 2, boxMin.y - textSize.y - 5);
    
    // Background with distance fade
    unsigned int bgAlpha = static_cast<unsigned int>(150 * fadeAlpha);
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1), 
                          ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1), 
                          IM_COL32(0, 0, 0, bgAlpha), 2.0f);
    
    // Text with shadow and distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    unsigned int textAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), distText);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), distText);
}

void ESPStageRenderer::RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color) {
    // Extract fade alpha from the color parameter
    float fadeAlpha = ((color >> 24) & 0xFF) / 255.0f;
    
    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(180 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.5f, IM_COL32(0, 0, 0, shadowAlpha));
    // Main dot using entity color (already has faded alpha)
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.0f, color);
}

void ESPStageRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, float fadeAlpha) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow with distance fade
    unsigned int shadowAlpha = static_cast<unsigned int>(120 * fadeAlpha);
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), 2.0f, IM_COL32(0, 0, 0, shadowAlpha));
    
    // Dot with distance fade
    unsigned int dotAlpha = static_cast<unsigned int>(255 * fadeAlpha);
    drawList->AddCircleFilled(pos, 1.5f, IM_COL32(255, 255, 255, dotAlpha));
}

void ESPStageRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details, float fadeAlpha) {
    if (details.empty()) return;

    float textY = boxMax.y + 5.0f;
    
    for (const auto& detail : details) {
        ImVec2 textSize = ImGui::CalcTextSize(detail.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);
        
        // Background with distance fade
        unsigned int bgAlpha = static_cast<unsigned int>(160 * fadeAlpha);
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1), 
                              ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1), 
                              IM_COL32(0, 0, 0, bgAlpha), 1.0f);
        
        // Text with shadow and distance fade
        unsigned int shadowAlpha = static_cast<unsigned int>(200 * fadeAlpha);
        unsigned int textAlpha = static_cast<unsigned int>(255 * fadeAlpha);
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, shadowAlpha), detail.c_str());
        drawList->AddText(textPos, IM_COL32(255, 255, 255, textAlpha), detail.c_str());
        
        textY += textSize.y + 3;
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
        details.reserve(5); // Pre-allocate to avoid reallocations
        if (settings.playerESP.renderDetails) {
            if (!player->playerName.empty()) {
                details.emplace_back("Player: " + player->playerName);
            }
            
            if (player->level > 0) {
                details.emplace_back("Level: " + std::to_string(player->level));
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
        details.reserve(4); // Pre-allocate to avoid reallocations
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
        details.reserve(2); // Pre-allocate for type and gatherable status
        if (settings.objectESP.renderDetails) {
            details.emplace_back("Type: " + ESPFormatting::GadgetTypeToString(gadget->type));
            if (gadget->isGatherable) {
                details.emplace_back("Status: Gatherable");
            }
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

// Distance fading helper functions
unsigned int ESPStageRenderer::ApplyAlphaToColor(unsigned int color, float alpha) {
    // Extract RGBA components
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = color & 0xFF;
    int originalAlpha = (color >> 24) & 0xFF;
    
    // Apply alpha multiplier while preserving original alpha intentions
    int newAlpha = static_cast<int>(originalAlpha * alpha);
    newAlpha = (newAlpha < 0) ? 0 : (newAlpha > 255) ? 255 : newAlpha; // Clamp to valid range
    
    return IM_COL32(r, g, b, newAlpha);
}

float ESPStageRenderer::CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    // Use the same logic as ESPFilter for consistency
    return ESPFilter::CalculateDistanceFadeAlpha(distance, useDistanceLimit, distanceLimit);
}

} // namespace kx