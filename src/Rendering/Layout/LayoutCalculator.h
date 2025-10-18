#pragma once

#include <vector>
#include <map>
#include <string>
#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/EntityRenderContext.h"
#include "../Data/ESPData.h"

namespace kx {

// Forward declarations
struct FrameContext;
struct VisualProperties;

/**
 * @brief Request structure for layout calculations
 */
struct LayoutRequest {
    const EntityRenderContext& entityContext;
    const VisualProperties& visualProps;
    const FrameContext& frameContext;
};

/**
 * @brief Result structure for layout calculations
 */
struct LayoutResult {
    std::map<std::string, glm::vec2> elementPositions;
    glm::vec2 healthBarAnchor;
    
    // Helper to get position for a specific element
    glm::vec2 GetElementPosition(const std::string& elementName) const {
        auto it = elementPositions.find(elementName);
        return (it != elementPositions.end()) ? it->second : glm::vec2(0.0f);
    }
};

/**
 * @brief Calculates layout positions for entity rendering elements
 * 
 * This class handles the complex layout logic for positioning various UI elements
 * around entities (health bars, text, distance, etc.). It separates layout concerns
 * from rendering logic for better maintainability and testability.
 */
class LayoutCalculator {
public:
    /**
     * @brief Calculate the complete layout for an entity's rendering elements
     * @param request The layout request containing entity and visual data
     * @return LayoutResult with calculated positions for all elements
     */
    static LayoutResult CalculateLayout(const LayoutRequest& request);

private:
    /**
     * @brief Gather all visible layout elements and calculate their required size
     * @param request The layout request
     * @param outAboveElements Elements to be positioned above the entity
     * @param outBelowElements Elements to be positioned below the entity
     */
    static void GatherLayoutElements(
        const LayoutRequest& request,
        std::vector<std::pair<std::string, ImVec2>>& outAboveElements,
        std::vector<std::pair<std::string, ImVec2>>& outBelowElements);

    /**
     * @brief Calculate vertical stacking positions for a list of elements
     * @param startAnchor Starting position for the stack
     * @param elements List of elements with their sizes
     * @param outPositions Output map of element positions
     * @param stackUpwards Whether to stack upwards (true) or downwards (false)
     */
    static void CalculateVerticalStack(
        glm::vec2 startAnchor,
        const std::vector<std::pair<std::string, ImVec2>>& elements,
        std::map<std::string, glm::vec2>& outPositions,
        bool stackUpwards);
};

} // namespace kx
