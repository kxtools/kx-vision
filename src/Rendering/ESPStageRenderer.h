#pragma once

#include <vector>
#include <string>
#include <vec3.hpp>
#include <vec2.hpp>
#include "RenderableData.h"
#include "ESPData.h"
#include "../Game/Camera.h"

// Forward declarations
struct ImDrawList;
struct ImVec2;

namespace kx {

// Forward declaration for context struct
struct EntityRenderContext;

/**
 * @brief Handles safe rendering from extracted data (Stage 2 of rendering pipeline)
 * 
 * This class performs all rendering operations using safe local data structures.
 * It never accesses game memory directly, eliminating the risk of crashes during rendering.
 */
class ESPStageRenderer {
public:
    /**
     * @brief Main rendering method - renders all extracted data for one frame
     * @param drawList ImGui draw list for rendering
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @param frameData All extracted data for this frame
     * @param camera Camera reference for world-to-screen calculations
     */
    static void RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                               const FrameRenderData& frameData, Camera& camera);

private:
    /**
     * @brief Render all players from extracted data
     */
    static void RenderPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                             const std::vector<RenderablePlayer>& players, Camera& camera);

    /**
     * @brief Render all NPCs from extracted data
     */
    static void RenderNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                          const std::vector<RenderableNpc>& npcs, Camera& camera);

    /**
     * @brief Render all gadgets from extracted data
     */
    static void RenderGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                             const std::vector<RenderableGadget>& gadgets, Camera& camera);

    /**
     * @brief Universal entity rendering function using context struct
     */
    static void RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera);

    // Helper rendering functions
    static void RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent);
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color);
    static void RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance);
    static void RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color);
    static void RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details);
};

} // namespace kx