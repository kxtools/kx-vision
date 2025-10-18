#pragma once

#include "glm.hpp"
#include <vector>

#include "../../libs/ImGui/imgui.h" // For ImU32, ImVec2
#include "ESPEntityTypes.h"
#include "EntityRenderContext.h"

// Forward declarations
struct ImDrawList;
namespace kx {
    class Camera;
    class CombatStateManager;
    struct Settings;
    struct RenderableEntity; // Forward declare from RenderableData.h
    struct RenderablePlayer;
    struct RenderableNpc;
    struct RenderableGadget;
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


// NEW: FrameContext struct
// This struct will hold all data that is constant for a single rendering frame.
struct FrameContext {
    const uint64_t now;
    kx::Camera& camera;
    kx::CombatStateManager& stateManager;
    const kx::Settings& settings;
    ImDrawList* drawList;
    const float screenWidth;
    const float screenHeight;
};

// NEW: FinalizedRenderable struct
// This pairs a renderable entity with its calculated visual properties and render context for the frame.
struct FinalizedRenderable {
    const kx::RenderableEntity* entity;
    kx::VisualProperties visuals;
    kx::EntityRenderContext context;
};

// MODIFIED: PooledFrameRenderData
// This struct is now simplified. The "filtered" data will still use the old structure,
// but we will introduce a new container for the finalized data.
struct PooledFrameRenderData {
    std::vector<RenderablePlayer*> players;
    std::vector<RenderableNpc*> npcs;
    std::vector<RenderableGadget*> gadgets;

    // NEW: A single vector to hold all entities after visuals have been calculated.
    std::vector<FinalizedRenderable> finalizedEntities;

    void Reset() {
        players.clear();
        npcs.clear();
        gadgets.clear();
        finalizedEntities.clear();
    }
};

} // namespace kx