#define NOMINMAX

#include "ESPRenderer.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <gtc/type_ptr.hpp>

#include "../Core/Config.h"
#include "ESP_Helpers.h"
#include "EnhancedESPHelpers.h" // Include the new enhanced helpers
#include "StringHelpers.h" // New include
#include "../../libs/ImGui/imgui.h"
#include "../Core/AppState.h"
#include "../Game/AddressManager.h"
#include "../Game/GameStructs.h"
#include "../Game/ReClassStructs.h"
#include "../Utils/EntityFilter.h" // Include for advanced filtering functions

namespace kx {

// Initialize the static camera pointer
Camera* ESPRenderer::s_camera = nullptr;

void ESPRenderer::Initialize(Camera& camera) {
    s_camera = &camera;
}

void ESPRenderer::Render(float screenWidth, float screenHeight, const MumbleLinkData* mumbleData) {
    if (!s_camera || ShouldHideESP(mumbleData)) {
        return;
    }

    ::ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    RenderAllEntities(drawList, screenWidth, screenHeight);
}

void ESPRenderer::RenderAllEntities(ImDrawList* drawList, float screenWidth, float screenHeight) {
    try {
        // This combines logic from both old rendering functions.
        // It assumes the data collection from `AddressManager` is working.

        // First, handle Characters (Players and NPCs) from the ChCliContext
        void* pContextCollection = AddressManager::GetContextCollectionPtr();
        if (pContextCollection) {
            kx::ReClass::ContextCollection ctxCollection(pContextCollection);
            kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
            if (charContext) {
                std::map<void*, const wchar_t*> characterNameToPlayerName;
                kx::ReClass::ChCliPlayer** playerList = charContext.GetPlayerList();
                uint32_t playerCount = charContext.GetPlayerListSize();
                if (playerList && playerCount < 2000) {
                    for (uint32_t i = 0; i < playerCount; ++i) {
                        kx::ReClass::ChCliPlayer player(playerList[i]);
                        if (!player) continue;
                        kx::ReClass::ChCliCharacter character = player.GetCharacter();
                        const wchar_t* name = player.GetName();
                        if (character.data() && name) {
                            characterNameToPlayerName[character.data()] = name;
                        }
                    }
                }

                kx::ReClass::ChCliCharacter** characterList = charContext.GetCharacterList();
                uint32_t characterCapacity = charContext.GetCharacterListCapacity();
                if (characterList && characterCapacity < 0x10000) {
                    for (uint32_t i = 0; i < characterCapacity; ++i) {
                        kx::ReClass::ChCliCharacter character(characterList[i]);
                        if (!character) continue;

                        // --- CATEGORIZATION LOGIC ---
                        if (characterNameToPlayerName.count(character.data())) {
                            // It's a Player
                            RenderPlayer(drawList, screenWidth, screenHeight, character, characterNameToPlayerName);
                        } else {
                            // It's an NPC
                            RenderNpc(drawList, screenWidth, screenHeight, character);
                        }
                    }
                }
            }
        }

        RenderGadgets(drawList, screenWidth, screenHeight);
    }
    catch (...) { /* Prevent crash */ }
}

void ESPRenderer::RenderPlayer(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character, const std::map<void*, const wchar_t*>& characterNameToPlayerName) {
    if (!g_settings.playerESP.enabled) return;

    const float scaleFactor = 1.23f;

    kx::ReClass::AgChar agent = character.GetAgent();
    if (!agent) return;
    kx::ReClass::CoChar coChar = agent.GetCoChar();
    if (!coChar) return;

    Coordinates3D gameWorldPos = coChar.GetVisualPosition();
    if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) return;

    glm::vec3 worldPos(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
    float distance = glm::length(worldPos - s_camera->GetPlayerPosition());

    unsigned int color = IM_COL32(0, 255, 100, 220); // Friendly player color

    kx::ReClass::ChCliHealth health = character.GetHealth();
    float healthPercent = -1.0f;
    if (health) {
        float maxHealth = health.GetMax();
        if (maxHealth > 0) healthPercent = health.GetCurrent() / maxHealth;
    }

    std::vector<std::string> details;
    if (g_settings.playerESP.renderDetails) {
        // Player name (highest priority)
        auto it = characterNameToPlayerName.find(character.data());
        if (it != characterNameToPlayerName.end()) {
            std::string playerName = WStringToString(it->second);
            details.push_back("Player: " + playerName);
        }

        kx::ReClass::ChCliCoreStats stats = character.GetCoreStats();
        if (stats) {
            // Enhanced character information using new enums
            Game::Profession profession = stats.GetProfession();
            Game::Race race = stats.GetRace();
            uint32_t level = stats.GetLevel();
            
            // Create compact character description
            std::string characterDesc = "Lvl " + std::to_string(level);
            
            if (g_settings.playerESP.showRace) {
                characterDesc += " " + RaceToString(race);
            }
            
            if (g_settings.playerESP.showProfession) {
                characterDesc += " " + ProfessionToString(profession);
            }
            
            if (g_settings.playerESP.showArmorWeight) {
                std::string armorWeight = ESPHelpers::GetArmorWeight(profession);
                characterDesc += " (" + armorWeight + ")";
            }
            
            details.push_back(characterDesc);
        }

        // Agent rank information (for special player states)
        uint32_t agentType = agent.GetType();
        if (agentType != AGENT_TYPE_CHARACTER) {
            details.push_back("Agent Type ID: " + std::to_string(agentType));
        }

        // Energy information
        kx::ReClass::ChCliEnergies energies = character.GetEnergies();
        if (energies) {
            float maxEnergy = energies.GetMax();
            float currentEnergy = energies.GetCurrent();
            if (maxEnergy > 0) {
                float energyPercent = (currentEnergy / maxEnergy) * 100.0f;
                char energyText[64];
                snprintf(energyText, sizeof(energyText), "Energy: %.0f/%.0f (%.0f%%)", currentEnergy, maxEnergy, energyPercent);
                details.push_back(energyText);
            }
        }
    }

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, healthPercent, g_settings.playerESP.renderBox, g_settings.playerESP.renderDistance, g_settings.playerESP.renderDot, g_settings.playerESP.renderDetails);
}

void ESPRenderer::RenderNpc(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character) {
    if (!g_settings.npcESP.enabled) return;
    
    Game::Attitude attitude = character.GetAttitude();
    uint32_t attitudeValue = static_cast<uint32_t>(attitude);
    if (g_settings.npcESP.ignoredAttitude & (1 << attitudeValue)) {
        return;
    }

    const float scaleFactor = 1.23f;

    kx::ReClass::AgChar agent = character.GetAgent();
    if (!agent) return;
    kx::ReClass::CoChar coChar = agent.GetCoChar();
    if (!coChar) return;

    Coordinates3D gameWorldPos = coChar.GetVisualPosition();
    if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) return;

    glm::vec3 worldPos(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
    float distance = glm::length(worldPos - s_camera->GetPlayerPosition());

    // Use the new attitude-based color system
    unsigned int color = ESPHelpers::GetAttitudeColor(attitude);

    kx::ReClass::ChCliHealth health = character.GetHealth();
    float healthPercent = -1.0f;
    if (health) {
        float maxHealth = health.GetMax();
        if (maxHealth > 0) healthPercent = health.GetCurrent() / maxHealth;
    }

    std::vector<std::string> details;
    if (g_settings.npcESP.renderDetails) {
        kx::ReClass::ChCliCoreStats stats = character.GetCoreStats();
        if (stats) {
            // Enhanced NPC information using new enums
            Game::Profession profession = stats.GetProfession();
            Game::Race race = stats.GetRace();
            uint32_t level = stats.GetLevel();
            
            // Comprehensive character description
            std::string characterDesc = GetCharacterDescription(profession, race, level);
            details.push_back(characterDesc);
            
            // Combat role analysis
            if (ESPHelpers::IsSupportProfession(profession)) {
                details.push_back("Role: Support");
            } else if (ESPHelpers::IsDpsProfession(profession)) {
                details.push_back("Role: DPS");
            } else {
                details.push_back("Role: Hybrid");
            }
        }

        // Enhanced attitude display with threat assessment
        std::string attitudeText = "Attitude: " + ESPHelpers::AttitudeToString(attitude);
        details.push_back(attitudeText);
        
        // Threat level calculation
        kx::ReClass::ChCliCoreStats stats2 = character.GetCoreStats();
        if (stats2) {
            int threatLevel = ESPHelpers::GetThreatLevel(attitude, stats2.GetProfession());
            if (threatLevel > 75) {
                details.push_back("Threat: HIGH");
            } else if (threatLevel > 50) {
                details.push_back("Threat: Medium");
            } else if (threatLevel > 25) {
                details.push_back("Threat: Low");
            } else {
                details.push_back("Threat: Minimal");
            }
        }

        // Agent rank/type information with better descriptions
        uint32_t agentType = agent.GetType();
        if (agentType != AGENT_TYPE_CHARACTER) {
            // Try to interpret special agent types
            const char* rankName = nullptr;
            switch (agentType) {
                case 1: rankName = "Veteran"; break;
                case 2: rankName = "Elite"; break;
                case 3: rankName = "Champion"; break;
                case 4: rankName = "Legendary"; break;
                default: 
                    // Show the actual ID instead of "Unknown"
                    details.push_back("Agent Type ID: " + std::to_string(agentType));
                    break;
            }
            if (rankName) {
                details.push_back(std::string("Rank: ") + rankName);
            }
        }

        // Enhanced energy display with percentage
        kx::ReClass::ChCliEnergies energies = character.GetEnergies();
        if (energies) {
            float maxEnergy = energies.GetMax();
            float currentEnergy = energies.GetCurrent();
            if (maxEnergy > 0) {
                float energyPercent = (currentEnergy / maxEnergy) * 100.0f;
                char energyText[64];
                snprintf(energyText, sizeof(energyText), "Energy: %.0f/%.0f (%.0f%%)", currentEnergy, maxEnergy, energyPercent);
                details.push_back(energyText);
            }
        }

        // Tactical range assessment
        if (distance <= 130.0f) { // Melee range
            details.push_back("Range: Melee");
        } else if (distance <= 300.0f) { // Ranged combat range
            details.push_back("Range: Ranged");
        } else if (distance <= 900.0f) { // Long range
            details.push_back("Range: Long");
        } else {
            details.push_back("Range: Very Long");
        }
    }

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, healthPercent, g_settings.npcESP.renderBox, g_settings.npcESP.renderDistance, g_settings.npcESP.renderDot, g_settings.npcESP.renderDetails);
}

void ESPRenderer::RenderGadgets(ImDrawList* drawList, float screenWidth, float screenHeight) {
    void* pContextCollection = AddressManager::GetContextCollectionPtr();
    if (!pContextCollection) return;

    try {
        kx::ReClass::ContextCollection ctxCollection(pContextCollection);
        kx::ReClass::GdCliContext gadgetCtx = ctxCollection.GetGdCliContext();
        if (!gadgetCtx) return;

        kx::ReClass::GdCliGadget** gadgetList = gadgetCtx.GetGadgetList();
        uint32_t gadgetCapacity = gadgetCtx.GetGadgetListCapacity();
        if (!gadgetList || gadgetCapacity > 0x10000) return;

        for (uint32_t i = 0; i < gadgetCapacity; ++i) {
            kx::ReClass::GdCliGadget gadget(gadgetList[i]);
            if (!gadget) continue;

            RenderObject(drawList, screenWidth, screenHeight, gadget);
        }
    }
    catch (...) { /* Prevent crash */ }
}

void ESPRenderer::RenderObject(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::GdCliGadget& gadget) {
    if (!g_settings.objectESP.enabled) return;

    // Use the new enum for type-safe gadget type checking
    Game::GadgetType gadgetType = gadget.GetGadgetType();
    uint32_t gadgetTypeValue = static_cast<uint32_t>(gadgetType);
    
    if (g_settings.objectESP.ignoredGadgets & (1 << gadgetTypeValue)) {
        return;
    }

    // The best feature! Filter out depleted resource nodes using enum comparison
    if (gadgetType == Game::GadgetType::ResourceNode) {
        if (!gadget.IsGatherable()) {
            return;
        }
    }

    // Get position from the new structures
    kx::ReClass::AgKeyFramed agKeyFramed = gadget.GetAgKeyFramed();
    kx::ReClass::CoKeyFramed coKeyFramed = agKeyFramed.GetCoKeyFramed();
    if (!coKeyFramed) return;

    Coordinates3D gameWorldPos = coKeyFramed.GetPosition();
    if (gameWorldPos.X == 0.0f && gameWorldPos.Y == 0.0f && gameWorldPos.Z == 0.0f) return;

    const float scaleFactor = 1.23f;
    glm::vec3 worldPos(gameWorldPos.X / scaleFactor, gameWorldPos.Z / scaleFactor, gameWorldPos.Y / scaleFactor);
    float distance = glm::length(worldPos - s_camera->GetPlayerPosition());

    // Use the new gadget type-based color system
    unsigned int color = ESPHelpers::GetGadgetTypeColor(gadgetType);
    
    // Make important gadgets more visible at distance
    if (ESPHelpers::IsImportantGadgetType(gadgetType)) {
        // Increase opacity for important gadgets
        color = (color & 0x00FFFFFF) | 0xFF000000;
    }

    std::vector<std::string> details;
    if (g_settings.objectESP.renderDetails) {
        // Primary identification
        std::string gadgetTypeName = ESPHelpers::GadgetTypeToString(gadgetType);
        details.push_back("Type: " + gadgetTypeName);
        
        // Priority and importance indicators
        int priority = Filtering::EntityFilter::GetRenderPriority(gadgetType);
        if (ESPHelpers::IsImportantGadgetType(gadgetType)) {
            details.push_back("Priority: HIGH (" + std::to_string(priority) + ")");
        } else {
            details.push_back("Priority: " + std::to_string(priority));
        }
        
        // Enhanced status information for different gadget types
        switch (gadgetType) {
            case Game::GadgetType::ResourceNode:
                details.push_back(gadget.IsGatherable() ? "Status: Gatherable ✓" : "Status: Depleted ✗");
                details.push_back("Category: Resource");
                break;
                
            case Game::GadgetType::Waypoint:
                details.push_back("Category: Travel");
                details.push_back("Function: Fast Travel");
                break;
                
            case Game::GadgetType::Vista:
                details.push_back("Category: Exploration");
                details.push_back("Reward: Experience + Achievement");
                break;
                
            case Game::GadgetType::Crafting:
                details.push_back("Category: Crafting");
                details.push_back("Function: Equipment Creation");
                break;
                
            case Game::GadgetType::AttackTarget:
                details.push_back("Category: Combat");
                details.push_back("Type: Boss/Elite Target");
                break;
                
            case Game::GadgetType::PlayerCreated:
                details.push_back("Category: Player Object");
                details.push_back("Examples: Siege, Turrets, Banners");
                break;
                
            case Game::GadgetType::Interact:
                details.push_back("Category: Interactive");
                details.push_back("Function: General Interaction");
                break;
                
            case Game::GadgetType::Door:
                details.push_back("Category: Environment");
                details.push_back("Function: Passage/Barrier");
                break;
                
            case Game::GadgetType::MapPortal:
                details.push_back("Category: Travel");
                details.push_back("Function: Map Transition");
                break;
                
            default:
                details.push_back("Category: Unknown");
                details.push_back("Gadget ID: " + std::to_string(static_cast<uint32_t>(gadgetType)));
                break;
        }
        
        // Interaction range assessment
        if (distance <= 300.0f) { // Standard interaction range
            details.push_back("Range: In Range");
        } else if (distance <= 600.0f) { // Medium range
            details.push_back("Range: Approaching");
        } else {
            details.push_back("Range: Far");
        }
        
        // Additional context based on object importance
        if (ESPHelpers::IsImportantGadgetType(gadgetType)) {
            details.push_back("⭐ Important Object");
            
            // Add helpful tips for important objects
            switch (gadgetType) {
                case Game::GadgetType::ResourceNode:
                    if (gadget.IsGatherable()) {
                        details.push_back("💡 Tip: Use appropriate gathering tool");
                    }
                    break;
                case Game::GadgetType::Vista:
                    details.push_back("💡 Tip: Look for climbing path");
                    break;
                case Game::GadgetType::AttackTarget:
                    details.push_back("💡 Tip: High-value target");
                    break;
            }
        }
    }

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, -1.0f, g_settings.objectESP.renderBox, g_settings.objectESP.renderDistance, g_settings.objectESP.renderDot, g_settings.objectESP.renderDetails);
}

void ESPRenderer::RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, unsigned int color, const std::vector<std::string>& details, float healthPercent, bool renderBox, bool renderDistance, bool renderDot, bool renderDetails) {
    if (g_settings.espUseDistanceLimit && distance > g_settings.espRenderDistanceLimit) {
        return;
    }

    glm::vec2 screenPos;
    if (ESP_Helpers::WorldToScreen(worldPos, *s_camera, screenWidth, screenHeight, screenPos)) {
        float boxSize = std::max(4.0f, 15.0f * (50.0f / (distance + 20.0f)));
        float boxTop = screenPos.y - boxSize / 2;
        float boxBottom = screenPos.y + boxSize / 2;
        float boxLeft = screenPos.x - boxSize / 2;
        float boxRight = screenPos.x + boxSize / 2;

        if (healthPercent >= 0.0f) {
            float barHeight = std::max(2.0f, boxSize);
            float barWidth = 3.0f;
            float healthHeight = barHeight * healthPercent;

            drawList->AddRectFilled(ImVec2(boxLeft - barWidth - 2, boxTop), ImVec2(boxLeft - 2, boxBottom), IM_COL32(0, 0, 0, 180));
            drawList->AddRectFilled(ImVec2(boxLeft - barWidth - 2, boxBottom - healthHeight), ImVec2(boxLeft - 2, boxBottom), IM_COL32(0, 200, 0, 255));
            drawList->AddRect(ImVec2(boxLeft - barWidth - 2, boxTop), ImVec2(boxLeft - 2, boxBottom), IM_COL32(0, 0, 0, 255));
        }

        if (renderBox) {
            drawList->AddRect(ImVec2(boxLeft, boxTop), ImVec2(boxRight, boxBottom), color, 1.0f, ImDrawFlags_RoundCornersAll, 1.5f);
        }

        float textY = boxTop;

        if (renderDistance) {
            char distText[32];
            snprintf(distText, sizeof(distText), "%.1fm", distance);
            ::ImVec2 textSize = ImGui::CalcTextSize(distText);
            drawList->AddRectFilled(ImVec2(screenPos.x - textSize.x / 2 - 2, textY - textSize.y - 4), ImVec2(screenPos.x + textSize.x / 2 + 2, textY), IM_COL32(0, 0, 0, 180));
            drawList->AddText(ImVec2(screenPos.x - textSize.x / 2, textY - textSize.y - 2), IM_COL32(255, 255, 255, 255), distText);
        }

        if (renderDot) {
            drawList->AddCircleFilled(::ImVec2(screenPos.x, screenPos.y), 2.0f, IM_COL32(255, 255, 255, 255));
        }

        if (renderDetails && !details.empty()) {
            float textYDetails = boxBottom + 2;
            for (const auto& detailText : details) {
                if (detailText.empty()) continue;
                ::ImVec2 textSize = ImGui::CalcTextSize(detailText.c_str());
                drawList->AddRectFilled(ImVec2(screenPos.x - textSize.x / 2 - 2, textYDetails), ImVec2(screenPos.x + textSize.x / 2 + 2, textYDetails + textSize.y + 4), IM_COL32(0, 0, 0, 180));
                drawList->AddText(ImVec2(screenPos.x - textSize.x / 2, textYDetails + 2), IM_COL32(255, 255, 255, 255), detailText.c_str());
                textYDetails += textSize.y + 6;
            }
        }
    }
}

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx
