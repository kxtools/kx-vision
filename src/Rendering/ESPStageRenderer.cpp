#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "../../libs/ImGui/imgui.h"
#include "../Utils/EntityFilter.h"
#include <limits>
#include <cmath>

namespace kx {

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const FrameRenderData& frameData, Camera& camera) {
    RenderPlayers(drawList, screenWidth, screenHeight, frameData.players, camera);
    RenderNpcs(drawList, screenWidth, screenHeight, frameData.npcs, camera);
    RenderGadgets(drawList, screenWidth, screenHeight, frameData.gadgets, camera);
}

void ESPStageRenderer::RenderPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                    const std::vector<RenderablePlayer>& players, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    if (!settings.playerESP.enabled) return;

    const glm::vec3 cameraPos = camera.GetPlayerPosition();
    const float maxDistanceSquared = settings.espUseDistanceLimit ? 
        (settings.espRenderDistanceLimit * settings.espRenderDistanceLimit) : (std::numeric_limits<float>::max)();

    for (const auto& player : players) {
        if (!player.isValid) continue;
        
        // Skip local player if showLocalPlayer is disabled
        if (player.isLocalPlayer && !settings.playerESP.showLocalPlayer) {
            continue;
        }
        
        // Early distance culling using squared distance (faster than sqrt)
        const glm::vec3 deltaPos = player.position - cameraPos;
        const float distanceSquared = glm::dot(deltaPos, deltaPos);
        if (distanceSquared > maxDistanceSquared) {
            continue;
        }
        
        // Skip dead entities (0 HP)
        if (player.currentHealth <= 0.0f) {
            continue;
        }
        
        const float distance = std::sqrt(distanceSquared);  // Only calculate sqrt when needed
        
        unsigned int color = IM_COL32(0, 255, 100, 220); // Friendly player color

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
            
            if (player.profession > 0) {
                details.emplace_back("Prof: " + std::to_string(player.profession));
            }
            
            if (player.maxHealth > 0) {
                details.emplace_back("HP: " + std::to_string(static_cast<int>(player.currentHealth)) + "/" + std::to_string(static_cast<int>(player.maxHealth)));
            }
            
            if (player.maxEnergy > 0) {
                const int energyPercent = static_cast<int>((player.currentEnergy / player.maxEnergy) * 100.0f);
                details.emplace_back("Energy: " + std::to_string(static_cast<int>(player.currentEnergy)) + "/" + std::to_string(static_cast<int>(player.maxEnergy)) + " (" + std::to_string(energyPercent) + "%)");
            }
        }

        RenderEntity(drawList, player.position, distance, screenWidth, screenHeight, color, details, healthPercent, 
                    settings.playerESP.renderBox, settings.playerESP.renderDistance, settings.playerESP.renderDot, 
                    settings.playerESP.renderDetails, settings.playerESP.renderHealthBar, ESPEntityType::Player, camera);
    }
}

void ESPStageRenderer::RenderNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                 const std::vector<RenderableNpc>& npcs, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    if (!settings.npcESP.enabled) return;

    const glm::vec3 cameraPos = camera.GetPlayerPosition();
    const float maxDistanceSquared = settings.espUseDistanceLimit ? 
        (settings.espRenderDistanceLimit * settings.espRenderDistanceLimit) : (std::numeric_limits<float>::max)();

    for (const auto& npc : npcs) {
        if (!npc.isValid) continue;
        
        // Early distance culling using squared distance (faster than sqrt)
        const glm::vec3 deltaPos = npc.position - cameraPos;
        const float distanceSquared = glm::dot(deltaPos, deltaPos);
        if (distanceSquared > maxDistanceSquared) {
            continue;
        }
        
        // Skip dead entities (0 HP)
        if (npc.currentHealth <= 0.0f) {
            continue;
        }
        
        const float distance = std::sqrt(distanceSquared);  // Only calculate sqrt when needed
        
        // Use attitude-based coloring for NPCs
        unsigned int color = IM_COL32(255, 165, 0, 220); // Default orange for NPCs
        
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
            
            details.emplace_back("Attitude: " + std::to_string(npc.attitude));
        }

        RenderEntity(drawList, npc.position, distance, screenWidth, screenHeight, color, details, healthPercent, 
                    settings.npcESP.renderBox, settings.npcESP.renderDistance, settings.npcESP.renderDot, 
                    settings.npcESP.renderDetails, settings.npcESP.renderHealthBar, ESPEntityType::NPC, camera);
    }
}

void ESPStageRenderer::RenderGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                    const std::vector<RenderableGadget>& gadgets, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    if (!settings.objectESP.enabled) return;

    const glm::vec3 cameraPos = camera.GetPlayerPosition();
    const float maxDistanceSquared = settings.espUseDistanceLimit ? 
        (settings.espRenderDistanceLimit * settings.espRenderDistanceLimit) : (std::numeric_limits<float>::max)();

    for (const auto& gadget : gadgets) {
        if (!gadget.isValid) continue;
        
        // Early distance culling using squared distance (faster than sqrt)
        const glm::vec3 deltaPos = gadget.position - cameraPos;
        const float distanceSquared = glm::dot(deltaPos, deltaPos);
        if (distanceSquared > maxDistanceSquared) {
            continue;
        }
        
        const float distance = std::sqrt(distanceSquared);  // Only calculate sqrt when needed
        
        unsigned int color = IM_COL32(255, 255, 0, 220); // Yellow for gadgets

        std::vector<std::string> details;
        details.reserve(1); // Pre-allocate to avoid reallocations
        if (settings.objectESP.renderDetails) {
            details.emplace_back("Type: " + std::to_string(gadget.type));
        }

        RenderEntity(drawList, gadget.position, distance, screenWidth, screenHeight, color, details, -1.0f, 
                    settings.objectESP.renderBox, settings.objectESP.renderDistance, settings.objectESP.renderDot, 
                    settings.objectESP.renderDetails, false, ESPEntityType::Gadget, camera);
    }
}

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, 
                                   unsigned int color, const std::vector<std::string>& details, float healthPercent, 
                                   bool renderBox, bool renderDistance, bool renderDot, bool renderDetails, bool renderHealthBar, 
                                   ESPEntityType entityType, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    // Early frustum culling - check if position projects to valid screen coordinates
    glm::vec2 screenPos;
    if (!kx::ESPMath::WorldToScreen(worldPos, camera, screenWidth, screenHeight, screenPos)) {
        return; // Entity is behind camera or outside frustum
    }
    
    // Screen bounds culling with small margin for partially visible entities
    const float margin = 50.0f;
    if (screenPos.x < -margin || screenPos.x > screenWidth + margin || 
        screenPos.y < -margin || screenPos.y > screenHeight + margin) {
        return; // Entity is off-screen
    }

    // Skip entity type check since we already checked enabled status in the caller functions

    // Calculate bounding box for entity
    const float boxHeight = 50.0f;
    const float boxWidth = 30.0f;
    
    ImVec2 boxMin(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    ImVec2 boxMax(screenPos.x + boxWidth / 2, screenPos.y);
    ImVec2 center(screenPos.x, screenPos.y - boxHeight / 2);

    // Render health bar
    if (renderHealthBar && healthPercent >= 0.0f) {
        RenderHealthBar(drawList, boxMin, boxMax, healthPercent);
    }

    // Render bounding box
    if (renderBox) {
        RenderBoundingBox(drawList, boxMin, boxMax, color);
    }

    // Render distance text
    if (renderDistance) {
        RenderDistanceText(drawList, center, boxMin, distance);
    }

    // Render center dot
    if (renderDot) {
        RenderCenterDot(drawList, glm::vec2(screenPos.x, screenPos.y));
    }

    // Render details text
    if (renderDetails && !details.empty()) {
        RenderDetailsText(drawList, center, boxMax, details);
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

void ESPStageRenderer::RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos) {
    const float dotRadius = 3.0f;
    
    // Outer glow
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), dotRadius + 1.0f, IM_COL32(0, 0, 0, 100));
    
    // Main dot
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), dotRadius, IM_COL32(255, 255, 255, 255));
    
    // Inner highlight
    drawList->AddCircleFilled(ImVec2(feetPos.x, feetPos.y), dotRadius - 1.0f, IM_COL32(255, 255, 0, 200));
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