#pragma once

#include "../Data/ESPData.h"
#include "../Layout/LayoutCursor.h"

namespace kx {

class ComponentRenderer {
public:
    static void RenderGeometry(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props);

    static void RenderIdentity(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor);

    static void RenderStatusBars(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor);

    static void RenderDetails(const FrameContext& ctx, const EntityRenderContext& eCtx, const VisualProperties& props, LayoutCursor& cursor);
};

} // namespace kx

