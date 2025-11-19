#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "Data/ESPData.h"

namespace kx {

// Forward declarations
class Camera;

/**
 * @brief Utility functions for rendering shapes in ESP
 * 
 * This class handles all shape-based rendering including bounding boxes and dots.
 * Separated for better organization and future extensibility.
 */
class ESPShapeRenderer {
public:
    /**
     * @brief Render a 3D gyroscope sphere effect.
     * REFACTOR: Decoupled from EntityRenderContext. Now takes raw position/distance.
     * This makes the renderer "dumb" (pure drawing) and reusable.
     * 
     * @param worldPos The 3D center of the sphere
     * @param gameplayDistance Distance from player (used for LOD fading)
     */
    static void RenderGyroscopicOverlay(ImDrawList* drawList, 
        const glm::vec3& worldPos, 
        float gameplayDistance,
        Camera& camera,
        float screenWidth, 
        float screenHeight, 
        float finalAlpha, 
        unsigned int fadedEntityColor, 
        float scale, 
        float globalOpacity);

    /**
     * @brief Render a simple 2D circle for gadgets
     */
    static void RenderGadgetCircle(ImDrawList* drawList, const glm::vec2& screenPos, float radius, unsigned int color, float thickness, float globalOpacity);

    /**
     * @brief Render a bounding box around an entity
     * @param drawList ImGui draw list for rendering
     * @param boxMin Upper-left corner of the bounding box
     * @param boxMax Lower-right corner of the bounding box
     * @param color Box color with alpha
     * @param thickness Line thickness
     * @param globalOpacity Global opacity multiplier (0.0-1.0)
     */
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                 unsigned int color, float thickness, float globalOpacity);

    /**
     * @brief Render a colored center dot for an entity
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param color Dot color with alpha
     * @param radius Dot radius
     * @param globalOpacity Global opacity multiplier (0.0-1.0)
     */
    static void RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos, 
                                unsigned int color, float radius, float globalOpacity);

    /**
     * @brief Render a natural white dot (for gadgets)
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param radius Dot radius
     * @param globalOpacity Global opacity multiplier (0.0-1.0)
     */
    static void RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, 
                                     float fadeAlpha, float radius, float globalOpacity);

    /**
     * @brief Render a 3D wireframe box using projected corner coordinates
     * @param drawList ImGui draw list for rendering
     * @param props VisualProperties containing projected corners and validity flags
     * @param color Box color with alpha
     * @param thickness Line thickness
     * @param globalOpacity Global opacity multiplier (0.0-1.0)
     */
    static void RenderWireframeBox(ImDrawList* drawList, const VisualProperties& props, unsigned int color, float thickness, float globalOpacity);

    /**
     * @brief Apply alpha multiplier to a color while preserving RGB values
     * @param color Original color (RGBA format)
     * @param alpha Alpha multiplier (0.0-1.0)
     * @return Modified color with adjusted alpha
     */
    static unsigned int ApplyAlphaToColor(unsigned int color, float alpha);
};

} // namespace kx
