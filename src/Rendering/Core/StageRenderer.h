#pragma once

#include "../Data/FrameData.h"

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
};

} // namespace kx