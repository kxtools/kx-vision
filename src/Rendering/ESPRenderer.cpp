#define NOMINMAX

#include "ESPRenderer.h"

#include <algorithm>
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <gtc/type_ptr.hpp>
#include <Windows.h>

#include "../Core/Config.h"
#include "../Core/AppState.h"  // For IsDebugLoggingEnabled
#include "ESP_Helpers.h"
#include "ESPData.h"
#include "EnhancedESPHelpers.h"
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
            // Debug: Log the ContextCollection pointer
            if (kx::AppState::Get().IsDebugLoggingEnabled()) {
                printf("DEBUG: ContextCollection ptr = 0x%p\n", pContextCollection);
            }
            
            kx::ReClass::ContextCollection ctxCollection(pContextCollection);
            kx::ReClass::ChCliContext charContext = ctxCollection.GetChCliContext();
            if (charContext) {
                std::map<void*, const wchar_t*> characterNameToPlayerName;
                kx::ReClass::ChCliPlayer** playerList = charContext.GetPlayerList();
                uint32_t playerCount = charContext.GetPlayerListSize();
                if (playerList && playerCount < 2000) {
                    for (uint32_t i = 0; i < playerCount; ++i) {
                        // Validate the pointer before constructing the object
                        uintptr_t ptrAddr = reinterpret_cast<uintptr_t>(playerList[i]);
                        if (!playerList[i] || ptrAddr < 0x1000 || ptrAddr > 0x7FFFFFFFFFFF) continue;
                        
                        MEMORY_BASIC_INFORMATION mbi = {};
                        if (VirtualQuery(playerList[i], &mbi, sizeof(mbi)) == 0 || 
                            mbi.State != MEM_COMMIT || 
                            !(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
                            continue;
                        }
                        
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
                        // Validate the pointer before constructing the object
                        uintptr_t ptrAddr = reinterpret_cast<uintptr_t>(characterList[i]);
                        if (!characterList[i] || ptrAddr < 0x1000 || ptrAddr > 0x7FFFFFFFFFFF) continue;
                        
                        MEMORY_BASIC_INFORMATION mbi = {};
                        if (VirtualQuery(characterList[i], &mbi, sizeof(mbi)) == 0 || 
                            mbi.State != MEM_COMMIT || 
                            !(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE))) {
                            continue;
                        }
                        
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
    // Access settings through AppState singleton
    const auto& settings = kx::AppState::Get().GetSettings();
    if (!settings.playerESP.enabled) return;

    // Skip local player unless specifically enabled
    void* localPlayer = AddressManager::GetLocalPlayer();
    if (localPlayer && character.data() == localPlayer && !settings.playerESP.showLocalPlayer) return;

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
    if (settings.playerESP.renderDetails) {
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
            
            if (settings.playerESP.showRace) {
                characterDesc += " " + RaceToString(race);
            }
            
            if (settings.playerESP.showProfession) {
                characterDesc += " " + ProfessionToString(profession);
            }
            
            if (settings.playerESP.showArmorWeight) {
                std::string armorWeight = kx::ESPHelpers::GetArmorWeight(profession);
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

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, healthPercent, settings.playerESP.renderBox, settings.playerESP.renderDistance, settings.playerESP.renderDot, settings.playerESP.renderDetails, settings.playerESP.renderHealthBar, ESPEntityType::Player);
}

void ESPRenderer::RenderNpc(ImDrawList* drawList, float screenWidth, float screenHeight, kx::ReClass::ChCliCharacter& character) {
    const auto& settings = kx::AppState::Get().GetSettings();
    if (!settings.npcESP.enabled) return;
    
    // Skip local player (though unlikely for NPCs)
    void* localPlayer = AddressManager::GetLocalPlayer();
    if (localPlayer && character.data() == localPlayer) return;
    
    Game::Attitude attitude = character.GetAttitude();
    
    // Use the new boolean flags for attitude filtering
    switch (attitude) {
        case Game::Attitude::Friendly:
            if (!settings.npcESP.showFriendly) return;
            break;
        case Game::Attitude::Hostile:
            if (!settings.npcESP.showHostile) return;
            break;
        case Game::Attitude::Neutral:
            if (!settings.npcESP.showNeutral) return;
            break;
        case Game::Attitude::Indifferent:
            if (!settings.npcESP.showIndifferent) return;
            break;
        default:
            // Show unknown attitudes by default
            break;
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
    unsigned int color = kx::ESPHelpers::GetAttitudeColor(attitude);

    kx::ReClass::ChCliHealth health = character.GetHealth();
    float healthPercent = -1.0f;
    if (health) {
        float maxHealth = health.GetMax();
        if (maxHealth > 0) healthPercent = health.GetCurrent() / maxHealth;
    }

    std::vector<std::string> details;
    if (settings.npcESP.renderDetails) {
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
            if (kx::Filtering::EntityFilter::IsSupportProfession(profession)) {
                details.push_back("Role: Support");
            } else if (kx::Filtering::EntityFilter::IsDpsProfession(profession)) {
                details.push_back("Role: DPS");
            } else {
                details.push_back("Role: Hybrid");
            }
        }

        // Enhanced attitude display with threat assessment
        std::string attitudeText = "Attitude: " + AttitudeToString(attitude);
        details.push_back(attitudeText);
        
        // Threat level calculation
        kx::ReClass::ChCliCoreStats stats2 = character.GetCoreStats();
        if (stats2) {
            int threatLevel = kx::Filtering::EntityFilter::GetThreatLevel(attitude, stats2.GetProfession());
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
            switch (static_cast<Game::CharacterRank>(agentType)) {
                case Game::CharacterRank::Veteran: rankName = "Veteran"; break;
                case Game::CharacterRank::Elite: rankName = "Elite"; break;
                case Game::CharacterRank::Champion: rankName = "Champion"; break;
                case Game::CharacterRank::Legendary: rankName = "Legendary"; break;
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
        if (distance <= kx::CombatRanges::MELEE_RANGE) { // Melee range
            details.push_back("Range: Melee");
        } else if (distance <= kx::CombatRanges::RANGED_COMBAT_RANGE) { // Ranged combat range
            details.push_back("Range: Ranged");
        } else if (distance <= kx::CombatRanges::LONG_RANGE) { // Long range
            details.push_back("Range: Long");
        } else {
            details.push_back("Range: Very Long");
        }
    }

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, healthPercent, settings.npcESP.renderBox, settings.npcESP.renderDistance, settings.npcESP.renderDot, settings.npcESP.renderDetails, settings.npcESP.renderHealthBar, ESPEntityType::NPC);
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
    const auto& settings = kx::AppState::Get().GetSettings();
    if (!settings.objectESP.enabled) return;

    // Use the new enum for type-safe gadget type checking
    Game::GadgetType gadgetType = gadget.GetGadgetType();
    
    // Use the new boolean flags for gadget filtering
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
            // For unknown types, check if we're in important-only mode
            shouldRender = !settings.objectESP.onlyImportantGadgets;
            break;
    }
    
    if (!shouldRender) return;

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
    unsigned int color = kx::ESPHelpers::GetGadgetTypeColor(gadgetType);
    
    // Make important gadgets more visible at distance
    if (Game::EnumHelpers::IsImportantGadgetType(gadgetType)) {
        // Increase opacity for important gadgets
        color = (color & 0x00FFFFFF) | 0xFF000000;
    }

    std::vector<std::string> details;
    if (settings.objectESP.renderDetails) {
        // Primary identification
        std::string gadgetTypeName = GadgetTypeToString(gadgetType);
        details.push_back("Type: " + gadgetTypeName);
        
        // Priority and importance indicators
        int priority = Filtering::EntityFilter::GetRenderPriority(gadgetType);
        if (Game::EnumHelpers::IsImportantGadgetType(gadgetType)) {
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
        if (distance <= kx::InteractionRanges::STANDARD_INTERACTION_RANGE) { // Standard interaction range
            details.push_back("Range: In Range");
        } else if (distance <= kx::InteractionRanges::MEDIUM_INTERACTION_RANGE) { // Medium range
            details.push_back("Range: Approaching");
        } else {
            details.push_back("Range: Far");
        }
        
        // Additional context based on object importance
        if (Game::EnumHelpers::IsImportantGadgetType(gadgetType)) {
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

    RenderEntity(drawList, worldPos, distance, screenWidth, screenHeight, color, details, -1.0f, settings.objectESP.renderBox, settings.objectESP.renderDistance, settings.objectESP.renderDot, settings.objectESP.renderDetails, false, ESPEntityType::Gadget);
}

// Universal ESP rendering helper functions
void ESPRenderer::RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent) {
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

void ESPRenderer::RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color) {
    // Shadow
    drawList->AddRect(ImVec2(boxMin.x + 1, boxMin.y + 1), ImVec2(boxMax.x + 1, boxMax.y + 1), 
                    IM_COL32(0, 0, 0, 80), 0.0f, 0, 1.5f);
    
    // Main box
    drawList->AddRect(boxMin, boxMax, color, 0.0f, 0, 1.5f);
}

void ESPRenderer::RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance) {
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

void ESPRenderer::RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos) {
    ImVec2 pos(feetPos.x, feetPos.y);
    
    // Shadow
    drawList->AddCircleFilled(ImVec2(pos.x + 1, pos.y + 1), 2.0f, IM_COL32(0, 0, 0, 120));
    
    // Dot
    drawList->AddCircleFilled(pos, 1.5f, IM_COL32(255, 255, 255, 255));
}

void ESPRenderer::RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details) {
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

void ESPRenderer::RenderEntity(ImDrawList* drawList, const glm::vec3& worldPos, float distance, float screenWidth, float screenHeight, unsigned int color, const std::vector<std::string>& details, float healthPercent, bool renderBox, bool renderDistance, bool renderDot, bool renderDetails, bool renderHealthBar, ESPEntityType entityType) {
    const auto& settings = kx::AppState::Get().GetSettings();
    if (settings.espUseDistanceLimit && distance > settings.espRenderDistanceLimit) {
        return;
    }

    // Get ESP data based on entity type
    ESPEntityData* entityData = nullptr;
    GadgetESPData gadgetData;
    PlayerESPData playerData;
    
    if (entityType == ESPEntityType::Gadget) {
        gadgetData = kx::ESP_Helpers::GetGadgetESPData(worldPos, *s_camera, screenWidth, screenHeight);
        entityData = &gadgetData;
    } else {
        if (entityType == ESPEntityType::Player) {
            playerData = kx::ESP_Helpers::GetPlayerESPData(worldPos, *s_camera, screenWidth, screenHeight);
        } else { // NPC
            playerData = kx::ESP_Helpers::GetNpcESPData(worldPos, *s_camera, screenWidth, screenHeight);
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

bool ESPRenderer::ShouldHideESP(const MumbleLinkData* mumbleData) {
    if (mumbleData && (mumbleData->context.uiState & IsMapOpen)) {
        return true;
    }
    return false;
}

} // namespace kx
