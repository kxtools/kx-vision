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

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
struct EntityRenderContext;
struct FrameContext;
class CombatStateManager;

struct CalculatedLayout {
    std::map<std::string, glm::vec2> elementPositions;
    glm::vec2 healthBarAnchor;
};

class ESPStageRenderer {
public:
    static void RenderFrameData(const FrameContext& context, const PooledFrameRenderData& frameData);

private:
    static void RenderEntityComponents(const FrameContext& context, EntityRenderContext& entityContext, const VisualProperties& props);
    static std::optional<VisualProperties> CalculateLiveVisuals(const FinalizedRenderable& item, const FrameContext& context);

    /**
     * @brief Gathers all visible layout elements and calculates their required size.
     */
    static void GatherLayoutElements(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        const VisualProperties& props,
        std::vector<std::pair<std::string, ImVec2>>& outAboveElements,
        std::vector<std::pair<std::string, ImVec2>>& outBelowElements);

    /**
     * @brief Renders all elements that are part of the dynamic layout system.
     */
    static void RenderLayoutElements(
        const FrameContext& context,
        EntityRenderContext& entityContext,
        const VisualProperties& props,
        CalculatedLayout& layout);

    /**
     * @brief Renders static, non-layout elements like the bounding box and center dot.
     */
    static void RenderStaticElements(
        const FrameContext& context,
        const EntityRenderContext& entityContext,
        const VisualProperties& props);

    static void RenderBoundingBox(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderGadgetVisuals(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, const VisualProperties& props, const Settings& settings);
    static void RenderDistanceText(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);
    static void RenderCenterDot(ImDrawList* drawList, const EntityRenderContext& context, const VisualProperties& props);

    // New methods for independent rendering
    static void RenderDamageNumbers(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const CalculatedLayout& layout);
    static void RenderBurstDps(const FrameContext& context, const EntityRenderContext& entityContext, const VisualProperties& props, const CalculatedLayout& layout);

    static void CalculateVerticalStack(
        glm::vec2 startAnchor,
        const std::vector<std::pair<std::string, ImVec2>>& elements, // name and size
        std::map<std::string, glm::vec2>& outPositions,
        bool stackUpwards);
};

} // namespace kx