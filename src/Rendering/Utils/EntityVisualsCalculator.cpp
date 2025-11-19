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

namespace { // Anonymous namespace for local helpers

} // anonymous namespace

std::optional<VisualProperties> EntityVisualsCalculator::Calculate(const RenderableEntity& entity,
                                                                   const FrameContext& context) {
    VisualProperties props;

    // --- REMOVED PROJECTION CULLING ---
    // We no longer check IsEntityOnScreen here.
    // The background thread shouldn't cull based on Frustum, only Distance.
    // Frustum culling is now handled exclusively in ESPStageRenderer::CalculateLiveVisuals.
    // --------------------------------------------------

    float activeLimit = context.settings.distance.GetActiveDistanceLimit(entity.entityType, context.isInWvW);
    bool useLimitMode = activeLimit > 0.0f;
    props.distanceFadeAlpha = CalculateDistanceFadeAlpha(entity.gameplayDistance, useLimitMode, activeLimit);

    // Optimization: If the entity is too far away (gameplay distance), we can cull it here
    // to save processing time.
    if (props.distanceFadeAlpha <= 0.0f) {
        return std::nullopt;
    }

    // 2. Determine color based on entity type and attitude
    unsigned int color = ESPStyling::GetEntityColor(entity);

    // 3. Apply distance fade to entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(color, props.distanceFadeAlpha);

    // 4. Calculate distance-based scale
    props.scale = CalculateEntityScale(entity.visualDistance, entity.entityType, context);

    // 5. Calculate adaptive alpha
    float normalizedDistance = 0.0f;
    props.finalAlpha = CalculateAdaptiveAlpha(entity.gameplayDistance, props.distanceFadeAlpha,
                                             useLimitMode, entity.entityType,
                                             normalizedDistance);

    // 6. Apply final alpha to the entity color
    props.fadedEntityColor = ESPShapeRenderer::ApplyAlphaToColor(props.fadedEntityColor, props.finalAlpha);

    // 7. Calculate scaled sizes (Abstract sizes, not screen coordinates)
    EntityMultipliers multipliers = CalculateEntityMultipliers(entity);
    CalculateFinalSizes(props, props.scale, multipliers);

    // NOTE: props.screenPos, props.boxMin, props.boxMax are NOT calculated here anymore.
    // They will be zero-initialized. ESPStageRenderer will calculate them using the live camera.

    return props;
}

float EntityVisualsCalculator::CalculateEntityScale(float visualDistance, ESPEntityType entityType, const FrameContext& context) {
    const auto& settings = context.settings;
    
    // Calculate the effective distance, which only starts counting after the "dead zone"
    float effectiveDistance = (std::max)(0.0f, visualDistance - settings.scaling.scalingStartDistance);

    float distanceFactor;
    float scalingExponent;

    float activeLimit = settings.distance.GetActiveDistanceLimit(entityType, context.isInWvW);
    bool useLimitMode = activeLimit > 0.0f;
    if (useLimitMode) {
        // --- LIMIT MODE ---
        // Use the static, user-configured curve for the short 0-90m range
        distanceFactor = settings.scaling.limitDistanceFactor;
        scalingExponent = settings.scaling.limitScalingExponent;
    } else {
        // --- NO LIMIT MODE ---
        if (entityType == ESPEntityType::Gadget || entityType == ESPEntityType::AttackTarget) {
            // GADGETS/ATTACK TARGETS: Use fully adaptive system (these can be 1000m+ away)
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
    case ESPEntityType::AttackTarget:
        // Gadgets/Attack Targets always use circle rendering (see CalculateGadgetDimensions)
        // This case should never be reached - fallback to base box dimensions
        outBoxHeight = settings.sizes.baseBoxHeight * scale;
        outBoxWidth = settings.sizes.baseBoxWidth * scale;
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
    VisualProperties& props,
    bool& outValid)
{
    // Define 8 corners of 3D bounding box in world space
    // Entity position is at feet center
    // Corner order: Bottom face (0-3), Top face (4-7)
    std::vector<glm::vec3> worldCorners = {
        // Bottom 4 corners (at entity feet level)
        entityPos + glm::vec3(-worldWidth/2, 0.0f, -worldDepth/2), // 0: Bottom-Left-Back
        entityPos + glm::vec3( worldWidth/2, 0.0f, -worldDepth/2), // 1: Bottom-Right-Back
        entityPos + glm::vec3(-worldWidth/2, 0.0f,  worldDepth/2), // 2: Bottom-Left-Front
        entityPos + glm::vec3( worldWidth/2, 0.0f,  worldDepth/2), // 3: Bottom-Right-Front
        // Top 4 corners (at entity head level)
        entityPos + glm::vec3(-worldWidth/2, worldHeight, -worldDepth/2), // 4: Top-Left-Back
        entityPos + glm::vec3( worldWidth/2, worldHeight, -worldDepth/2), // 5: Top-Right-Back
        entityPos + glm::vec3(-worldWidth/2, worldHeight,  worldDepth/2), // 6: Top-Left-Front
        entityPos + glm::vec3( worldWidth/2, worldHeight,  worldDepth/2)  // 7: Top-Right-Front
    };
    
    // Project all 8 corners to screen space and store them
    float minX = FLT_MAX, minY = FLT_MAX;
    float maxX = -FLT_MAX, maxY = -FLT_MAX;
    int validCornerCount = 0;
    
    for (int i = 0; i < 8; ++i) {
        if (ESPMath::ProjectToScreen(worldCorners[i], camera, screenWidth, screenHeight, props.projectedCorners[i])) {
            props.cornerValidity[i] = true;
            validCornerCount++;
            
            // Update the 2D bounding box for culling and flat box rendering
            minX = std::min(minX, props.projectedCorners[i].x);
            minY = std::min(minY, props.projectedCorners[i].y);
            maxX = std::max(maxX, props.projectedCorners[i].x);
            maxY = std::max(maxY, props.projectedCorners[i].y);
        } else {
            props.cornerValidity[i] = false;
        }
    }
    
    // The overall projection is valid if at least one corner is in front of the camera
    outValid = (validCornerCount > 0);
    
    if (outValid) {
        // Still provide the 2D min/max for the flat box and culling logic
        props.boxMin = ImVec2(minX, minY);
        props.boxMax = ImVec2(maxX, maxY);
    }
}

void EntityVisualsCalculator::GetWorldBoundsForEntity(
    ESPEntityType entityType,
    float& outWidth,
    float& outDepth,
    float& outHeight)
{
    switch (entityType) {
        case ESPEntityType::Player:
            outWidth = EntityWorldBounds::PLAYER_WORLD_WIDTH;
            outDepth = EntityWorldBounds::PLAYER_WORLD_DEPTH;
            outHeight = EntityWorldBounds::PLAYER_WORLD_HEIGHT;
            break;
        
        case ESPEntityType::AttackTarget:
        case ESPEntityType::Gadget:
            outWidth = EntityWorldBounds::GADGET_WORLD_WIDTH;
            outDepth = EntityWorldBounds::GADGET_WORLD_DEPTH;
            outHeight = EntityWorldBounds::GADGET_WORLD_HEIGHT;
            break;
        
        case ESPEntityType::NPC:
        default:
            outWidth = EntityWorldBounds::NPC_WORLD_WIDTH;
            outDepth = EntityWorldBounds::NPC_WORLD_DEPTH;
            outHeight = EntityWorldBounds::NPC_WORLD_HEIGHT;
            break;
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
    Camera& camera,
    float screenWidth,
    float screenHeight,
    VisualProperties& props,
    float scale)
{
    const auto& settings = AppState::Get().GetSettings();
    
    // Gadgets use circle rendering - calculate radius from base box width
    float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
    props.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * scale);

    // Initial center for gadgets (circle center at screen position)
    // Note: This will be recalculated in CalculateLiveVisuals if using box rendering
    props.center = ImVec2(props.screenPos.x, props.screenPos.y);
    
    // Calculate bounding box - prefer physics dimensions, fallback to reasonable defaults
    float worldWidth, worldDepth, worldHeight;
    
    if (entity.hasPhysicsDimensions) {
        worldWidth = entity.physicsWidth;
        worldDepth = entity.physicsDepth;
        worldHeight = entity.physicsHeight;
    } else {
        // Fallback to reasonable default dimensions
        GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    }
    
    // Project 3D bounding box to screen space
    bool boxValid = false;
    Calculate3DBoundingBox(
        entity.position,
        worldWidth,
        worldDepth,
        worldHeight,
        camera,
        screenWidth,
        screenHeight,
        props,
        boxValid
    );
    
    if (!boxValid) {
        // Fallback to circle-based box if 3D projection fails
        props.boxMin = ImVec2(props.screenPos.x - props.circleRadius, props.screenPos.y - props.circleRadius);
        props.boxMax = ImVec2(props.screenPos.x + props.circleRadius, props.screenPos.y + props.circleRadius);
    }
}

void EntityVisualsCalculator::CalculatePlayerNPCDimensions(
    const RenderableEntity& entity,
    Camera& camera,
    float screenWidth,
    float screenHeight,
    VisualProperties& props,
    float scale)
{
    // Get world-space dimensions - prefer physics dimensions if available
    float worldWidth, worldDepth, worldHeight;
    
    if (entity.hasPhysicsDimensions) {
        // Use actual physics box shape dimensions from game memory
        worldWidth = entity.physicsWidth;
        worldDepth = entity.physicsDepth;
        worldHeight = entity.physicsHeight;
    } else {
        // Fallback to hardcoded constants if physics dimensions unavailable
        GetWorldBoundsForEntity(entity.entityType, worldWidth, worldDepth, worldHeight);
    }
    
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
        props,
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
    
    if (entityType == ESPEntityType::Gadget || entityType == ESPEntityType::AttackTarget) {
        // --- TIER 2: GADGETS/ATTACK TARGETS (Fully Adaptive Fade) ---
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
        // Always enabled for depth perception (consistent with gadget fading)
        // Users control overall visibility via Global Opacity slider
        
        const float fadeStart = AdaptiveScaling::PLAYER_NPC_FADE_START;  // 90m
        const float fadeEnd = AdaptiveScaling::PLAYER_NPC_FADE_END;      // 300m
        const float minAlpha = AdaptiveScaling::PLAYER_NPC_MIN_ALPHA;

        if (gameplayDistance <= fadeStart) {
            return 1.0f; // Fully visible up close
        }
        if (gameplayDistance >= fadeEnd) {
            return minAlpha; // 50% at max range
        }

        // Linear fade from 90m to 300m
        float fadeRange = fadeEnd - fadeStart;
        float progress = (gameplayDistance - fadeStart) / fadeRange;
        return 1.0f - (progress * (1.0f - minAlpha));
    }
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
    const float fadeZonePercentage = RenderingEffects::FADE_ZONE_PERCENTAGE;
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
        const auto& settings = AppState::Get().GetSettings();
        if (player->attitude == Game::Attitude::Hostile) {
            multipliers.hostile = settings.playerESP.hostileBoostMultiplier;
        }
    }
    
    // Calculate rank multiplier
    if (entity.entityType == ESPEntityType::NPC) {
        const auto* npc = static_cast<const RenderableNpc*>(&entity);
        multipliers.rank = ESPStyling::GetRankMultiplier(npc->rank);
    }
    
    // Calculate gadget health multiplier
    if (entity.entityType == ESPEntityType::Gadget || entity.entityType == ESPEntityType::AttackTarget) {
        multipliers.gadgetHealth = ESPStyling::GetGadgetHealthMultiplier(entity.maxHealth);
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
