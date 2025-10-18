#pragma once

#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"

namespace kx {

// Forward declarations
struct EntityRenderContext;
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
     * @brief Render a 3D gyroscope sphere for gadgets, with LOD transition to a 2D circle.
     */
    static void RenderGadgetSphere(ImDrawList* drawList, const EntityRenderContext& entityContext, Camera& camera,
        const glm::vec2& screenPos, float finalAlpha, unsigned int fadedEntityColor, float scale, float screenWidth, float screenHeight);

    /**
     * @brief Render a bounding box around an entity
     * @param drawList ImGui draw list for rendering
     * @param boxMin Upper-left corner of the bounding box
     * @param boxMax Lower-right corner of the bounding box
     * @param color Box color with alpha
     * @param thickness Line thickness
     */
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax,
                                 unsigned int color, float thickness);

    /**
     * @brief Render a colored center dot for an entity
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param color Dot color with alpha
     * @param radius Dot radius
     */
    static void RenderColoredDot(ImDrawList* drawList, const glm::vec2& feetPos, 
                                unsigned int color, float radius);

    /**
     * @brief Render a natural white dot (for gadgets)
     * @param drawList ImGui draw list for rendering
     * @param feetPos Entity feet position
     * @param fadeAlpha Distance-based fade alpha (0.0-1.0)
     * @param radius Dot radius
     */
    static void RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos, 
                                     float fadeAlpha, float radius);

    /**
     * @brief Apply alpha multiplier to a color while preserving RGB values
     * @param color Original color (RGBA format)
     * @param alpha Alpha multiplier (0.0-1.0)
     * @return Modified color with adjusted alpha
     */
    static unsigned int ApplyAlphaToColor(unsigned int color, float alpha);
};

} // namespace kx
