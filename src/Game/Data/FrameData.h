#pragma once

#include "glm.hpp"
#include <vector>
#include <array>

#include "../../libs/ImGui/imgui.h" // For ImU32, ImVec2

// Forward declarations
struct ImDrawList;
namespace kx {
    class Camera;
    class CombatStateManager;
    struct Settings;
    struct GameEntity; // Forward declare from RenderableData.h
    struct PlayerEntity;
    struct NpcEntity;
    struct RenderableGadget;
    struct RenderableAttackTarget;
    struct RenderableItem;
}

namespace kx {

/**
 * @brief Visual style properties calculated on the update thread
 * 
 * Contains opacity, color, scale factors, and abstract sizes (e.g., font size in px)
 * based on game state and settings. These are stable and don't depend on camera.
 */
struct VisualStyle {
    float scale;                   // Distance-based scale factor
    float distanceFadeAlpha;       // Raw distance fade (for logic culling)
    float finalAlpha;              // Final visible alpha
    unsigned int fadedEntityColor; // Pre-calculated color

    // Abstract Sizes (calculated once per low-freq update)
    float finalFontSize;
    float finalBoxThickness;
    float finalDotRadius;
    float finalHealthBarWidth;
    float finalHealthBarHeight;

    VisualStyle()
        : scale(0.0f), distanceFadeAlpha(0.0f), finalAlpha(0.0f),
          fadedEntityColor(0),
          finalFontSize(0.0f), finalBoxThickness(0.0f), finalDotRadius(0.0f),
          finalHealthBarWidth(0.0f), finalHealthBarHeight(0.0f) {
    }
};

/**
 * @brief Screen geometry properties calculated on the render thread
 * 
 * Contains 3D-to-2D projection data, bounding boxes, and screen positions.
 * These depend on the live camera and are recalculated every frame.
 */
struct ScreenGeometry {
    glm::vec2 screenPos;           // Origin/Feet position
    ImVec2 boxMin;                 // 2D Bounding Box Min
    ImVec2 boxMax;                 // 2D Bounding Box Max
    ImVec2 center;                 // Visual Center
    float circleRadius;            // For gadgets
    
    // 3D projection data
    std::array<glm::vec2, 8> projectedCorners;
    std::array<bool, 8> cornerValidity;
    bool isOnScreen;               // Result of frustum check

    ScreenGeometry()
        : screenPos(0.0f), boxMin(), boxMax(), center(), circleRadius(0.0f),
          projectedCorners(), cornerValidity(), isOnScreen(false) {
        cornerValidity.fill(false);
    }
};

/**
 * @brief Visual properties calculated for rendering an entity
 *
 * This struct contains all the pre-calculated visual properties needed
 * to render an entity. It separates calculation from drawing.
 *
 * In the direct render pipeline, it is typically created on the stack
 * inside the StageRenderer loop, populated, used, and discarded each frame.
 */
struct VisualProperties {
    VisualStyle style;
    ScreenGeometry geometry;

    VisualProperties() : style(), geometry() {
    }
};


// NEW: FrameContext struct
// This struct will hold all data that is constant for a single rendering frame.
struct FrameContext {
    const uint64_t now;
    Camera& camera;
    CombatStateManager& stateManager;
    const Settings& settings;
    ImDrawList* drawList;
    const float screenWidth;
    const float screenHeight;
    const bool isInWvW; // Game context: true if player is on a WvW map
};

struct PooledFrameRenderData {
    std::vector<PlayerEntity*> players;
    std::vector<NpcEntity*> npcs;
    std::vector<RenderableGadget*> gadgets;
    std::vector<RenderableAttackTarget*> attackTargets;
    std::vector<RenderableItem*> items;

    void Reset() {
        players.clear();
        npcs.clear();
        gadgets.clear();
        attackTargets.clear();
        items.clear();
    }
};

} // namespace kx