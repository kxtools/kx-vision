#include "ESPStageRenderer.h"
#include "../Renderers/ESPContextFactory.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/ESPFormatting.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "ESPFilter.h"
#include "../Renderers/ESPFeatureRenderer.h"
#include "../../Game/Generated/StatData.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

const ImU32 DEFAULT_TEXT_COLOR = IM_COL32(255, 255, 255, 255); // White

// Context struct for unified entity rendering
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
    float distanceFadeAlpha = CalculateEntityDistanceFadeAlpha(context.gameplayDistance,
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

    // Calculate the effective distance, which only starts counting after the "dead zone"
    float effectiveDistance = (std::max)(0.0f, context.visualDistance - settings.espScalingStartDistance);

    // Calculate scale using the effective distance
    float rawScale = settings.espDistanceFactor / (settings.espDistanceFactor + pow(effectiveDistance, settings.espScalingExponent));

    float clampedScale = (std::max)(settings.espMinScale, (std::min)(rawScale, settings.espMaxScale));

    // Show standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        const float finalHealthBarWidth = (std::max)(10.0f, (std::min)(settings.espBaseHealthBarWidth * clampedScale, 100.0f));
        const float finalHealthBarHeight = (std::max)(2.0f, (std::min)(settings.espBaseHealthBarHeight * clampedScale, 20.0f));
        ESPFeatureRenderer::RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, fadedEntityColor, finalHealthBarWidth, finalHealthBarHeight);
    }

    // Calculate bounding box for entity based on type and distance-based scaling
    float boxHeight, boxWidth;
    
    const float finalFontSize = (std::max)(settings.espMinFontSize, (std::min)(settings.espBaseFontSize * clampedScale, 40.0f));

    switch (context.entityType)
    {
    case ESPEntityType::Player:
	    boxHeight = settings.espBaseBoxHeight * clampedScale; // Use setting
	    boxWidth = settings.espBaseBoxWidth * clampedScale; // Use setting
	    if (boxHeight < MinimumSizes::PLAYER_MIN_HEIGHT)
	    {
		    boxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
		    boxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
	    }
	    break;
    case ESPEntityType::NPC:
        // For NPCs, use a square based on a smaller version of the player box WIDTH.
        // Use width for both height and width to create a square.
        boxHeight = (settings.espBaseBoxWidth * 0.8f) * clampedScale;
        boxWidth = (settings.espBaseBoxWidth * 0.8f) * clampedScale;
        if (boxHeight < MinimumSizes::NPC_MIN_HEIGHT) {
            boxHeight = MinimumSizes::NPC_MIN_HEIGHT;
            boxWidth = MinimumSizes::NPC_MIN_WIDTH;
        }
        break;
    case ESPEntityType::Gadget:
	    // Gadgets can remain very small
	    boxHeight = (settings.espBaseBoxWidth * 0.3f) * clampedScale;
	    boxWidth = (settings.espBaseBoxWidth * 0.3f) * clampedScale;
	    if (boxHeight < MinimumSizes::GADGET_MIN_HEIGHT)
	    {
		    boxHeight = MinimumSizes::GADGET_MIN_HEIGHT;
		    boxWidth = MinimumSizes::GADGET_MIN_WIDTH;
	    }
	    break;
    default:
	    boxHeight = settings.espBaseBoxHeight * clampedScale;
	    boxWidth = settings.espBaseBoxWidth * clampedScale;
	    if (boxHeight < MinimumSizes::PLAYER_MIN_HEIGHT)
	    {
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
        const float finalBoxThickness = (std::max)(1.0f, (std::min)(settings.espBaseBoxThickness * clampedScale, 10.0f));
        ESPFeatureRenderer::RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor, finalBoxThickness);
    }

    // Render distance text
    if (context.renderDistance) {
        ESPFeatureRenderer::RenderDistanceText(drawList, center, boxMin, context.gameplayDistance, distanceFadeAlpha, finalFontSize);
    }

    // Render center dot
    if (context.renderDot) {
        const float finalDotRadius = (std::max)(1.0f, (std::min)(settings.espBaseDotRadius * clampedScale, 15.0f));
        if (context.entityType == ESPEntityType::Gadget) {
            // Always render natural white dot for gadgets with distance fade
            ESPFeatureRenderer::RenderNaturalWhiteDot(drawList, screenPos, distanceFadeAlpha, finalDotRadius);
        } else {
            // Use colored dots for players and NPCs with distance fade
            ESPFeatureRenderer::RenderColoredDot(drawList, screenPos, fadedEntityColor, finalDotRadius);
        }
    }

    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName && !context.playerName.empty()) {
        ESPFeatureRenderer::RenderPlayerName(drawList, screenPos, context.playerName, fadedEntityColor, finalFontSize);
    }

    // Render details text (for all entities when enabled, but not player names for players)
    if (context.renderDetails && !context.details.empty()) {
        ESPFeatureRenderer::RenderDetailsText(drawList, center, boxMax, context.details, distanceFadeAlpha, finalFontSize);
    }

    // --- Specialized Summary Rendering (Players Only) ---
    if (context.entityType == ESPEntityType::Player && context.player != nullptr) {
        switch (settings.playerESP.gearDisplayMode) {
        case 1: { // Compact (Stat Names)
            auto compactSummary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(context.player);
            ESPFeatureRenderer::RenderGearSummary(drawList, screenPos, compactSummary, distanceFadeAlpha, finalFontSize);
            break;
        }
        case 2: { // Compact (Top 3 Attributes)
            auto dominantStats = ESPPlayerDetailsBuilder::BuildDominantStats(context.player);
            ESPFeatureRenderer::RenderDominantStats(drawList, screenPos, dominantStats, distanceFadeAlpha, finalFontSize);
            break;
        }
        default:
            // Modes 0 (Off) and 3 (Detailed) do not have a separate summary view.
            break;
        }
    }
}

void ESPStageRenderer::RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight,
    const std::vector<RenderablePlayer*>& players, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();

    for (const auto* player : players) {
        if (!player) continue;

        // --- 1. DATA PREPARATION ---
        // Use the builder to prepare all text details. This includes general info and detailed gear view if selected.
        std::vector<ColoredDetail> details = ESPPlayerDetailsBuilder::BuildPlayerDetails(player, settings.playerESP);

        // If "Detailed" mode is selected, build the full gear list and add it to the details.
        if (settings.playerESP.gearDisplayMode == 3) { // 3 = Detailed
            auto gearDetails = ESPPlayerDetailsBuilder::BuildGearDetails(player);
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
        // Use factory to create context and render
        auto context = ESPContextFactory::CreateContextForPlayer(player, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                       const std::vector<RenderableNpc*>& npcs, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* npc : npcs) {
        if (!npc) continue; // Safety check

        // Use the builder to prepare NPC details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildNpcDetails(npc, settings.npcESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForNpc(npc, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

void ESPStageRenderer::RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                          const std::vector<RenderableGadget*>& gadgets, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();
    
    for (const auto* gadget : gadgets) {
        if (!gadget) continue; // Safety check

        // Use the builder to prepare Gadget details
        std::vector<ColoredDetail> details = ESPEntityDetailsBuilder::BuildGadgetDetails(gadget, settings.objectESP, settings.showDebugAddresses);

        auto context = ESPContextFactory::CreateContextForGadget(gadget, settings, details, screenWidth, screenHeight);
        RenderEntity(drawList, context, camera);
    }
}

float ESPStageRenderer::CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    // Use the same logic as ESPFilter for consistency
    return ESPFilter::CalculateDistanceFadeAlpha(distance, useDistanceLimit, distanceLimit);
}

} // namespace kx