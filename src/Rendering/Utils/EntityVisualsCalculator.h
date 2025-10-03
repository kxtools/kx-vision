#pragma once

#include <optional>
#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/ESPData.h"

// Forward declarations
namespace kx {
    class Camera;
    struct EntityRenderContext;
}

namespace kx {

/**
 * @brief Visual properties calculated for rendering an entity
 * 
 * This struct contains all the pre-calculated visual properties needed
 * to render an entity. It separates calculation from drawing.
 */
struct VisualProperties {
    glm::vec2 screenPos;           // 2D screen position
    float scale;                   // Distance-based scale factor
    float distanceFadeAlpha;       // Distance-based fade alpha
    float finalAlpha;              // Final alpha after adaptive effects
    unsigned int fadedEntityColor; // Entity color with distance fade applied
    
    // Box/Circle dimensions
    ImVec2 boxMin;                 // Bounding box minimum (or circle bounds for gadgets)
    ImVec2 boxMax;                 // Bounding box maximum (or circle bounds for gadgets)
    ImVec2 center;                 // Center point
    float circleRadius;            // Circle radius for gadgets (0 for players/NPCs)
    
    // Scaled sizes
    float finalFontSize;
    float finalBoxThickness;
    float finalDotRadius;
    float finalHealthBarWidth;
    float finalHealthBarHeight;
    
    VisualProperties() 
        : screenPos(0.0f), scale(0.0f), distanceFadeAlpha(0.0f), finalAlpha(0.0f),
          fadedEntityColor(0), boxMin(), boxMax(), center(), circleRadius(0.0f),
          finalFontSize(0.0f), finalBoxThickness(0.0f), finalDotRadius(0.0f),
          finalHealthBarWidth(0.0f), finalHealthBarHeight(0.0f) {}
};

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
     * @param context Entity rendering context with all entity data
     * @param camera Camera for world-to-screen projection
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @return Visual properties if entity should be rendered, nullopt otherwise
     */
    static std::optional<VisualProperties> Calculate(const EntityRenderContext& context,
                                                     Camera& camera,
                                                     float screenWidth,
                                                     float screenHeight);

private:
    /**
     * @brief Check if entity is on screen and calculate screen position
     * @param position World position
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @param outScreenPos Output screen position (only valid if returns true)
     * @return True if entity is visible on screen, false otherwise
     */
    static bool IsEntityOnScreen(const glm::vec3& position, Camera& camera,
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
};

} // namespace kx
