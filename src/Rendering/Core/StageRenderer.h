#pragma once

#include <optional>

#include "../Data/FrameData.h"
#include "../Data/EntityRenderContext.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
struct FrameContext;
struct LayoutCursor;
class CombatStateManager;


class StageRenderer {
public:
    static void RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData);

private:
    static void RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props);
    static std::optional<VisualProperties> CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context);
};

} // namespace kx