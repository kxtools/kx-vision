#include "EntityVisualsCalculator.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "ESPMath.h"
#include "ESPConstants.h"
#include "../Data/EntityRenderContext.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Core/ESPFilter.h"
#include <algorithm>
#include <cmath>
#include <Windows.h>

namespace kx {

std::optional<VisualProperties> EntityVisualsCalculator::Calculate(const EntityRenderContext& context,
                                                                   Camera& camera,
                                                                   float screenWidth,
                                                                   float screenHeight) {
    VisualProperties props;
    
    // 0. Calculate interpolated/extrapolated position for smooth rendering (HYBRID MODEL)
    glm::vec3 renderPosition;
    
    if (context.entity && context.entity->lastUpdateTime > 0.0) {
        // Get current time and ESP update interval from settings
        const auto& settings = AppState::Get().GetSettings();
        double currentTime = GetTickCount64() / 1000.0;
        float espUpdateInterval = 1.0f / (std::max)(1.0f, settings.espUpdateRate);
        
        // Calculate how far we are into the current update interval
        double timeSinceUpdate = currentTime - context.entity->lastUpdateTime;
        float alpha = static_cast<float>(timeSinceUpdate / espUpdateInterval);
        
        if (alpha >= 0.0f && alpha <= 1.0f) {
            // --- INTERPOLATION PATH [0, 1] ---
            // Perfectly smooth. Lerp between the last two known positions.
            renderPosition = glm::mix(
                context.entity->previousPosition,
                context.entity->currentPosition,
                alpha
            );
        } else if (alpha > 1.0f && alpha <= RenderingEffects::MAX_EXTRAPOLATION_ALPHA) {
            // --- EXTRAPOLATION PATH (1, 2.0] ---
            // Predict forward using ultra-smoothed velocity for maximum visual fluidity
            // Calculate the exact amount of time we need to predict forward
            float timeToExtrapolate = static_cast<float>(timeSinceUpdate) - espUpdateInterval;
            renderPosition = context.entity->currentPosition + 
                           (context.entity->smoothedVelocity * timeToExtrapolate);
        } else {
            // --- FAILSAFE PATH > 2.0 ---
            // Too much time has passed to make a good prediction. Snap to last known position.
            renderPosition = context.entity->currentPosition;
        }
    } else {
        // Fallback for entities without interpolation data
        renderPosition = context.position;
    }
    
    // 1. Check if entity is on screen (using hybrid interpolated/extrapolated position)
    if (!IsEntityOnScreen(renderPosition, camera, screenWidth, screenHeight, props.screenPos)) {
        return std::nullopt; // Entity is not visible
    }

    // 2. Calculate distance-based fade alpha
    const auto& settings = AppState::Get().GetSettings();
    props.distanceFadeAlpha = ESPFilter::CalculateDistanceFadeAlpha(context.gameplayDistance,
                                                                    settings.distance.useDistanceLimit,
                                                                    settings.distance.renderDistanceLimit);
    
    if (props.distanceFadeAlpha <= 0.0f) {
        return std::nullopt; // Entity is fully transparent
    }
    
    // 3. Apply distance fade to entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(context.color, props.distanceFadeAlpha);

    // 4. Calculate distance-based scale
    props.scale = CalculateEntityScale(context.visualDistance, context.entityType);

    // 5. Calculate rendering dimensions (box or circle)
    if (context.entityType == ESPEntityType::Gadget) {
        // Gadgets use circle rendering - calculate radius from base box width
        float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
        props.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * props.scale);
        
        // For gadgets, screenPos IS the center (no box needed)
        props.center = ImVec2(props.screenPos.x, props.screenPos.y);
        // Set dummy box values for text positioning (will be overridden for circles)
        props.boxMin = ImVec2(props.screenPos.x - props.circleRadius, props.screenPos.y - props.circleRadius);
        props.boxMax = ImVec2(props.screenPos.x + props.circleRadius, props.screenPos.y + props.circleRadius);
    } else {
        // Players/NPCs use traditional box rendering
        float boxWidth, boxHeight;
        CalculateEntityBoxDimensions(context.entityType, props.scale, boxWidth, boxHeight);
        
        props.boxMin = ImVec2(props.screenPos.x - boxWidth / 2, props.screenPos.y - boxHeight);
        props.boxMax = ImVec2(props.screenPos.x + boxWidth / 2, props.screenPos.y);
        props.center = ImVec2(props.screenPos.x, props.screenPos.y - boxHeight / 2);
        props.circleRadius = 0.0f; // No circle for players/NPCs
    }

    // 6. Calculate adaptive alpha
    float normalizedDistance = 0.0f;
    props.finalAlpha = CalculateAdaptiveAlpha(context.gameplayDistance, props.distanceFadeAlpha,
                                             settings.distance.useDistanceLimit, context.entityType,
                                             normalizedDistance);
    
    // Hostile players always render at 100% opacity (critical for PvP combat awareness)
    bool isHostilePlayer = (context.entityType == ESPEntityType::Player && 
                           context.attitude == Game::Attitude::Hostile);
    if (isHostilePlayer) {
        props.finalAlpha = 1.0f;
    }
    
    // Apply final alpha to the entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(props.fadedEntityColor, props.finalAlpha);
    
    // 7. Calculate scaled sizes with limits to prevent extreme values
    // Apply hostile player multiplier for enhanced visibility (PvP combat awareness)
    float hostileMultiplier = isHostilePlayer ? RenderingEffects::HOSTILE_PLAYER_VISUAL_MULTIPLIER : 1.0f;
    
    props.finalFontSize = (std::max)(settings.sizes.minFontSize, 
                                    (std::min)(settings.sizes.baseFontSize * props.scale * hostileMultiplier, 
                                              ScalingLimits::MAX_FONT_SIZE));
    props.finalBoxThickness = (std::max)(ScalingLimits::MIN_BOX_THICKNESS, 
                                        (std::min)(settings.sizes.baseBoxThickness * props.scale * hostileMultiplier, 
                                                  ScalingLimits::MAX_BOX_THICKNESS));
    props.finalDotRadius = (std::max)(ScalingLimits::MIN_DOT_RADIUS, 
                                     (std::min)(settings.sizes.baseDotRadius * props.scale, 
                                               ScalingLimits::MAX_DOT_RADIUS));
    props.finalHealthBarWidth = (std::max)(ScalingLimits::MIN_HEALTH_BAR_WIDTH, 
                                          (std::min)(settings.sizes.baseHealthBarWidth * props.scale * hostileMultiplier, 
                                                    ScalingLimits::MAX_HEALTH_BAR_WIDTH));
    props.finalHealthBarHeight = (std::max)(ScalingLimits::MIN_HEALTH_BAR_HEIGHT, 
                                           (std::min)(settings.sizes.baseHealthBarHeight * props.scale * hostileMultiplier, 
                                                     ScalingLimits::MAX_HEALTH_BAR_HEIGHT));
    
    return props;
}

bool EntityVisualsCalculator::IsEntityOnScreen(const glm::vec3& position, Camera& camera,
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

float EntityVisualsCalculator::CalculateEntityScale(float visualDistance, ESPEntityType entityType) {
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
        // --- NO LIMIT MODE ---
        if (entityType == ESPEntityType::Gadget) {
            // GADGETS: Use fully adaptive system (these can be 1000m+ away)
            // The Distance Factor is calculated dynamically based on the adaptive far plane.
            // We set the 50% scale point to be halfway to the furthest visible group of objects.
            float adaptiveFarPlane = AppState::Get().GetAdaptiveFarPlane();
            
            // Ensure the factor is always reasonable (minimum matches players/NPCs baseline)
            distanceFactor = (std::max)(AdaptiveScaling::GADGET_MIN_DISTANCE_FACTOR, adaptiveFarPlane / 2.0f);
            
            // The user can still control the shape of the curve
            scalingExponent = settings.scaling.noLimitScalingExponent;
        } else {
            // PLAYERS & NPCs: Use fixed scaling (they're limited to ~200m by game mechanics)
            // No need for adaptive system - they never go beyond 200m
            distanceFactor = AdaptiveScaling::PLAYER_NPC_DISTANCE_FACTOR;
            scalingExponent = settings.scaling.noLimitScalingExponent;
        }
    }
    
    // Calculate scale using the dynamically determined parameters
    float rawScale = distanceFactor / (distanceFactor + pow(effectiveDistance, scalingExponent));

    // Clamp to min/max bounds
    return (std::max)(settings.scaling.minScale, (std::min)(rawScale, settings.scaling.maxScale));
}

void EntityVisualsCalculator::CalculateEntityBoxDimensions(ESPEntityType entityType, float scale,
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
        // NPCs use square boxes - same width as players for visual consistency
        outBoxHeight = settings.sizes.baseBoxWidth * scale;  // Use baseBoxWidth directly (45px)
        outBoxWidth = settings.sizes.baseBoxWidth * scale;   // Square = width x width
        if (outBoxHeight < MinimumSizes::NPC_MIN_HEIGHT) {
            outBoxHeight = MinimumSizes::NPC_MIN_HEIGHT;
            outBoxWidth = MinimumSizes::NPC_MIN_WIDTH;
        }
        break;
        
    case ESPEntityType::Gadget:
        // NOTE: This case is unused - gadgets use circle rendering (see Calculate())
        // Keeping for safety/fallback, but gadgets are rendered as circles with radius = baseBoxWidth x 0.15
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

float EntityVisualsCalculator::CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha,
                                                      bool useDistanceLimit, ESPEntityType entityType,
                                                      float& outNormalizedDistance) {
    const auto& settings = AppState::Get().GetSettings();
    outNormalizedDistance = 0.0f; // Initialize output
    
    if (useDistanceLimit) {
        // --- TIER 1: RENDER LIMIT MODE ---
        // Goal: Natural Integration - clean, seamless extension of game's UI
        // Uses the simple 80-90m fade curve from ESPFilter
        return distanceFadeAlpha;
    }

    // --- "NO LIMIT" MODE LOGIC (THREE-TIERED SYSTEM) ---
    
    if (entityType == ESPEntityType::Gadget) {
        // --- TIER 2: GADGETS (Fully Adaptive Fade) ---
        // Goal: Maximum Information Clarity - handle extreme distances (1000m+)
        float finalAlpha = 1.0f; // Default to fully visible
        
        const float farPlane = AppState::Get().GetAdaptiveFarPlane(); // Intelligent, scene-aware range
        const float effectStartDistance = AdaptiveScaling::FADE_START_DISTANCE; // 90m - beyond game's culling
        
        if (gameplayDistance > effectStartDistance) {
            // Calculate normalized distance (0.0 at effectStartDistance, 1.0 at farPlane)
            float range = farPlane - effectStartDistance;
            if (range > 0.0f) {
                float progress = (gameplayDistance - effectStartDistance) / range;
                outNormalizedDistance = (std::clamp)(progress, 0.0f, 1.0f);
            }
            
            // Atmospheric perspective: Linear fade from 100% to minimum
            finalAlpha = 1.0f - outNormalizedDistance;
            finalAlpha = (std::max)(AdaptiveScaling::MIN_ALPHA, finalAlpha); // 50% minimum for readability
            
            // Future: LOD effects can use normalizedDistance here
        }
        
        return finalAlpha;
    }
    else {
        // --- TIER 3: PLAYERS & NPCs (Subtle Fixed-Range Fade) ---
        // Goal: Depth perception without compromising combat clarity
        
        if (!settings.distance.enablePlayerNpcFade) {
            return 1.0f; // Effect disabled - always 100% opaque
        }

        const float fadeStart = AdaptiveScaling::PLAYER_NPC_FADE_START;
        const float fadeEnd = AdaptiveScaling::PLAYER_NPC_FADE_END;

        if (gameplayDistance <= fadeStart) {
            return 1.0f; // Fully opaque up close
        }
        if (gameplayDistance >= fadeEnd) {
            return settings.distance.playerNpcMinAlpha; // Clamp to user-defined minimum at max range
        }

        // Linear interpolation within the fixed fade zone
        float fadeRange = fadeEnd - fadeStart;
        float progress = (gameplayDistance - fadeStart) / fadeRange;
        return 1.0f - (progress * (1.0f - settings.distance.playerNpcMinAlpha));
    }
}

} // namespace kx
