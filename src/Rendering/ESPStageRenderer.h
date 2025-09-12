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
     * @brief Main rendering method - renders pooled data for one frame (OPTIMIZED)
     * @param drawList ImGui draw list for rendering
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     * @param frameData Pooled extracted data for this frame (contains pointers)
     * @param camera Camera reference for world-to-screen calculations
     */
    static void RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                               const PooledFrameRenderData& frameData, Camera& camera);

private:
    // Pooled rendering functions for optimized performance
    static void RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderablePlayer*>& players, Camera& camera);

    static void RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                const std::vector<RenderableNpc*>& npcs, Camera& camera);

    static void RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderableGadget*>& gadgets, Camera& camera);

    /**
     * @brief Universal entity rendering function using context struct
     */
    static void RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera);

    // Helper rendering functions
    static void RenderHealthBar(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, float healthPercent);
    static void RenderStandaloneHealthBar(ImDrawList* drawList, const glm::vec2& centerPos, float healthPercent, unsigned int entityColor);
    static void RenderPlayerName(ImDrawList* drawList, const glm::vec2& feetPos, const std::string& playerName, unsigned int entityColor);
    static void RenderBoundingBox(ImDrawList* drawList, const ImVec2& boxMin, const ImVec2& boxMax, unsigned int color);
    static void RenderDistanceText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMin, float distance);
    static void RenderCenterDot(ImDrawList* drawList, const glm::vec2& feetPos, unsigned int color);
    static void RenderNaturalWhiteDot(ImDrawList* drawList, const glm::vec2& feetPos);
    static void RenderDetailsText(ImDrawList* drawList, const ImVec2& center, const ImVec2& boxMax, const std::vector<std::string>& details);
};

} // namespace kx