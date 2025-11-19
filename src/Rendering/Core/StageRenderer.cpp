#include "StageRenderer.h"
#include "../../Core/AppState.h"
#include "../../Game/Camera.h"
#include "../Shared/LayoutConstants.h"
#include "../Renderers/ShapeRenderer.h"
#include "../Renderers/TrailRenderer.h"
#include "../Renderers/EntityComponentRenderer.h"
#include "../Data/EntityRenderContext.h"
#include "../../../libs/ImGui/imgui.h"
#include <optional>

#include "Renderers/ScreenProjector.h"
#include "Shared/MathUtils.h"
#include "Shared/RenderSettingsHelper.h"

namespace kx {

namespace {
    inline float GetGlobalOpacity(const FrameContext& context) {
        return context.settings.appearance.globalOpacity;
    }
}

std::optional<VisualProperties> StageRenderer::CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context) {
    // 1. Copy existing style from the pooled data
    VisualProperties liveProps = item.visuals;
    
    // 2. Perform Geometry Projection (HOT PATH)
    bool isOnScreen = Renderers::ScreenProjector::Project(
        *item.entity,
        context.camera,
        context.screenWidth,
        context.screenHeight,
        liveProps.style,
        liveProps.geometry
    );
    
    if (!isOnScreen) {
        return std::nullopt;
    }
    
    // 3. If origin point projection failed, use center of projected box
    if (liveProps.geometry.screenPos.x == 0.0f && liveProps.geometry.screenPos.y == 0.0f) {
        liveProps.geometry.screenPos.x = (liveProps.geometry.boxMin.x + liveProps.geometry.boxMax.x) * 0.5f;
        liveProps.geometry.screenPos.y = (liveProps.geometry.boxMin.y + liveProps.geometry.boxMax.y) * 0.5f;
    }
    
    return liveProps;
}

void StageRenderer::RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData) {
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
            if (entityContext.entityType == EntityTypes::Player) {
                TrailRenderer::RenderPlayerTrail(context, entityContext, *liveVisualsOpt);
            }
        }
    }
}

void StageRenderer::RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props) {
    EntityComponentRenderer::RenderGeometry(context, entityContext, props);

    bool shouldRenderBox = RenderSettingsHelper::ShouldRenderBox(context.settings, entityContext.entityType);
    LayoutCursor bottomStack({props.geometry.center.x, props.geometry.boxMax.y}, 1.0f);
    
    if (entityContext.entityType == EntityTypes::Gadget && !shouldRenderBox) {
        bottomStack = LayoutCursor(glm::vec2(props.geometry.screenPos.x, props.geometry.screenPos.y), 1.0f);
    }

    EntityComponentRenderer::RenderIdentity(context, entityContext, props, bottomStack);
    EntityComponentRenderer::RenderStatusBars(context, entityContext, props, bottomStack);
    EntityComponentRenderer::RenderDetails(context, entityContext, props, bottomStack);
}


} // namespace kx
