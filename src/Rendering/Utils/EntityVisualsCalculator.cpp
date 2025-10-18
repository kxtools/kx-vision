#include "EntityVisualsCalculator.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "ESPMath.h"
#include "ESPConstants.h"
#include "ESPStyling.h"
#include "../Data/RenderableData.h"
#include "../Renderers/ESPShapeRenderer.h"
#include <algorithm>
#include <cmath>

namespace kx {


std::optional<VisualProperties> EntityVisualsCalculator::Calculate(const RenderableEntity& entity,
                                                                   Camera& camera,
                                                                   float screenWidth,
                                                                   float screenHeight) {
    VisualProperties props;

    // 1. Check if entity is on screen
    if (!IsEntityOnScreen(entity.position, camera, screenWidth, screenHeight, props.screenPos)) {
        return std::nullopt; // Entity is not visible
    }

    // Determine color based on entity type and attitude
    unsigned int color = ESPStyling::GetEntityColor(entity);

    // 2. Calculate distance-based fade alpha
    const auto& settings = AppState::Get().GetSettings();
    props.distanceFadeAlpha = CalculateDistanceFadeAlpha(entity.gameplayDistance,
                                                         settings.distance.useDistanceLimit,
                                                         settings.distance.renderDistanceLimit);

    if (props.distanceFadeAlpha <= 0.0f) {
        return std::nullopt; // Entity is fully transparent
    }

    // 3. Apply distance fade to entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(color, props.distanceFadeAlpha);

    // 4. Calculate distance-based scale
    props.scale = CalculateEntityScale(entity.visualDistance, entity.entityType);

    // 5. Calculate rendering dimensions (box or circle)
    if (entity.entityType == ESPEntityType::Gadget) {
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
        CalculateEntityBoxDimensions(entity.entityType, props.scale, boxWidth, boxHeight);

        props.boxMin = ImVec2(props.screenPos.x - boxWidth / 2, props.screenPos.y - boxHeight);
        props.boxMax = ImVec2(props.screenPos.x + boxWidth / 2, props.screenPos.y);
        props.center = ImVec2(props.screenPos.x, props.screenPos.y - boxHeight / 2);
        props.circleRadius = 0.0f; // No circle for players/NPCs
    }

    // 6. Calculate adaptive alpha
    float normalizedDistance = 0.0f;
    props.finalAlpha = CalculateAdaptiveAlpha(entity.gameplayDistance, props.distanceFadeAlpha,
                                             settings.distance.useDistanceLimit, entity.entityType,
                                             normalizedDistance);

    // Removed: Hostile players now fade naturally with distance for better depth perception
    // Red color + 2x text/health bars provide sufficient emphasis

    // Apply final alpha to the entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(props.fadedEntityColor, props.finalAlpha);

    // 7. Calculate scaled sizes with limits
    EntityMultipliers multipliers = CalculateEntityMultipliers(entity);
    CalculateFinalSizes(props, props.scale, multipliers);

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

float EntityVisualsCalculator::GetDamageNumberFontSizeMultiplier(float damageToDisplay) {
    if (damageToDisplay <= 0.0f) {
        return 2.0f; // Minimum multiplier for any damage
    }

    const float MIN_MULTIPLIER = 2.0f; // Minimum multiplier for any damage
    const float MAX_MULTIPLIER = 4.0f; // Cap the multiplier
    const float DAMAGE_TO_REACH_MAX_MULTIPLIER = 200000.0f; // Damage at which multiplier reaches MAX_MULTIPLIER

    // Calculate how much additional multiplier to add based on damage
    // The scaling range is (MAX_MULTIPLIER - MIN_MULTIPLIER)
    // We want to scale from MIN_MULTIPLIER to MAX_MULTIPLIER over DAMAGE_TO_REACH_MAX_MULTIPLIER
    
    float progress = damageToDisplay / DAMAGE_TO_REACH_MAX_MULTIPLIER;
    progress = (std::min)(progress, 1.0f); // Clamp progress to 1.0

    float multiplier = MIN_MULTIPLIER + progress * (MAX_MULTIPLIER - MIN_MULTIPLIER);
    
    return multiplier; // No need for std::min with MAX_MULTIPLIER here, as progress is clamped
}

// Helper method implementations
float EntityVisualsCalculator::GetRankMultiplier(Game::CharacterRank rank) {
    switch (rank) {
        case Game::CharacterRank::Veteran:    return 1.25f;
        case Game::CharacterRank::Elite:      return 1.5f;
        case Game::CharacterRank::Champion:   return 1.75f;
        case Game::CharacterRank::Legendary:  return 2.0f;
        default:                              return 1.0f;
    }
}

float EntityVisualsCalculator::GetGadgetHealthMultiplier(float maxHealth) {
    if (maxHealth >= 1000000.0f) return 2.0f;
    if (maxHealth >= 500000.0f) return 1.75f;
    if (maxHealth >= 250000.0f) return 1.5f;
    if (maxHealth >= 100000.0f) return 1.25f;
    return 1.0f;
}

float EntityVisualsCalculator::CalculateFinalSize(float baseSize, float scale, float minLimit, float maxLimit, float multiplier) {
    float scaledSize = baseSize * scale * multiplier;
    return std::clamp(scaledSize, minLimit, maxLimit);
}

float EntityVisualsCalculator::CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    if (!useDistanceLimit) {
        return 1.0f; // Fully visible when no distance limit
    }
    
    // Calculate fade zone distances
    const float fadeZonePercentage = 0.11f; // RenderingEffects::FADE_ZONE_PERCENTAGE
    const float fadeZoneDistance = distanceLimit * fadeZonePercentage;
    const float fadeStartDistance = distanceLimit - fadeZoneDistance; // e.g., 80m for 90m limit
    const float fadeEndDistance = distanceLimit; // e.g., 90m for 90m limit
    
    if (distance <= fadeStartDistance) {
        return 1.0f; // Fully visible
    } else if (distance >= fadeEndDistance) {
        return 0.0f; // Fully transparent (should be culled in filter)
    } else {
        // Linear interpolation in fade zone
        const float fadeProgress = (distance - fadeStartDistance) / fadeZoneDistance;
        return 1.0f - fadeProgress; // Fade from 1.0 to 0.0
    }
}

EntityMultipliers EntityVisualsCalculator::CalculateEntityMultipliers(const RenderableEntity& entity) {
    EntityMultipliers multipliers;
    
    // Calculate hostile multiplier
    if (entity.entityType == ESPEntityType::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(&entity);
        if (player->attitude == Game::Attitude::Hostile) {
            multipliers.hostile = RenderingEffects::HOSTILE_PLAYER_VISUAL_MULTIPLIER;
        }
    }
    
    // Calculate rank multiplier
    if (entity.entityType == ESPEntityType::NPC) {
        const auto* npc = static_cast<const RenderableNpc*>(&entity);
        multipliers.rank = GetRankMultiplier(npc->rank);
    }
    
    // Calculate gadget health multiplier
    if (entity.entityType == ESPEntityType::Gadget) {
        multipliers.gadgetHealth = GetGadgetHealthMultiplier(entity.maxHealth);
    }
    
    // Calculate combined health bar multiplier
    multipliers.healthBar = multipliers.hostile * multipliers.rank * multipliers.gadgetHealth;
    
    return multipliers;
}

void EntityVisualsCalculator::CalculateFinalSizes(VisualProperties& props, 
                                                 float scale,
                                                 const EntityMultipliers& multipliers) {
    const auto& settings = AppState::Get().GetSettings();
    
    // Font size uses hostile multiplier (combat-critical: keep 2x for readability)
    props.finalFontSize = CalculateFinalSize(settings.sizes.baseFontSize, scale, settings.sizes.minFontSize, ScalingLimits::MAX_FONT_SIZE, multipliers.hostile);
    
    // Box thickness NO hostile multiplier (reduce visual clutter)
    props.finalBoxThickness = CalculateFinalSize(settings.sizes.baseBoxThickness, scale, ScalingLimits::MIN_BOX_THICKNESS, ScalingLimits::MAX_BOX_THICKNESS, 1.0f);
    
    // Dot radius never uses hostile multiplier
    props.finalDotRadius = CalculateFinalSize(settings.sizes.baseDotRadius, scale, ScalingLimits::MIN_DOT_RADIUS, ScalingLimits::MAX_DOT_RADIUS);

    // Health bar uses combined multiplier (combat-critical: keep 2x for health visibility)
    props.finalHealthBarWidth = CalculateFinalSize(settings.sizes.baseHealthBarWidth, scale, ScalingLimits::MIN_HEALTH_BAR_WIDTH, ScalingLimits::MAX_HEALTH_BAR_WIDTH, multipliers.healthBar);
    props.finalHealthBarHeight = CalculateFinalSize(settings.sizes.baseHealthBarHeight, scale, ScalingLimits::MIN_HEALTH_BAR_HEIGHT, ScalingLimits::MAX_HEALTH_BAR_HEIGHT, multipliers.healthBar);
}

} // namespace kx
