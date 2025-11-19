#pragma once

#include "../Data/FrameData.h"
#include "../Data/EntityTypes.h"
#include "../../Game/Camera.h"

// Forward declarations
namespace kx {
    struct RenderableEntity;
}

namespace kx::Renderers {

/**
 * @brief Handles 3D world-to-2D screen projection and frustum culling
 * 
 * This class handles the geometry side of visual calculation - determining where
 * an entity appears on screen based on camera state. It runs on the render thread
 * and uses the live camera for accurate projection.
 * 
 * All methods are static and operate on provided data without side effects.
 */
class ScreenProjector {
public:
    /**
     * @brief Projects 3D entity to 2D screen space.
     * Populates the 'geometry' field of VisualProperties.
     * @param entity The entity to project
     * @param camera Camera for projection
     * @param screenW Screen width in pixels
     * @param screenH Screen height in pixels
     * @param style VisualStyle (needed for circle radius scaling)
     * @param outGeometry Output geometry structure to populate
     * @return true if entity is effectively on screen, false if behind camera/culled
     */
    static bool Project(
        const RenderableEntity& entity,
        const Camera& camera,
        float screenW,
        float screenH,
        const VisualStyle& style,
        ScreenGeometry& outGeometry
    );

private:
    static void Calculate3DBoundingBox(
        const glm::vec3& entityPos,
        float worldWidth,
        float worldDepth,
        float worldHeight,
        const Camera& camera,
        float screenWidth,
        float screenHeight,
        ScreenGeometry& geometry,
        bool& outValid);

    static void GetWorldBoundsForEntity(
        EntityTypes entityType,
        float& outWidth,
        float& outDepth,
        float& outHeight);

    static void ApplyFallback2DBox(
        const RenderableEntity& entity,
        ScreenGeometry& geometry,
        float scale,
        const glm::vec2& screenPos);

    static void CalculateEntityBoxDimensions(EntityTypes entityType, float scale,
                                            float& outBoxWidth, float& outBoxHeight);

    static void ProjectGadget(
        const RenderableEntity& entity,
        const Camera& camera,
        float screenWidth,
        float screenHeight,
        ScreenGeometry& geometry,
        float scale,
        bool isOriginValid);

    static void ProjectCharacter(
        const RenderableEntity& entity,
        const Camera& camera,
        float screenWidth,
        float screenHeight,
        ScreenGeometry& geometry,
        float scale,
        bool isOriginValid);
};

} // namespace kx::Renderers

