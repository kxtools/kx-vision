#pragma once

#include <vector>

#include "Settings.h"
#include "../Data/RenderableData.h"
#include "../Data/ESPData.h"
#include "../../Game/Camera.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
struct EntityRenderContext;
struct FrameContext;
class CombatStateManager;

class ESPStageRenderer {
public:
    static void RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData);

private:
    static void RenderEntityComponents(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props);

    // Component rendering methods
    static void RenderHealthBar(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings);
    static void RenderEnergyBar(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings);
    static void RenderBoundingBox(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderGadgetVisuals(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, const VisualProperties& props, const Settings& settings);
    static void RenderDistanceText(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderCenterDot(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderPlayerName(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderDetailsText(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderGearSummary(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props, const Settings& settings);

    // New methods for independent rendering
    static void RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props);
    static void RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props);
};

} // namespace kx