#pragma once

#include <vector>
#include "../../../libs/ImGui/imgui.h"
#include "../Combat/CombatState.h"
#include "../../Game/GameEnums.h"

namespace kx {

struct FrameContext;
struct RenderablePlayer;
struct VisualProperties;

struct TrailSegmentData {
    std::vector<std::vector<PositionHistoryPoint>> segments;
    std::vector<std::pair<PositionHistoryPoint, PositionHistoryPoint>> teleportConnections;
};

class TrailRenderer {
public:
    static void RenderPlayerTrail(
        const FrameContext& context,
        const RenderablePlayer& player,
        Game::Attitude attitude,
        const VisualProperties& props);

private:
    static std::vector<PositionHistoryPoint> CollectTrailPoints(
        const FrameContext& context,
        const RenderablePlayer& player,
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

