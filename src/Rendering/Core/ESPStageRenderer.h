#pragma once

#include <optional>
#include <glm.hpp>

#include "../Data/ESPData.h"
#include "../Data/EntityRenderContext.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
struct FrameContext;
struct LayoutCursor;
class CombatStateManager;


class ESPStageRenderer {
public:
    static void RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData);

private:
    static void RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props);
    static std::optional<VisualProperties> CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context);

    // Combat text rendering methods
    static void RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos);
    static void RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const glm::vec2& healthBarPos);

    // Component rendering helper methods
    static void RenderIdentityLine(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, LayoutCursor& cursor);
    static void RenderHealthComponents(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, LayoutCursor& cursor);
    static void RenderEnergyBar(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, LayoutCursor& cursor);
    static void RenderInfoDetails(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, LayoutCursor& cursor);
};

} // namespace kx