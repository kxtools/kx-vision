#pragma once

#include <vector>
#include <map>
#include <string>
#include <array>
#include "glm.hpp"
#include "../../../libs/ImGui/imgui.h"
#include "../Data/EntityRenderContext.h"
#include "../Data/ESPData.h"
#include "LayoutElementKeys.h"

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
    std::array<glm::vec2, (size_t)LayoutElementKey::Count> elementPositions;
    std::array<bool, (size_t)LayoutElementKey::Count> hasElement;
    glm::vec2 healthBarAnchor;
    
    // Helper to get position for a specific element
    glm::vec2 GetElementPosition(LayoutElementKey key) const {
        size_t index = (size_t)key;
        return hasElement[index] ? elementPositions[index] : glm::vec2(0.0f);
    }
    
    // Helper to check if an element exists
    bool HasElement(LayoutElementKey key) const {
        return hasElement[(size_t)key];
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
        std::vector<std::pair<LayoutElementKey, ImVec2>>& outAboveElements,
        std::vector<std::pair<LayoutElementKey, ImVec2>>& outBelowElements);

    // Helper functions for GatherLayoutElements
    static void GatherStatusBarElements(
        const LayoutRequest& request,
        std::vector<std::pair<LayoutElementKey, ImVec2>>& outBelowElements);
    
    static void GatherPlayerIdentityElements(
        const LayoutRequest& request,
        std::vector<std::pair<LayoutElementKey, ImVec2>>& outBelowElements);
    
    static void GatherDetailElements(
        const LayoutRequest& request,
        std::vector<std::pair<LayoutElementKey, ImVec2>>& outBelowElements);

    /**
     * @brief Calculate vertical stacking positions for a list of elements
     * @param startAnchor Starting position for the stack
     * @param elements List of elements with their sizes
     * @param outPositions Output array of element positions
     * @param outHasElement Output array tracking which elements are active
     * @param stackUpwards Whether to stack upwards (true) or downwards (false)
     */
    static void CalculateVerticalStack(
        glm::vec2 startAnchor,
        const std::vector<std::pair<LayoutElementKey, ImVec2>>& elements,
        std::array<glm::vec2, (size_t)LayoutElementKey::Count>& outPositions,
        std::array<bool, (size_t)LayoutElementKey::Count>& outHasElement,
        bool stackUpwards);
};

} // namespace kx
