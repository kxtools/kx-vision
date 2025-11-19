#include "ESPStageRenderer.h"
#include "../Layout/LayoutCursor.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Utils/ESPMath.h"
#include "../Utils/ESPStyling.h"
#include "../Utils/CombatConstants.h"
#include "../Utils/LayoutConstants.h"
#include "../Renderers/ESPShapeRenderer.h"
#include "../Renderers/ESPHealthBarRenderer.h"
#include "../Renderers/ESPTrailRenderer.h"
#include "../Renderers/ComponentRenderers.h"
#include "../Data/EntityRenderContext.h"
#include "../Utils/RenderSettingsHelper.h"
#include "../../../libs/ImGui/imgui.h"
#include <optional>

#include "Logic/VisualsCalculator.h"

namespace kx {

namespace {
    inline float GetGlobalOpacity(const FrameContext& context) {
        return context.settings.appearance.globalOpacity;
    }
}

std::optional<VisualProperties> ESPStageRenderer::CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context) {
    // 1. Start with the pre-calculated, non-geometric properties from the low-frequency update.
    VisualProperties liveVisuals = item.visuals;

    // 2. Determine the entity's 3D world-space dimensions for the bounding box.
    float worldWidth, worldDepth, worldHeight;
    if (item.entity->hasPhysicsDimensions) {
        worldWidth = item.entity->physicsWidth;
        worldDepth = item.entity->physicsDepth;
        worldHeight = item.entity->physicsHeight;
    } else {
        // Fallback to constants for entities without physics data.
        VisualsCalculator::GetWorldBoundsForEntity(item.entity->entityType, worldWidth, worldDepth, worldHeight);
    }
    
    // 3. Project the 8 corners of the 3D world-space box into 2D screen space.
    // This is the core logic that ensures perspective correctness.
    bool isProjectionValid = false;
    VisualsCalculator::Calculate3DBoundingBox(
        item.entity->position,
        worldWidth,
        worldDepth,
        worldHeight,
        context.camera,
        context.screenWidth,
        context.screenHeight,
        liveVisuals,
        isProjectionValid
    );

    // If no corners could be validly projected (e.g., entity is entirely behind the camera), cull it.
    if (!isProjectionValid) {
        return std::nullopt;
    }

    // 4. Perform the final, high-frequency culling check.
    // An entity is visible if its TRUE projected 2D box overlaps the screen area.
    bool overlapsX = liveVisuals.boxMin.x < context.screenWidth && liveVisuals.boxMax.x > 0;
    bool overlapsY = liveVisuals.boxMin.y < context.screenHeight && liveVisuals.boxMax.y > 0;

    if (!overlapsX || !overlapsY) {
        return std::nullopt;
    }

    // 5. Finalize remaining screen-space properties based on the new, correct box.
    // Project the origin point separately for anchoring UI elements.
    if (!ESPMath::ProjectToScreen(item.entity->position, context.camera, context.screenWidth, context.screenHeight, liveVisuals.screenPos)) {
        // If origin point is behind camera, use center of projected box
        liveVisuals.screenPos.x = (liveVisuals.boxMin.x + liveVisuals.boxMax.x) * 0.5f;
        liveVisuals.screenPos.y = (liveVisuals.boxMin.y + liveVisuals.boxMax.y) * 0.5f;
    }
    
    // Calculate the visual center of the final projected box.
    liveVisuals.center = ImVec2(
        (liveVisuals.boxMin.x + liveVisuals.boxMax.x) * 0.5f,
        (liveVisuals.boxMin.y + liveVisuals.boxMax.y) * 0.5f
    );
    
    // 6. Calculate gadget circle radius if needed (scale-based, so can be calculated here)
    if (item.entity->entityType == ESPEntityType::Gadget || item.entity->entityType == ESPEntityType::AttackTarget) {
        const auto& settings = AppState::Get().GetSettings();
        float baseRadius = settings.sizes.baseBoxWidth * EntitySizeRatios::GADGET_CIRCLE_RADIUS_RATIO;
        liveVisuals.circleRadius = (std::max)(MinimumSizes::GADGET_MIN_WIDTH / 2.0f, baseRadius * liveVisuals.scale);
    } else {
        liveVisuals.circleRadius = 0.0f; // No circle for players/NPCs
    }
    
    // Return the complete, correct, and visible properties for this frame.
    return liveVisuals;
}

void ESPStageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
    for (const auto& item : frameData.finalizedEntities) {
        
        // First, perform the high-frequency update to get live visual properties for this frame.
        auto liveVisualsOpt = CalculateLiveVisuals(item, context);
        
        // If the entity is on-screen, proceed with rendering.
        if (liveVisualsOpt) {
            // Use the pre-built context from the finalized renderable.
            EntityRenderContext entityContext = item.context;
            
            // Render using the fresh visual properties.
            RenderEntityComponents(context, entityContext, *liveVisualsOpt);
            
            // Render movement trail for players
            if (entityContext.entityType == ESPEntityType::Player) {
                ESPTrailRenderer::RenderPlayerTrail(context, entityContext, *liveVisualsOpt);
            }
        }
    }
}

void ESPStageRenderer::RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props) {
    ComponentRenderer::RenderGeometry(context, entityContext, props);

    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(context.settings, entityContext.entityType);
    LayoutCursor bottomStack({props.center.x, props.boxMax.y}, 1.0f);
    
    if (entityContext.entityType == ESPEntityType::Gadget && !shouldRenderBox) {
        bottomStack = LayoutCursor(glm::vec2(props.screenPos.x, props.screenPos.y), 1.0f);
    }

    ComponentRenderer::RenderIdentity(context, entityContext, props, bottomStack);
    ComponentRenderer::RenderStatusBars(context, entityContext, props, bottomStack);
    ComponentRenderer::RenderDetails(context, entityContext, props, bottomStack);
}


} // namespace kx
