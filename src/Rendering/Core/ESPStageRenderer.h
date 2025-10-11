#pragma once

#include <vector>
#include "../Data/RenderableData.h"
#include "../Data/ESPData.h"
#include "../../Game/Camera.h"

// Forward declarations
struct ImDrawList;

namespace kx {

// Forward declaration for context struct
struct VisualProperties;
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
                               CombatStateManager& stateManager);

private:
    // Pooled rendering functions for optimized performance
    static void RenderPooledPlayers(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderablePlayer*>& players, Camera& camera, 
                                   CombatStateManager& stateManager);

    static void RenderPooledNpcs(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                const std::vector<RenderableNpc*>& npcs, Camera& camera, 
                                CombatStateManager& stateManager);

    static void RenderPooledGadgets(ImDrawList* drawList, float screenWidth, float screenHeight, 
                                   const std::vector<RenderableGadget*>& gadgets, Camera& camera, 
                                   CombatStateManager& stateManager);

    /**
     * @brief Universal entity rendering function using context struct
     */
    static void RenderEntity(ImDrawList* drawList, const EntityRenderContext& context, Camera& camera, 
                           CombatStateManager& stateManager);

    /**
     * @brief Render all visual components for an entity
     * @param drawList ImGui draw list
     * @param context Entity rendering context
     * @param camera The game camera
     * @param props The calculated visual properties for the entity
     */
    static void RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                             Camera& camera, const struct VisualProperties& props);


};

} // namespace kx