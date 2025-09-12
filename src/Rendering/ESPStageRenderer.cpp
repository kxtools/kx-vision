#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "../../libs/ImGui/imgui.h"
#include <cmath>

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

    // For players and NPCs, prioritize natural health bars over artificial boxes
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    
    // Always show standalone health bars for living entities when health is available
    if (isLivingEntity && context.healthPercent >= 0.0f) {
        RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, context.color);
    }

    // Calculate bounding box for entity based on type (mainly for gadgets and optional boxes)
    float boxHeight, boxWidth;
    
    switch (context.entityType) {
        case ESPEntityType::Player:
            // Players: tall rectangle (humanoid)
            boxHeight = BoxDimensions::PLAYER_HEIGHT;
            boxWidth = BoxDimensions::PLAYER_WIDTH;
            break;
        case ESPEntityType::NPC:
            // NPCs: square box
            boxHeight = BoxDimensions::NPC_HEIGHT;
            boxWidth = BoxDimensions::NPC_WIDTH;
            break;
        case ESPEntityType::Gadget:
            // Gadgets: very small square
            boxHeight = BoxDimensions::GADGET_HEIGHT;
            boxWidth = BoxDimensions::GADGET_WIDTH;
            break;
        default:
            // Fallback to player dimensions
            boxHeight = BoxDimensions::PLAYER_HEIGHT;
            boxWidth = BoxDimensions::PLAYER_WIDTH;
            break;
    }
    
    ImVec2 boxMin(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    ImVec2 boxMax(screenPos.x + boxWidth / 2, screenPos.y);
    ImVec2 center(screenPos.x, screenPos.y - boxHeight / 2);

    // Render old-style health bar only if requested and no standalone health bar was shown
    if (context.renderHealthBar && context.healthPercent >= 0.0f && !isLivingEntity) {
        RenderHealthBar(drawList, boxMin, boxMax, context.healthPercent);
    }

    // Render bounding box (should be disabled by default for living entities)
    if (context.renderBox) {
        RenderBoundingBox(drawList, boxMin, boxMax, context.color);
    }

    // Render distance text
    if (context.renderDistance) {
        RenderDistanceText(drawList, center, boxMin, context.distance);
    }

    // Render center dot
    if (context.renderDot) {
        if (context.entityType == ESPEntityType::Gadget) {
            // Always render natural white dot for gadgets
            RenderNaturalWhiteDot(drawList, screenPos);
        } else {
            // Use colored dots for players and NPCs
            RenderCenterDot(drawList, screenPos, context.color);
        }
    }

    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName && !context.playerName.empty()) {
        RenderPlayerName(drawList, screenPos, context.playerName, context.color);
    }

    // Render details text (for all entities when enabled, but not player names for players)
    if (context.renderDetails && !context.details.empty()) {
        RenderDetailsText(drawList, center, boxMax, context.details);
    }
}

void ESPStageRenderer::RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    const float barWidth = 4.0f;
    const float barHeight = boxMax.y - boxMin.y;
    
    ImVec2 barMin(boxMin.x - barWidth - 2.0f, boxMin.y);
    ImVec2 barMax(boxMin.x - 2.0f, boxMax.y);
    
    // Background
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 150));
    
    // Health bar - fill from bottom to top
    ImVec2 healthBarMin(barMin.x, barMax.y - (barHeight * healthPercent));
    ImVec2 healthBarMax(barMax.x, barMax.y);
    unsigned int healthColor = IM_COL32(
        static_cast<int>(255 * (1.0f - healthPercent)),
        static_cast<int>(255 * healthPercent),
        0, 255
    );
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor);
    
    // Border
    drawList->AddRect(barMin, barMax, IM_COL32(255, 255, 255, 100));
}

void ESPStageRenderer::RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos, float healthPercent, unsigned int entityColor) {
    if (healthPercent < 0.0f || healthPercent > 1.0f) return;

    // Health bar dimensions - more natural looking
    const float barWidth = 40.0f;
    const float barHeight = 6.0f;
    const float yOffset = 15.0f; // Distance below the center point
    
    // Position the health bar below the entity center
    ImVec2 barMin(centerPos.x - barWidth / 2, centerPos.y + yOffset);
    ImVec2 barMax(centerPos.x + barWidth / 2, centerPos.y + yOffset + barHeight);
    
    // Background with subtle transparency
    drawList->AddRectFilled(barMin, barMax, IM_COL32(0, 0, 0, 120), 1.0f);
    
    // Health fill - horizontal bar
    float healthWidth = barWidth * healthPercent;
    ImVec2 healthBarMin(barMin.x, barMin.y);
    ImVec2 healthBarMax(barMin.x + healthWidth, barMax.y);
    
    // Health color: green -> yellow -> red based on percentage (more transparent for natural look)
    unsigned int healthColor;
    if (healthPercent > 0.66f) {
        // Green to yellow transition
        float t = (1.0f - healthPercent) / 0.34f;
        healthColor = IM_COL32(static_cast<int>(255 * t), 255, 0, 160);
    } else if (healthPercent > 0.33f) {
        // Yellow to orange transition
        healthColor = IM_COL32(255, static_cast<int>(255 * (healthPercent - 0.33f) / 0.33f), 0, 160);
    } else {
        // Red
        healthColor = IM_COL32(255, 0, 0, 160);
    }
    
    drawList->AddRectFilled(healthBarMin, healthBarMax, healthColor, 1.0f);
    
    // Subtle border using entity color for identification (more transparent)
    drawList->AddRect(barMin, barMax, IM_COL32((entityColor >> 16) & 0xFF, (entityColor >> 8) & 0xFF, entityColor & 0xFF, 80), 1.0f, 0, 1.0f);
}

void ESPStageRenderer::RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, const std::string& playerName, unsigned int entityColor) {
    if (playerName.empty()) return;

    // Calculate text size and position
    ImVec2 textSize = ImGui::CalcTextSize(playerName.c_str());
    
    // Position name just below the feet position (below health bar area)
    const float nameOffset = 25.0f; // Below feet with more padding to avoid health bar overlap
    ImVec2 textPos(feetPos.x - textSize.x / 2, feetPos.y + nameOffset);
    
    // Extract RGB from entity color for a natural look
    int r = (entityColor >> 16) & 0xFF;
    int g = (entityColor >> 8) & 0xFF;
    int b = entityColor & 0xFF;
    
    // Subtle background with rounded corners (like game UI)
    ImVec2 bgMin(textPos.x - 4, textPos.y - 2);
    ImVec2 bgMax(textPos.x + textSize.x + 4, textPos.y + textSize.y + 2);
    drawList->AddRectFilled(bgMin, bgMax, IM_COL32(0, 0, 0, 100), 3.0f);
    
    // Subtle border using entity color
    drawList->AddRect(bgMin, bgMax, IM_COL32(r, g, b, 120), 3.0f, 0, 1.0f);
    
    // Player name text in a clean, readable color
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 180), playerName.c_str()); // Shadow
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 220), playerName.c_str()); // Main text
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

void ESPStageRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImVec2 textSize = ImGui::CalcTextSize(distText);
    ImVec2 textPos(center.x - textSize.x / 2, boxMin.y - textSize.y - 5);
    
    // Background
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1), 
                          ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1), 
                          IM_COL32(0, 0, 0, 150), 2.0f);
    
    // Text with shadow
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 255), distText);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), distText);
}

void ESPStageRenderer::RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color) {
    // Small, minimalistic dot with subtle outline for visibility
    // Dark outline
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.5f, IM_COL32(0, 0, 0, 180));
    // Main dot using entity color
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), 2.0f, color);
}

void ESPStageRenderer::RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), 2.0f, IM_COL32(0, 0, 0, 120));
    
    // Dot
    drawList->AddCircleFilled(pos, 1.5f, IM_COL32(255, 255, 255, 255));
}

void ESPStageRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details) {
    if (details.empty()) return;

    float textY = boxMax.y + 5.0f;
    
    for (const auto& detail : details) {
        ImVec2 textSize = ImGui::CalcTextSize(detail.c_str());
        ImVec2 textPos(center.x - textSize.x / 2, textY);
        
        // Background
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1), 
                              ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1), 
                              IM_COL32(0, 0, 0, 160), 1.0f);
        
        // Text with shadow
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), detail.c_str());
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), detail.c_str());
        
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
        
        // Skip resource nodes that are not gatherable
        if (gadget->type == Game::GadgetType::ResourceNode && !gadget->isGatherable) {
            continue;
        }
        
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

} // namespace kx