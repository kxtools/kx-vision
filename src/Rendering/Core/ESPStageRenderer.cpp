#include "ESPStageRenderer.h"
#include "../Renderers/ESPContextFactory.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPConstants.h"
#include "../Utils/ESPPlayerDetailsBuilder.h"
#include "../Utils/ESPEntityDetailsBuilder.h"
#include "ESPFilter.h"
#include "../Renderers/ESPFeatureRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../../libs/ImGui/imgui.h"
#include <algorithm>

namespace kx {

void ESPStageRenderer::RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                      const PooledFrameRenderData& frameData, Camera& camera) {
    // Simple rendering - no filtering logic, just draw everything that was passed in
    RenderPooledPlayers(drawList, screenWidth, screenHeight, frameData.players, camera);
    RenderPooledNpcs(drawList, screenWidth, screenHeight, frameData.npcs, camera);
    RenderPooledGadgets(drawList, screenWidth, screenHeight, frameData.gadgets, camera);
}

// === Entity Rendering Helper Functions ===

bool ESPStageRenderer::IsEntityOnScreen(const glm::vec3& position, Camera& camera, 
                                       float screenWidth, float screenHeight, glm::vec2& outScreenPos) {
    // Calculate screen position every frame for smooth movement
    if (!ESPMath::WorldToScreen(position, camera, screenWidth, screenHeight, outScreenPos)) {
        return false; // Entity is behind camera or invalid projection
    }
    
    // Screen bounds culling with small margin for partially visible entities
    const float margin = ScreenCulling::VISIBILITY_MARGIN;
    if (outScreenPos.x < -margin || outScreenPos.x > screenWidth + margin || 
        outScreenPos.y < -margin || outScreenPos.y > screenHeight + margin) {
        return false; // Entity is off-screen
    }
    
    return true;
}

float ESPStageRenderer::CalculateEntityScale(float visualDistance) {
    const auto& settings = AppState::Get().GetSettings();
    
    // Calculate the effective distance, which only starts counting after the "dead zone"
    float effectiveDistance = (std::max)(0.0f, visualDistance - settings.scaling.scalingStartDistance);

    float distanceFactor;
    float scalingExponent;

    if (settings.distance.useDistanceLimit) {
        // --- LIMIT MODE ---
        // Use the static, user-configured curve for the short 0-90m range
        distanceFactor = settings.scaling.limitDistanceFactor;
        scalingExponent = settings.scaling.limitScalingExponent;
    } else {
        // --- NO LIMIT MODE (FULLY DYNAMIC) ---
        // The Distance Factor is now calculated dynamically based on the adaptive far plane.
        // We set the 50% scale point to be halfway to the furthest visible group of entities.
        float adaptiveFarPlane = AppState::Get().GetAdaptiveFarPlane();
        
        // Ensure the factor is always a reasonable value (minimum 250m for 50% scale point)
        distanceFactor = (std::max)(250.0f, adaptiveFarPlane / 2.0f);
        
        // The user can still control the shape of the curve
        scalingExponent = settings.scaling.noLimitScalingExponent;
    }
    
    // Calculate scale using the dynamically determined parameters
    float rawScale = distanceFactor / (distanceFactor + pow(effectiveDistance, scalingExponent));

    // Clamp to min/max bounds
    return (std::max)(settings.scaling.minScale, (std::min)(rawScale, settings.scaling.maxScale));
}

void ESPStageRenderer::CalculateEntityBoxDimensions(ESPEntityType entityType, float scale, 
                                                   float& outBoxWidth, float& outBoxHeight) {
    const auto& settings = AppState::Get().GetSettings();
    
    switch (entityType) {
    case ESPEntityType::Player:
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        if (outBoxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
        }
        break;
        
    case ESPEntityType::NPC:
        // For NPCs, use a square based on a smaller version of the player box WIDTH
        outBoxHeight = (settings.sizes.baseBoxWidth * 0.8f) * scale;
        outBoxWidth = (settings.sizes.baseBoxWidth * 0.8f) * scale;
        if (outBoxHeight < MinimumSizes::NPC_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::NPC_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::NPC_MIN_WIDTH;
        }
        break;
        
    case ESPEntityType::Gadget:
        // Gadgets can remain very small
        outBoxHeight = (settings.sizes.baseBoxWidth * 0.3f) * scale;
        outBoxWidth = (settings.sizes.baseBoxWidth * 0.3f) * scale;
        if (outBoxHeight < MinimumSizes::GADGET_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::GADGET_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::GADGET_MIN_WIDTH;
        }
        break;
        
    default:
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
        if (outBoxHeight < MinimumSizes::PLAYER_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::PLAYER_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::PLAYER_MIN_WIDTH;
        }
        break;
    }
}

float ESPStageRenderer::CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha,
                                               bool useDistanceLimit, float& outNormalizedDistance) {
    outNormalizedDistance = 0.0f; // Initialize output
    
    if (useDistanceLimit) {
        // --- RENDER LIMIT MODE ---
        // Goal: Natural Integration - clean, seamless extension of game's UI
        // Use the pre-calculated distance fade alpha from ESPFilter
        return distanceFadeAlpha;
    } else {
        // --- NO LIMIT MODE (ADAPTIVE SYSTEM) ---
        // Goal: Maximum Information Clarity - present vast amounts of data readably
        float finalAlpha = 1.0f; // Default to fully visible
        
        const float farPlane = AppState::Get().GetAdaptiveFarPlane(); // Get the intelligent, pre-calculated adaptive range
        const float effectStartDistance = 90.0f; // Match game's natural entity range - effects begin beyond game's culling distance
        
        if (gameplayDistance > effectStartDistance) {
            // Calculate normalized distance (0.0 at effectStartDistance, 1.0 at farPlane)
            float range = farPlane - effectStartDistance;
            if (range > 0.0f) {
                float progress = (gameplayDistance - effectStartDistance) / range;
                outNormalizedDistance = (std::clamp)(progress, 0.0f, 1.0f);
            }
            
            // --- TECHNIQUE 1: Far-Distance Alpha Fading (Atmospheric Perspective) ---
            // Simulates atmospheric haze - objects become fainter with distance
            // Linearly interpolate alpha from 1.0 (opaque) down to a minimum visibility
            // In "No Limit" mode, prioritize clarity - even far entities must be readable
            finalAlpha = 1.0f - outNormalizedDistance;
            finalAlpha = (std::max)(0.5f, finalAlpha); // Clamp to minimum 50% opacity for readability
            
            // Future LOD effects can use normalizedDistance here
            // For example: reduce detail, simplify rendering, etc.
        }
        
        return finalAlpha;
    }
}

void ESPStageRenderer::RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                             const glm::vec2& screenPos, const ImVec2& boxMin, const ImVec2& boxMax,
                                             const ImVec2& center, unsigned int fadedEntityColor, 
                                             float distanceFadeAlpha, float scale) {
    const auto& settings = AppState::Get().GetSettings();
    bool isLivingEntity = (context.entityType == ESPEntityType::Player || context.entityType == ESPEntityType::NPC);
    
    // --- ADAPTIVE DISTANCE EFFECTS (New System) ---
    // This replaces the old "finalAlpha" approach with two distinct modes:
    // 1. Render Limit Mode (ON): Natural Integration - respects game's rules, uses distance-based fade
    // 2. No Limit Mode (OFF): Maximum Information Clarity - uses adaptive far plane for intelligent scaling
    
    float normalizedDistance = 0.0f; // Used for LOD effects in No Limit mode
    float finalAlpha = CalculateAdaptiveAlpha(context.gameplayDistance, distanceFadeAlpha, 
                                             settings.distance.useDistanceLimit, normalizedDistance);
    
    // Apply final alpha to the entity color
    fadedEntityColor = ESPFeatureRenderer::ApplyAlphaToColor(fadedEntityColor, finalAlpha);
    
    // Calculate scaled sizes
    const float finalFontSize = (std::max)(settings.sizes.minFontSize, (std::min)(settings.sizes.baseFontSize * scale, 40.0f));
    const float finalBoxThickness = (std::max)(1.0f, (std::min)(settings.sizes.baseBoxThickness * scale, 10.0f));
    const float finalDotRadius = (std::max)(1.0f, (std::min)(settings.sizes.baseDotRadius * scale, 15.0f));
    const float finalHealthBarWidth = (std::max)(10.0f, (std::min)(settings.sizes.baseHealthBarWidth * scale, 100.0f));
    const float finalHealthBarHeight = (std::max)(2.0f, (std::min)(settings.sizes.baseHealthBarHeight * scale, 20.0f));
    
    // Render standalone health bars for living entities when health is available AND setting is enabled
    if (isLivingEntity && context.healthPercent >= 0.0f && context.renderHealthBar) {
        ESPFeatureRenderer::RenderStandaloneHealthBar(drawList, screenPos, context.healthPercent, 
                                                     fadedEntityColor, finalHealthBarWidth, finalHealthBarHeight,
                                                     context.entityType, context.attitude);
    }

    // Render bounding box (should be disabled by default for living entities)
    if (context.renderBox) {
        ESPFeatureRenderer::RenderBoundingBox(drawList, boxMin, boxMax, fadedEntityColor, finalBoxThickness);
    }

    // Render distance text
    if (context.renderDistance) {
        ESPFeatureRenderer::RenderDistanceText(drawList, center, boxMin, context.gameplayDistance, 
                                              finalAlpha, finalFontSize);
    }

    // Render center dot
    if (context.renderDot) {
        if (context.entityType == ESPEntityType::Gadget) {
            ESPFeatureRenderer::RenderNaturalWhiteDot(drawList, screenPos, finalAlpha, finalDotRadius);
        } else {
            ESPFeatureRenderer::RenderColoredDot(drawList, screenPos, fadedEntityColor, finalDotRadius);
        }
    }

    // Render player name for natural identification (players only)
    if (context.entityType == ESPEntityType::Player && context.renderPlayerName) {
        // For hostile players, display "HOSTILE" label instead of their name
        std::string displayName = (context.attitude == Game::Attitude::Hostile) ? "HOSTILE" : context.playerName;
        if (!displayName.empty()) {
            // Use entity color directly (already attitude-based from ESPContextFactory)
            ESPFeatureRenderer::RenderPlayerName(drawList, screenPos, displayName, fadedEntityColor, finalFontSize);
        }
    }

    // Render details text (for all entities when enabled)
    if (context.renderDetails && !context.details.empty()) {
        ESPFeatureRenderer::RenderDetailsText(drawList, center, boxMax, context.details, finalAlpha, finalFontSize);
    }

    // Specialized Summary Rendering (Players Only)
    if (context.entityType == ESPEntityType::Player && context.player != nullptr) {
        switch (settings.playerESP.gearDisplayMode) {
        case GearDisplayMode::Compact: { // Compact (Stat Names)
            auto compactSummary = ESPPlayerDetailsBuilder::BuildCompactGearSummary(context.player);
            ESPFeatureRenderer::RenderGearSummary(drawList, screenPos, compactSummary, finalAlpha, finalFontSize);
            break;
        }
        case GearDisplayMode::Attributes: { // Top 3 Attributes
            auto dominantStats = ESPPlayerDetailsBuilder::BuildDominantStats(context.player);
            ESPFeatureRenderer::RenderDominantStats(drawList, screenPos, dominantStats, finalAlpha, finalFontSize);
            break;
        }
        default:
            // Modes Off and Detailed do not have a separate summary view
            break;
        }
    }
}

void ESPStageRenderer::RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera) {
    // 1. Check if entity is on screen
    glm::vec2 screenPos;
    if (!IsEntityOnScreen(context.position, camera, context.screenWidth, context.screenHeight, screenPos)) {
        return; // Entity is not visible
    }

    // 2. Calculate distance-based fade alpha
    const auto& settings = AppState::Get().GetSettings();
    float distanceFadeAlpha = CalculateEntityDistanceFadeAlpha(context.gameplayDistance,
                                                              settings.distance.useDistanceLimit, 
                                                              settings.distance.renderDistanceLimit);
    
    if (distanceFadeAlpha <= 0.0f) {
        return; // Entity is fully transparent
    }
    
    // 3. Apply distance fade to entity color
    unsigned int fadedEntityColor = ESPFeatureRenderer::ApplyAlphaToColor(context.color, distanceFadeAlpha);

    // 4. Calculate distance-based scale
    float scale = CalculateEntityScale(context.visualDistance);

    // 5. Calculate bounding box dimensions
    float boxWidth, boxHeight;
    CalculateEntityBoxDimensions(context.entityType, scale, boxWidth, boxHeight);
    
    ImVec2 boxMin(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    ImVec2 boxMax(screenPos.x + boxWidth / 2, screenPos.y);
    ImVec2 center(screenPos.x, screenPos.y - boxHeight / 2);

    // 6. Render all visual components
    RenderEntityComponents(drawList, context, screenPos, boxMin, boxMax, center, 
                          fadedEntityColor, distanceFadeAlpha, scale);
}

void ESPStageRenderer::RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight,
    const std::vector<RenderablePlayer*>& players, Camera& camera) {
    const auto& settings = AppState::Get().GetSettings();

    for (const auto* player : players) {
        if (!player) continue;

        // --- 1. DATA PREPARATION ---
        // Use the builder to prepare all text details. This includes general info and detailed gear view if selected.
        std::vector<ColoredDetail> details = ESPPlayerDetailsBuilder::BuildPlayerDetails(player, settings.playerESP, settings.showDebugAddresses);

        // If "Detailed" mode is selected, build the full gear list and add it to the details.
        if (settings.playerESP.gearDisplayMode == GearDisplayMode::Detailed) {
            auto gearDetails = ESPPlayerDetailsBuilder::BuildGearDetails(player);
            if (!gearDetails.empty()) {
                if (!details.empty()) {
                    details.push_back({ "--- Gear Stats ---", ESPColors::DEFAULT_TEXT });
                }
                details.insert(details.end(), gearDetails.begin(), gearDetails.end());
            }
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