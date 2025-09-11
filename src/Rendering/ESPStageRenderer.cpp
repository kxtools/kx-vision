#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/Camera.h"
#include "ESP_Helpers.h"
#include "EnhancedESPHelpers.h"
#include "../../libs/ImGui/imgui.h"
#include "../Utils/EntityFilter.h"

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

    for (const auto& player : players) {
        if (!player.isValid) continue;
        
        // Skip local player if showLocalPlayer is disabled
        if (player.isLocalPlayer && !settings.playerESP.showLocalPlayer) {
            continue;
        }
        
        float distance = glm::length(player.position - camera.GetPlayerPosition());
        
        unsigned int color = IM_COL32(0, 255, 100, 220); // Friendly player color

        float healthPercent = -1.0f;
        if (player.maxHealth > 0) {
            healthPercent = player.currentHealth / player.maxHealth;
        }

        std::vector<std::string> details;
        if (settings.playerESP.renderDetails) {
            if (!player.playerName.empty()) {
                details.push_back("Player: " + player.playerName);
            }
            
            if (player.level > 0) {
                details.push_back("Level: " + std::to_string(player.level));
            }
            
            if (player.profession > 0) {
                details.push_back("Prof: " + std::to_string(player.profession));
            }
            
            if (player.maxHealth > 0) {
                details.push_back("HP: " + std::to_string((int)player.currentHealth) + "/" + std::to_string((int)player.maxHealth));
            }
            
            if (player.maxEnergy > 0) {
                float energyPercent = (player.currentEnergy / player.maxEnergy) * 100.0f;
                details.push_back("Energy: " + std::to_string((int)player.currentEnergy) + "/" + std::to_string((int)player.maxEnergy) + " (" + std::to_string((int)energyPercent) + "%)");
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

    for (const auto& npc : npcs) {
        if (!npc.isValid) continue;
        
        float distance = glm::length(npc.position - camera.GetPlayerPosition());
        
        // Use attitude-based coloring for NPCs
        unsigned int color = IM_COL32(255, 165, 0, 220); // Default orange for NPCs
        
        float healthPercent = -1.0f;
        if (npc.maxHealth > 0) {
            healthPercent = npc.currentHealth / npc.maxHealth;
        }

        std::vector<std::string> details;
        if (settings.npcESP.renderDetails) {
            if (!npc.name.empty()) {
                details.push_back("NPC: " + npc.name);
            }
            
            if (npc.level > 0) {
                details.push_back("Level: " + std::to_string(npc.level));
            }
            
            if (npc.maxHealth > 0) {
                details.push_back("HP: " + std::to_string((int)npc.currentHealth) + "/" + std::to_string((int)npc.maxHealth));
            }
            
            details.push_back("Attitude: " + std::to_string(npc.attitude));
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

    for (const auto& gadget : gadgets) {
        if (!gadget.isValid) continue;
        
        float distance = glm::length(gadget.position - camera.GetPlayerPosition());
        
        // Convert back to GadgetType enum for filtering
        Game::GadgetType gadgetType = static_cast<Game::GadgetType>(gadget.type);
        
        // Use the existing filtering logic
        bool shouldRender = false;
        switch (gadgetType) {
            case Game::GadgetType::ResourceNode:
                shouldRender = settings.objectESP.showResourceNodes;
                break;
            case Game::GadgetType::Waypoint:
                shouldRender = settings.objectESP.showWaypoints;
                break;
            case Game::GadgetType::Vista:
                shouldRender = settings.objectESP.showVistas;
                break;
            case Game::GadgetType::Crafting:
                shouldRender = settings.objectESP.showCraftingStations;
                break;
            case Game::GadgetType::AttackTarget:
                shouldRender = settings.objectESP.showAttackTargets;
                break;
            case Game::GadgetType::PlayerCreated:
                shouldRender = settings.objectESP.showPlayerCreated;
                break;
            case Game::GadgetType::Interact:
                shouldRender = settings.objectESP.showInteractables;
                break;
            case Game::GadgetType::Door:
                shouldRender = settings.objectESP.showDoors;
                break;
            case Game::GadgetType::MapPortal:
                shouldRender = settings.objectESP.showPortals;
                break;
            default:
                shouldRender = !settings.objectESP.onlyImportantGadgets;
                break;
        }
        
        if (!shouldRender) continue;

        // Use the gadget type-based color system
        unsigned int color = kx::ESPHelpers::GetGadgetTypeColor(gadgetType);
        
        // Make important gadgets more visible
        if (Game::EnumHelpers::IsImportantGadgetType(gadgetType)) {
            color = (color & 0x00FFFFFF) | 0xFF000000;
        }

        std::vector<std::string> details;
        if (settings.objectESP.renderDetails) {
            std::string gadgetTypeName = GadgetTypeToString(gadgetType);
            details.push_back("Type: " + gadgetTypeName);
            
            int priority = Filtering::EntityFilter::GetRenderPriority(gadgetType);
            if (Game::EnumHelpers::IsImportantGadgetType(gadgetType)) {
                details.push_back("Priority: HIGH (" + std::to_string(priority) + ")");
            } else {
                details.push_back("Priority: " + std::to_string(priority));
            }
            
            if (!gadget.name.empty()) {
                details.push_back("Name: " + gadget.name);
            }
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
    if (settings.espUseDistanceLimit && distance > settings.espRenderDistanceLimit) {
        return;
    }

    // Get ESP data based on entity type
    ESPEntityData* entityData = nullptr;
    GadgetESPData gadgetData;
    PlayerESPData playerData;
    
    if (entityType == ESPEntityType::Gadget) {
        gadgetData = kx::ESP_Helpers::GetGadgetESPData(worldPos, camera, screenWidth, screenHeight);
        entityData = &gadgetData;
    } else {
        if (entityType == ESPEntityType::Player) {
            playerData = kx::ESP_Helpers::GetPlayerESPData(worldPos, camera, screenWidth, screenHeight);
        } else { // NPC
            playerData = kx::ESP_Helpers::GetNpcESPData(worldPos, camera, screenWidth, screenHeight);
        }
        entityData = &playerData;
    }
    
    if (!entityData->valid) return;
    
    // Calculate universal box bounds
    ImVec2 boxMin(entityData->min.x, entityData->min.y);
    ImVec2 boxMax(entityData->max.x, entityData->max.y);
    ImVec2 center((boxMin.x + boxMax.x) * 0.5f, (boxMin.y + boxMax.y) * 0.5f);
    
    // Render health bar (only for entities with health and if enabled)
    if (healthPercent >= 0.0f && renderHealthBar) {
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
        RenderCenterDot(drawList, entityData->feet);
    }
    
    // Render details
    if (renderDetails && !details.empty()) {
        RenderDetailsText(drawList, center, boxMax, details);
    }
}

// Helper rendering functions
void ESPStageRenderer::RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent) {
    const float barWidth = 3.0f;
    const float barOffset = 2.0f;
    
    ImVec2 healthBarMin(boxMin.x - barWidth - barOffset, boxMin.y);
    ImVec2 healthBarMax(boxMin.x - barOffset, boxMax.y);
    
    float barHeight = healthBarMax.y - healthBarMin.y;
    float healthHeight = barHeight * healthPercent;
    
    // Background
    drawList->AddRectFilled(healthBarMin, healthBarMax, IM_COL32(20, 20, 20, 200));
    
    // Health color
    ImU32 healthColor = IM_COL32(255, 0, 0, 255); // Red
    if (healthPercent > 0.6f) healthColor = IM_COL32(0, 255, 0, 255); // Green
    else if (healthPercent > 0.3f) healthColor = IM_COL32(255, 255, 0, 255); // Yellow
    
    // Health fill
    drawList->AddRectFilled(ImVec2(healthBarMin.x, healthBarMax.y - healthHeight), 
                          ImVec2(healthBarMax.x, healthBarMax.y), healthColor);
    
    // Border
    drawList->AddRect(healthBarMin, healthBarMax, IM_COL32(0, 0, 0, 255), 0.0f, 0, 1.0f);
}

void ESPStageRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color) {
    // Shadow
    drawList->AddRect(ImVec2(boxMin.x + 1, boxMin.y + 1), ImVec2(boxMax.x + 1, boxMax.y + 1), 
                    IM_COL32(0, 0, 0, 80), 0.0f, 0, 1.5f);
    
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, 1.5f);
}

void ESPStageRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance) {
    char distText[32];
    snprintf(distText, sizeof(distText), "%.1fm", distance);
    
    ImVec2 textSize = ImGui::CalcTextSize(distText);
    ImVec2 textPos(center.x - textSize.x * 0.5f, boxMin.y - textSize.y - 3);
    
    // Background
    drawList->AddRectFilled(ImVec2(textPos.x - 2, textPos.y - 1), 
                          ImVec2(textPos.x + textSize.x + 2, textPos.y + textSize.y + 1), 
                          IM_COL32(0, 0, 0, 150), 1.0f);
    
    // Text with shadow
    drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 180), distText);
    drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), distText);
}

void ESPStageRenderer::RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), 2.0f, IM_COL32(0, 0, 0, 120));
    
    // Dot
    drawList->AddCircleFilled(pos, 1.5f, IM_COL32(255, 255, 255, 255));
}

void ESPStageRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details) {
    float textY = boxMax.y + 3;
    
    for (const auto& detailText : details) {
        if (detailText.empty()) continue;
        
        ImVec2 textSize = ImGui::CalcTextSize(detailText.c_str());
        ImVec2 textPos(center.x - textSize.x * 0.5f, textY);
        
        // Background
        drawList->AddRectFilled(ImVec2(textPos.x - 3, textPos.y - 1), 
                              ImVec2(textPos.x + textSize.x + 3, textPos.y + textSize.y + 1), 
                              IM_COL32(0, 0, 0, 160), 1.0f);
        
        // Text with shadow
        drawList->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), detailText.c_str());
        drawList->AddText(textPos, IM_COL32(255, 255, 255, 255), detailText.c_str());
        
        textY += textSize.y + 3;
    }
}

} // namespace kx