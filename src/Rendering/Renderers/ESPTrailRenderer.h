#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include "../../../libs/ImGui/imgui.h"
#include "../Combat/CombatState.h"

namespace kx {

struct FrameContext;
struct EntityRenderContext;
struct VisualProperties;

struct TrailSegmentData {
    std::vector<std::vector<PositionHistoryPoint>> segments;
    std::vector<std::pair<PositionHistoryPoint, PositionHistoryPoint>> teleportConnections;
};

class ESPTrailRenderer {
public:
    static void RenderPlayerTrail(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        const VisualProperties& props);

private:
    static std::vector<PositionHistoryPoint> CollectTrailPoints(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        uint64_t now);

    static TrailSegmentData GenerateSmoothTrail(
        const std::vector<PositionHistoryPoint>& worldPoints,
        float teleportThreshold);

    static void ProjectAndRenderTrail(
        const FrameContext& context,
        const TrailSegmentData& segmentData,
        float thickness,
        ImU32 baseColor,
        float finalAlpha,
        float globalOpacity,
        bool renderTeleportConnections);
};

} // namespace kx

