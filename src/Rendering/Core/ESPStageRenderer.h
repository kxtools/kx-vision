#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../Data/ESPData.h"
#include "../../Game/Camera.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct EntityRenderContext;
class CombatStateManager;

/**
 * @brief Handles safe rendering from extracted data (Stage 2 of rendering pipeline)
 * 
 * This class performs all rendering operations using safe local data structures.
 * It never accesses game memory directly, eliminating the risk of crashes during rendering.
 * 
 * Scalability: Detail building logic has been extracted to:
 * - ESPPlayerDetailsBuilder: Player-specific text generation and gear analysis
 * - ESPEntityDetailsBuilder: NPC and Gadget detail building
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
     * @param stateManager Reference to the combat state manager
     */
    static void RenderFrameData(ImDrawList* drawList, float screenWidth, float screenHeight, 
                               const PooledFrameRenderData& frameData, Camera& camera, 
                               const CombatStateManager& stateManager);

private:
    // Pooled rendering functions for optimized performance
    static void RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderablePlayer*>& players, Camera& camera, 
                                   const CombatStateManager& stateManager);

    static void RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                const std::vector<RenderableNpc*>& npcs, Camera& camera, 
                                const CombatStateManager& stateManager);

    static void RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderableGadget*>& gadgets, Camera& camera, 
                                   const CombatStateManager& stateManager);

    /**
     * @brief Universal entity rendering function using context struct
     */
    static void RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, 
                           const CombatStateManager& stateManager);

    /**
     * @brief Render all visual components for an entity
     * @param drawList ImGui draw list
     * @param context Entity rendering context
     * @param screenPos Screen position
     * @param boxMin Bounding box minimum point (or circle bounds for gadgets)
     * @param boxMax Bounding box maximum point (or circle bounds for gadgets)
     * @param center Center point
     * @param fadedEntityColor Entity color with distance fade
     * @param distanceFadeAlpha Distance-based alpha
     * @param scale Scale factor
     * @param circleRadius Circle radius for gadgets (0 for players/NPCs)
     * @param finalAlpha Final alpha after adaptive effects
     * @param finalFontSize Final scaled font size
     * @param finalBoxThickness Final scaled box thickness
     * @param finalDotRadius Final scaled dot radius
     * @param finalHealthBarWidth Final scaled health bar width
     * @param finalHealthBarHeight Final scaled health bar height
     * @param stateManager Reference to the combat state manager
     */
    static void RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                      Camera& camera,
                                      const glm::vec2& screenPos, const ImVec2& boxMin, const ImVec2& boxMax,
                                      const ImVec2& center, unsigned int fadedEntityColor, 
                                      float distanceFadeAlpha, float scale, float circleRadius,
                                      float finalAlpha, float finalFontSize, float finalBoxThickness,
                                      float finalDotRadius, float finalHealthBarWidth, float finalHealthBarHeight,
                                      const CombatStateManager& stateManager);

};

} // namespace kx