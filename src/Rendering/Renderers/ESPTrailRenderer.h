#pragma once

#include <vector>
#include <glm/vec3.hpp>
#include "../../../libs/ImGui/imgui.h"

namespace kx {

struct FrameContext;
struct EntityRenderContext;
struct VisualProperties;

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

    static std::vector<glm::vec3> GenerateSmoothTrail(
        const std::vector<glm::vec3>& worldPoints,
        float teleportThreshold);

    static void ProjectAndRenderTrail(
        const FrameContext& context,
        const std::vector<glm::vec3>& smoothedWorldPoints,
        float thickness,
        ImU32 baseColor);
};

} // namespace kx

