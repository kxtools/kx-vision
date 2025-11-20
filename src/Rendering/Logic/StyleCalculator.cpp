#define NOMINMAX

#include "StyleCalculator.h"
#include "../../Core/AppState.h"
#include "../Data/RenderableData.h"
#include "../Renderers/ShapeRenderer.h"
#include "../Shared/ScalingConstants.h"
#include "Presentation/Styling.h"
#include <algorithm>
#include <cmath>

namespace kx::Logic {

bool StyleCalculator::Calculate(const RenderableEntity& entity,
                                 const FrameContext& context,
                                 VisualStyle& outStyle) {
    float activeLimit = context.settings.distance.GetActiveDistanceLimit(entity.entityType, context.isInWvW);
    bool useLimitMode = activeLimit > 0.0f;
    outStyle.distanceFadeAlpha = CalculateDistanceFadeAlpha(entity.gameplayDistance, useLimitMode, activeLimit);

    if (outStyle.distanceFadeAlpha <= 0.0f) {
        return false;
    }

    unsigned int color = Styling::GetEntityColor(entity);

    outStyle.fadedEntityColor = ShapeRenderer::ApplyAlphaToColor(color, outStyle.distanceFadeAlpha);

    outStyle.scale = CalculateEntityScale(entity.visualDistance, entity.entityType, context);

    float normalizedDistance = 0.0f;
    outStyle.finalAlpha = CalculateAdaptiveAlpha(entity.gameplayDistance, outStyle.distanceFadeAlpha,
                                             useLimitMode, entity.entityType,
                                             normalizedDistance);

    outStyle.fadedEntityColor = ShapeRenderer::ApplyAlphaToColor(outStyle.fadedEntityColor, outStyle.finalAlpha);

    EntityMultipliers multipliers = CalculateEntityMultipliers(entity);
    CalculateFinalSizes(outStyle, outStyle.scale, multipliers);

    return true;
}

float StyleCalculator::CalculateEntityScale(float visualDistance, EntityTypes entityType, const FrameContext& context) {
    const auto& settings = context.settings;
    
    float effectiveDistance = (std::max)(0.0f, visualDistance - settings.scaling.scalingStartDistance);

    float distanceFactor;
    float scalingExponent;

    float activeLimit = settings.distance.GetActiveDistanceLimit(entityType, context.isInWvW);
    bool useLimitMode = activeLimit > 0.0f;
    if (useLimitMode) {
        distanceFactor = settings.scaling.limitDistanceFactor;
        scalingExponent = settings.scaling.limitScalingExponent;
    } else {
        if (entityType == EntityTypes::Gadget || entityType == EntityTypes::AttackTarget) {
            float adaptiveFarPlane = AppState::Get().GetAdaptiveFarPlane();
            distanceFactor = (std::max)(AdaptiveScaling::GADGET_MIN_DISTANCE_FACTOR, adaptiveFarPlane / 2.0f);
            scalingExponent = settings.scaling.noLimitScalingExponent;
        } else {
            distanceFactor = AdaptiveScaling::PLAYER_NPC_DISTANCE_FACTOR;
            scalingExponent = settings.scaling.noLimitScalingExponent;
        }
    }
    
    float rawScale = distanceFactor / (distanceFactor + pow(effectiveDistance, scalingExponent));

    return (std::max)(settings.scaling.minScale, (std::min)(rawScale, settings.scaling.maxScale));
}

float StyleCalculator::CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha,
                                              bool useDistanceLimit, EntityTypes entityType,
                                              float& outNormalizedDistance) {
    const auto& settings = AppState::Get().GetSettings();
    outNormalizedDistance = 0.0f;
    
    if (useDistanceLimit) {
        return distanceFadeAlpha;
    }

    if (entityType == EntityTypes::Gadget || entityType == EntityTypes::AttackTarget) {
        float finalAlpha = 1.0f;
        
        const float farPlane = AppState::Get().GetAdaptiveFarPlane();
        const float effectStartDistance = AdaptiveScaling::FADE_START_DISTANCE;
        
        if (gameplayDistance > effectStartDistance) {
            float range = farPlane - effectStartDistance;
            if (range > 0.0f) {
                float progress = (gameplayDistance - effectStartDistance) / range;
                outNormalizedDistance = (std::clamp)(progress, 0.0f, 1.0f);
            }
            
            finalAlpha = 1.0f - outNormalizedDistance;
            finalAlpha = (std::max)(AdaptiveScaling::MIN_ALPHA, finalAlpha);
        }
        
        return finalAlpha;
    }
    else {
        const float fadeStart = AdaptiveScaling::PLAYER_NPC_FADE_START;
        const float fadeEnd = AdaptiveScaling::PLAYER_NPC_FADE_END;
        const float minAlpha = AdaptiveScaling::PLAYER_NPC_MIN_ALPHA;

        if (gameplayDistance <= fadeStart) {
            return 1.0f;
        }
        if (gameplayDistance >= fadeEnd) {
            return minAlpha;
        }

        float fadeRange = fadeEnd - fadeStart;
        float progress = (gameplayDistance - fadeStart) / fadeRange;
        return 1.0f - (progress * (1.0f - minAlpha));
    }
}

float StyleCalculator::CalculateFinalSize(float baseSize, float scale, float minLimit, float maxLimit, float multiplier) {
    float scaledSize = baseSize * scale * multiplier;
    return std::clamp(scaledSize, minLimit, maxLimit);
}

float StyleCalculator::CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit) {
    if (!useDistanceLimit) {
        return 1.0f;
    }
    
    const float fadeZonePercentage = RenderingEffects::FADE_ZONE_PERCENTAGE;
    const float fadeZoneDistance = distanceLimit * fadeZonePercentage;
    const float fadeStartDistance = distanceLimit - fadeZoneDistance;
    const float fadeEndDistance = distanceLimit;
    
    if (distance <= fadeStartDistance) {
        return 1.0f;
    } else if (distance >= fadeEndDistance) {
        return 0.0f;
    } else {
        const float fadeProgress = (distance - fadeStartDistance) / fadeZoneDistance;
        return 1.0f - fadeProgress;
    }
}

StyleCalculator::EntityMultipliers StyleCalculator::CalculateEntityMultipliers(const RenderableEntity& entity) {
    EntityMultipliers multipliers;
    
    if (entity.entityType == EntityTypes::Player) {
        const auto* player = static_cast<const RenderablePlayer*>(&entity);
        const auto& settings = AppState::Get().GetSettings();
        if (player->attitude == Game::Attitude::Hostile) {
            multipliers.hostile = settings.playerESP.hostileBoostMultiplier;
        }
    }
    
    if (entity.entityType == EntityTypes::NPC) {
        const auto* npc = static_cast<const RenderableNpc*>(&entity);
        multipliers.rank = Styling::GetRankMultiplier(npc->rank);
    }
    
    if (entity.entityType == EntityTypes::Gadget || entity.entityType == EntityTypes::AttackTarget) {
        multipliers.gadgetHealth = Styling::GetGadgetHealthMultiplier(entity.maxHealth);
    }
    
    multipliers.healthBar = multipliers.hostile * multipliers.rank * multipliers.gadgetHealth;
    
    return multipliers;
}

void StyleCalculator::CalculateFinalSizes(VisualStyle& style, 
                                          float scale,
                                          const EntityMultipliers& multipliers) {
    const auto& settings = AppState::Get().GetSettings();
    
    style.finalFontSize = CalculateFinalSize(settings.sizes.baseFontSize, scale, settings.sizes.minFontSize, ScalingLimits::MAX_FONT_SIZE, multipliers.hostile);
    
    style.finalBoxThickness = CalculateFinalSize(settings.sizes.baseBoxThickness, scale, ScalingLimits::MIN_BOX_THICKNESS, ScalingLimits::MAX_BOX_THICKNESS, 1.0f);
    
    style.finalDotRadius = CalculateFinalSize(settings.sizes.baseDotRadius, scale, ScalingLimits::MIN_DOT_RADIUS, ScalingLimits::MAX_DOT_RADIUS);
    
    style.finalHealthBarWidth = CalculateFinalSize(settings.sizes.baseHealthBarWidth, scale, ScalingLimits::MIN_HEALTH_BAR_WIDTH, ScalingLimits::MAX_HEALTH_BAR_WIDTH, multipliers.healthBar);
    style.finalHealthBarHeight = CalculateFinalSize(settings.sizes.baseHealthBarHeight, scale, ScalingLimits::MIN_HEALTH_BAR_HEIGHT, ScalingLimits::MAX_HEALTH_BAR_HEIGHT, multipliers.healthBar);
}

} // namespace kx::Logic

