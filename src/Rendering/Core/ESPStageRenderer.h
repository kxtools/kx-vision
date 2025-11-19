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
};

} // namespace kx