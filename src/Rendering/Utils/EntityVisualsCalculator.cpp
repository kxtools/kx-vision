#define NOMINMAX

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
        CalculateGadgetDimensions(entity, props, props.scale);
    } else {
        CalculatePlayerNPCDimensions(entity, camera, screenWidth, screenHeight, props, props.scale);
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

void EntityVisualsCalculator::Calculate3DBoundingBox(
    const glm::vec3& entityPos,
    float worldWidth,
    float worldDepth,
    float worldHeight,
    Camera& camera,
    float screenWidth,
    float screenHeight,
    ImVec2& outBoxMin,
    ImVec2& outBoxMax,
    bool& outValid)
{
    // Define 8 corners of 3D bounding box in world space
    // Entity position is at feet center
    std::vector<glm::vec3> worldCorners = {
        // Bottom 4 corners (at entity feet level)
        entityPos + glm::vec3(-worldWidth/2, 0.0f, -worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, 0.0f, -worldDepth/2),
        entityPos + glm::vec3(-worldWidth/2, 0.0f,  worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, 0.0f,  worldDepth/2),
        // Top 4 corners (at entity head level)
        entityPos + glm::vec3(-worldWidth/2, worldHeight, -worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, worldHeight, -worldDepth/2),
        entityPos + glm::vec3(-worldWidth/2, worldHeight,  worldDepth/2),
        entityPos + glm::vec3( worldWidth/2, worldHeight,  worldDepth/2)
    };
    
    // Project all 8 corners to screen space
    float minX = FLT_MAX, minY = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;
    int validCorners = 0;
    
    for (const auto& corner : worldCorners) {
        glm::vec2 screenCorner;
        if (ESPMath::WorldToScreen(corner, camera, screenWidth, screenHeight, screenCorner)) {
            minX = std::min(minX, screenCorner.x);
            minY = std::min(minY, screenCorner.y);
            maxX = std::max(maxX, screenCorner.x);
            maxY = std::max(maxY, screenCorner.y);
            validCorners++;
        }
    }
    
    // Box is valid if at least 3 corners projected successfully
    outValid = (validCorners >= 3);
    
    if (outValid) {
        outBoxMin = ImVec2(minX, minY);
        outBoxMax = ImVec2(maxX, maxY);
    }
}

void EntityVisualsCalculator::GetWorldBoundsForEntity(
    ESPEntityType entityType,
    float& outWidth,
    float& outDepth,
    float& outHeight)
{
    if (entityType == ESPEntityType::Player) {
        outWidth = EntityWorldBounds::PLAYER_WORLD_WIDTH;
        outDepth = EntityWorldBounds::PLAYER_WORLD_DEPTH;
        outHeight = EntityWorldBounds::PLAYER_WORLD_HEIGHT;
    } else {
        outWidth = EntityWorldBounds::NPC_WORLD_WIDTH;
        outDepth = EntityWorldBounds::NPC_WORLD_DEPTH;
        outHeight = EntityWorldBounds::NPC_WORLD_HEIGHT;
    }
}

void EntityVisualsCalculator::ApplyFallback2DBox(
    const RenderableEntity& entity,
    VisualProperties& props,
    float scale,
    const glm::vec2& screenPos)
{
    float boxWidth, boxHeight;
    CalculateEntityBoxDimensions(entity.entityType, scale, boxWidth, boxHeight);
    props.boxMin = ImVec2(screenPos.x - boxWidth / 2, screenPos.y - boxHeight);
    props.boxMax = ImVec2(screenPos.x + boxWidth / 2, screenPos.y);
}

void EntityVisualsCalculator::CalculateGadgetDimensions(
    const RenderableEntity& entity,
    VisualProperties& props,
    float scale)
{
    const auto& settings = AppState::Get().GetSettings();
    
    // Gadgets use circle rendering - calculate radius from base box width
    float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
    props.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * scale);

    // For gadgets, screenPos IS the center (no box needed)
    props.center = ImVec2(props.screenPos.x, props.screenPos.y);
    
    // Set dummy box values for text positioning (will be overridden for circles)
    props.boxMin = ImVec2(props.screenPos.x - props.circleRadius, props.screenPos.y - props.circleRadius);
    props.boxMax = ImVec2(props.screenPos.x + props.circleRadius, props.screenPos.y + props.circleRadius);
}

void EntityVisualsCalculator::CalculatePlayerNPCDimensions(
    const RenderableEntity& entity,
    Camera& camera,
    float screenWidth,
    float screenHeight,
    VisualProperties& props,
    float scale)
{
    // Get world-space dimensions for entity type
    float worldWidth, worldDepth, worldHeight;
    GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    
    // Try 3D bounding box projection
    bool boxValid = false;
    Calculate3DBoundingBox(
        entity.position,
        worldWidth,
        worldDepth,
        worldHeight,
        camera,
        screenWidth,
        screenHeight,
        props.boxMin,
        props.boxMax,
        boxValid
    );
    
    // Fallback to 2D method if 3D projection fails (edge cases)
    if (!boxValid) {
        ApplyFallback2DBox(entity, props, scale, props.screenPos);
    }
    
    // Calculate center from projected box
    props.center = ImVec2(
        (props.boxMin.x + props.boxMax.x) / 2,
        (props.boxMin.y + props.boxMax.y) / 2
    );
    props.circleRadius = 0.0f; // No circle for players/NPCs
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
        return DamageNumberScaling::MIN_MULTIPLIER;
    }

    const float MIN_MULTIPLIER = DamageNumberScaling::MIN_MULTIPLIER;
    const float MAX_MULTIPLIER = DamageNumberScaling::MAX_MULTIPLIER;
    const float DAMAGE_TO_REACH_MAX = DamageNumberScaling::DAMAGE_TO_REACH_MAX;
    
    float progress = damageToDisplay / DAMAGE_TO_REACH_MAX;
    progress = (std::min)(progress, 1.0f);

    return MIN_MULTIPLIER + progress * (MAX_MULTIPLIER - MIN_MULTIPLIER);
}

// Helper method implementations
float EntityVisualsCalculator::GetRankMultiplier(Game::CharacterRank rank) {
    switch (rank) {
        case Game::CharacterRank::Veteran:    return RankMultipliers::VETERAN;
        case Game::CharacterRank::Elite:      return RankMultipliers::ELITE;
        case Game::CharacterRank::Champion:   return RankMultipliers::CHAMPION;
        case Game::CharacterRank::Legendary:  return RankMultipliers::LEGENDARY;
        default:                              return RankMultipliers::NORMAL;
    }
}

float EntityVisualsCalculator::GetGadgetHealthMultiplier(float maxHealth) {
    if (maxHealth <= 0.0f) {
        return 1.0f; // No boost for invalid/zero health
    }
    
    const float MIN_MULTIPLIER = GadgetHealthScaling::MIN_MULTIPLIER;    // Normal gadgets
    const float MAX_MULTIPLIER = GadgetHealthScaling::MAX_MULTIPLIER;    // Epic structures (matches legendary rank)
    const float HP_TO_REACH_MAX = GadgetHealthScaling::HP_TO_REACH_MAX; // 1M HP = max emphasis
    
    float progress = maxHealth / HP_TO_REACH_MAX;
    progress = (std::min)(progress, 1.0f); // Clamp to prevent > 2.0x
    
    return MIN_MULTIPLIER + progress * (MAX_MULTIPLIER - MIN_MULTIPLIER);
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
