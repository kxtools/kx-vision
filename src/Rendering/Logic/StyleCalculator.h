#pragma once

#include <optional>
#include "../Data/FrameData.h"
#include "../Data/EntityTypes.h"

// Forward declarations
namespace kx {
    struct RenderableEntity;
    struct FrameContext;
}

namespace kx::Logic {

/**
 * @brief Calculates visual style properties (opacity, color, scale, sizes)
 * 
 * This class handles the logic side of visual calculation - determining what
 * an entity should look like based on game state and settings. It runs on
 * the update thread and does not depend on camera state.
 * 
 * All methods are static and operate on provided data without side effects.
 */
class StyleCalculator {
public:
    /**
     * @brief Calculates abstract visual properties (Color, Alpha, Sizes).
     * @param entity The entity to process
     * @param context Frame context containing settings and game state
     * @return VisualStyle if entity should be rendered (distance-based), nullopt if fully transparent
     */
    static std::optional<VisualStyle> Calculate(
        const RenderableEntity& entity, 
        const FrameContext& context
    );

private:
    static float CalculateEntityScale(float visualDistance, EntityTypes entityType, const FrameContext& context);
    static float CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha,
                                       bool useDistanceLimit, EntityTypes entityType,
                                       float& outNormalizedDistance);
    static float CalculateFinalSize(float baseSize, float scale, float minLimit, float maxLimit, float multiplier = 1.0f);
    static float CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit);
    
    struct EntityMultipliers {
        float hostile = 1.0f;
        float rank = 1.0f;
        float gadgetHealth = 1.0f;
        float healthBar = 1.0f;
    };
    
    static EntityMultipliers CalculateEntityMultipliers(const RenderableEntity& entity);
    static void CalculateFinalSizes(VisualStyle& style, 
                                   float scale,
                                   const EntityMultipliers& multipliers);
};

} // namespace kx::Logic

