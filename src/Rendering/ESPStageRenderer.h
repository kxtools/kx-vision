#pragma once

#include <map>
#include <vector>
#include "RenderableData.h"
#include "ESPData.h"
#include "../Game/Camera.h"
#include "../Core/Settings.h"
#include "Generated/EnumsAndStructs.h"

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

    // Player-specific detail builders
    static std::vector<ColoredDetail> BuildPlayerDetails(const RenderablePlayer* player, const PlayerEspSettings& settings);
    static std::vector<ColoredDetail> BuildGearDetails(const RenderablePlayer* player);
    static std::vector<CompactStatInfo> BuildCompactGearSummary(const RenderablePlayer* player);
    static std::map<kx::data::ApiAttribute, int> BuildAttributeSummary(const RenderablePlayer* player);
    static std::vector<DominantStat> BuildDominantStats(const RenderablePlayer* player);

    /**
     * @brief Universal entity rendering function using context struct
     */
    static void RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera);

    // Distance fading helper functions
    static float CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit);
};

} // namespace kx