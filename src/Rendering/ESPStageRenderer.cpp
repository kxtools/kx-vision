#include "ESPStageRenderer.h"
#include "../Core/AppState.h"
#include "../Game/Camera.h"
#include "ESPMath.h"
#include "ESPStyling.h"
#include "ESPFormatting.h"
#include "ESPConstants.h"
#include "ESPFilter.h"
#include "ESPFeatureRenderer.h"
#include "../Game/Generated/StatData.h"
#include "../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

	const ImU32 DEFAULT_TEXT_COLOR = IM_COL32(255, 255, 255, 255); // White

// Context struct for unified entity rendering
struct EntityRenderContext {
    // Entity data
    const glm::vec3& position;    // World position for real-time screen projection
    float distance;
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
        if (!player) continue;

        // --- 1. DATA PREPARATION ---
        // Prepare all text details first. This includes general info and the detailed gear view if selected.

        std::vector<ColoredDetail> details;
        if (settings.playerESP.renderDetails) {
            details = BuildPlayerDetails(player, settings.playerESP);
        }

        // If "Detailed" mode is selected, build the full gear list and add it to the details.
        if (settings.playerESP.gearDisplayMode == 3) { // 3 = Detailed
            auto gearDetails = BuildGearDetails(player);
            if (!gearDetails.empty()) {
                if (!details.empty()) {
                    details.push_back({ "--- Gear Stats ---", DEFAULT_TEXT_COLOR });
                }
                details.insert(details.end(), gearDetails.begin(), gearDetails.end());
            }
        }

        if (settings.showDebugAddresses) {
            char addrStr[32];
            snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", player->address);
            details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
        }

        // --- 2. CORE RENDERING ---
        // Project to screen and render all common elements (name, health bar, box, dot, details).

        glm::vec2 screenPos;
        if (!ESPMath::WorldToScreen(player->position, camera, screenWidth, screenHeight, screenPos)) {
            continue; // Skip if off-screen
        }

        float distanceFadeAlpha = CalculateEntityDistanceFadeAlpha(player->distance, settings.espUseDistanceLimit, settings.espRenderDistanceLimit);
        if (distanceFadeAlpha <= 0.0f) {
            continue; // Skip if fully faded out
        }

        unsigned int fadedPlayerColor = ESPFeatureRenderer::ApplyAlphaToColor(ESPColors::PLAYER, distanceFadeAlpha);
        float healthPercent = (player->maxHealth > 0) ? (player->currentHealth / player->maxHealth) : -1.0f;

        EntityRenderContext context{
            player->position,
            player->distance,
            fadedPlayerColor,
            details,
            healthPercent,
            settings.playerESP.renderBox,
            settings.playerESP.renderDistance,
            settings.playerESP.renderDot,
            !details.empty(), // renderDetails is true only if the details vector is not empty
            settings.playerESP.renderHealthBar,
            settings.playerESP.renderPlayerName,
            ESPEntityType::Player,
            screenWidth,
            screenHeight,
            player->playerName
        };
        RenderEntity(drawList, context, camera);

        // --- 3. SPECIALIZED SUMMARY RENDERING ---
        // After the main elements are drawn, render the specific compact summary if one is selected.

        switch (settings.playerESP.gearDisplayMode) {
        case 1: { // Compact (Stat Names)
            auto compactSummary = BuildCompactGearSummary(player);
            ESPFeatureRenderer::RenderGearSummary(drawList, screenPos, compactSummary, distanceFadeAlpha);
            break;
        }
        case 2: { // Compact (Top 3 Attributes)
            auto dominantStats = BuildDominantStats(player);
            ESPFeatureRenderer::RenderDominantStats(drawList, screenPos, dominantStats, distanceFadeAlpha);
            break;
        }
        default:
            // Modes 0 (Off) and 3 (Detailed) do not have a separate summary view.
            break;
        }
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

        std::vector<ColoredDetail> details;
        details.reserve(6);
        if (settings.npcESP.renderDetails) {
            if (!npc->name.empty()) {
                details.push_back({ "NPC: " + npc->name, DEFAULT_TEXT_COLOR });
            }

            if (npc->level > 0) {
                details.push_back({ "Level: " + std::to_string(npc->level), DEFAULT_TEXT_COLOR });
            }

            if (npc->maxHealth > 0) {
                details.push_back({ "HP: " + std::to_string(static_cast<int>(npc->currentHealth)) + "/" + std::to_string(static_cast<int>(npc->maxHealth)), DEFAULT_TEXT_COLOR });
            }

            details.push_back({ "Attitude: " + ESPFormatting::AttitudeToString(npc->attitude), DEFAULT_TEXT_COLOR });
            details.push_back({ "Rank: " + ESPFormatting::RankToString(npc->rank), DEFAULT_TEXT_COLOR });

            if (settings.showDebugAddresses) {
                char addrStr[32];
                snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", npc->address);
                details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
            }
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
            emptyPlayerName,
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

        std::vector<ColoredDetail> details;
        details.reserve(4);
        if (settings.objectESP.renderDetails) {
            details.push_back({ "Type: " + ESPFormatting::GadgetTypeToString(gadget->type), DEFAULT_TEXT_COLOR });

            if (gadget->type == Game::GadgetType::ResourceNode) {
                details.push_back({ "Node: " + ESPFormatting::ResourceNodeTypeToString(gadget->resourceType), DEFAULT_TEXT_COLOR });
            }

            if (gadget->isGatherable) {
                details.push_back({ "Status: Gatherable", DEFAULT_TEXT_COLOR });
            }

            if (settings.showDebugAddresses) {
                char addrStr[32];
                snprintf(addrStr, sizeof(addrStr), "Addr: 0x%p", gadget->address);
                details.push_back({ std::string(addrStr), DEFAULT_TEXT_COLOR });
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
            emptyPlayerName,
        };
        RenderEntity(drawList, context, camera);
    }
}

std::vector<ColoredDetail> ESPStageRenderer::BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings) {
    std::vector<ColoredDetail> details;
    details.reserve(10); // Reserve space

    if (!player->playerName.empty()) {
        details.push_back({ "Player: " + player->playerName, DEFAULT_TEXT_COLOR });
    }

    if (player->level > 0) {
        std::string levelText = "Level: " + std::to_string(player->level);
        if (player->scaledLevel != player->level && player->scaledLevel > 0) {
            levelText += " (" + std::to_string(player->scaledLevel) + ")";
        }
        details.push_back({ levelText, DEFAULT_TEXT_COLOR });
    }

    if (player->profession != Game::Profession::None) {
        details.push_back({ "Prof: " + ESPFormatting::ProfessionToString(player->profession), DEFAULT_TEXT_COLOR });
    }

    if (player->race != Game::Race::None) {
        details.push_back({ "Race: " + ESPFormatting::RaceToString(player->race), DEFAULT_TEXT_COLOR });
    }

    if (player->maxHealth > 0) {
        details.push_back({ "HP: " + std::to_string(static_cast<int>(player->currentHealth)) + "/" + std::to_string(static_cast<int>(player->maxHealth)), DEFAULT_TEXT_COLOR });
    }

    if (player->maxEnergy > 0) {
        const int energyPercent = static_cast<int>((player->currentEnergy / player->maxEnergy) * 100.0f);
        details.push_back({ "Energy: " + std::to_string(static_cast<int>(player->currentEnergy)) + "/" + std::to_string(static_cast<int>(player->maxEnergy)) + " (" + std::to_string(energyPercent) + "%)", DEFAULT_TEXT_COLOR });
    }
    return details;
}

std::vector<CompactStatInfo> ESPStageRenderer::BuildCompactGearSummary(const RenderablePlayer* player) {
    if (!player || player->gear.empty()) {
        return {};
    }

    // Use a map to group stats and find the highest rarity for each
    std::map<std::string, CompactStatInfo> statSummary;
    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            auto statIt = kx::data::stat::DATA.find(info.statId);
            if (statIt != kx::data::stat::DATA.end()) {
                std::string statName = statIt->second.name;

                // Increment count
                statSummary[statName].count++;
                statSummary[statName].statName = statName;

                // Keep track of the highest rarity
                if (info.rarity > statSummary[statName].highestRarity) {
                    statSummary[statName].highestRarity = info.rarity;
                }
            }
        }
    }

    if (statSummary.empty()) {
        return {};
    }

    // Convert the map to a vector for ordered rendering
    std::vector<CompactStatInfo> result;
    result.reserve(statSummary.size());
    for (const auto& pair : statSummary) {
        result.push_back(pair.second);
    }
    return result;
}

std::map<kx::data::ApiAttribute, int> ESPStageRenderer::BuildAttributeSummary(const RenderablePlayer* player) {
    std::map<kx::data::ApiAttribute, int> attributeCounts;
    if (!player || player->gear.empty()) {
        return attributeCounts;
    }

    for (const auto& pair : player->gear) {
        const GearSlotInfo& info = pair.second;
        if (info.statId > 0) {
            auto statIt = kx::data::stat::DATA.find(info.statId);
            if (statIt != kx::data::stat::DATA.end()) {
                for (const auto& attr : statIt->second.attributes) {
                    attributeCounts[attr.attribute]++;
                }
            }
        }
    }
    return attributeCounts;
}

std::vector<DominantStat> ESPStageRenderer::BuildDominantStats(const RenderablePlayer* player) {
    std::vector<DominantStat> result;

    // 1. Get the raw attribute counts
    std::map<kx::data::ApiAttribute, int> attributeCounts = BuildAttributeSummary(player);
    if (attributeCounts.empty()) {
        return result;
    }

    // 2. Calculate the total number of attribute instances
    float totalAttributes = 0.0f;
    for (const auto& pair : attributeCounts) {
        totalAttributes += pair.second;
    }
    if (totalAttributes == 0) return result;

    // 3. Convert to a vector of DominantStat with percentages
    std::vector<DominantStat> allStats;
    allStats.reserve(attributeCounts.size());
    for (const auto& pair : attributeCounts) {
        const char* name = "??";
        switch (pair.first) {
        case kx::data::ApiAttribute::Power:           name = "Power"; break;
        case kx::data::ApiAttribute::Precision:       name = "Precision"; break;
        case kx::data::ApiAttribute::Toughness:       name = "Toughness"; break;
        case kx::data::ApiAttribute::Vitality:        name = "Vitality"; break;
        case kx::data::ApiAttribute::CritDamage:      name = "Ferocity"; break;
        case kx::data::ApiAttribute::Healing:         name = "Healing"; break;
        case kx::data::ApiAttribute::ConditionDamage: name = "Condi Dmg"; break;
        case kx::data::ApiAttribute::BoonDuration:    name = "Boon Dmg"; break;
        case kx::data::ApiAttribute::ConditionDuration: name = "Condi Dura"; break;
        }
        allStats.push_back({ name, (pair.second / totalAttributes) * 100.0f });
    }

    // 4. Sort the vector in descending order of percentage
    std::sort(allStats.begin(), allStats.end(), [](const DominantStat& a, const DominantStat& b) {
        return a.percentage > b.percentage;
        });

    // 5. Return the top 3
    result.reserve(3);
    for (size_t i = 0; i < allStats.size() && i < 3; ++i) {
        result.push_back(allStats[i]);
    }

    return result;
}

std::vector<ColoredDetail> ESPStageRenderer::BuildGearDetails(const RenderablePlayer* player) {
    std::vector<ColoredDetail> gearDetails;
    gearDetails.reserve(12);

    const std::vector<Game::EquipmentSlot> displayOrder = {
        // Armor
        Game::EquipmentSlot::Helm,
        Game::EquipmentSlot::Shoulders,
        Game::EquipmentSlot::Chest,
        Game::EquipmentSlot::Gloves,
        Game::EquipmentSlot::Pants,
        Game::EquipmentSlot::Boots,
        // Trinkets
        Game::EquipmentSlot::Back,
        Game::EquipmentSlot::Amulet,
        Game::EquipmentSlot::Ring1,
        Game::EquipmentSlot::Ring2,
        Game::EquipmentSlot::Accessory1,
        Game::EquipmentSlot::Accessory2,
        // Weapons
        Game::EquipmentSlot::MainhandWeapon1,
        Game::EquipmentSlot::OffhandWeapon1,
        Game::EquipmentSlot::MainhandWeapon2,
        Game::EquipmentSlot::OffhandWeapon2,
    };

    for (const auto& slotEnum : displayOrder) {
        auto gearIt = player->gear.find(slotEnum);
        if (gearIt != player->gear.end()) {
            const char* slotName = ESPFormatting::EquipmentSlotToString(gearIt->first);
            const GearSlotInfo& info = gearIt->second;
            ImU32 rarityColor = ESPHelpers::GetRarityColor(info.rarity);

            std::string statName = "No Stats";
            if (info.statId > 0) {
                auto statIt = kx::data::stat::DATA.find(info.statId);
                if (statIt != kx::data::stat::DATA.end()) {
                    statName = statIt->second.name;
                }
                else {
                    statName = "stat(" + std::to_string(info.statId) + ")";
                }
            }

            gearDetails.push_back({ std::string(slotName) + ": " + statName, rarityColor });
        }
    }
    return gearDetails;
}

float ESPStageRenderer::CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    // Use the same logic as ESPFilter for consistency
    return ESPFilter::CalculateDistanceFadeAlpha(distance, useDistanceLimit, distanceLimit);
}

} // namespace kx