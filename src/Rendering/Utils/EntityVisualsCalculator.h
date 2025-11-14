#pragma once

#include <optional>
#include "glm.hpp"
#include "../Data/ESPData.h"
#include "../Data/ESPEntityTypes.h"

// Forward declarations
namespace kx {
    class Camera;
    struct RenderableEntity;
}

namespace kx {

namespace {
    struct EntityMultipliers {
        float hostile = 1.0f;
        float rank = 1.0f;
        float gadgetHealth = 1.0f;
        float healthBar = 1.0f;  // Combined multiplier for health bars
    };
}

/**
 * @brief Utility class for calculating entity visual properties
 * 
 * This class separates the calculation of visual properties (screen position,
 * scale, alpha, dimensions) from the actual drawing. This enforces the Single
 * Responsibility Principle and makes the calculation logic pure, stateless,
 * and easily testable.
 * 
 * All methods are static and operate on provided data without side effects.
 */
class EntityVisualsCalculator {
public:
    /**
     * @brief Calculate all visual properties for an entity
     * 
     * This is the main entry point that calculates everything needed to render
     * an entity. If the entity is not visible (off-screen, fully transparent),
     * returns std::nullopt.
     * 
     * @param entity The entity to process
     * @param camera Camera for world-to-screen projection
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return Visual properties if entity should be rendered, nullopt otherwise
     */
    static std::optional<VisualProperties> Calculate(const RenderableEntity& entity,
                                                     Camera& camera,
                                                     float screenWidth,
                                                     float screenHeight);

    /**
     * @brief Calculate a font size multiplier for the damage number based on the damage value.
     *        100,000 damage should result in a 2x multiplier.
     * @param damageToDisplay The total damage value to display.
     * @return A float multiplier for the font size.
     */
    static float GetDamageNumberFontSizeMultiplier(float damageToDisplay);

    /**
     * @brief Calculate 3D bounding box projection to screen space
     * 
     * Projects an 8-corner 3D bounding box from world space to screen space,
     * providing perspective-correct bounding boxes for players/NPCs.
     * 
     * @param entityPos Entity position in world space
     * @param worldWidth Width of bounding box in world space (meters)
     * @param worldDepth Depth of bounding box in world space (meters)
     * @param worldHeight Height of bounding box in world space (meters)
     * @param camera Camera for projection
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @param outBoxMin Output minimum screen coordinates
     * @param outBoxMax Output maximum screen coordinates
     * @param outValid Output whether projection was successful
     */
    static void Calculate3DBoundingBox(
        const glm::vec3& entityPos,
        float worldWidth,
        float worldDepth,
        float worldHeight,
        Camera& camera,
        float screenWidth,
        float screenHeight,
        ImVec2& outBoxMin,
        ImVec2& outBoxMax,
        bool& outValid);

    /**
     * @brief Get world-space bounds for entity type
     */
    static void GetWorldBoundsForEntity(
        ESPEntityType entityType,
        float& outWidth,
        float& outDepth,
        float& outHeight);

private:
    /**
     * @brief Check if entity is on screen and calculate screen position
     * @param entity Entity to check (needed for type and dimensions)
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @param outScreenPos Output screen position (only valid if returns true)
     * @return True if entity is visible on screen, false otherwise
     */
    static bool IsEntityOnScreen(const RenderableEntity& entity, Camera& camera,
                                float screenWidth, float screenHeight, glm::vec2& outScreenPos);

    /**
     * @brief Calculate distance-based scale factor for entity rendering
     * @param visualDistance Visual distance from camera
     * @param entityType Type of entity (affects which scaling curve is used)
     * @return Clamped scale factor (between espMinScale and espMaxScale)
     */
    static float CalculateEntityScale(float visualDistance, ESPEntityType entityType);

    /**
     * @brief Calculate box dimensions for entity based on type and scale
     * @param entityType Type of entity (Player, NPC, Gadget)
     * @param scale Scale factor
     * @param outBoxWidth Output box width
     * @param outBoxHeight Output box height
     */
    static void CalculateEntityBoxDimensions(ESPEntityType entityType, float scale,
                                            float& outBoxWidth, float& outBoxHeight);

    /**
     * @brief Apply fallback 2D box if 3D projection fails
     */
    static void ApplyFallback2DBox(
        const RenderableEntity& entity,
        VisualProperties& props,
        float scale,
        const glm::vec2& screenPos);

    /**
     * @brief Calculate gadget circle dimensions
     */
    static void CalculateGadgetDimensions(
        const RenderableEntity& entity,
        Camera& camera,
        float screenWidth,
        float screenHeight,
        VisualProperties& props,
        float scale);

    /**
     * @brief Calculate player/NPC 3D bounding box dimensions
     */
    static void CalculatePlayerNPCDimensions(
        const RenderableEntity& entity,
        Camera& camera,
        float screenWidth,
        float screenHeight,
        VisualProperties& props,
        float scale);

    /**
     * @brief Calculate adaptive alpha based on rendering mode and entity type
     * @param gameplayDistance Distance from player to entity
     * @param distanceFadeAlpha Pre-calculated distance fade alpha (for Limit Mode)
     * @param useDistanceLimit Whether distance limit mode is enabled
     * @param entityType Type of entity (adaptive alpha only applied to gadgets)
     * @param outNormalizedDistance Output normalized distance (0.0-1.0, for future LOD effects)
     * @return Final alpha value with atmospheric fading applied
     */
    static float CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha,
                                       bool useDistanceLimit, ESPEntityType entityType,
                                       float& outNormalizedDistance);

    // Helper methods for internal calculations
    static float GetRankMultiplier(Game::CharacterRank rank);
    static float GetGadgetHealthMultiplier(float maxHealth);
    static float CalculateFinalSize(float baseSize, float scale, float minLimit, float maxLimit, float multiplier = 1.0f);
    static float CalculateDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit);
    
    // Multiplier calculation
    static EntityMultipliers CalculateEntityMultipliers(const RenderableEntity& entity);
    
    // Final sizes calculation
    static void CalculateFinalSizes(VisualProperties& props, 
                                   float scale,
                                   const EntityMultipliers& multipliers);
};

} // namespace kx
