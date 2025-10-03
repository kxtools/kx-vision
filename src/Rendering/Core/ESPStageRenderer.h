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

    // === Entity Rendering Helpers ===
    
    /**
     * @brief Check if entity is on screen and calculate screen position
     * @param position World position
     * @param camera Camera for projection
     * @param screenWidth Screen width
     * @param screenHeight Screen height
     * @param outScreenPos Output screen position (only valid if returns true)
     * @return True if entity is visible on screen, false otherwise
     */
    static bool IsEntityOnScreen(const glm::vec3& position, Camera& camera, 
                                float screenWidth, float screenHeight, glm::vec2& outScreenPos);

    /**
     * @brief Calculate distance-based scale factor for entity rendering
     * @param visualDistance Visual distance from camera
     * @param entityType Type of entity (affects which scaling curve is used)
     * @return Clamped scale factor (between espMinScale and espMaxScale)
     */
    static float CalculateEntityScale(float visualDistance, ESPEntityType entityType);

    /**
     * @brief Calculate box dimensions for entity based on type and scale
     * @param entityType Type of entity (Player, NPC, Gadget)
     * @param scale Scale factor
     * @param outBoxWidth Output box width
     * @param outBoxHeight Output box height
     */
    static void CalculateEntityBoxDimensions(ESPEntityType entityType, float scale, 
                                            float& outBoxWidth, float& outBoxHeight);

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
     */
    static void RenderEntityComponents(ImDrawList* drawList, const EntityRenderContext& context,
                                      const glm::vec2& screenPos, const ImVec2& boxMin, const ImVec2& boxMax,
                                      const ImVec2& center, unsigned int fadedEntityColor, 
                                      float distanceFadeAlpha, float scale, float circleRadius = 0.0f);

    // Distance fading helper function
    static float CalculateEntityDistanceFadeAlpha(float distance, bool useDistanceLimit, float distanceLimit);

    /**
     * @brief Calculate adaptive alpha based on rendering mode and entity type
     * @param gameplayDistance Distance from player to entity
     * @param distanceFadeAlpha Pre-calculated distance fade alpha (for Limit Mode)
     * @param useDistanceLimit Whether distance limit mode is enabled
     * @param entityType Type of entity (adaptive alpha only applied to gadgets)
     * @param outNormalizedDistance Output normalized distance (0.0-1.0, for future LOD effects)
     * @return Final alpha value with atmospheric fading applied
     */
    static float CalculateAdaptiveAlpha(float gameplayDistance, float distanceFadeAlpha, 
                                       bool useDistanceLimit, ESPEntityType entityType, float& outNormalizedDistance);
};

} // namespace kx