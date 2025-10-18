#pragma once

#include <optional>
#include <map>
#include <string>
#include <vector>
#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"

#include "Settings.h"
#include "../Data/RenderableData.h"
#include "../Data/ESPData.h"
#include "../Data/EntityRenderContext.h"
#include "../Layout/LayoutCalculator.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
struct FrameContext;
class CombatStateManager;


class ESPStageRenderer {
public:
    static void RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData);

private:
    static void RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props);
    static std::optional<VisualProperties> CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context);

    /**
     * @brief Renders all elements that are part of the dynamic layout system.
     */
    static void RenderLayoutElements(
        const FrameContext& context,
        EntityRenderContext& entityContext,
        const VisualProperties& props,
        const LayoutResult& layout);

    /**
     * @brief Renders static, non-layout elements like the bounding box and center dot.
     */
    static void RenderStaticElements(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        const VisualProperties& props);


    // New methods for independent rendering
    static void RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const LayoutResult& layout);
    static void RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const LayoutResult& layout);
};

} // namespace kx