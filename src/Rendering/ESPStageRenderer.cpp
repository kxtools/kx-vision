#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "../../libs/ImGui/imgui.h"
#include <cmath>

namespace kx {

// Context struct for unified entity rendering
struct EntityRenderContext {
    // Pre-calculated data from the Renderable object
    const glm::vec2& screenPos;
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
    ESPEntityType entityType;
    
    // Screen dimensions for bounds checking
    float screenWidth;
    float screenHeight;
};

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const FrameRenderData& frameData, Camera& camera) {
    // Simple rendering - no filtering logic, just draw everything that was passed in
    RenderPlayers(drawList, screenWidth, screenHeight, frameData.players);
    RenderNpcs(drawList, screenWidth, screenHeight, frameData.npcs);
    RenderGadgets(drawList, screenWidth, screenHeight, frameData.gadgets);
}

void ESPStageRenderer::RenderPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                    const std::vector<RenderablePlayer>& players) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto& player : players) {
        // All filtering has been done - just render everything
        
        unsigned int color = IM_COL32(100, 255, 100, 200); // Friendly player - bright green (like GW2 friendly indicators)

        float healthPercent = -1.0f;
        if (player.maxHealth > 0) {
            healthPercent = player.currentHealth / player.maxHealth;
        }

        std::vector<std::string> details;
        details.reserve(5); // Pre-allocate to avoid reallocations
        if (settings.playerESP.renderDetails) {
            if (!player.playerName.empty()) {
                details.emplace_back("Player: " + player.playerName);
            }
            
            if (player.level > 0) {
                details.emplace_back("Level: " + std::to_string(player.level));
            }
            
            if (player.profession != Game::Profession::None) {
                details.emplace_back("Prof: " + ESPFormatting::ProfessionToString(player.profession));
            }
            
            if (player.race != Game::Race::None) {
                details.emplace_back("Race: " + ESPFormatting::RaceToString(player.race));
            }
            
            if (player.maxHealth > 0) {
                details.emplace_back("HP: " + std::to_string(static_cast<int>(player.currentHealth)) + "/" + std::to_string(static_cast<int>(player.maxHealth)));
            }
            
            if (player.maxEnergy > 0) {
                const int energyPercent = static_cast<int>((player.currentEnergy / player.maxEnergy) * 100.0f);
                details.emplace_back("Energy: " + std::to_string(static_cast<int>(player.currentEnergy)) + "/" + std::to_string(static_cast<int>(player.maxEnergy)) + " (" + std::to_string(energyPercent) + "%)");
            }
        }

        EntityRenderContext context{
            player.screenPos,
            player.distance,
            color,
            details,
            healthPercent,
            settings.playerESP.renderBox,
            settings.playerESP.renderDistance,
            settings.playerESP.renderDot,
            settings.playerESP.renderDetails,
            settings.playerESP.renderHealthBar,
            ESPEntityType::Player,
            screenWidth,
            screenHeight
        };
        RenderEntity(drawList, context);
    }
}

void ESPStageRenderer::RenderNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                 const std::vector<RenderableNpc>& npcs) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto& npc : npcs) {
        // All filtering has been done - just render everything
        
        // Use attitude-based coloring for NPCs
        unsigned int color = IM_COL32(255, 80, 80, 210); // NPCs - red/orange (like GW2 enemy indicators)
        
        float healthPercent = -1.0f;
        if (npc.maxHealth > 0) {
            healthPercent = npc.currentHealth / npc.maxHealth;
        }

        std::vector<std::string> details;
        details.reserve(4); // Pre-allocate to avoid reallocations
        if (settings.npcESP.renderDetails) {
            if (!npc.name.empty()) {
                details.emplace_back("NPC: " + npc.name);
            }
            
            if (npc.level > 0) {
                details.emplace_back("Level: " + std::to_string(npc.level));
            }
            
            if (npc.maxHealth > 0) {
                details.emplace_back("HP: " + std::to_string(static_cast<int>(npc.currentHealth)) + "/" + std::to_string(static_cast<int>(npc.maxHealth)));
            }
            
            details.emplace_back("Attitude: " + ESPFormatting::AttitudeToString(npc.attitude));
        }

        EntityRenderContext context{
            npc.screenPos,
            npc.distance,
            color,
            details,
            healthPercent,
            settings.npcESP.renderBox,
            settings.npcESP.renderDistance,
            settings.npcESP.renderDot,
            settings.npcESP.renderDetails,
            settings.npcESP.renderHealthBar,
            ESPEntityType::NPC,
            screenWidth,
            screenHeight
        };
        RenderEntity(drawList, context);
    }
}

void ESPStageRenderer::RenderGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                    const std::vector<RenderableGadget>& gadgets) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto& gadget : gadgets) {
        // All filtering has been done - just render everything
        
        unsigned int color = IM_COL32(150, 150, 255, 190); // Gadgets - light blue (like GW2 interactable objects)

        std::vector<std::string> details;
        details.reserve(2); // Pre-allocate for type and gatherable status
        if (settings.objectESP.renderDetails) {
            details.emplace_back("Type: " + ESPFormatting::GadgetTypeToString(gadget.type));
            if (gadget.isGatherable) {
                details.emplace_back("Status: Gatherable");
            }
        }

        EntityRenderContext context{
            gadget.screenPos,
            gadget.distance,
            color,
            details,
            -1.0f,
            settings.objectESP.renderBox,
            settings.objectESP.renderDistance,
            settings.objectESP.renderDot,
            settings.objectESP.renderDetails,
            false,
            ESPEntityType::Gadget,
            screenWidth,
            screenHeight
        };
        RenderEntity(drawList, context);
    }
}

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context) {
    // Screen bounds culling with small margin for partially visible entities
    const float margin = 50.0f;
    if (context.screenPos.x < -margin || context.screenPos.x > context.screenWidth + margin || 
        context.screenPos.y < -margin || context.screenPos.y > context.screenHeight + margin) {
        return; // Entity is off-screen
    }

    // Calculate bounding box for entity based on type
    float boxHeight, boxWidth;
    
    switch (context.entityType) {
        case ESPEntityType::Player:
            // Players: tall rectangle (humanoid)
            boxHeight = 50.0f;
            boxWidth = 30.0f;
            break;
        case ESPEntityType::NPC:
            // NPCs: square box
            boxHeight = 40.0f;
            boxWidth = 40.0f;
            break;
        case ESPEntityType::Gadget:
            // Gadgets: very small square
            boxHeight = 15.0f;
            boxWidth = 15.0f;
            break;
        default:
            // Fallback
            boxHeight = 50.0f;
            boxWidth = 30.0f;
            break;
    }
    
    ImVec2 boxMin(context.screenPos.x - boxWidth / 2, context.screenPos.y - boxHeight);
    ImVec2 boxMax(context.screenPos.x + boxWidth / 2, context.screenPos.y);
    ImVec2 center(context.screenPos.x, context.screenPos.y - boxHeight / 2);

    // Render health bar
    if (context.renderHealthBar && context.healthPercent >= 0.0f) {
        RenderHealthBar(drawList, boxMin, boxMax, context.healthPercent);
    }

    // Render bounding box
    if (context.renderBox) {
        RenderBoundingBox(drawList, boxMin, boxMax, context.color);
    }

    // Render distance text
    if (context.renderDistance) {
        RenderDistanceText(drawList, center, boxMin, context.distance);
    }

    // Render center dot
    if (context.renderDot) {
        RenderCenterDot(drawList, glm::vec2(context.screenPos.x, context.screenPos.y), context.color);
    }

    // Render details text
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

} // namespace kx