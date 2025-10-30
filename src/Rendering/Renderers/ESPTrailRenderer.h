#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include "../../../libs/ImGui/imgui.h"

namespace kx {

struct FrameContext;
struct EntityRenderContext;
struct VisualProperties;

struct TrailSegmentData {
    std::vector<std::vector<glm::vec3>> segments;
    std::vector<std::pair<glm::vec3, glm::vec3>> teleportConnections;
};

class ESPTrailRenderer {
public:
    static void RenderPlayerTrail(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        const VisualProperties& props);

private:
    static std::vector<glm::vec3> CollectTrailPoints(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        uint64_t now);

    static TrailSegmentData GenerateSmoothTrail(
        const std::vector<glm::vec3>& worldPoints,
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

